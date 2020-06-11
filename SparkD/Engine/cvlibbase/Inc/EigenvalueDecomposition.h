/*! 
 * \file    EigenvalueDecomposition.h
 * \ingroup math
 * \brief   
 * \author  
 */

#pragma once

#include "Mat.h"

namespace cvlib
{
	
    /// <summary>
    ///     Determines the eigenvalues and eigenvectors of a real square matrix.
    /// </summary>
    /// <remarks>
    ///     In the mathematical discipline of linear algebra, eigendecomposition
    ///     or sometimes spectral decomposition is the factorization of a matrix
    ///     into a canonical form, whereby the matrix is represented in terms of
    ///     its eigenvalues and eigenvectors.
    /// 
    ///     If <c>A</c> is symmetric, then <c>A = V * D * V'</c> and <c>A = V * V'</c>
    ///     where the eigenvalue matrix <c>D</c> is diagonal and the eigenvector matrix <c>V</c> is orthogonal.
    ///     If <c>A</c> is not symmetric, the eigenvalue matrix <c>D</c> is block diagonal
    ///     with the real eigenvalues in 1-by-1 blocks and any complex eigenvalues,
    ///     <c>lambda+i*mu</c>, in 2-by-2 blocks, <c>[lambda, mu; -mu, lambda]</c>.
    ///     The columns of <c>V</c> represent the eigenvectors in the sense that <c>A * V = V * D</c>.
    ///     The matrix V may be badly conditioned, or even singular, so the validity of the equation
    ///     <c>A=V*D*inverse(V)</c> depends upon the condition of <c>V</c>.
    /// </remarks>
class CVLIB_DECLSPEC EigenvalueDecomposition
{
public:
	/** 
	 * @brief
	  <pre>  
	 *
	 * @param	pArg [in] : 
	 *
	 * @return	D, V
	 *
	 */
	EigenvalueDecomposition(Mat* pArg, Matd* pmD=NULL, Matd* pmV=NULL);

	virtual ~EigenvalueDecomposition();

	virtual const double* realEigenvalues() const;

	virtual const double* imagEigenvalues() const;

	virtual Mat* D() const;

	/** 
	 * @brief	
	 *
	 * @return	V
	 *
	 * @par Example:
	 * @code
	 *  
	 *  int i, j;
	 *  int nDim = 3;
	 *  char szFileName[] = "c:/testEVD.bin";
	 *  
	 *  Mat mTest(nDim, nDim, MAT_Tdouble);
	 *  mTest.zero();
     * 	for(i = 0; i < nDim; i ++)
	 *  	for(j = 0; j <= i; j ++)
	 *  	{
	 *  		if( i == j)  
	 *  			mTest.data.db[i][j] = Random::Normal();
	 *  		else
	 *  			mTest.data.db[i][j] = mTest.data.db[j][i] = Random::Normal();
	 *  	}
	 *  
	 *  EigenvalueDecomposition EVD(&mTest);
	 *  
	 *  Mat* pmD = EVD.D();
	 *  Mat* pmV = EVD.GetV();
	 *  double* rValue = EVD.realEigenvalues();
	 *  double* iValue = EVD.ImagEigenvalues();
	 *  
	 *  FILE* pF = fopen(szFileName, "w");
	 *  fprintf(pF, "A = V * D * V' \n" );
	 *  
	 *  fprintf(pF, "A = \n" );
	 *  for(i = 0; i < nDim; i ++)
	 *  {
	 *  	for(j = 0; j < nDim; j ++)
	 *  	{
	 *  		fprintf(pF, "\t%.2f", mTest.data.db[i][j] );
	 *  	}
	 *  	fprintf(pF, "\n" );
	 *  }
	 *  
	 *  // D
	 *  fprintf(pF, "D = \n" );
	 *  for(i = 0; i < nDim; i ++)
	 *  {
	 *  	for(j = 0; j < nDim; j ++)
	 *  	{
	 *  		fprintf(pF, "\t%.2f", pmD->data.db[i][j] );
	 *  		
	 *  	}
	 *  	fprintf(pF, "\n" );
	 *  }
	 *  
	 *  // V
	 *  fprintf(pF, "V = \n" );
	 *  for(i = 0; i < nDim; i ++)
	 *  {
	 *  	for(j = 0; j < nDim; j ++)
	 *  	{
	 *  		fprintf(pF, "\t%.2f", pmV->data.db[i][j] );
	 *  		
	 *  	}
	 *		fprintf(pF, "\n" );
	 *  }
	 *  
	 *  fprintf(pF, "d = (" );
	 *  for(i = 0; i < nDim; i ++)
	 *  	fprintf(pF, "\t%.2f", rValue[i] );
	 *  fprintf(pF, "\t)\n" );
	 *  
	 *  fprintf(pF, "e = (" );
	 *  for(i = 0; i < nDim; i ++)
	 *  	fprintf(pF, "\t%.2f", iValue[i] );
	 *  fprintf(pF, "\t)\n" );
	 *  
	 *  fclose(pF);
	 *  mTest.release();
	 *  delete pmD);
	 *  delete pmV);
	 *  
	 *
	 *  Output
	 *  
	 *  A = V * D * V'
	 *  A = 
	 *  	1.95	0.15	-0.92
	 *  	0.15	-0.45	-0.40
	 *  	-0.92	-0.40	-0.98
	 *  D = 
	 *  	2.24	0.00	0.00
	 * 		0.00	-0.35	0.00
	 *		0.00	0.00	-1.37
	 *  V = 
	 *		-0.95	-0.18	-0.24
	 *		-0.09	0.93	-0.35
	 *		0.28	-0.31	-0.91
	 *  d = (	2.24	-0.35	-1.37	)
	 *  e = (	0.00	0.00	0.00	)
	 *
	 * @endcode
	 * 
	 */
	virtual Mat* GetV() const;
	
private:
	int n;
	bool m_fIsSymmetric;
	double* d;
	double* e;
	double** V;
	double** H;
	double* ort;
	double cdivr;
	double cdivi;

	void  tred2();
	void  tql2();
	void  orthes();
	void  cdiv(double xr, double xi, double yr, double yi);
	void  hqr2();
};

#define EVD EigenvalueDecomposition

}