
/*!
 * \file	ImageFunc.h
 * \ingroup VOR
 * \brief	
 * \author	
 */

#pragma once

#include "cvlibbase/Inc/_cvlibbase.h"

namespace cvlib { namespace ip{

struct CVLIB_DECLSPEC PeakInfo
{
	int idx;
	bool nFlagMin;
	double value;
	int spread;
	int Sharp(const Vecb& bLum);
};
CVLIB_DECLSPEC void		detectPeak(const Vec& vLum, Vector<PeakInfo>& peaks);

}}
