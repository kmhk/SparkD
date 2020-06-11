/*! 
 * \file CholeskyDecomposition.cpp
 * \ingroup math
 * \brief 
 * \author 
 */

#include "CholeskyDecomposition.h"

namespace cvlib
{

CholeskyDecomposition::CholeskyDecomposition(Mat* pArg, Matd* /*pmL*//*=NULL*/)
{
	// Initialize.
	double** A = pArg->data.db;
	n = pArg->rows();
	L = (double**)new double*[n];
	for (int i = 0; i < n; i++)
	{
		L[i] = new double[n];
	}
	isspd = (pArg->cols() == n);
	// Main loop.
	for (int j = 0; j < n; j++)
	{
		int k;
		double* Lrowj = L[j];
		double d = 0.0;
		for (k = 0; k < j; k++)
		{
			double* Lrowk = L[k];
			double s = 0.0;
			for (int i = 0; i < k; i++)
			{
				s += Lrowk[i] * Lrowj[i];
			}
			Lrowj[k] = s = (A[j][k] - s) / L[k][k];
			d = d + s * s;
			isspd = isspd & (A[k][j] == A[j][k]);
		}
		d = A[j][j] - d;
		isspd = isspd & (d > 0.0);
		L[j][j] = sqrt(MAX(d, 0.0));
		for (k = j + 1; k < n; k++)
		{
			L[j][k] = 0.0;
		}
	}
}

CholeskyDecomposition::~CholeskyDecomposition()
{
	for (int i = 0; i < n; i++)
	{
		delete []L[i];
	}
	delete []L;
}

bool CholeskyDecomposition::spd()
{
	return isspd;
}

Mat* CholeskyDecomposition::getL()
{
	return new Mat(L, n, n, MAT_Tdouble);
}


Mat* CholeskyDecomposition::solve(Mat* pB)
{
	if (pB->rows() != n)
	{
		return NULL;
		//assert (false) ;
	}
	if (!isspd)
	{
		return NULL;
		//assert (false) ;
	}
	
	int k;
	// copy right hand side.
	Mat* pmRet = new Mat (*pB);
	double** X = pmRet->data.db;
	int nx = pB->cols();
	
	// Solve L * Y = B;
	for (k = 0; k < n; k++)
	{
		for (int i = k + 1; i < n; i++)
		{
			for (int j = 0; j < nx; j++)
			{
				X[i][j] -= X[k][j] * L[i][k];
			}
		}
		for (int j = 0; j < nx; j++)
		{
			X[k][j] /= L[k][k];
		}
	}
	
	// Solve L' * X = Y;
	for (k = n - 1; k >= 0; k--)
	{
		for (int j = 0; j < nx; j++)
		{
			X[k][j] /= L[k][k];
		}
		for (int i = 0; i < k; i++)
		{
			for (int j = 0; j < nx; j++)
			{
				X[i][j] -= X[k][j] * L[k][i];
			}
		}
	}
	return pmRet;
}

}
