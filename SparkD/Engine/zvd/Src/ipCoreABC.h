/*!
 * \file	ipCoreABC.h
 * \ingroup cvlibip
 * \brief   cvlibip
 * \author   */
#pragma once

#include "cvlibbase/Inc/_cvlibbase.h"

namespace cvlib
{

	class ipBaseCore
	{
	public:
		ipBaseCore() : m_fVerbose(false) {}
		virtual ~ipBaseCore() {}
		bool lut(uchar* pbLut, Mat* pmSrc, Mat* pmDst = NULL);
	public:
		bool	m_fVerbose;
	public:
		static	bool	equalType(Mat* p1, Mat* p2);
		static	bool	equalTypeSize(Mat* p1, Mat* p2);
	};

	class ipCoreABC : public ipBaseCore
	{
	public:
		ipCoreABC() {}
		virtual ~ipCoreABC() { release(); }

		virtual bool process(Mat* /*pmSrc*/, Mat* /*pmDst = NULL*/);
		virtual void release() {}
	};

	class ipCorePump : public ipCoreABC, public PumpABC, public IParamABC
	{
		DECLARE_PUMP(ipCorePump)
	public:
		ipCorePump()
		{
			m_numInOut = 2; m_linktypes[0] = LT_INPUT; m_linktypes[1] = LT_OUTPUT;
			memset(m_pumpDatas, 0, sizeof(m_pumpDatas[0])*MAX_LINKS);
			resetLinks();
		}
		virtual ~ipCorePump() { PumpRelease(); }
		virtual Mat* pumpProcess(Mat* pImage) { Mat* pdst = new Mat; process(pImage, pdst); return pdst; }
		virtual PumpRet pumpProcess();
	public:
		virtual void releaseData();
	};

	class ipCoreFeaturePump : public ipBaseCore, public PumpABC
	{
		DECLARE_PUMP(ipCoreFeaturePump)
	public:
		ipCoreFeaturePump() {}
		virtual ~ipCoreFeaturePump() { release(); }

		virtual bool process(Mat* /*pmSrc*/) { return false; }
		virtual bool getResultImage(Mat* /*pSrc*/) { return false; }
		virtual void release() {}
		virtual Mat* pumpProcess(Mat* pImage) { process(pImage); getResultImage(pImage); return pImage; }
	};

	class ipCompoundCorePump : protected ipCoreABC, public CompoundPump
	{
		DECLARE_PUMP(ipCompoundCorePump)
	public:
		ipCompoundCorePump();
		virtual ~ipCompoundCorePump();

		ipCompoundCorePump& add(ipCorePump* pCore);
		ipCompoundCorePump& detachAt(int at);

		virtual bool process(Mat* pmSrc, Mat* pmDst = NULL);
		virtual void release();

		virtual Mat* pumpProcess(Mat* pImage) { process(pImage); return pImage; }
		virtual PumpRet pumpProcess();
		virtual void releaseData();
	};

}