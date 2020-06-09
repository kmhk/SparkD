/*!
 * \file	QRDecomposition.h
 * \ingroup math
 * \brief	QR.
 * \author	
 */

#pragma once

#include "Mat.h"

namespace cvlib
{

	/// <summary>
	///	  QR decomposition for a rectangular matrix.
	/// </summary>
	/// <remarks>
	///   For an m-by-n matrix <c>A</c> with <c>m &gt;= n</c>, the QR decomposition is an m-by-n
	///   orthogonal matrix <c>Q</c> and an n-by-n upper triangular 
	///   matrix <c>R</c> so that <c>A = Q * R</c>.
	///   The QR decompostion always exists, even if the matrix does not have
	///   full rank, so the constructor will never fail.  The primary use of the
	///   QR decomposition is in the least squares solution of nonsquare systems
	///   of simultaneous linear equations.
	///   This will fail if <see cref="FullRank"/> returns <see langword="false"/>.
	/// </remarks>
class CVLIB_DECLSPEC QRDecomposition
{
public:
	QRDecomposition(Mat* pA, Matd* pmQR=NULL);
	virtual ~QRDecomposition();

	virtual bool FullRank();
	virtual Mat* H();
	virtual Mat* R();

	/** 
	 * @brief	
	 *
	 * @return	Q
	 *
	 * @par Example:
	 * @code
	 *  
	 *  int i, j;
	 *  int nDim = 3;
	 *  
	 *  Mat mTest(nDim, nDim, MAT_Tdouble);
	 *  mTest.zero();
     * 	for(i = 0; i < nDim; i ++)
	 *  	for(j = 0; j <= i; j ++)
	 *  	{
	 *  		if( i == j)  
	 *  			mTest.data.db[i][j] = Random::Normal();
	 *  		else
     * 			    mTest.data.db[i][j] = mTest.data.db[j][i] = Random::Normal();
	 *  	}
	 *	
	 *  QRDecomposition QRD(&mTest);
	 *  bool flag = QRD.FullRank();
	 *  Mat* pmQ = QRD.Q();
	 *  Mat* pmR = QRD.R();
	 *  Mat* pmH = QRD.H();
	 *  
     * 	printf( "QR¹¤ÃÍÌ© »¨¶Ê: A = Q * R \n" );
	 *  
	 *  printf( "A = \n" );
	 *  for(i = 0; i < nDim; i ++)
	 *  {
     * 		for(j = 0; j < nDim; j ++)
	 *  	{
	 *  		printf( "\t%.2f", mTest.data.db[i][j] );
	 *  	}
     * 		printf( "\n" );
	 *  }
	 *  
	 *  printf( "Q = \n" );
	 *  for(i = 0; i < nDim; i ++)
	 *  {
	 *  	for(j = 0; j < nDim; j ++)
	 *  	{
	 *  		printf( "\t%.2f", pmQ->data.db[i][j] );
	 *  		
	 *  	}
	 *  	printf( "\n" );
	 *  }
	 *  
	 *  printf( "R = \n" );
	 *  for(i = 0; i < nDim; i ++)
	 *  {
	 *  	for(j = 0; j < nDim; j ++)
	 *  	{
	 *  		printf( "\t%.2f", pmR->data.db[i][j] );
	 *  		
	 *  	}
	 *  	printf( "\n" );
	 *  }
	 *  
	 *  printf( "H = \n" );
	 *  for(i = 0; i < nDim; i ++)
	 *  {
	 *  	for(j = 0; j < nDim; j ++)
	 *  	{
	 *  		printf( "\t%.2f", pmH->data.db[i][j] );
	 *  		
	 *  	}
	 *  	printf( "\n" );
	 *  }
	 * 
	 *  Mat mB(nDim, nDim, MAT_Tdouble);
	 *  mB.zero();
	 *  for (i = 0; i < nDim; i ++)
	 *  	mB.data.db[i][i] = 1;
	 *	Mat* pmI = QRD.Solve(&mB);
	 *  ... ... 
	 *  mTest.release();
	 *  ReleaseMat(pmQ);
	 *  ReleaseMat(pmR);
	 *  ReleaseMat(pmH);
	 *  
	 *  mB.release();
	 *  ReleaseMat(pmI);
	 * 
	 *
	 *  Output
	 *
	 *   A = Q * R 
	 * 	A = 
	 * 		-0.01	-0.58	0.53
	 * 		-0.58	0.25	0.13
	 * 		0.53	0.13	-0.89
	 * 	Q = 
	 * 		-0.02	0.91	-0.41
	 * 		-0.74	-0.29	-0.60
	 * 		0.67	-0.30	-0.68
	 *  R = 
	 * 		0.79	-0.09	-0.70
	 * 		0.00	-0.64	0.70
	 * 		0.00	0.00	0.31
	 * 	H = 
	 * 		1.02	0.00	0.00
	 *  	0.74	1.95	0.00
	 * 		-0.67	-0.30	2.00
	 *
	 * @endcode
	 * 
	 */
	virtual Mat* Q();

	/** 
	 * @brief	A * X = B. 
	 *
	 * @param	pB [in] : 
	  <exception cref="System.ArgumentException"> </exception>
	 *
	 * @see  Q(), LUDecomposition::solve()
     *	
	 */
	virtual Mat* solve(Mat* pB);

private:
	Mat* m_pmQR;
	double** QR;
	int m;
	int n;
	double* Rdiag;
};

#define QRD QRDecomposition
}