#pragma once

#include "Mat.h"

namespace cvlib
{

	enum enBMPFormatType
	{
		CT_DefaultMode,
		CT_GRAY,
		CT_RGB,
		CT_RGBA,
		CT_BGR,
		CT_ABGR,
		CT_ARGB,
		CT_BGRA,
	};

#define HSL_H_MAX 360.0f
#define HSL_S_MAX 1.0f
#define HSL_L_MAX 1.0f
#define YUV_Y_MAX 255.0f
#define YUV_U_MAX 255.0f
#define YUV_V_MAX 255.0f

#define LAB_L_MIN	0.0f
#define LAB_L_MAX	100.f
#define LAB_A_MIN	-86.1884384f
#define LAB_A_MAX	98.2497330f
#define LAB_B_MIN	-107.853745f
#define LAB_B_MAX	94.4877167f

	/**
	 @brief
	 */
	class CVLIB_DECLSPEC ColorSpace
	{
	public:
		static float GetColorDistance(float r1, float g1, float b1, float r2, float g2, float b2);

		static void RGBtoGray(const Mat& colorImg, Mat& mgray, enBMPFormatType ct = CT_RGB);
		static void GraytoRGB(const Mat& gray, Mat& color, enBMPFormatType ct = CT_RGB);
		static void split(const Mat& color, Mat& x, Mat& y, Mat& z);
		static void split(const Mat& color, std::vector<Mat>& channels);
		static void RGBtoBGRA(const Mat& color1, Mat& color2);
		static void toRGB(const Mat& colorImg, Mat& color, enBMPFormatType srcct);
		static void toRGBA(const Mat& colorImg, Mat& color, enBMPFormatType srcct = CT_DefaultMode);
		static void extractChannel(const Mat& src, Mat& dst, int ch);
		static void merge(Mat** const srcs, int n, Mat& dst);
		static void merge(std::vector<Mat>& srcs, Mat& dst);
		static void merge(const Mat& colorMat, const Mat& malpha, Mat& dst);

		static void RGBtoHSV(const uchar* bR, float *prH);

		static void RGBtoHSL(const uchar* pbR, float *prH);
		static void RGBtoLab(const uchar* pbR, float *prL);
		static void RGBtoXYZ(const uchar* pbR, float *prX_);
		static void RGBtoYUV(const uchar* pbR, float *prY_);

		static void RGBtoHSL(const Mat& src, Mat& dst);
		static void RGBtoLab(const Mat& src, Mat& dst);
		static void RGBtoXYZ(const Mat& src, Mat& dst);
		static void RGBtoYUV(const Mat& src, Mat& dst);
		static void RGBtoHSV(const Mat& src, Mat& dst);

		static void XYZtoRGB(const float* x, uchar* pbR);
		static void XYZtoLab(const float* px, float* prL);
		static void XYZtoRGB(const Mat& src, Mat& dst);
		static void XYZtoLab(const Mat& src, Mat& dst);

		static void LabtoXYZ(const float* l, float* x);
		static void LabtoRGB(const float* l, uchar* red);
		static void LabtoXYZ(const Mat& src, Mat& dst);
		static void LabtoRGB(const Mat& src, Mat& dst);

		static void HSLtoRGB(const float* h, uchar* pbR);
		static void HSLtoRGB(const Mat& src, Mat& dst);

		static void YUVtoRGB(const float* y, uchar* pbR);
		static void YUVtoRGB(const Mat& src, Mat& dst);
	private:
		static float Fxyz(float t);
	};

}