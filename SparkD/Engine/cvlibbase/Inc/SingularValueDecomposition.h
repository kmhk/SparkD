
/*!
 * \file	SingularValueDecomposition.h
 * \ingroup math
 * \brief	
 * \author	
 */

#pragma once

#include "Mat.h"

namespace cvlib
{

/// <summary>
///   Singular value Decomposition for a rectangular matrix.
/// </summary>
/// <remarks>
///	  For an m-by-n matrix <c>A</c> with <c>m >= n</c>, the singular value decomposition
///   is an m-by-n orthogonal matrix <c>U</c>, an n-by-n diagonal matrix <c>S</c>, and
///   an n-by-n orthogonal matrix <c>V</c> so that <c>A = U * S * V'</c>.
///   The singular values, <c>sigma[k] = S[k,k]</c>, are ordered so that
///   <c>sigma[0] >= sigma[1] >= ... >= sigma[n-1]</c>.
/// 
///   The singular value decompostion always exists, so the constructor will
///   never fail. The matrix condition number and the effective numerical
///   rank can be computed from this decomposition.
/// </remarks>
class CVLIB_DECLSPEC SingularValueDecomposition
{
public:

	SingularValueDecomposition(const Mat* pmArg, Matd* pmS=NULL, Matd* pmU=NULL, Matd* pmV=NULL);
	virtual ~SingularValueDecomposition();

	virtual const double* singularValues();
	
	/**
	 *
	 * @return  S
	 *
	 * @par Example:
	 * @code
	 *  
	 *  int i, j;
	 *  int nRows = 6;
	 *  int nCols = 4;
	 *  char szFileName[] = "c:/testSVD.bin";
	 *  
	 *  Mat mTest(nRows, nCols, MAT_Tdouble);
     * 	mTest.zero();
	 *  for(i = 0; i < nRows; i ++)
	 *  	for(j = 0; j < nCols; j ++)
	 *  		mTest.data.db[i][j] = Random::Normal();
	 *  
	 *  SingularValueDecomposition SVD(&mTest);
	 *  
	 *  Mat* pmU = SVD.GetU();
	 *  Mat* pmS = SVD.GetS();
	 *  Mat* pmV = SVD.GetV();
	 *  double* rSingular = SVD.singularValues();
	 *  int nRank = SVD.Rank();
	 *  double rCond = SVD.Condition();
	 *  ... ... 
	 *  mTest.release();
	 *  ReleaseMat(pmU);
	 *  ReleaseMat(pmS);
	 *  ReleaseMat(pmV);
	 *  
	 * @endcode
	 * 
	 */
	virtual Mat* GetS();
	virtual Mat* GetU();
	virtual Mat* GetV();


	virtual double norm2();

	/**
	 * @brief   
	 *
	 * @return  max(S) / min(S).
	 *
	 * @see  GetS()
     *
	 */
	virtual double condition();

	virtual int rank();

private:

	Mat* m_pmA;
	double** U;
	double** V;
	double* s;
	int m;
	int n;
	bool m_bTrans;
};

#define SVD SingularValueDecomposition

CVLIB_DECLSPEC void svd(const Mat& m, Mat& u, Mat& d, Mat& v);

}