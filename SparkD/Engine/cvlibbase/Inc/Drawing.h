#pragma once

#include "Mat.h"

namespace cvlib 
{

	//! returns bounding box of the text string
	CVLIB_DECLSPEC Size getTextSize(const String& text, int fontFace, double fontScale, int thickness, int* baseLine);
	CVLIB_DECLSPEC bool selectionPolygonFill(const Mat& image, Mat& selection, Rect& boundingRegion, const vec2i* points, int npoints, int level);
	CVLIB_DECLSPEC bool selectionPolygonLine(const Mat& image, Mat& selection, Rect& boundingRegion, const vec2i* points, int npoints, int level, int thickness);
	CVLIB_DECLSPEC void	linePoints(int nX0, int nY0, int nX1, int nY1, int* &pnX, int* &pnY, int &nNum);
	CVLIB_DECLSPEC void	linePoints(const vec2i& pt1, const vec2i& pt2, Vector<vec2i>& points);
	CVLIB_DECLSPEC void	circlePoints(const Rect& rect, Vector<vec2i>& points);
	CVLIB_DECLSPEC void ellipse2Poly(const vec2i& center, Size axes, int angle, int arc_start, int arc_end, int delta, Vector<vec2i>& pts);
	CVLIB_DECLSPEC bool clipLine(const Size& img_size, vec2i& pt1, vec2i& pt2);
	CVLIB_DECLSPEC bool clipLine(const Rect& img_rect, vec2i& pt1, vec2i& pt2);

	void ifillConvexPoly(Mat& image, const Point* points, int npoints, Scalar color, float rOpacity = 1.0f);

}