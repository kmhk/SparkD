/*!
 * \file
 * \brief
 * \author
 */
#pragma once

#include "cvlibbase/Inc/_cvlibbase.h"
#include "ImgFilter.h"

namespace cvlib {
	namespace ip {

		void resize(const Mat& src, Mat& dst, int newx, int newy, int interpolation = INTER_NEAREST);
		void resize(const Mat& src, Mat& dst, float rate);
		void resize(Mat& src, int newx, int newy);
		void resize(Mat& src, float rate);

	}
}