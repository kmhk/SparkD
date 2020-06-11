
/*! 
 * \file    LUDecomposition.h
 * \ingroup math
 * \brief   LU.
 * \author  
 */

#pragma once

#include "Mat.h"

namespace cvlib
{
	
    /// <summary>
    ///   LU decomposition of a rectangular matrix.
    /// </summary>
    /// <remarks>
    ///   For an m-by-n matrix <c>A</c> with m >= n, the LU decomposition is an m-by-n
    ///   unit lower triangular matrix <c>L</c>, an n-by-n upper triangular matrix <c>U</c>,
    ///   and a permutation vector <c>piv</c> of length m so that <c>A(piv)=L*U</c>.
    ///   If m &lt; n, then <c>L</c> is m-by-m and <c>U</c> is m-by-n.
    ///   The LU decompostion with pivoting always exists, even if the matrix is
    ///   singular, so the constructor will never fail.  The primary use of the
    ///   LU decomposition is in the solution of square systems of simultaneous
    ///   linear equations. This will fail if <see cref="NonSingular"/> returns <see langword="false"/>.
    /// </remarks>
class CVLIB_DECLSPEC LUDecomposition
{
public:		
	LUDecomposition(const Mat* pA, Mat* pmLU = NULL);
	virtual ~LUDecomposition();
	virtual bool isNonSingular() const;

	/** 
	 * @brief  	
	 *
	 * @return 	
	 *
	 * @par Example:
	 * @code
	 *  
	 *  int i, j;
	 *  int nRows = 6;
	 *  int nCols = 4;
	 *  float rSigma = 0.5f; 
	 *  
	 *  Mat mTest(nRows, nCols, MAT_Tdouble);
     * 	mTest.zero();
	 *  for(i = 0; i < nRows; i ++)
	 *  	for(j = 0; j < nCols; j ++)
	 *  		mTest.data.db[i][j] = rSigma * Random::Normal() + 0.3f;
	 *  
	 *  LUDecomposition LUD(&mTest);
	 *  Mat* pmL = LUD.L();
	 *  Mat* pmU = LUD.U();
	 *  int* pPiv = LUD.Pivot();
	 *  double* prPiv = LUD.doublePivot();
	 *  
	 *  printf( "A = L * U \n" );
	 *  
	 *  // A
	 *  printf( "A = \n" );
	 *  for(i = 0; i < nRows; i ++)
	 *  {
	 *  	for(j = 0; j < nCols; j ++)
	 *  	{
	 *  		printf( "\t%.2f", mTest.data.db[i][j] );
	 *  	}
	 *  	printf( "\n" );
	 *  }
	 *  
	 *  // L
	 *  printf( "L = \n" );
	 *  for(i = 0; i < nRows; i ++)
	 *  {
	 *  	for(j = 0; j < nCols; j ++) 
	 *  	{
	 *  		printf( "\t%.2f", pmL->data.db[i][j] );
	 *  		
	 *  	}
	 *  	printf( "\n" );
	 *  }
	 *  
	 *  // U
	 *  printf( "U = \n" );
	 *  for(i = 0; i < nCols; i ++)
	 *  {
	 *  	for(j = 0; j < nCols; j ++)
	 *  	{
	 *  		printf( "\t%.2f", pmU->data.db[i][j] ); 
	 *  		
	 *  	}
	 *  	printf( "\n" );
	 *  }
	 *  
	 *  // piv
	 *  printf( "piv = (" );
	 *  for(i = 0; i < nCols; i ++)
	 *	printf( "\t%d", *pPiv++ );
	 *  printf( "\t)\n" );
	 *  
	 *  mTest.release();
	 *  ReleaseMat(pmL);
	 *  ReleaseMat(pmU); 
	 * 
	 *
	 *  Output
	 *  
	 *  A = L * U 
	 *  A = 
	 *  	0.26	0.51	0.66	0.46
	 *  	-0.43	-0.41	-0.13	-0.14
	 *  	0.92	0.30	0.39	0.03
	 *  	0.10	0.27	0.24	-0.09
	 *  	1.97	-0.33	0.41	0.59
	 *  	0.44	-0.47	-0.07	0.56
	 *  L = 
	 *  	1.00	0.00	0.00	0.00
	 *  	0.13	1.00	0.00	0.00
	 *  	-0.22	-0.88	1.00	0.00
	 *  	0.22	-0.71	0.55	1.00
	 *  	0.47	0.83	-0.61	-0.70
	 *  	0.05	0.53	-0.20	-0.49
	 *  U = 
	 *  	1.97	-0.33	0.41	0.59
	 *  	0.00	0.55	0.60	0.38
	 *  	0.00	0.00	0.49	0.32
	 *  	0.00	0.00	0.00	0.53
	 *  piv = (	4	0	1	5	)
	 *  
	 * @endcode
	 * 
	 */
	virtual Mat* L();
	virtual Mat* U();
	virtual int* Pivot();
	virtual double* doublePivot();
	virtual double determinant() const;
	
	/** 
	 * @brief	A * X = B
	 *
	 * @param	pB [in] : 
	 * @return	L * U * X = B(piv,:)
	 <exception cref="System.ArgumentException"> </exception>
	 *
	 * @par Example:
	 * @code
	 *  
	 *  int i, j;
	 *  int nDim = 3;
	 *  
	 *  Mat mTest(nDim, nDim, MAT_Tdouble);
	 *  mTest.zero();
	 *  for(i = 0; i < nDim; i ++)
	 *  	for(j = 0; j < nDim; j ++)
	 *  		mTest.data.db[i][j] = Random::Normal();
	 *  	
	 *  LUDecomposition LUD(&mTest);
	 *  double rDet = LUD.determinant();
	 *  if( LUD.isNonSingular() )
	 *  {
	 *  	Mat mB(nDim, nDim, MAT_Tdouble);
	 *  	
	 *  	mB.zero();
	 *  	for (i = 0; i < nDim; i ++)
	 *  		mB.data.db[i][i] = 1;
	 *      Mat* pmI = LUD.Solve(&mB);
	 *  
	 *      printf( "LU:\n inv(M) : \n" );
	 *      for(i = 0; i < nDim; i ++)
	 *      {
	 *  	   for(j = 0; j < nDim; j ++)
	 *  	   {
	 *  		   printf( "\t%.2f", pmI->data.db[i][j] );
	 * 	       }
	 *	       printf( "\n" );
	 *      }
	 *
	 *      printf( "M * inv(M): \n" );
	 *      Mat mC(nDim, nDim, MAT_Tdouble);
	 *      MatMulMat(&mTest, pmI, &mC);
	 *
	 *      for(i = 0; i < nDim; i ++)
	 *      {
	 *          for(j = 0; j < nDim; j ++)
	 *          {
	 * 		        printf( "\t%.2f", mC.data.db[i][j] );
	 *	        }
	 *	        printf( "\n" );
	 *      }
	 *
	 *      ReleaseMat(pmI);
	 *      mB.release();
	 *  }
	 *  
	 *  output
	 *  
	 *  LU: 
	 *   inv(M) : 
	 *  	1.14	-0.21	-2.01
	 *  	0.91	-0.41	0.02
	 *  	2.22	1.18	0.02
	 *   M * inv(M): 
	 *  	1.00	-0.00	0.00
	 *  	-0.00	1.00	0.00
	 *  	0.00	0.00	1.00
	 *  
	 * @endcode
	 * 
	 */	
	virtual Mat* solve(Mat* pB);

private:
	Mat* m_pmLU;
	double** LU;

	int m;
	int n;
	int pivsign;
	int* piv;
};

#define LUD LUDecomposition

}