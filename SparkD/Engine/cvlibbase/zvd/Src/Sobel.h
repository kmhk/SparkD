#pragma once

#include "Mat.h"

namespace cvlib
{


	class CVLIB_DECLSPEC Sobel
	{
	public:
		Sobel();
		~Sobel();
		virtual void	process(Mat* pIn, Mat* pOut = NULL);

		void			getIntensityX(const Mat* pmatIn, Mat* pmatOut = NULL);
		void			getIntensityY(const Mat* pmatIn, Mat* pmatOut = NULL);

		static void		gradientMagnitude(const Mat* pmIn, Mat* pmOut);
		static void		gradientMagnitudeX(const Mat* pmIn, Mat* pmOut, bool fABS = true);
		static void		gradientMagnitudeY(const Mat* pmIn, Mat* pmOut, bool fABS = true);

		static void		processEOF(const Mat* pmatIn, Mat* pmatOut/* = NULL*/, int nThreshold = 0);
		static void		EOField(const Mat* pmatIn, Mat* pmatOut, Mati* pmEdge = NULL);
	};

	namespace ip
	{

		CVLIB_DECLSPEC void edgeOrientaitonField(const Mat* pmatIn, Mat* pmatOut, Mati* pmEdge = NULL);
		CVLIB_DECLSPEC void edgeOrientaitonField(const Mat& in, Mat& out);
		CVLIB_DECLSPEC void sobel(const Mat& src, Mat& dst, int dx, int dy, int aperture_size);
		CVLIB_DECLSPEC void laplace(const Mat& src, Mat& dst, int aperture_size);

	}

}
