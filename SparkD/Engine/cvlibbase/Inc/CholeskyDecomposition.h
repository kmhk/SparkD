/*! 
 * \file    CholeskyDecomposition.h
 * \ingroup math
 * \brief   
 * \author 
 */

#pragma once

#include "Mat.h"

namespace cvlib
{
	
    /// <summary>
    ///		Cholesky Decomposition of a symmetric, positive definite matrix.
    ///	</summary>
    /// <remarks>
    ///		For a symmetric, positive definite matrix <c>A</c>, the Cholesky decomposition is a
    ///		lower triangular matrix <c>L</c> so that <c>A = L * L'</c>.
    ///		If the matrix is not symmetric or positive definite, the constructor returns a partial 
    ///		decomposition and sets two internal variables that can be queried using the
    ///		<see cref="Symmetric"/> and <see cref="PositiveDefinite"/> properties.
    /// 
    ///     Any square matrix A with non-zero pivots can be written as the product of a
    ///     lower triangular matrix L and an upper triangular matrix U; this is called
    ///     the LU decomposition. However, if A is symmetric and positive definite, we
    ///     can choose the factors such that U is the transpose of L, and this is called
    ///     the Cholesky decomposition. Both the LU and the Cholesky decomposition are
    ///     used to solve systems of linear equations.
    /// 
    ///     When it is applicable, the Cholesky decomposition is twice as efficient
    ///     as the LU decomposition.
    ///	</remarks>
class CVLIB_DECLSPEC CholeskyDecomposition
{
public:
	CholeskyDecomposition(Mat* pArg, Matd* pmL=NULL);
	virtual ~CholeskyDecomposition();

	/** 
	 * @brief
	 * @return
	 * @see  Solve()
	 */
	virtual bool spd();

	/** 
	 * @brief
	 * @return
	 * @see  Solve()
	 */
	virtual Mat* getL();

	/** 
	 * @brief	A * X = B
	 *
	 * @param	pB [in] :
	 * @return	L * L'* X = B 
	   <exception cref="System.ArgumentException"> </exception>
	 *
	 * @see  LUDecomposition::solve()
	 *
	 * @par Example:
	 * @code
	 *  
	 *  int i, j;
	 *  int nDim = 3;
	 *  
	 *  Mat mTest(nDim, nDim, MAT_Tdouble);
	 *  mTest.zero();
	 *  double** ppD = mTest.data.db;
	 *  ppD[0][0] = 5;	ppD[0][1] = 3;	ppD[0][2] = 0;
	 *  ppD[1][0] = 3;	ppD[1][1] = 5;	ppD[1][2] = 1;
	 *  ppD[2][0] = 0;	ppD[2][1] = 1;	ppD[2][2] = 4;
	 *  
	 *  CholeskyDecomposition ChD(&mTest);
	 *  bool flag = ChD.SPD();
	 *  
	 *  if( flag )
	 *  {	
	 *  	Mat* pmL = ChD.GetL();
	 *    
	 *      Mat mB(nDim, nDim, MAT_Tdouble); 
	 *      mB.zero();
	 *      for (i = 0; i < nDim; i ++)
	 *  	    mB.data.db[i][i] = 1;
	 *      Mat* pmI = ChD.Solve(&mB);
	 *      ... ...
	 *      mTest.release();
	 *      mB.release();
	 *      delete pmI);
	 *  }
	 *  ... 
	 * @endcode
	 * 
	 */
	virtual Mat* solve(Mat* pB);

private:
	double** L;
	int n;
	bool isspd;
};

}