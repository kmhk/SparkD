#pragma once

#include "cvlibbase/Inc/_cvlibbase.h"

namespace cvlib
{

	namespace ip
	{

		CVLIB_DECLSPEC void houghCircle(const Mat&  image, Point2i& center, int& radius);
		CVLIB_DECLSPEC void houghLines(const Mat& image, Vector<Point2i>& lines, float rho, float theta, int threshold,
			int lineLength, int lineGap);
		CVLIB_DECLSPEC void houghLinePoints(const Vector<Point2i>& points, int nNeighbors, Veci& vOIdx);
		CVLIB_DECLSPEC void houghLinesStd(const Mat& image, Vector<Point2i>& lines, float rho, float theta, int threshold, int linesMax);

	}

}