
/*!
 * \file	ipTransformation.h
 * \ingroup ipCore
 * \brief
 * \author
 */
#pragma once

#include "_cvlibbase.h"

namespace cvlib
{

	//overflow methods:
enum OverflowMethod {
	OM_COLOR=1,
 	OM_BACKGROUND=2,
 	OM_TRANSPARENT=3,
	OM_WRAP=4,
	OM_REPEAT=5,
	OM_MIRROR=6
};

	//interpolation methods:
enum InterpolationMethod {
	IM_NEAREST_NEIGHBOUR=1,
	IM_BILINEAR		=2,
	IM_BSPLINE		=3,
	IM_BICUBIC		=4,
	IM_BICUBIC2		=5,
	IM_LANCZOS		=6,
	IM_BOX			=7,
	IM_HERMITE		=8,
	IM_HAMMING		=9,
	IM_SINC			=10,
	IM_BLACKMAN		=11,
	IM_BESSEL		=12,
	IM_GAUSSIAN		=13,
	IM_QUADRATIC	=14,
	IM_MITCHELL		=15,
	IM_CATROM		=16
};

enum RotateType{RLeft, RRight, R180};

namespace ip
{
CVLIB_DECLSPEC void rotate(const Mat& src, Mat& dst, RotateType type);
CVLIB_DECLSPEC void rotate(const Mat& src, Mat& dst, float rAngle);
CVLIB_DECLSPEC void rotate(const Mat& src, Mat& dst, float rAngle, InterpolationMethod inMethod/*=IM_BILINEAR*/,
						   OverflowMethod ofMethod=OM_BACKGROUND, COLOR *replColor=0,
						   bool const optimizeRightAngles=true, bool const bKeepOriginalSize=false);

CVLIB_DECLSPEC void resample(const Mat& src, Mat& dst, int newx, int newy, int mode = 1);
CVLIB_DECLSPEC void resample(const Mat& src, Mat& dst, int newx, int newy, InterpolationMethod const inMethod=IM_BICUBIC2,
		   OverflowMethod const ofMethod=OM_REPEAT, bool const disableAveraging=false);

CVLIB_DECLSPEC void crop(const Mat& src, Mat& dst, int left, int top, int right, int bottom);
CVLIB_DECLSPEC void crop(const Mat& src, Mat& dst, const Rect& rect);
CVLIB_DECLSPEC void crop(const Mat& src, Mat& dst, int topx, int topy, int width, int height, float angle);
CVLIB_DECLSPEC void cropEx(const Mat& src, Mat& dst, const Rect& rect, const Scalar& color);

CVLIB_DECLSPEC void skew(const Mat& src, Mat& dst, float xgain, float ygain, int xpivot=0, int ypivot=0, bool bEnableInterpolation = false);
CVLIB_DECLSPEC void expand(const Mat& src, Mat& dst, int left, int top, int right, int bottom, COLOR canvascolor);
CVLIB_DECLSPEC void expand(const Mat& src, Mat& dst, int newx, int newy, COLOR canvascolor);
CVLIB_DECLSPEC void thumbnail(const Mat& src, Mat& dst, int newx, int newy, COLOR canvascolor);

}

}
