
/*!
 * \file
 * \brief
 * \author
 */

#pragma once

#include "Mat.h"

namespace cvlib
{

	namespace ip
	{

		CVLIB_DECLSPEC void enhanceEdgeX(const Mat& src, Mat& dst, const Veci& smooth_filter, const Veci& diff_filter, bool fnormal = true);
		CVLIB_DECLSPEC void enhanceEdgeY(const Mat& src, Mat& dst, const Veci& diff_filter, const Veci& smooth_filter, bool fnormal = true);
		CVLIB_DECLSPEC void sepFilter2D(const Mat& src, Mat& dstx, Mat& dsty, const Veci& kx, const Veci& ky, bool fnormal = true);
		CVLIB_DECLSPEC void enhanceEdge(const Mat& _src, Mat& dst);

		CVLIB_DECLSPEC void gaussian(const Mat& src, Mat& dst, float sigma);
		CVLIB_DECLSPEC void gaussian(Mat& src, float sigma);
		CVLIB_DECLSPEC void gaussian(const Mat& src, Mat& dst, float sigma, float radius);
		CVLIB_DECLSPEC void gaussian(Mat& src, float sigma, float radius);
		CVLIB_DECLSPEC void dogfilter(const Mat& src, Mat& dst, float sigma_in, float sigma_out, float radius);
		CVLIB_DECLSPEC int blurringCircle(const Mat& src, Mat& dst, int nCircleRadius, int nSigma, int nSmoothRaduis, const Point& pt);
		CVLIB_DECLSPEC void gaussiankernel(float rSigma, float rRadius, Vecd& vKernel);
		CVLIB_DECLSPEC void gaussiankernel2D(float rSigma1, float rSigma2, float rRadius, Matd& mKernel);
		CVLIB_DECLSPEC void gaussianDiffX(const Mat& src, Mat& dst, float sigma, float radius);
		CVLIB_DECLSPEC void gaussianDiffY(const Mat& src, Mat& dst, float sigma, float radius);

	}

}
