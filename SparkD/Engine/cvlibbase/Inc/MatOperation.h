/*!
 * \file	MatOperation.h
 * \ingroup base
 * \brief	.
 * \author	
 */
#pragma once

#include "Mat.h"
#include "Template.h"

namespace cvlib
{

#define CVLIB_C            1
#define CVLIB_L1           2
#define CVLIB_L2           4
#define CVLIB_NORM_MASK    7
#define CVLIB_RELATIVE     8
#define CVLIB_DIFF         16
#define CVLIB_MINMAX       32

	class CVLIB_DECLSPEC MatOp
	{
	public:

		static void copyMat(Mat& dst, const Mat& src, const Rect& srcRect, const Point2i& dstpt = Point2i());

		static void normalize(const Mat& src, Mat& dst, double alpha = 1, double beta = 0,
			int norm_type = NORM_L2, TYPE dtype = MAT_Tbyte, const Mat& mask = Mat());

		//  A = B + C 
		static void add(Mat* pA, const Mat* pB, const Mat* pC);

		//	A = B - C  
		static void sub(Mat* pA, const Mat* pB, const Mat* pC);

		// A = B * C  
		static void mul(Vec* pA, const Mat* pB, const Vec* pC);

		// A = B * C 
		static void mul(Vec* pA, const Vec* pB, const Mat* pC);

		// x' * A * x.
		static float trXAX(const Vec* pvX, const Mat* pmA);

		// A = B + C  
		static void add(Vec* pA, const Vec* pB, const Vec* pC);

		// A = B - C  
		static void sub(Vec* pA, const Vec* pB, const Vec* pC);

		// C = A * B'  
		static void mul(Mat* pC, const Vec* pA, const Vec* pB);

		// A' * B  
		static double dotProduct(const Vec* pA, const Vec* pB);

		// C = A * B * A'  
		static void ABTrA(Mat* pmC, const Mat* pmA, const Mat* pmB);

		// C = A' * B * A  
		static void trABA(Mat* pmC, const Mat* pmA, const Mat* pmB);

		// B = r * A  
		static void mul(Mat* pB, double rReal, const Mat* pA);

		// .
		static void mul(Vec* pB, double rReal, const Vec* pA);

		// C = A * B  
		static void mul(Mat* pC, const Mat* pA, const Mat* pB);

		// B = A' * A. 
		static void trAA(Mat* pmB, const Mat* pmA);

		// B = A * A'. 
		static void ATrA(Mat* pmB, const Mat* pmA);

		static void inRange(const Mat& src, const Mat& lb, const Mat& rb, Mat& dst);
		static void inRange(const Mat& src, const double lb, const double rb, Mat& dst);
		static void inRange(const Mat& src, const double lb[3], const double rb[3], Mat& dst, int dims);

		static void addWeighted(const Mat& src1, double alpha, const Mat& src2, double beta, double gamma, Mat& dst);
		static void gemm(const Mat* matA, const Mat* matB, double alpha,
			const Mat* matC, double beta, Mat* matD, int flags = 0);
		/* Matrix transform: dst = A*B + C, C is optional */
		static void matMulAdd(const Mat* src1, const Mat* src2, const Mat* src3, Mat* dst);
		static double scalarProduct(Mat* pmA, Mat *pmB);

		static void split(const Mat& src, Mat* mv);
		static void split(const Mat& src, Vector<Mat>& dsts);
		static void merge(const Mat* mv, size_t n, Mat& dst);
	};

	void CVLIB_DECLSPEC cartToPolar(const Mat& X, const Mat& Y, Mat& Mag, Mat& Angle, bool angleInDegrees);

}