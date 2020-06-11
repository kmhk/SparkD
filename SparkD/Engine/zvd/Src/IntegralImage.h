/*!
 * \file
 * \brief 
 */
#pragma once

#include "cvlibbase/Inc/Mat.h"

namespace cvlib
{

	namespace ip
	{

		CVLIB_DECLSPEC	void integralImage(const Mat& src, Mat& sum);
		CVLIB_DECLSPEC	void integralImage(const Mat& src, Mat& sum, Mat& square);
		CVLIB_DECLSPEC	void integralImage(const Mat& src, Mat& sum, Mat& tilted, Mat& square);
		CVLIB_DECLSPEC	void integralTiltedImage(const Mat& src, Mat& sum, Mat& tilted);
		CVLIB_DECLSPEC	void integralSquareImage(const Mat& src, Mat& square);

		CVLIB_DECLSPEC	double sumIntegralImage(const Mat& sum, const Rect& region);

	}

}