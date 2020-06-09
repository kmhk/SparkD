#pragma once

#include "Mat.h"

namespace cvlib {
	namespace ip {

		// mode : 0 // dx, dy uses
		// mode : 1 // dx
		// mode : 2 // dy
		CVLIB_DECLSPEC	void canny(const Mat& src, Mat& dst, float low_thresh, float high_thresh, int mode = 0);
		CVLIB_DECLSPEC	void canny(Mat& src, float low_thresh, float high_thresh, int mode = 0);

	}
}
