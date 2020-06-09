
/*!
 * \file	ipNoise.h
 * \ingroup ipCore
 * \brief   
 * \author  
 */
#pragma once

#include "_cvlibbase.h"
#include "ImgFilter.h"

namespace cvlib
{

	namespace ip
	{

		CVLIB_DECLSPEC void erode(const Mat& src, Mat& dst, int nkx, int nky);
		CVLIB_DECLSPEC void erode(Mat& src, int nkx, int nky);
		CVLIB_DECLSPEC void dilate(const Mat& src, Mat& dst, int nkx, int nky);
		CVLIB_DECLSPEC void dilate(Mat& src, int nkx, int nky);
		CVLIB_DECLSPEC void opening(const Mat& src, Mat& dst, int nkx, int nky);
		CVLIB_DECLSPEC void opening(Mat& src, int nkx, int nky);
		CVLIB_DECLSPEC void closing(const Mat& src, Mat& dst, int nkx, int nky);
		CVLIB_DECLSPEC void closing(Mat& src, int nkx, int nky);

	}

}
