
/*!
 * \file    Distance.h 
 * \ingroup	dist
 * \brief
 * \author
 */
#pragma once

#include "cvlibbaseDef.h"
#include "EnumString.h"

namespace cvlib {

	enum enDistType
	{
		DT_L1 = 0,
		DT_L2,
		DT_Cor,
		DT_NormalizedCor,
	};

	Begin_Enum_String(enDistType)
	{
		Enum_String(DT_L1);
		Enum_String(DT_L2);
		Enum_String(DT_Cor);
		Enum_String(DT_NormalizedCor);
	}
	End_Enum_String;

	class CVLIB_DECLSPEC Distance
	{

	public:
		Distance();
		virtual ~Distance();

		static float dist(const float* prVector1, const float* prVector2, int nSize, enDistType disttype);
		static double dist(const double* prVector1, const double* prVector2, int nSize, enDistType disttype);
		static float distanceCityBlock(const float* prVector1, const float* prVector2, int nSize);
		static double distanceCityBlock(const double* prVector1, const double* prVector2, int nSize);
		static float distanceEuclidean(const float* prVector1, const float* prVector2, int nSize);
		static double distanceEuclidean(const double* prVector1, const double* prVector2, int nSize);
		static float distanceNorm(const float* prVector1, const float* prVector2, int nSize);
		static double distanceNorm(const double* prVector1, const double* prVector2, int nSize);
		static float distanceCorrelation(const float* prVector1, const float* prVector2, int nSize);
		static double distanceCorrelation(const double* prVector1, const double* prVector2, int nSize);
		static float distanceNormalizedCorrelation(const float* prVector1, const float* prVector2, int nSize);
		static double distanceNormalizedCorrelation(const double* prVector1, const double* prVector2, int nSize);
		static float distanceBunch(const float* prVector1, const float* prVector2, int nSize, int nBunchSize, int bPixel);
		static double distanceBunch(const double* prVector1, const double* prVector2, int nSize, int nBunchSize, int bPixel);
		static float distanceNovelEuclidean(const float* prVector1, const float* prVector2, int nSize, int nBunchSize, float rBeta, int bPixel);
		static double distanceNovelEuclidean(const double* prVector1, const double* prVector2, int nSize, int nBunchSize, double rBeta, int bPixel);
	};

}