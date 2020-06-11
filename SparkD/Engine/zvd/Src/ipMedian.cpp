/*!
 * \file	ipMedian.cpp
 * \ingroup ipCore
 * \brief   median
 * \author  
 */
#include "cvlibbase/Inc/cvlibutil.h"
#include "cvlibbase/Inc/ColorSpace.h"
#include "ipCoreABC.h"
#include "ipFilter.h"

namespace cvlib
{

	class ipMedian : public ipCorePump
	{
		DECLARE_PUMP(ipMedian)
	public:
		enum scantype { Row, Col };
		ipMedian(const ipMedian& from) { m_nKsize = from.m_nKsize; }
		ipMedian(int nKsize = 3) : m_nKsize(nKsize) {}
		virtual bool process(Mat* pmSrc, Mat* pmDst = NULL);
		bool process(Mat* pmSrc, scantype scantype, Mat* pmDst = NULL);
		bool process(Vec* pvSrc, Vec* pvDst = NULL);
	public:
		int m_nKsize;
	};

	IMPLEMENT_PUMP(ipMedian, ipCorePump)
		bool ipMedian::process(Mat* pmSrc, Mat* pmDst/* = NULL*/)
	{
		if (pmDst && !equalTypeSize(pmSrc, pmDst))
		{
			pmDst->release();
			pmDst->create(*pmSrc);
		}
		int k2 = m_nKsize / 2;
		int kmax = m_nKsize - k2;
		int i, j, k;

		int* kernel = (int*)malloc(m_nKsize*m_nKsize * sizeof(int));

		int xmin, xmax, ymin, ymax;
		xmin = ymin = 0;
		xmax = pmSrc->cols(); ymax = pmSrc->rows();
		int cn = pmSrc->channels();
		if (cn == 1)
		{
			uchar** ppbSrc;
			uchar** ppbDst;
			Mat mTemp;
			if (pmDst)
			{
				ppbSrc = pmSrc->data.ptr;
				ppbDst = pmDst->data.ptr;
			}
			else
			{
				mTemp = *pmSrc;
				ppbSrc = mTemp.data.ptr;
				ppbDst = pmSrc->data.ptr;
			}
			for (int y = ymin; y < ymax; y++)
			{
				for (int x = xmin; x < xmax; x++)
				{
					for (j = -k2, i = 0; j < kmax; j++)
					{
						for (k = -k2; k < kmax; k++, i++)
						{
							if (pmSrc->isInside(x + j, y + k))
								kernel[i] = ppbSrc[y + k][x + j];
							else
								i--;
						}
					}
					cvutil::sort(kernel, i, cvutil::SORT_INC);
					ppbDst[y][x] = (uchar)kernel[i / 2];
				}
			}
		}
		else
		{
			uchar** ppbSrc;
			uchar** ppbDst;
			Mat mTemp;
			if (pmDst)
			{
				ppbSrc = pmSrc->data.ptr;
				ppbDst = pmDst->data.ptr;
			}
			else
			{
				mTemp = *pmSrc;
				ppbSrc = mTemp.data.ptr;
				ppbDst = pmSrc->data.ptr;
			}
			for (int y = ymin; y < ymax; y++)
			{
				for (int x = xmin; x < xmax; x++)
				{
					for (int icn = 0; icn < cn; icn++)
					{
						for (j = -k2, i = 0; j < kmax; j++)
						{
							for (k = -k2; k < kmax; k++, i++)
							{
								if (pmSrc->isInside(x + j, y + k))
									kernel[i] = ppbSrc[y + k][(x + j)*cn + icn];
								else
									i--;
							}
						}
						cvutil::sort(kernel, i, cvutil::SORT_INC);
						ppbDst[y][x*cn + icn] = (uchar)kernel[i / 2];
					}

				}
			}
		}
		free(kernel);
		return true;
	}

	bool ipMedian::process(Mat* pmSrc, scantype scantype, Mat* pmDst)
	{
		Mat* pvsrc = NULL;
		Mat* pvdst = NULL;
		Mat vTemp;
		if (pmDst)
		{
			pmDst->release();
			pmDst->create(*pmSrc);
			pvsrc = pmSrc;
			pvdst = pmDst;
		}
		else
		{
			vTemp.create(*pmSrc, true);
			pvsrc = &vTemp;
			pvdst = pmSrc;
		}
		uchar** ppbSrc = pvsrc->data.ptr;
		uchar** ppbDst = pvdst->data.ptr;
		int k2 = m_nKsize / 2;
		int kmax = m_nKsize - k2;
		int i, j;

		int* kernel = (int*)malloc(m_nKsize * sizeof(int));

		if (scantype == Row)
		{
			int xmin, xmax;
			xmin = 0;
			xmax = pvsrc->cols();
			for (int y = 0; y < pvsrc->rows(); y++)
			{
				for (int x = xmin; x < xmax; x++)
				{
					for (j = -k2, i = 0; j < kmax; j++, i++)
					{
						if (pvsrc->isInside(x + j, y))
							kernel[i] = ppbSrc[y][x + j];
						else
							i--;
					}
					cvutil::sort(kernel, i, cvutil::SORT_INC);
					ppbDst[y][x] = (uchar)kernel[i / 2];
				}
			}
		}
		else
		{
			int xmin, xmax;
			xmin = 0;
			xmax = pvsrc->rows();
			for (int y = 0; y < pvsrc->cols(); y++)
			{
				for (int x = xmin; x < xmax; x++)
				{
					for (j = -k2, i = 0; j < kmax; j++, i++)
					{
						if (pvsrc->isInside(y, x + j))
							kernel[i] = ppbSrc[x + j][y];
						else
							i--;
					}
					cvutil::sort(kernel, i, cvutil::SORT_INC);
					ppbDst[x][y] = (uchar)kernel[i / 2];
				}
			}
		}
		free(kernel);
		return true;
	}

	bool ipMedian::process(Vec* pvSrc, Vec* pvDst)
	{
		Vec* pvsrc = NULL;
		Vec* pvdst = NULL;
		Vec vTemp;
		if (pvDst)
		{
			pvDst->release();
			pvDst->create(*pvSrc);
			pvsrc = pvSrc;
			pvdst = pvDst;
		}
		else
		{
			vTemp.create(*pvSrc, true);
			pvsrc = &vTemp;
			pvdst = pvSrc;
		}
		uchar* pbSrc = pvsrc->data.ptr;
		uchar* pbDst = pvdst->data.ptr;

		int k2 = m_nKsize / 2;
		int kmax = m_nKsize - k2;
		int i, j;

		int* kernel = (int*)malloc(m_nKsize * sizeof(int));

		int xmin, xmax;
		xmin = 0;
		xmax = pvsrc->length();

		for (int x = xmin; x < xmax; x++)
		{
			for (j = -k2, i = 0; j < kmax; j++, i++)
			{
				if (pvsrc->isInside(x + j))
					kernel[i] = pbSrc[x + j];
				else
					i--;
			}
			cvutil::sort(kernel, i, cvutil::SORT_INC);
			pbDst[x] = (uchar)kernel[i / 2];
		}
		free(kernel);
		return true;
	}

	namespace ip
	{
		void median(const Mat& src, Mat& dst, int nKsize, filterintype scantype)
		{
			ipMedian t(nKsize);
			if (scantype == ip::VECRow)
				t.process((Mat*)&src, ipMedian::Row, &dst);
			else if (scantype == ip::VECCol)
				t.process((Mat*)&src, ipMedian::Col, &dst);
			else
				t.process((Mat*)&src, &dst);
		}
		void median(Mat& src, int nKsize, filterintype scantype)
		{
			ipMedian t(nKsize);
			if (scantype == ip::VECRow)
				t.process(&src, ipMedian::Row, 0);
			else if (scantype == ip::VECCol)
				t.process(&src, ipMedian::Col, 0);
			else
				t.process(&src, 0);
		}

	}

}
