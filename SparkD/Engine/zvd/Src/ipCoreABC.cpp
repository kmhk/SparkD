/*!
 * \file	ipCoreABC.cpp
 * \ingroup cvlibip
 * \brief   cvlibip
 * \author  
 */

#include "ipCoreABC.h"
#include "cvlibbase/Inc/MatOperation.h"

namespace cvlib
{

bool ipBaseCore::equalType(Mat* p1, Mat* p2)
{
	if (p1 == NULL || p2 == NULL)
		return true;
	if (p1->type() == p2->type())
		return true;
	return false;
}

bool ipBaseCore::equalTypeSize(Mat* p1, Mat* p2)
{
	if (p1 == NULL || p2 == NULL)
		return true;
	if (p1->type() == p2->type() && 
		p1->rows() == p2->rows() &&
		p1->cols() == p2->cols() )
		return true;
	return false;
}

bool ipBaseCore::lut(uchar* pbLut, Mat* pmSrc, Mat* pmDst /*= NULL*/)
{
	switch (pmSrc->type())
	{
	case MAT_Tuchar:
		{
			uchar* pbSrc = pmSrc->data.ptr[0];
			uchar* pbDst = pmDst ? pmDst->data.ptr[0]: pbSrc;
			int cn=pmSrc->channels();
			int size=pmSrc->rows() * pmSrc->cols()*cn;
			for(int i=0; i < size; i++)
				pbDst[i] = pbLut[pbSrc[i]];
		}
		break;
	case MAT_Tfloat:
		{
			float* prSrc = pmSrc->data.fl[0];
			float* prDst = pmDst ? pmDst->data.fl[0]: prSrc;
			int cn=pmSrc->channels();
			int size=pmSrc->rows() * pmSrc->cols()*cn;
			for(int i=0; i < size; i++)
				prDst[i] = pbLut[(uchar)prSrc[i]];
		}
		break;
            default:
            assert (false);
	}

	return true;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
bool ipCoreABC::process (Mat* /*pmSrc*/, Mat* /*pmDst = NULL*/)
{
	return false;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
IMPLEMENT_PUMP(ipCorePump, PumpABC)
PumpRet ipCorePump::pumpProcess()
{
 	Mat* pSrc=(Mat*)getData();
 	Mat* pDst=new Mat;
	pDst->create (*pSrc);
 	process(pSrc, pDst);
 	regDataEx(1, pDst, "image");
	return PR_AFTER;
}

void ipCorePump::releaseData()
{
	if (getPin(1)->data())
	{
		delete ((Mat*)getPin(1)->data());
		getPin(1)->setData(NULL);
	}
}
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_PUMP(ipCoreFeaturePump, PumpABC)

/************************************************************************/
/*                                                                      */
/************************************************************************/
IMPLEMENT_PUMP(ipCompoundCorePump, CompoundPump)

ipCompoundCorePump::ipCompoundCorePump()
{
// 	m_numInOut=2; m_linktypes[0]=LT_INPUT; m_linktypes[1]=LT_OUTPUT;
// 	memset (m_pumpDatas, 0, sizeof(m_pumpDatas[0])*MAX_LINKS);
}

ipCompoundCorePump::~ipCompoundCorePump()
{
	release(); 
//	PumpRelease();
}

ipCompoundCorePump& ipCompoundCorePump::add (ipCorePump* pCore)
{
	CompoundPump::add(pCore);
	if (CompoundPump::m_Pumps.getSize()>=2)
	{
		PumpABC* pPrev=(PumpABC*)CompoundPump::m_Pumps[CompoundPump::m_Pumps.getSize()-2];
		pCore->connect(0, pPrev, 1);
	}
	buildOrder();
	
	return *this;
}

ipCompoundCorePump& ipCompoundCorePump::detachAt (int at)
{
	PumpABC* pPump=getAt(at);
	int nCount = getPumpCount();
	if (at >= nCount)
		return *this;

	CompoundPump::detachAt(at);
	if (at == nCount-1 || nCount == 1)
	{
		buildOrder();
		return *this;
	}

	pPump = getAt(at);
	if (at == 0)
	{
		pPump->disconnect(0);
	}
	else
	{
		PumpABC* pPrev=getAt(at-1);
		pPump->connect(0, pPrev, 1);
	}
	buildOrder();
	
	return *this;
}

bool ipCompoundCorePump::process (Mat* pmSrc, Mat* pmDst /*= NULL*/)
{
	if (m_Pumps.getSize() == 0)
		return false;

	ipCorePump* pCore=NULL;

	if (pmDst)
	{
		Mat mTemp(pmSrc->rows(), pmSrc->cols(), pmSrc->type());
		Mat* pmin=pmSrc;
		Mat* pmout=&mTemp;

		pCore=(ipCorePump*)m_Pumps[0];
		pCore->process(pmin, pmout);
		for (int i=1; i<m_Pumps.getSize(); i++)
		{
			pmin = pmout;
			pmout = (pmin==pmDst) ? &mTemp: pmDst;
			pCore = (ipCorePump*)m_Pumps[i];
			pCore->process(pmin, pmout);
		}
		if (pmDst != pmout)
			pmout->convertTo(*pmDst, pmDst->type(), CT_Cast);
	}
	else
	{
		Mat mTemp(pmSrc->rows(), pmSrc->cols(), pmSrc->type());
		Mat* pmin=pmSrc;
		Mat* pmout=&mTemp;
		for (int i=0; i<m_Pumps.getSize(); i++)
		{
			pCore = (ipCorePump*)m_Pumps[i];
			pCore->process(pmin, pmout);
			pmin = pmout;
			pmout = (pmin==pmSrc) ? &mTemp: pmSrc;
		}
		if (pmSrc != pmout)
			pmout->convertTo(*pmSrc, pmSrc->type(), CT_Cast);
	}
	return true;
}

void ipCompoundCorePump::releaseData()
{
//	CompoundPump::releaseData();
}

void ipCompoundCorePump::release ()
{
}

PumpRet ipCompoundCorePump::pumpProcess()
{
	Mat* pImage=(Mat*)getData();
	Mat* pDst= new Mat;
	bool fSucess = process(pImage, pDst);
	regDataEx(1, pDst, "image");
	if (fSucess)
		return PR_AFTER;
	else
		return PR_ABORT;
}

}
