
/*!
 * \file	ipFilter.h
 * \ingroup ipCore
 * \brief   
 * \author  
 */
#pragma once

#include "_cvlibbase.h"

namespace cvlib
{

namespace ip
{
	enum filterintype{MAT, VEC, VECRow, VECCol};

	CVLIB_DECLSPEC int		kernelFactor(const int* pnKernel, int nLen);
	CVLIB_DECLSPEC float	kernelFactor(const float* prKernel, int nLen);
	CVLIB_DECLSPEC double	kernelFactor(const double* prKernel, int nLen);

	CVLIB_DECLSPEC void convolve(Vec& src, const int* pnKernel, int nKSize, int nKFactor, int nKOffset);
	CVLIB_DECLSPEC void convolve(const Vec& src, Vec& dst, const int* pnKernel, int nKSize, int nKFactor, int nKOffset);

	CVLIB_DECLSPEC void convolve2(Mat& src, const int* pnKernel, int nKSize, int nKFactor, int nKOffset, filterintype type=MAT);
	CVLIB_DECLSPEC void convolve2(Mat& src, const float* pnKernel, int nKSize, float nKFactor, int nKOffset, filterintype type=MAT);
	CVLIB_DECLSPEC void convolve2(Mat& src, const double* pnKernel, int nKSize, double nKFactor, int nKOffset, filterintype type=MAT);
	CVLIB_DECLSPEC void convolve2(const Mat& src, Mat& dst, const int* pnKernel, int nKSize, int nKFactor, int nKOffset, filterintype type=MAT);
	CVLIB_DECLSPEC void convolve2(const Mat& src, Mat& dst, const float* pnKernel, int nKSize, float nKFactor, int nKOffset, filterintype type=MAT);
	CVLIB_DECLSPEC void convolve2(const Mat& src, Mat& dst, const double* pnKernel, int nKSize, double nKFactor, int nKOffset, filterintype type=MAT);

	CVLIB_DECLSPEC void convolve2(Mat& src, const int* pnKernel, int nKSize, int nKFactor, int nKOffset, const Rect& r);
	CVLIB_DECLSPEC void convolve2(const Mat& src, Mat& dst, const int* pnKernel, int nKSize, int nKFactor, int nKOffset, const Rect& r);

	CVLIB_DECLSPEC void convolve2sep(Mat& src, const int* pnKernel, int nKSize, int nKFactor);
	CVLIB_DECLSPEC void convolve2sep(Mat& src, const float* prKernel, int nKSize, float rKFactor);
	CVLIB_DECLSPEC void convolve2sep(const Mat& src, Mat& dst, const int* pnKernel, int nKSize, int nKFactor);
	CVLIB_DECLSPEC void convolve2sep(const Mat& src, Mat& dst, const float* prKernel, int nKSize, float rKFactor);
	// CVLIB_DECLSPEC void sepFilter2D(const Mat& src, Mat& dst, int ddepth, Mat& kernelX, Mat& _kernelY, Point2i anchor,
	//                     double delta, int borderType );
	// CVLIB_DECLSPEC void filter2D(const Mat& src, Mat& dst, int ddepth, Mat& kernel, Point2i anchor, 
	// 					double delta, int borderType );

	CVLIB_DECLSPEC void median(const Mat& src, Mat& dst, int nKsize = 3, filterintype scantype = ip::MAT);
	CVLIB_DECLSPEC void median(Mat& src, int nKsize = 3, filterintype scantype = ip::MAT);

}

}
