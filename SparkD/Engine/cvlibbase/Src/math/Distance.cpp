/*!
 * \file
 * \brief	
 * \author	
 */
#include "Distance.h"
#include <math.h>

namespace cvlib
{

Distance::Distance()
{
}

Distance::~Distance()
{
}

float Distance::distanceCityBlock(const float* prVector1, const float* prVector2, int nSize)
{
    int k;
    float sum = 0.0f;
    for (k = 0; k < nSize; k++)
	{
        sum += (float)fabs(prVector1[k] - prVector2[k]);
    }
    return sum;
}

double Distance::distanceCityBlock(const double* prVector1, const double* prVector2, int nSize)
{
    int k;
    double sum = 0.0;
    for (k = 0; k < nSize; k++)
	{
        sum += fabs(prVector1[k] - prVector2[k]);
    }
    return sum;
}

float Distance::distanceEuclidean(const float* prVector1, const float* prVector2, int nSize)
{
    int k;
    float sum = 0.0f;
    for (k = 0; k < nSize; k++)
	{
        sum += (prVector1[k] - prVector2[k]) * (prVector1[k] - prVector2[k]);
    }
    return (float)sqrt(sum);
}

double Distance::distanceEuclidean(const double* prVector1, const double* prVector2, int nSize)
{
    int k;
    double sum = 0.0;
    for (k = 0; k < nSize; k++)
	{
        sum += (prVector1[k] - prVector2[k]) * (prVector1[k] - prVector2[k]);
    }
    return sqrt(sum);
}

float Distance::distanceNorm(const float* prVector1, const float* prVector2, int nSize)
{
    int k;
    float sum = 0.0f;
    for (k = 0; k < nSize; k++)
	{
        sum += (prVector1[k] - prVector2[k]) * (prVector1[k] - prVector2[k]);
    }
    return (float)sum;
}

double Distance::distanceNorm(const double* prVector1, const double* prVector2, int nSize)
{
    int k;
    double sum = 0.0;
    for (k = 0; k < nSize; k++)
	{
        sum += (prVector1[k] - prVector2[k]) * (prVector1[k] - prVector2[k]);
    }
    return sum;
}

float Distance::distanceCorrelation(const float* prVector1, const float* prVector2, int nSize)
{
    int k;
    float mag1 = 0.0f, mag2 = 0.0f, dot = 0.0f;
	
    for (k = 0; k < nSize; k++)
	{
        mag1 += prVector1[k] * prVector1[k];
        mag2 += prVector2[k] * prVector2[k];
        dot += prVector1[k] * prVector2[k];
    }

	if (mag1 == 0 && mag2 == 0)
		return 0.0f;
	float rDD = (float)sqrt(mag1) * (float)sqrt(mag2);
	if (rDD == 0)
		return 1.0f;
	else
		return (float)(1.0f - dot / rDD);
}

double Distance::distanceCorrelation(const double* prVector1, const double* prVector2, int nSize)
{
    int k;
    double mag1 = 0.0, mag2 = 0.0, dot = 0.0;
	
    for (k = 0; k < nSize; k++)
	{
        mag1 += prVector1[k] * prVector1[k];
        mag2 += prVector2[k] * prVector2[k];
        dot += prVector1[k] * prVector2[k];
    }
	if (mag1 == 0 && mag2 == 0)
		return 0.0f;
	double rDD = sqrt(mag1) * sqrt(mag2);
	if (rDD == 0)
		return 1.0f;
	else
		return (1.0 - dot / rDD);
}

float Distance::distanceNormalizedCorrelation(const float* prVector1, const float* prVector2, int nSize)
{
    int k;
    float mu1 = 0.0f, mu2 = 0.0f, std1 = 0.0f, std2 = 0.0f, dot = 0.0f, corr;

    for (k = 0; k < nSize; k++)
	{
        mu1 += prVector1[k];
        mu2 += prVector2[k];
    }
    mu1 = mu1 / nSize;
    mu2 = mu2 / nSize;
	
    for (k = 0; k < nSize; k++)
	{
        std1 += (prVector1[k] - mu1) * (prVector1[k] - mu1);
        std2 += (prVector2[k] - mu2) * (prVector2[k] - mu2);
        dot += (prVector1[k] - mu1) * (prVector2[k] - mu2);
    }
	if (std1 == 0 && std2 == 0)
		return 0.0f;
	float rDD = (float)sqrt(std1) * (float)sqrt(std2);
	if (rDD == 0)
		return 1.0f;

    corr = (float)(dot / rDD);
	
    return (1.0f - corr);
}

double Distance::distanceNormalizedCorrelation(const double* prVector1, const double* prVector2, int nSize)
{
    int k;
    double mu1 = 0.0, mu2 = 0.0, std1 = 0.0, std2 = 0.0, dot = 0.0, corr;
	
    for (k = 0; k < nSize; k++) {
        mu1 += prVector1[k];
        mu2 += prVector2[k];
    }
    mu1 = mu1 / nSize;
    mu2 = mu2 / nSize;
	
    for (k = 0; k < nSize; k++)
	{
        std1 += (prVector1[k] - mu1) * (prVector1[k] - mu1);
        std2 += (prVector2[k] - mu2) * (prVector2[k] - mu2);
        dot += (prVector1[k] - mu1) * (prVector2[k] - mu2);
    }
 	if (std1 == 0 && std2 == 0)
		return 0.0;
	double rDD = sqrt(std1) * sqrt(std2);
	if (rDD == 0)
		return 1.0;

    corr = dot / rDD;
	
    return (1.0 - corr);
}

float Distance::distanceBunch(const float* prVector1, const float* prVector2, int nSize, int nBunchSize, int bPixel)
{
	float sum = 0.0f;
	int k;
	int	nBunchNum = nSize / nBunchSize;

	if (bPixel)
	{
		for (k = 0; k < nBunchNum; k++)
		{
			sum += distanceCorrelation(prVector1 + k * nBunchSize, prVector2 + k * nBunchSize, nBunchSize);
		}
		sum /= nBunchNum;
	}
	else
	{
		sum = distanceCorrelation(prVector1, prVector2, nSize);
	}
	
	return sum;
}

double Distance::distanceBunch(const double* prVector1, const double* prVector2, int nSize, int nBunchSize, int bPixel)
{
	double sum = 0.0;
	int k;
	int	nBunchNum = nSize / nBunchSize;
	
	if (bPixel)
	{
		for (k = 0; k < nBunchNum; k++)
		{
			sum += distanceCorrelation(prVector1 + k * nBunchSize, prVector2 + k * nBunchSize, nBunchSize);
		}
		sum /= nBunchNum;
	}
	else
	{
		sum = distanceCorrelation(prVector1, prVector2, nSize);
	}
	
	return sum;
}

float Distance::distanceNovelEuclidean(const float* prVector1, const float* prVector2, int nSize, int nBunchSize, float rBeta, int bPixel)
{
	float sum = 0.0f, euc = 0.0f;
	int k;
	int	nBunchNum = nSize / nBunchSize;
	
	if (bPixel)
	{	
		for (k = 0; k < nBunchNum; k++)
		{
			euc = distanceNorm(prVector1 + k * nBunchSize, prVector2 + k * nBunchSize, nBunchSize);
			euc = 1.0f - (float)exp(euc / rBeta);
			sum += euc;
		}
		sum /= nBunchNum;
	}
	else
	{
		euc = distanceNorm(prVector1, prVector2, nSize);
		euc = 1.0f - (float)exp(euc / rBeta);
		sum = euc;
	}
	
	return sum;
}

double Distance::distanceNovelEuclidean(const double* prVector1, const double* prVector2, int nSize, int nBunchSize, double rBeta, int bPixel)
{
	double sum = 0.0, euc = 0.0;
	int k;
	int	nBunchNum = nSize / nBunchSize;
	
	if (bPixel)
	{
		for (k = 0; k < nBunchNum; k++)
		{
			euc = distanceNorm(prVector1 + k * nBunchSize, prVector2 + k * nBunchSize, nBunchSize);
			euc = 1.0 - exp(euc / rBeta);
			sum += euc;
		}
		sum /= nBunchNum;
	}
	else
	{
		euc = distanceNorm(prVector1, prVector2, nSize);
		euc = 1.0 - exp(euc / rBeta);
		sum = euc;
	}
	
	return sum;
}

float Distance::dist (const float* prVector1, const float* prVector2, int nSize, enDistType disttype)
{
	switch(disttype)
	{
	case DT_L1:
		return distanceCityBlock(prVector1, prVector2, nSize);
	case DT_L2:
		return distanceEuclidean(prVector1, prVector2, nSize);
	case DT_Cor:
		return distanceCorrelation(prVector1, prVector2, nSize);
	case DT_NormalizedCor:
		return distanceNormalizedCorrelation(prVector1, prVector2, nSize);
	default:
		return 0.0f;
	}
}

double Distance::dist (const double* prVector1, const double* prVector2, int nSize, enDistType disttype)
{
	switch(disttype)
	{
	case DT_L1:
		return distanceCityBlock(prVector1, prVector2, nSize);
	case DT_L2:
		return distanceEuclidean(prVector1, prVector2, nSize);
	case DT_Cor:
		return distanceCorrelation(prVector1, prVector2, nSize);
	case DT_NormalizedCor:
		return distanceNormalizedCorrelation(prVector1, prVector2, nSize);
	default:
		assert (false);
		return 0.0;
	}
}

}