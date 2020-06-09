
/*!
 * \file	ImgProc.h
 * \ingroup Engine
 * \brief	
 * \author	
 */

#pragma once

#include "cvlibbaseDef.h"
#include "cvlibutil.h"

namespace cvlib {
	namespace ip {

		CVLIB_DECLSPEC void sepConvSmall(float*  prSrc, int nSrc_step, float*  prDst, int nDst_step, Size src_size,
			const float* prKx, const float* prKy, float*  prBuffer);

		CVLIB_DECLSPEC bool getRectSubPixel(const uchar* pbSrc, int nSrc_step, Size src_size, float* prDst,
			int nDst_step, Size win_size, Point2f center);

		CVLIB_DECLSPEC void findCornerSubPix(const Mat& src, Point2f* corners, int count, Size win, Size zeroZone, 
			TermCriteria criteria);

		CVLIB_DECLSPEC Rect getBoundRect(const Mat& src, int value = 0);

		CVLIB_DECLSPEC Rect getBoundRect(const Vector<Point2i>& points);

	}
}
