
/*!
 * \file
 * \brief .
 * \author
 */


#pragma once

#include "Mat.h"
#include "AutoBuffer.h"

namespace cvlib {

	/* contour retrieval mode */
#define CVLIB_RETR_EXTERNAL 0
#define CVLIB_RETR_LIST     1
#define CVLIB_RETR_CCOMP    2
#define CVLIB_RETR_TREE     3

	/* contour approximation method */
#define CVLIB_CHAIN_CODE               0
#define CVLIB_CHAIN_APPROX_NONE        1
#define CVLIB_CHAIN_APPROX_SIMPLE      2
#define CVLIB_CHAIN_APPROX_TC89_L1     3
#define CVLIB_CHAIN_APPROX_TC89_KCOS   4
#define CVLIB_LINK_RUNS                5

	namespace ip {
		CVLIB_DECLSPEC int findContours(Mat* img, Matrix<Point2i>& contours, int mode = CVLIB_RETR_LIST,
			int method = CVLIB_CHAIN_APPROX_SIMPLE, Point2i offset = Point2i());

		CVLIB_DECLSPEC int approxPolyDP(const Point2i* src_contour, int count0, Point2i* dst_contour,
			bool is_closed0, double eps, AutoBuffer<Range>* _stack);

	}
}
