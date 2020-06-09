/*
 *  Image Function Engine
 *  All rights reserved.
 */

/*!
 * \file	ImageFunc.cpp
 * \ingroup VOR
 * \brief	
 * \author	
 */

#include "detectPeaks.h"
#include "MatOperation.h"
/*
#include "cvlibml/Inc/Dataset.h"
#include "cvlibml/Inc/RegressorTrainer.h"
#include "cvlibml/Inc/Regressor.h"
*/

namespace cvlib { namespace ip{

int PeakInfo::Sharp(const Vecb& bLum)
{
	int k=0;
	int nSharp=0;
	for (int i=-5; i<=0; i++)
	{
		if (idx+i<0 || idx+i>=bLum.length())
			continue;
		k ++;
		
		nSharp+=::abs(bLum[idx+i]-bLum[idx]);
	}
	nSharp/=k;
	return nSharp;
}

typedef void (*DetectPeakFunc)( uchar* const _src, int len, Vector<PeakInfo>& peaks);
template<typename T> static void detectPeak_( uchar* const _src, int len, Vector<PeakInfo>& peaks)
{
	T* const src=(T* const)_src;
	int i;
	Vecf vD(len);
	for (i=1; i<len; i++)
		vD[i]=(float)(src[i]-src[i-1]);
	
	enum PeakStatus {NON, MEET};
	int nStatus=NON;
	PeakInfo peak;
	int nStart=0;
	int nEnd=0;
	for (i=2; i<len; i++)
	{
		switch (nStatus)
		{
		case NON:
			{
				if (vD[i-1] * vD[i] < 0)
				{
					peak.spread=1;
					peak.idx = i-1;
					peak.value=(double)src[peak.idx];
					if (vD[i-1] > 0)
						peak.nFlagMin=false;
					else
						peak.nFlagMin=true;
					peaks.add(peak);
				}
				if (vD[i]==0)
				{
					nStatus=MEET;
					nStart=i;
				}
			}
			break;
		case MEET:
			{
				if (vD[i]!=0)
				{
					nEnd=i-1;
					if (vD[nStart-1]>0 && vD[nEnd+1]<0) // local maximum
					{
						peak.nFlagMin=false;
						peak.spread=nEnd-nStart+2;
						peak.idx = (nEnd+nStart)/2;
						peak.value=(double)src[peak.idx];
						peaks.add(peak);
					}
					else if (vD[nStart-1]<0 && vD[nEnd+1]>0)// local minimum
					{
						peak.nFlagMin=true;
						peak.spread=nEnd-nStart+2;
						peak.idx = (nEnd+nStart)/2;
						peak.value=(double)src[peak.idx];
						peaks.add(peak);
					}
					nStatus=NON;
				}
			}
			break;
		}
	}
}

void detectPeak(const Vec& vLum, Vector<PeakInfo>& peaks)
{
	TYPE t = vLum.type();
	DetectPeakFunc funcs[]={detectPeak_<char>, detectPeak_<uchar>, detectPeak_<short>, detectPeak_<int>, detectPeak_<float>, detectPeak_<double>};
	funcs[t](vLum.data.ptr, vLum.length(), peaks);
}

}}
