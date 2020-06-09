
/*!
 * \file	Random.h
 * \ingroup base
 * \brief	.
 * \author	
 */

#pragma once

#include "cvlibmacros.h"

namespace cvlib
{
	/**
	 @brief   random class
	 */
	class CVLIB_DECLSPEC Random
	{
	public:
		// Constructor and Destructor
		Random();
		virtual ~Random();

		// Initialization
		static void init();

		// Operations for generating random number according to different distribution.
		static int uniform(bool fInit = true);

		static float boundedUniform(float rMin = 0.0001f, float rMax = 1.0f, bool fInit = true);

		static float normal(float rMean = 0.0f, float rStd = 1.0f);

		static float exponential(float rLambda);

		static float cauchy(float rMedian, float rSigma);

		static float logNormal(float rMean, float rStdv);

		static int geometric(float rP);
	};

}