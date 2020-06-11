#pragma once

#include "cvlibbase/Inc/Mat.h"
#include "cvlibbase/Inc/Template.h"
#include "TriangleModel.h"
#include "ImgFilter.h"

namespace cvlib {
	namespace ip {

		enum RETMode { RET_RGB, RET_BGRA, RET_RGBA };

		CVLIB_DECLSPEC void warping(const Mat& src, Mat& dst, Rect* pWarpingRect,
			Point2f* pSrcPoints, Point2f* pDstPoints, TriangleModel* pTriangleModel);
		CVLIB_DECLSPEC void computeCoefficients(const Point2f* pPoints, const Triangle* pTriangle, float rX, float rY, float* prA, float* prB, float* prC);
		CVLIB_DECLSPEC float triangleArea(float rX1, float rY1, float rX2, float rY2, float rX3, float rY3);

		CVLIB_DECLSPEC void warpMesh(const Mat& src, Mat& dst, Rect& warpingRect, const Point2f* pSrcPoints, const Point2f* pDstPoints, const TriangleModel* pTriangleModel);
		CVLIB_DECLSPEC void warpTriangle(const Mat& src, Mat& dst, const Vector<vec2f>& srcTriangle, const Vector<vec2f>& dstTriangle);
		/*
		*        | 1+a  b   tx |
		* W(p) = | c   1+d   ty |
		*        | 0     0     1 |
		*/
		CVLIB_DECLSPEC void initWarp(Mat& W, float a, float b, float c, float d, float tx, float ty);
		CVLIB_DECLSPEC void warpAffine(const Mat& src, Mat& dst, const Mat& W, bool finvert = true);
		CVLIB_DECLSPEC void warpPoint(const Vector<Point2i>& src, Vector<Point2i>& dst, const Mat& W);
		CVLIB_DECLSPEC void warpPoint(const Vector<Point2f>& src, Vector<Point2f>& dst, const Mat& W);
		CVLIB_DECLSPEC Point2f warpPoint(const Point2f& src, const Mat& W);
		CVLIB_DECLSPEC void drawWarpRect(Mat& image, const Rect& rect, const Mat& W);
		//! returns 3x3 perspective transformation for the corresponding 4 point pairs.
		CVLIB_DECLSPEC Mat	getPerspectiveTransform(const Vector<Point2f>& src, const Vector<Point2f>& dst);
		//! returns 2x3 affine transformation for the corresponding 3 point pairs.

		CVLIB_DECLSPEC Mat getRotationMatrix2D(Point2f center, double angle, double scale);

		CVLIB_DECLSPEC Mat	getAffineTransform(const Vector<Point2f>& src, const Vector<Point2f>& dst);
		CVLIB_DECLSPEC Mat	getAffineTransform(const Point2f src[], const Point2f dst[]);
		CVLIB_DECLSPEC void warpPerspective(const Mat& src, Mat& dst, const Mat& M);
		CVLIB_DECLSPEC void warpPerspectiveMask(const Mat& src, Mat& dst, const Mat& M);

		//! warps the image using the precomputed maps. The maps are stored in either floating-point or integer fixed-point format
		CVLIB_DECLSPEC void remap(const Mat& src, Mat& dst, const Mat& map1, const Mat& map2,
			int interpolation, int borderMode = BORDER_CONSTANT, const Scalar& borderValue = Scalar());

		//! converts maps for remap from floating-point to fixed-point format or backwards
		CVLIB_DECLSPEC void convertMaps(const Mat& map1, const Mat& map2, Mat& dstmap1, Mat& dstmap2,
			int dstmap1type, bool nninterpolation = false);

	}
}
