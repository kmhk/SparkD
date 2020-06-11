/*!
 * \file	ipThreshold.h
 * \ingroup ipCore
 * \brief   Jitter
 * \author  
 */
#pragma once

#include "cvlibbase/Inc/Mat.h"

namespace cvlib
{

	namespace ip
	{
		enum ThresholdTypes
		{
			/* Types of thresholding */
			THRESH_BINARY = 0,  /* value = value > threshold ? max_value : 0       */
			THRESH_BINARY_INV = 1,  /* value = value > threshold ? 0 : max_value       */
			THRESH_TRUNC = 2,  /* value = value > threshold ? threshold : value   */
			THRESH_TOZERO = 3,  /* value = value > threshold ? value : 0           */
			THRESH_TOZERO_INV = 4,  /* value = value > threshold ? 0 : value           */
			THRESH_MASK = 7,
			THRESH_TILE = 9,
			THRESH_TILE_INV = 10,
			THRESH_OTSU = 8  /* use Otsu algorithm to choose the optimal threshold value;
		combine the flag with one of the above THRESH_* values */
		};

		enum AdaptiveThresholdTypes {
			/** the threshold value \f$_Tp(x,y)\f$ is a mean of the \f$\texttt{blockSize} \times
			\texttt{blockSize}\f$ neighborhood of \f$(x, y)\f$ minus C */
			ADAPTIVE_THRESH_MEAN_C = 0,
			/** the threshold value \f$_Tp(x, y)\f$ is a weighted sum (cross-correlation with a Gaussian
			window) of the \f$\texttt{blockSize} \times \texttt{blockSize}\f$ neighborhood of \f$(x, y)\f$
			minus C . The default sigma (standard deviation) is used for the specified blockSize . See
			cv::getGaussianKernel*/
			ADAPTIVE_THRESH_GAUSSIAN_C = 1
		};

		/* Applies fixed-level threshold to grayscale image. This is a basic operation applied before retrieving contours */
		CVLIB_DECLSPEC double Threshold(const Mat& src, Mat& dst, double thresh, double maxval, int type);
		CVLIB_DECLSPEC uchar otsuThreshold(const Mat& img, const Mat& mask = Mat());
		CVLIB_DECLSPEC int thresholdOfGray(const Mat& gray);
		CVLIB_DECLSPEC void adaptiveThreshold(const Mat& src, Mat& dst, double maxValue, int method, int type, int blockSize, double delta);

	}

}