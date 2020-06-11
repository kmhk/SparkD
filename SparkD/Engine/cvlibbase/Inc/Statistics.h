
/*!
 * \file	statistics.h
 * \ingroup math
 * \brief	
 * \author	
 */

#pragma once

#include "Mat.h"

namespace cvlib 
{

void	CVLIB_DECLSPEC meanVector (const Vec* pvecExamples, int nNum, Vec* pvecMean, const Vec* pvecWeight = NULL);

void	CVLIB_DECLSPEC covariance (const Vec* pvecExamples, int nNum, Mat* pmatCov, Vec* pvecMean = NULL, const Vec* pvecWeight = NULL);

void	CVLIB_DECLSPEC variation (const Vec* pvecExamples, int nNum, Mat* pmatVar, Vec* pvecMean = NULL, const Vec* pvecWeight = NULL);

}