/*!
 * \file	Random.cpp
 * \ingroup base
 * \brief	
 * \author	
 */

#include "Random.h"

namespace cvlib
{

Random::Random()
{
}

Random::~Random()
{
}

void Random::init()
{
	srand( (unsigned)time( NULL ) );
}

int Random::uniform (bool fInit)
{
	if (fInit)
	{
		static int nInit = 0 ;

		if( !nInit ) 
		{
			init() ;
			nInit = 1 ;
		}
	}

	return rand(); 
}

float Random::boundedUniform (float rMin /*= 0.0001f*/, float rMax /*= 1.0f*/, bool fInit)
{
	return ( (rMax - rMin) * (float)Random::uniform(fInit) / (float)RAND_MAX ) + rMin ; 
}

float Random::normal(float rMean /*= 0.0f*/, float rStd /*= 1.0f*/)
{
	static int nFlag = 0;
	float rUnif1, rUnif2 ;
	float rGauss;

	rUnif1 = boundedUniform(0.00001f, 1.0f);
	rUnif2 = boundedUniform(0.00001f, 1.0f);

	rGauss = nFlag ?  rMean + rStd * (float)sqrt(-2.0*log(rUnif1))*(float)cos(2*CVLIB_PI*rUnif2) :
		     rMean + rStd* (float)sqrt(-2.0*log(rUnif1))*(float)sin(2*CVLIB_PI*rUnif2);

	nFlag = nFlag ? 0 : 1;
	return rGauss;
}

float Random::exponential(float rLambda)
{
	return (float)(-1. / rLambda * log(1 - Random::boundedUniform()));
}

float Random::cauchy(float rMedian, float rSigma)
{
	return(rMedian + rSigma * (float)tan(CVLIB_PI*(Random::boundedUniform()-0.5)));
}

float Random::logNormal(float rMean, float rStdv)
{
	float zm = rMean*rMean;
	float zs = rStdv*rStdv;
	return((float)exp(Random::normal((float)log(zm/sqrt(zs + zm)), (float)sqrt(log(zs/zm+1)) )));
}

int Random::geometric(float rP)
{
	return((int)(log(1-Random::boundedUniform()) / log(rP)) + 1);
}

}
