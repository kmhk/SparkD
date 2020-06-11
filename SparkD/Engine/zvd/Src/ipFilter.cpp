/*!
 * \file	ipFilter.cpp
 * \ingroup ipCore
 * \brief   
 * \author  
 */
#include "ipFilter.h"
#include "ipCoreABC.h"

namespace cvlib
{
	class ipFilter
	{
	public:
		ipFilter();
		ipFilter(const ipFilter& from);
		ipFilter(const int* pnKernel, int nKsize, int nKFactor, int nKOffset, ip::filterintype type = ip::MAT);
		ipFilter(const float* prKernel, int nKsize, float rKFactor, int nKOffset, ip::filterintype type = ip::MAT);
		ipFilter(const double* prKernel, int nKsize, double rKFactor, int nKOffset, ip::filterintype type = ip::MAT);
		virtual ~ipFilter() { release(); }

		static int		KernelFactor(const int* pnKernel, int nLen);
		static float	KernelFactor(const float* prKernel, int nLen);
		static double	KernelFactor(const double* prKernel, int nLen);

		static bool Process2(const int* kernel, int Ksize, int Kfactor, int Koffset, Mat* pmatOrg, Mat* pmatDest = NULL);
		static bool Process2(const int* kernel, int Ksize, int Kfactor, int Koffset, const Rect* pFilteringRect, Mat* pmatOrg, Mat* pmatDest = NULL);

		int create(const int* pnKernel, int nKsize, int nKFactor, int nKOffset, ip::filterintype type = ip::MAT);
		int create(const float* prKernel, int nKsize, float rKFactor, int nKOffset, ip::filterintype type = ip::MAT);
		int create(const double* prKernel, int nKsize, double rKFactor, int nKOffset, ip::filterintype type = ip::MAT);
		inline bool flagCreate() const { return m_fcreate; }
		virtual bool process(Mat* pmSrc, Mat* pmDst = NULL);
		virtual bool process(Vec* pvSrc, Vec* pvDst = NULL);
		virtual void release();
	public:
		// parameters
		ip::filterintype	m_filterType;
		int* m_pnKernel;
		float* m_prKernel;
		int m_nKSize;
		int m_nKFactor;
		float m_rKFactor;
		int m_nKOffset;

	protected:
		bool m_fcreate;
		bool iProcessMat(Mat* pmSrc, Mat* pmDst = NULL);
		bool iProcessVecRow(Mat* pmSrc, Mat* pmDst = NULL);
		bool iProcessVecCol(Mat* pmSrc, Mat* pmDst = NULL);
	};

ipFilter::ipFilter(const ipFilter& from)
{
	if (from.m_pnKernel)
	{
		m_pnKernel = new int[from.m_nKSize*from.m_nKSize];
		memcpy(m_pnKernel, from.m_pnKernel, sizeof(int) * from.m_nKSize*from.m_nKSize);
	}
	else
		m_pnKernel=NULL;

	if (from.m_prKernel)
	{
		m_prKernel = new float[from.m_nKSize*from.m_nKSize];
		memcpy(m_prKernel, from.m_prKernel, sizeof(int) * from.m_nKSize*from.m_nKSize);
	}
	else
		m_prKernel=NULL;

	m_nKSize = from.m_nKSize;
	m_nKFactor = from.m_nKFactor;
	m_rKFactor = from.m_rKFactor;
	m_nKOffset = from.m_nKOffset;
	m_filterType=from.m_filterType;
	m_fcreate=true;
}

ipFilter::ipFilter()
{
	m_filterType=ip::MAT;
	m_pnKernel = NULL;
	m_prKernel = NULL;
	m_nKSize = 0;
	m_nKFactor = 0;
	m_rKFactor = 0;
	m_nKOffset = 0;
	m_fcreate=false;
}

ipFilter::ipFilter(const int* pnKernel, int nKsize, int nKFactor, int nKOffset, ip::filterintype type)
{
	m_fcreate=false;
	create (pnKernel, nKsize, nKFactor, nKOffset, type);
}

ipFilter::ipFilter(const float* prKernel, int nKsize, float rKFactor, int nKOffset, ip::filterintype type)
{
	m_fcreate=false;
	create (prKernel, nKsize, rKFactor, nKOffset, type);
}

ipFilter::ipFilter(const double* prKernel, int nKsize, double rKFactor, int nKOffset, ip::filterintype type)
{
	m_fcreate=false;
	create (prKernel, nKsize, rKFactor, nKOffset, type);
}

int ipFilter::create(const int* pnKernel, int nKsize, int nKFactor, int nKOffset, ip::filterintype type)
{
	if (!m_fcreate)
	{
		m_fcreate=true;
		m_filterType=type;
		if (m_filterType == ip::MAT)
		{
			m_pnKernel = new int[nKsize*nKsize];
			memcpy(m_pnKernel, pnKernel, sizeof(int) * nKsize*nKsize);
		}
		else if (m_filterType == ip::VECRow || m_filterType == ip::VECCol || m_filterType == ip::VEC)
		{
			m_pnKernel = new int[nKsize];
			memcpy(m_pnKernel, pnKernel, sizeof(int) * nKsize);
		}
		m_prKernel = NULL;
		m_nKSize = nKsize;
		m_nKFactor = nKFactor;
		m_rKFactor = 0;
		m_nKOffset = nKOffset;
		if (m_nKFactor == 0)
			m_nKFactor = 1;
		return 1;
	}
	return 0;
}
int ipFilter::create(const float* prKernel, int nKsize, float rKFactor, int nKOffset, ip::filterintype type)
{
	if (!m_fcreate)
	{
		m_fcreate=true;
		m_filterType=type;
		m_pnKernel = NULL;

		if (m_filterType == ip::MAT)
		{
			m_prKernel = new float[nKsize*nKsize];
			memcpy(m_prKernel, prKernel, sizeof(float)*nKsize*nKsize);
		}
		else if (m_filterType == ip::VECRow || m_filterType == ip::VECCol || m_filterType == ip::VEC)
		{
			m_prKernel = new float[nKsize];
			memcpy(m_prKernel, prKernel, sizeof(float)*nKsize);
		}
		m_nKSize = nKsize;
		m_nKFactor = 0;
		m_rKFactor = rKFactor;
		m_nKOffset = nKOffset;
		if (m_rKFactor == 0)
			m_rKFactor = 1.0f;
		return 1;
	}
	return 0;
}
int ipFilter::create(const double* prKernel, int nKsize, double rKFactor, int nKOffset, ip::filterintype type)
{
	if (!m_fcreate)
	{
		m_fcreate=true;
		m_filterType=type;
		m_pnKernel = NULL;

		if (m_filterType == ip::MAT)
		{
			m_prKernel = new float[nKsize*nKsize];
			for (int i=0;i<nKsize*nKsize; i++)
				m_prKernel[i]=(float)prKernel[i];
		}
		else if (m_filterType == ip::VECRow || m_filterType == ip::VECCol || m_filterType == ip::VEC)
		{
			m_prKernel = new float[nKsize];
			memcpy(m_prKernel, prKernel, sizeof(float)*nKsize);
			for (int i=0;i<nKsize; i++)
				m_prKernel[i]=(float)prKernel[i];
		}
		m_nKSize = nKsize;
		m_nKFactor = 0;
		m_rKFactor = (float)rKFactor;
		m_nKOffset = nKOffset;
		if (m_rKFactor == 0)
			m_rKFactor = 1.0f;
		return 1;
	}
	return 0;
}

void ipFilter::release ()
{
	if (m_fcreate)
	{
		if (m_pnKernel)
			delete []m_pnKernel;
		m_pnKernel = NULL;
		if (m_prKernel)
			delete []m_prKernel;
		m_prKernel = NULL;
		m_fcreate=false;
	}
}

int	ipFilter::KernelFactor(const int* pnKernel, int nLen)
{
	int nRet = 0;
	for (int ii = 0; ii < nLen; ii ++)
		nRet += pnKernel[ii];
	return nRet;
}

float ipFilter::KernelFactor(const float* prKernel, int nLen)
{
	float rRet = 0.0f;
	for (int ii = 0; ii < nLen; ii ++)
		rRet += prKernel[ii];
	return rRet;
}

double ipFilter::KernelFactor(const double* prKernel, int nLen)
{
	double rRet = 0.0f;
	for (int ii = 0; ii < nLen; ii ++)
		rRet += prKernel[ii];
	return rRet;
}

/**
* 2D linear filter
* \param kernel: convolving matrix, in row format.
* \param Ksize: size of the kernel.
* \param Kfactor: normalization constant.
* \param Koffset: bias.
* \verbatim Example: the "soften" filter uses this kernel:
1 1 1
1 8 1
1 1 1
the function needs: kernel={1,1,1,1,8,1,1,1,1}; Ksize=3; Kfactor=16; Koffset=0; \endverbatim
* \return true if everything is ok
*/
bool ipFilter::iProcessMat (Mat* pmSrc, Mat* pmDst /*= NULL*/)
{
	if (m_pnKernel == NULL)
		return false;
	if (pmDst && !ipBaseCore::equalTypeSize(pmSrc, pmDst))
	{
		pmDst->release();
		pmDst->create (*pmSrc, false);
	}

	int k2 = m_nKSize/2;
	int kmax= m_nKSize-k2;
	
	int xmin,xmax,ymin,ymax;
	xmin = ymin = 0;
	xmax = pmSrc->cols();		// width
	ymax = pmSrc->rows();		// height			
	int cn = pmSrc->channels();
	
	int iCount;

	switch(pmSrc->type())
	{
	case MAT_Tuchar:
		{
			int b,i;
			uchar**	ppbSrc;
			short**	ppsTemp;
			uchar**	ppbDst;
			
			Mat matTemp(pmSrc->rows(), pmSrc->cols(), (TYPE)CVLIB_MAKETYPE(MAT_Tshort,cn));
			ppbSrc = pmSrc->data.ptr;
			ppsTemp = matTemp.data.s;
			ppbDst = pmDst ? pmDst->data.ptr : ppbSrc;
			if (cn==1) {
				for(int y = ymin ; y < ymax ; y++) {
					for(int x = xmin ; x < xmax ; x++) {
						if (y-k2 > 0 && (y+kmax-1) < ymax && x-k2 > 0 && (x+kmax-1) < xmax)
						{
							b = 0;
							iCount = 0;
							for(int j = -k2 ; j < kmax ; j++)
							{
								for(int k = -k2 ; k < kmax ; k++)
								{
									i = m_pnKernel[iCount];
									b += ppbSrc[y+j][x+k] * i;
									iCount++;
								}
							}
							ppsTemp[y][x] = (uchar)MIN(255, MAX(0,(int)(b/m_nKFactor + m_nKOffset)));
						}
						else
							ppsTemp[y][x] = ppbSrc[y][x];
					}
				}
			}
			else {
				for(int y = ymin ; y < ymax ; y++) {
					for(int x = xmin ; x < xmax ; x++) {
						if (y-k2 > 0 && (y+kmax-1) < ymax && x-k2 > 0 && (x+kmax-1) < xmax)
						{
							for (int ii=0; ii<cn; ii++) {
								b = 0;
								iCount = 0;
								for(int j = -k2 ; j < kmax ; j++)
								{
									for(int k = -k2 ; k < kmax ; k++)
									{
										i = m_pnKernel[iCount];
										b += ppbSrc[y+j][(x+k)*cn+ii] * i;
										iCount++;
									}
								}
								ppsTemp[y][x*cn+ii] = (uchar)MIN(255, MAX(0,(int)(b/m_nKFactor + m_nKOffset)));
							}
						}
						else{
							for (i=0; i<cn; i++)
								ppsTemp[y][x*cn+i] = ppbSrc[y][x*cn+i];
						}
					}
				}
			}
			
			for (int iRow = 0 ; iRow < matTemp.rows() ; iRow ++)
			{
				for (int iCol = 0 ; iCol < matTemp.cols()*cn ; iCol ++)
					ppbDst[iRow][iCol] = (uchar)MIN(ppsTemp[iRow][iCol], 255);
			}
		}
		break;
	case MAT_Tfloat:
		{
			float**	pprSrc;
			float**	pprTemp;
			float**	pprDst;
			float b;
			int i;
			
			Mat matTemp(pmSrc->rows(), pmSrc->cols(), MAT_Tfloat);
			pprSrc = pmSrc->data.fl;
			pprTemp = matTemp.data.fl;
			pprDst = pmDst ? pmDst->data.fl : pprSrc;
			
			for(int y = ymin ; y < ymax ; y++)
			{
				for(int x = xmin ; x < xmax ; x++)
				{
					if (y-k2 > 0 && (y+kmax-1) < ymax && x-k2 > 0 && (x+kmax-1) < xmax)
					{
						b = 0;
						iCount = 0;
						for(int j = -k2 ; j < kmax ; j++)
						{
							for(int k = -k2 ; k < kmax ; k++)
							{
								i = m_pnKernel[iCount];
								b += pprSrc[y+j][x+k] * i;
								iCount++;
							}
						}
						pprTemp[y][x] = b/m_nKFactor + m_nKOffset;
					}
					else
						pprTemp[y][x] = pprSrc[y][x];
				}
			}
			
			for (int iRow = 0 ; iRow < matTemp.rows() ; iRow ++)
			{
				for (int iCol = 0 ; iCol < matTemp.cols() ; iCol ++)
					pprDst[iRow][iCol] = pprTemp[iRow][iCol];
			}
		}
		break;
        default:
            assert(false);
    }

	return true;
}

bool ipFilter::process (Mat* pmSrc, Mat* pmDst /*= NULL*/)
{
	switch (m_filterType)
	{
	case ip::MAT:
		return iProcessMat(pmSrc, pmDst);
	case ip::VECRow:
		return iProcessVecRow(pmSrc, pmDst);
	case ip::VECCol:
		return iProcessVecCol(pmSrc, pmDst);
        default:
            assert(false);
    }
	return false;
}

bool ipFilter::iProcessVecRow (Mat* pmSrc, Mat* pmDst /*= NULL*/)
{
	if (m_prKernel == NULL)
		return false;
	Mat mTemp;
	Mat* pmsrc=NULL;
	Mat* pmdst=NULL;
	if (pmDst)
	{
		pmDst->release();
		pmDst->create (*pmSrc, false);
		pmsrc=pmSrc;
		pmdst=pmDst;
	}
	else
	{
		mTemp.create (*pmSrc, true);
		pmsrc=&mTemp;
		pmdst=pmSrc;
	}

	int k2 = m_nKSize/2;
	int kmax= m_nKSize-k2;
	
	int xmin,xmax,ymin,ymax;
	xmin = ymin = 0;
	xmax = pmsrc->cols();		// width
	ymax = pmsrc->rows();		// height			
	
	int iCount;

	switch(pmsrc->type())
	{
	case MAT_Tuchar:
		{
			uchar**	ppbSrc=pmsrc->data.ptr;
			uchar**	ppbDst=pmdst->data.ptr;
			for(int y = ymin ; y < ymax ; y++)
			{
				for(int x = xmin ; x < xmax ; x++)
				{
					if (x-k2 > 0 && (x+kmax-1) < xmax)
					{
						int b = 0;
						iCount = 0;
						for(int k = -k2; k < kmax; k++, iCount++)
							b += ppbSrc[y][x+k] * m_pnKernel[iCount];
						ppbDst[y][x] = (uchar)MIN(255, MAX(0,(int)(b/m_nKFactor + m_nKOffset)));
					}
					else
						ppbDst[y][x] = ppbSrc[y][x];
				}
			}
		}
		break;
	case MAT_Tfloat:
		{
			float**	ppbSrc=pmsrc->data.fl;
			float**	ppbDst=pmdst->data.fl;
			for(int y = ymin ; y < ymax ; y++)
			{
				for(int x = xmin ; x < xmax ; x++)
				{
					if (x-k2 > 0 && (x+kmax-1) < xmax)
					{
						float b = 0;
						iCount = 0;
						for(int k = -k2; k < kmax; k++, iCount++)
							b += ppbSrc[y][x+k] * (float)m_prKernel[iCount];
						ppbDst[y][x] = b/(float)m_rKFactor + (float)m_nKOffset;
					}
					else
						ppbDst[y][x] = ppbSrc[y][x];
				}
			}
		}
		break;
        default:
            assert(false);
    }

	return true;
}
bool ipFilter::iProcessVecCol (Mat* pmSrc, Mat* pmDst /*= NULL*/)
{
	if (m_pnKernel == NULL)
		return false;
	Mat mTemp;
	Mat* pmsrc=NULL;
	Mat* pmdst=NULL;
	if (pmDst)
	{
		pmDst->release();
		pmDst->create (*pmSrc, false);
		pmsrc=pmSrc;
		pmdst=pmDst;
	}
	else
	{
		mTemp.create (*pmSrc, true);
		pmsrc=&mTemp;
		pmdst=pmSrc;
	}
	
	int k2 = m_nKSize/2;
	int kmax= m_nKSize-k2;
	
	int xmin,xmax,ymin,ymax;
	xmin = ymin = 0;
	xmax = pmsrc->cols();		// width
	ymax = pmsrc->rows();		// height			
	
	int iCount;
	
	switch(pmsrc->type())
	{
	case MAT_Tuchar:
		{
			uchar**	ppbSrc=pmsrc->data.ptr;
			uchar**	ppbDst=pmdst->data.ptr;
			for(int x = xmin ; x < xmax ; x++)
			{
				for(int y = ymin ; y < ymax ; y++)
				{
					if (y-k2 > 0 && (y+kmax-1) < ymax)
					{
						int b = 0;
						iCount = 0;
						for(int k = -k2; k < kmax; k++, iCount++)
							b += ppbSrc[y+k][x] * m_pnKernel[iCount];
						ppbDst[y][x] = (uchar)MIN(255, MAX(0,(int)(b/m_nKFactor + m_nKOffset)));
					}
					else
						ppbDst[y][x] = ppbSrc[y][x];
				}
			}
		}
		break;
	case MAT_Tfloat:
		{
			float**	ppbSrc=pmsrc->data.fl;
			float**	ppbDst=pmdst->data.fl;
			for(int x = xmin ; x < xmax ; x++)
			{
				for(int y = ymin ; y < ymax ; y++)
				{
					if (y-k2 > 0 && (y+kmax-1) < ymax)
					{
						float b = 0;
						iCount = 0;
						for(int k = -k2; k < kmax; k++, iCount++)
							b += ppbSrc[y+k][x] * (float)m_pnKernel[iCount];
						ppbDst[y][x] = b/(float)m_nKFactor + (float)m_nKOffset;
					}
					else
						ppbDst[y][x] = ppbSrc[y][x];
				}
			}
		}
		break;
        default:
            assert(false);
    }
	
	return true;
}

bool ipFilter::process (Vec* pvSrc, Vec* pvDst/* = NULL*/)
{
	if (m_pnKernel == NULL)
		return false;
	Vec mTemp;
	Vec* pvsrc=NULL;
	Vec* pvdst=NULL;
	if (pvDst)
	{
		pvDst->release();
		pvDst->create (*pvSrc, false);
		pvsrc=pvSrc;
		pvdst=pvDst;
	}
	else
	{
		mTemp.create (*pvSrc, true);
		pvsrc=&mTemp;
		pvdst=pvSrc;
	}
	
	int k2 = m_nKSize/2;
	int kmax= m_nKSize-k2;
	
	int xmin,xmax;
	xmin = 0;
	xmax = pvsrc->length();

	switch (pvsrc->type())
	{
	case MAT_Tuchar:
		{
			uchar*	pbSrc=pvsrc->data.ptr;
			uchar*	pbDst=pvdst->data.ptr;
			for(int x = xmin ; x < xmax ; x++)
			{
				if (x-k2 > 0 && (x+kmax-1) < xmax)
				{
					int b = 0;
					int iCount = 0;
					for(int k = -k2; k < kmax; k++, iCount++)
						b += pbSrc[x+k] * m_pnKernel[iCount];
					pbDst[x] = (uchar)MIN(255, MAX(0,(int)(b/m_nKFactor + m_nKOffset)));
				}
				else
					pbDst[x] = pbSrc[x];
			}
		}
	case MAT_Tfloat:
		{
			float*	pbSrc=pvsrc->data.fl;
			float*	pbDst=pvdst->data.fl;
			for(int x = xmin ; x < xmax ; x++)
			{
				if (x-k2 > 0 && (x+kmax-1) < xmax)
				{
					float b = 0;
					int iCount = 0;
					for(int k = -k2; k < kmax; k++, iCount++)
						b += pbSrc[x+k] * (float)m_pnKernel[iCount];
					pbDst[x] = (float)(b/m_nKFactor + m_nKOffset);
				}
				else
					pbDst[x] = pbSrc[x];
			}
		}
        default:
            assert(false);
    }
	return true;
}

bool ipFilter::Process2(const int* kernel, int Ksize, int Kfactor, int Koffset, Mat* pmatOrg, Mat* pmatDest/* = NULL*/)
{
	assert (pmatOrg->type() == MAT_Tuchar);
	assert (Kfactor != 0);

	if (pmatDest)
	{
		assert (pmatDest->rows() == pmatOrg->rows());
		assert (pmatDest->cols() == pmatOrg->cols());
		assert (pmatDest->type() == pmatOrg->type());
	}

	Mat matTemp(pmatOrg->rows(), pmatOrg->cols(), MAT_Tuchar);

	int k2 = Ksize/2;
	int kmax= Ksize-k2;
	int b,i;

	int xmin,xmax,ymin,ymax;
	xmin = ymin = 0;
	xmax = pmatOrg->cols();		// width
	ymax = pmatOrg->rows();		// height			

	int iCount;
    uchar** ppOrg;
	uchar** ppDest;

	ppOrg = pmatOrg->data.ptr;
	ppDest = matTemp.data.ptr;

	for(int y = ymin ; y < ymax ; y++)
	{
		for(int x = xmin ; x < xmax ; x++)
		{
			if (y-k2 > 0 && (y+kmax-1) < ymax && x-k2 > 0 && (x+kmax-1) < xmax)
			{
				b = 0;
				iCount = 0;
				for(int j = -k2 ; j < kmax ; j++)
				{
					for(int k = -k2 ; k < kmax ; k++)
					{
						i = kernel[iCount];
						b += ppOrg[y+j][x+k] * i;
						iCount++;
					}
				}
				ppDest[y][x] = (uchar)MIN(255, MAX(0,(int)(b/Kfactor + Koffset)));
			}
			else
				ppDest[y][x] = ppOrg[y][x];
		}
	}
	
	uchar** ppbOut;
	if (pmatDest)
		ppbOut = pmatDest->data.ptr;
	else
		ppbOut = pmatOrg->data.ptr;

	for (int iRow = 0 ; iRow < matTemp.rows() ; iRow ++)
		for (int iCol = 0 ; iCol < matTemp.cols() ; iCol ++)
			ppbOut[iRow][iCol] = ppDest[iRow][iCol];
		
	matTemp.release();

	return true;
}

bool ipFilter::Process2(const int* kernel, int Ksize, int Kfactor, int Koffset, const Rect* pFilteringRect, Mat* pmatOrg, Mat* pmatDest/* = NULL*/)
{
	assert (pmatOrg->type() == MAT_Tuchar);
	assert (pmatOrg->rows() > 0 && pmatOrg->cols() > 0);
	assert (Kfactor != 0);
	assert (pFilteringRect);
	assert ((pFilteringRect->x >= 0) && (pFilteringRect->y >= 0) && 
		((pFilteringRect->x + pFilteringRect->width) < pmatOrg->cols()) &&
		((pFilteringRect->y + pFilteringRect->height) < pmatOrg->rows()));
	
	if (pmatDest)
	{
		assert (pmatDest->rows() == pmatOrg->rows());
		assert (pmatDest->cols() == pmatOrg->cols());
		assert (pmatDest->type() == pmatOrg->type());
	}
	
	int nImgWidth = pmatOrg->cols();
	int nImgHeight = pmatOrg->rows();
	
	Mat matTemp(pmatOrg->rows(), pmatOrg->cols(), MAT_Tuchar);
	matTemp.zero();
	
	int k2 = Ksize/2;
	int kmax= Ksize-k2;
	int b,i;
	
	int xmin,xmax,ymin,ymax;
	xmin = pFilteringRect->x;
	ymin = pFilteringRect->y;
	xmax = pFilteringRect->x + pFilteringRect->width;		// width
	ymax = pFilteringRect->y + pFilteringRect->height;		// height			
	
	int iCount;
    uchar** ppOrg;
	uchar** ppDest;
	
	ppOrg = pmatOrg->data.ptr;
	ppDest = matTemp.data.ptr;
	
	for(int y = ymin ; y < ymax ; y++)
	{
		for(int x = xmin ; x < xmax ; x++)
		{
			if (y-k2 > 0 && (y+kmax-1) < nImgHeight && x-k2 > 0 && (x+kmax-1) < nImgWidth)
			{
				b = 0;
				iCount = 0;
				for(int j = -k2 ; j < kmax ; j++)
				{
					for(int k = -k2 ; k < kmax ; k++)
					{
						i = kernel[iCount];
						b += ppOrg[y+j][x+k] * i;
						iCount++;
					}
				}
				ppDest[y][x] = (uchar)MIN(255, MAX(0,(int)(b/Kfactor + Koffset)));
			}
			else
				ppDest[y][x] = ppOrg[y][x];
		}
	}
	
	uchar** ppbOut;
	if (pmatDest)
		ppbOut = pmatDest->data.ptr;
	else
		ppbOut = pmatOrg->data.ptr;
	
	for (int iRow = 0 ; iRow < matTemp.rows() ; iRow ++)
		for (int iCol = 0 ; iCol < matTemp.cols() ; iCol ++)
			ppbOut[iRow][iCol] = ppDest[iRow][iCol];
		
		matTemp.release();
		
		return true;
}

//////////////////////////////////////////////////////////////////////////
/*
class ipFilterSep : public ipFilter
{
public:
	ipFilterSep();
	ipFilterSep(const ipFilter& from);
	ipFilterSep(int* pnKernel, int nKsize);
	ipFilterSep(float* pnKernel, int nKsize);
	ipFilterSep(double* pnKernel, int nKsize);
	virtual ~ipFilterSep();

	virtual bool process (Mat* pmSrc, Mat* pmDst = NULL);
private:
	Mat* m_pmTemp;
};

ipFilterSep::ipFilterSep() :ipFilter(){m_pmTemp=NULL;}
ipFilterSep::ipFilterSep(const ipFilter& from) : ipFilter(from) {m_pmTemp=NULL;}
ipFilterSep::ipFilterSep(int* pnKernel, int nKsize) : ipFilter(pnKernel, nKsize, ipFilter::KernelFactor(pnKernel, nKsize),0, ipFilter::VEC) {m_pmTemp=NULL;}
ipFilterSep::ipFilterSep(float* pnKernel, int nKsize) : ipFilter(pnKernel, nKsize, ipFilter::KernelFactor(pnKernel, nKsize),0, ipFilter::VEC) {m_pmTemp=NULL;}
ipFilterSep::ipFilterSep(double* pnKernel, int nKsize) : ipFilter(pnKernel, nKsize, ipFilter::KernelFactor(pnKernel, nKsize),0, ipFilter::VEC) {m_pmTemp=NULL;}
ipFilterSep::~ipFilterSep() {if (m_pmTemp) delete m_pmTemp;}

bool ipFilterSep::process (Mat* pmSrc, Mat* pmDst)
{
	if (pmDst && !ipBaseCore::equalTypeSize(pmSrc, pmDst))
	{
		pmDst->release();
		pmDst->create (*pmSrc, false);
	}
	Mat* pmTemp=NULL;
	if (pmDst==NULL)
	{
		if (!m_pmTemp)
			m_pmTemp=new Mat(*pmSrc);
		else if (!m_pmTemp->isEqualMat(*pmSrc))
		{
			delete m_pmTemp;
			m_pmTemp=new Mat(*pmSrc);
		}
		else
		{
			for (int i=0; i<pmSrc->rows(); i++)
				memcpy(m_pmTemp->data.ptr[i], pmSrc->data.ptr[i], pmSrc->cols()*pmSrc->step());
		}
	}
	pmTemp=m_pmTemp;

	bool fProcess = false;

	if (m_pnKernel)
	{
		switch (pmSrc->type())
		{
		case MAT_Tuchar:
			{
				uchar** ppbIn;
				uchar** ppbOut;

				int i, j, nKSize2 = m_nKSize / 2, nRows = pmSrc->rows(), nCols = pmSrc->cols();
				int ii, jj, k;
				if (pmDst)
				{
					ppbIn = pmSrc->data.ptr;
					ppbOut = pmTemp->data.ptr;
				}
				else
				{
					ppbIn = pmSrc->data.ptr;
					ppbOut = pmTemp->data.ptr;
				}
				for (i = 0; i < nRows; i ++)
				{
					for (j = 0; j < nCols; j ++)
					{
						int nSum = 0;
						for (jj = MAX(0,j-nKSize2), k = jj-(j-nKSize2); jj <= MIN(j + nKSize2, nCols-1); jj++, k++)
							nSum += ppbIn[i][jj] * m_pnKernel[k];
						ppbOut[i][j] = nSum / m_nKFactor;
					}
				}

				if (pmDst)
				{
					ppbIn = pmTemp->data.ptr;
					ppbOut = pmDst->data.ptr;
				}
				else
				{
					ppbIn = pmTemp->data.ptr;
					ppbOut = pmSrc->data.ptr;
				}

				for (i = 0; i < nRows; i ++)
				{
					for (j = 0; j < nCols; j ++)
					{
						int nSum = 0;
						for (ii = MAX(0,i-nKSize2), k = ii-(i-nKSize2); ii <= MIN(i + nKSize2, nCols-1); ii++, k++)
							nSum += ppbIn[ii][j] * m_pnKernel[k];
						ppbOut[i][j] = nSum / m_nKFactor;
					}
				}
			}
			break;
		case MAT_Tfloat:
			{
				float** pprIn;
				float** pprOut;

				int i, j, nKSize2 = m_nKSize / 2, nRows = pmSrc->rows(), nCols = pmSrc->cols();
				int ii, jj, k;

				if (pmDst)
				{
					pprIn = pmSrc->data.fl;
					pprOut = pmTemp->data.fl;
				}
				else
				{
					pprIn = pmSrc->data.fl;
					pprOut = pmTemp->data.fl;
				}
				for (i = 0; i < nRows; i ++)
				{
					for (j = 0; j < nCols; j ++)
					{
						float rSum = 0;
						for (jj = MAX(0,j-nKSize2), k = jj-(j-nKSize2); jj <= MIN(j + nKSize2, nCols-1); jj++, k++)
							rSum += (float)(pprIn[i][jj] * m_pnKernel[k]);
						pprOut[i][j] = rSum / m_nKFactor;
					}
				}

				if (pmDst)
				{
					pprIn = pmTemp->data.fl;
					pprOut = pmDst->data.fl;
				}
				else
				{
					pprIn = pmTemp->data.fl;
					pprOut = pmSrc->data.fl;
				}

				for (i = 0; i < nRows; i ++)
				{
					for (j = 0; j < nCols; j ++)
					{
						float rSum = 0;
						for (ii = MAX(0,i-nKSize2), k = ii-(i-nKSize2); ii <= MIN(i + nKSize2, nCols-1); ii++, k++)
							rSum += (float)(pprIn[ii][j] * m_pnKernel[k]);
						pprOut[i][j] = rSum / m_nKFactor;
					}
				}
			}
			break;
		default:
			fProcess = false;
		}
	}
	else if (m_prKernel)
	{
		switch (pmSrc->type())
		{
		case MAT_Tuchar:
			{
				uchar** ppbIn;
				uchar** ppbOut;

				int i, j, nKSize2 = m_nKSize / 2, nRows = pmSrc->rows(), nCols = pmSrc->cols();
				int ii, jj, k;

				if (pmDst)
				{
					ppbIn = pmSrc->data.ptr;
					ppbOut = pmTemp->data.ptr;
				}
				else
				{
					ppbIn = pmSrc->data.ptr;
					ppbOut = pmTemp->data.ptr;
				}
				for (i = 0; i < nRows; i ++)
				{
					for (j = 0; j < nCols; j ++)
					{
						float rSum = 0;
						for (jj = MAX(0,j-nKSize2), k = jj-(j-nKSize2); jj <= MIN(j + nKSize2, nCols-1); jj++, k++)
							rSum += ppbIn[i][jj] * m_prKernel[k];
						ppbOut[i][j] = (uchar)(rSum / m_rKFactor);
					}
				}

				if (pmDst)
				{
					ppbIn = pmTemp->data.ptr;
					ppbOut = pmDst->data.ptr;
				}
				else
				{
					ppbIn = pmTemp->data.ptr;
					ppbOut = pmSrc->data.ptr;
				}

				for (i = 0; i < nRows; i ++)
				{
					for (j = 0; j < nCols; j ++)
					{
						float rSum = 0;
						for (ii = MAX(0,i-nKSize2), k = ii-(i-nKSize2); ii <= MIN(i + nKSize2, nCols-1); ii++, k++)
							rSum += ppbIn[ii][j] * m_prKernel[k];
						ppbOut[i][j] = (uchar)(rSum / m_rKFactor);
					}
				}
			}
			break;
		case MAT_Tfloat:
			{
				float** pprIn;
				float** pprOut;

				int i, j, nKSize2 = m_nKSize / 2, nRows = pmSrc->rows(), nCols = pmSrc->cols();
				int ii, jj, k;

				if (pmDst)
				{
					pprIn = pmSrc->data.fl;
					pprOut = pmTemp->data.fl;
				}
				else
				{
					pprIn = pmSrc->data.fl;
					pprOut = pmTemp->data.fl;
				}
				for (i = 0; i < nRows; i ++)
				{
					for (j = 0; j < nCols; j ++)
					{
						float rSum = 0;
						for (jj = MAX(0,j-nKSize2), k = jj-(j-nKSize2); jj <= MIN(j + nKSize2, nCols-1); jj++, k++)
							rSum += (float)(pprIn[i][jj] * m_prKernel[k]);
						pprOut[i][j] = rSum / m_rKFactor;
					}
				}

				if (pmDst)
				{
					pprIn = pmTemp->data.fl;
					pprOut = pmDst->data.fl;
				}
				else
				{
					pprIn = pmTemp->data.fl;
					pprOut = pmSrc->data.fl;
				}				
				for (i = 0; i < nRows; i ++)
				{
					for (j = 0; j < nCols; j ++)
					{
						float rSum = 0;
						for (ii = MAX(0,i-nKSize2), k = ii-(i-nKSize2); ii <= MIN(i + nKSize2, nRows-1); ii++, k++)
							rSum += (float)(pprIn[ii][j] * m_prKernel[k]);
						pprOut[i][j] = rSum / m_rKFactor;
					}
				}
			}
			break;
		default:
			fProcess = false;
		}
	}
	return true;
}
*/

class filter
{
public:
//	static bool Process2D (int* pnKernel, int nKsize, int nKFactor, int nKOffset, Mat* pmatOrg, Mat* pmatDest);
	static bool Conv2Sep(const int* pnKernel, int nKSize, int nKFactor, Mat* pmIn, Mat* pmDest);
	static bool Conv2Sep(const float* prKernel, int nKSize, float rKFactor, Mat* pmIn, Mat* pmDest);
};

//////////////////////////////////////////////////////////////////////////


// bool filter::Process2D (int* pnKernel, int nKsize, int nKFactor, 
// 						int nKOffset, Mat* pmatOrg, Mat* pmatDest)
// {
// 	assert (pmatOrg->type() == MAT_Tuchar);
// 	assert (pmatDest->type() == MAT_Tshort);
// 
// 
// 	int k2 = nKsize/2;
// 	int kmax= nKsize-k2;
// 	int b,i;
// 
// 	int xmin,xmax,ymin,ymax;
// 	xmin = ymin = 0;
// 	xmax = pmatOrg->cols();		// width
// 	ymax = pmatOrg->rows();		// height			
// 
// 	int iCount;
// 	uchar**	ppbOrg;
// 	short**	ppbTemp;
// 	short**	ppbDest;
// 
// 	Mat matTemp(pmatOrg->rows(), pmatOrg->cols(), MAT_Tshort);
// 	ppbOrg = pmatOrg->data.ptr;
// 	ppbTemp = matTemp.data.s;
// 	ppbDest = pmatDest->data.s;
// 
// 	for(int y = ymin ; y < ymax ; y++)
// 	{
// 		for(int x = xmin ; x < xmax ; x++)
// 		{
// 			if (y-k2 > 0 && (y+kmax-1) < ymax && x-k2 > 0 && (x+kmax-1) < xmax)
// 			{
// 				b = 0;
// 				iCount = 0;
// 				for(int j = -k2 ; j < kmax ; j++)
// 				{
// 					for(int k = -k2 ; k < kmax ; k++)
// 					{
// 						i = pnKernel[iCount];
// 						b += ppbOrg[y+j][x+k] * i;
// 						iCount++;
// 					}
// 				}
// 				ppbTemp[y][x] = (uchar)MIN(255, MAX(0,(int)(b/nKFactor + nKOffset)));
// 			}
// 			else
// 				ppbTemp[y][x] = ppbOrg[y][x];
// 		}
// 	}
// 
// 	for (int iRow = 0 ; iRow < matTemp.rows() ; iRow ++)
// 		for (int iCol = 0 ; iCol < matTemp.cols() ; iCol ++)
// 			ppbDest[iRow][iCol] = ppbTemp[iRow][iCol];
// 
// 	return true;
// }

bool filter::Conv2Sep(const int* pnKernel, int nKSize, int nKFactor, Mat* pmIn, Mat* pmDest)
{
	if ((pmIn->rows() != pmDest->rows()) ||
		(pmIn->cols() != pmDest->cols()))
		return false;
	if (pmIn->type() != pmDest->type())
		return false;

	Mat* pmTemp = new Mat (*pmIn);
	switch (pmIn->type())
	{
	case MAT_Tuchar:
		{
			pmIn->convertTo(*pmDest, MAT_Tuchar, CT_Cast);

			uchar** ppbIn;
			uchar** ppbOut;

			int i, j, nKSize2 = nKSize / 2, nRows = pmIn->rows(), nCols = pmIn->cols();
			int ii, jj, k;

			ppbIn = pmDest->data.ptr;
			ppbOut = pmTemp->data.ptr;
			for (i = 0; i < nRows; i ++)
			{
				uchar* pbIn=ppbIn[i];
				uchar* pbOut=ppbOut[i];
				for (j=0; j<nKSize2; j++)
				{
					int nsum=0;
					for (jj=nKSize-j-1; jj<nKSize; jj++)
						nsum += pbIn[nKSize-jj-1] * pnKernel[jj];
					pbOut[j] = (uchar)(nsum/nKFactor);
				}
				for (j = nKSize2; j < nCols-nKSize2; j ++)
				{
					int nsum = 0;
					for (jj = j-nKSize2, k=0; jj<= j+nKSize2; jj++, k++)
						nsum += pbIn[jj] * pnKernel[k];
					pbOut[j] = (uchar)(nsum/nKFactor);
				}
				for (j=nCols-nKSize2; j<nCols; j++)
				{
					int nsum=0;
					for (jj=j; jj<nCols; jj++)
						nsum += pbIn[jj] * pnKernel[jj-j];
					pbOut[j] = (uchar)(nsum/nKFactor);
				}
			}

			ppbIn = pmTemp->data.ptr;
			ppbOut = pmDest->data.ptr;

			for (i = 0; i < nRows; i ++)
			{
				for (j = 0; j < nCols; j ++)
				{
					int nSum = 0;
					for (ii = MAX(0,i-nKSize2), k = ii-(i-nKSize2); ii <= MIN(i + nKSize2, nRows-1); ii++, k++)
						nSum += ppbIn[ii][j] * pnKernel[k];
					ppbOut[i][j] = (uchar)(nSum / nKFactor);
				}
			}
		}
		break;
	case MAT_Tfloat:
		{
			pmIn->convertTo(*pmDest, MAT_Tfloat, CT_Cast);

			float** pprIn;
			float** pprOut;

			int i, j, nKSize2 = nKSize / 2, nRows = pmIn->rows(), nCols = pmIn->cols();
			int ii, jj, k;

			pprIn = pmDest->data.fl;
			pprOut = pmTemp->data.fl;
			for (i = 0; i < nRows; i ++)
			{
				for (j = 0; j < nCols; j ++)
				{
					float rSum = 0;
					for (jj = MAX(0,j-nKSize2), k = jj-(j-nKSize2); jj <= MIN(j + nKSize2, nCols-1); jj++, k++)
						rSum += (float)(pprIn[i][jj] * pnKernel[k]);
					pprOut[i][j] = rSum / nKFactor;
				}
			}

			pprIn = pmTemp->data.fl;
			pprOut = pmDest->data.fl;

			for (i = 0; i < nRows; i ++)
			{
				for (j = 0; j < nCols; j ++)
				{
					float rSum = 0;
					for (ii = MAX(0,i-nKSize2), k = ii-(i-nKSize2); ii <= MIN(i + nKSize2, nRows-1); ii++, k++)
						rSum += (float)(pprIn[ii][j] * pnKernel[k]);
					pprOut[i][j] = rSum / nKFactor;
				}
			}
		}
		break;
        default:
            assert(false);
    }

	delete pmTemp;

	return true;
}

bool filter::Conv2Sep(const float* prKernel, int nKSize, float rKFactor, Mat* pmIn, Mat* pmDest)
{
	if ((pmIn->rows() != pmDest->rows()) ||
		(pmIn->cols() != pmDest->cols()))
		return false;
	if (pmIn->type() != pmDest->type())
		return false;

	Mat* pmTemp = new Mat (*pmIn);
	switch (pmIn->type())
	{
	case MAT_Tuchar:
		{
			pmIn->convertTo(*pmDest, MAT_Tuchar, CT_Cast);

			uchar** ppbIn;
			uchar** ppbOut;

			int i, j, nKSize2 = nKSize / 2, nRows = pmIn->rows(), nCols = pmIn->cols();
			int ii, jj, k;

			ppbIn = pmDest->data.ptr;
			ppbOut = pmTemp->data.ptr;
			for (i = 0; i < nRows; i ++)
			{
				for (j = 0; j < nCols; j ++)
				{
					float rSum = 0;
					for (jj = MAX(0,j-nKSize2), k = jj-(j-nKSize2); jj <= MIN(j + nKSize2, nCols-1); jj++, k++)
						rSum += ppbIn[i][jj] * prKernel[k];
					ppbOut[i][j] = (uchar)(rSum / rKFactor);
				}
			}

			ppbIn = pmTemp->data.ptr;
			ppbOut = pmDest->data.ptr;

			for (i = 0; i < nRows; i ++)
			{
				for (j = 0; j < nCols; j ++)
				{
					float rSum = 0;
					for (ii = MAX(0,i-nKSize2), k = ii-(i-nKSize2); ii <= MIN(i + nKSize2, nCols-1); ii++, k++)
						rSum += ppbIn[ii][j] * prKernel[k];
					ppbOut[i][j] = (uchar)(rSum / rKFactor);
				}
			}
		}
		break;
	case MAT_Tfloat:
		{
			pmIn->convertTo(*pmDest, MAT_Tfloat, CT_Cast);

			float** pprIn;
			float** pprOut;

			int i, j, nKSize2 = nKSize / 2, nRows = pmIn->rows(), nCols = pmIn->cols();
			int ii, jj, k;

			pprIn = pmDest->data.fl;
			pprOut = pmTemp->data.fl;
			for (i = 0; i < nRows; i ++)
			{
				for (j = 0; j < nCols; j ++)
				{
					float rSum = 0;
					for (jj = MAX(0,j-nKSize2), k = jj-(j-nKSize2); jj <= MIN(j + nKSize2, nCols-1); jj++, k++)
						rSum += (float)(pprIn[i][jj] * prKernel[k]);
					pprOut[i][j] = rSum / rKFactor;
				}
			}

			pprIn = pmTemp->data.fl;
			pprOut = pmDest->data.fl;

			for (i = 0; i < nRows; i ++)
			{
				for (j = 0; j < nCols; j ++)
				{
					float rSum = 0;
					for (ii = MAX(0,i-nKSize2), k = ii-(i-nKSize2); ii <= MIN(i + nKSize2, nRows-1); ii++, k++)
						rSum += (float)(pprIn[ii][j] * prKernel[k]);
					pprOut[i][j] = rSum / rKFactor;
				}
			}
		}
		break;
        default:
            assert(false);
    }

	delete pmTemp;

	return true;
}

namespace ip
{

int		kernelFactor(const int* pnKernel, int nLen)
{
	return ipFilter::KernelFactor(pnKernel, nLen);
}
float	kernelFactor(const float* prKernel, int nLen)
{
	return ipFilter::KernelFactor(prKernel, nLen);
}
double	kernelFactor(const double* prKernel, int nLen)
{
	return ipFilter::KernelFactor(prKernel, nLen);
}
void convolve(Vec& src, const int* pnKernel, int nKSize, int nKFactor, int nKOffset)
{
	Vec t=src;
	convolve(t, src, pnKernel, nKSize, nKFactor, nKOffset);
}
void convolve(const Vec& src, Vec& dst, const int* pnKernel, int nKSize, int nKFactor, int nKOffset)
{
	ipFilter ipfilter(pnKernel, nKSize, nKFactor, nKOffset, ip::VEC);
	ipfilter.process((Vec*)&src, &dst);
}

void convolve2(Mat& src, const int* pnKernel, int nKSize, int nKFactor, int nKOffset, ip::filterintype type)
{
	Mat t=src;
	convolve2(t, src, pnKernel, nKSize, nKFactor, nKOffset, type);
}
void convolve2(Mat& src, const float* pnKernel, int nKSize, float nKFactor, int nKOffset, ip::filterintype type)
{
	Mat t=src;
	convolve2(t, src, pnKernel, nKSize, nKFactor, nKOffset, type);
}
void convolve2(Mat& src, const double* pnKernel, int nKSize, double nKFactor, int nKOffset, ip::filterintype type)
{
	Mat t=src;
	convolve2(t, src, pnKernel, nKSize, nKFactor, nKOffset, type);
}
void convolve2(const Mat& src, Mat& dst, const int* pnKernel, int nKSize, int nKFactor, int nKOffset, ip::filterintype type)
{
	ipFilter ipfilter(pnKernel, nKSize, nKFactor, nKOffset, type);
	ipfilter.process((Mat*)&src, &dst);
}
void convolve2(const Mat& src, Mat& dst, const float* pnKernel, int nKSize, float nKFactor, int nKOffset, ip::filterintype type)
{
	ipFilter ipfilter(pnKernel, nKSize, nKFactor, nKOffset, type);
	ipfilter.process((Mat*)&src, &dst);
}
void convolve2(const Mat& src, Mat& dst, const double* pnKernel, int nKSize, double nKFactor, int nKOffset, ip::filterintype type)
{
	ipFilter ipfilter(pnKernel, nKSize, nKFactor, nKOffset, type);
	ipfilter.process((Mat*)&src, &dst);
}
void convolve2(Mat& src, const int* pnKernel, int nKSize, int nKFactor, int nKOffset, const Rect& r)
{
	Mat t = src;
	convolve2(t, src, pnKernel, nKSize, nKFactor, nKOffset, r);
}
void convolve2(const Mat& src, Mat& dst, const int* pnKernel, int nKSize, int nKFactor, int nKOffset, const Rect& r)
{
	ipFilter::Process2(pnKernel, nKSize, nKFactor, nKOffset, &r, (Mat*)&src, &dst);
}

void convolve2sep(Mat& src, const int* pnKernel, int nKSize, int nKFactor)
{
	Mat t=src;
	filter::Conv2Sep (pnKernel, nKSize, nKFactor, &t, &src);
}
void convolve2sep(Mat& src, const float* prKernel, int nKSize, float rKFactor)
{
	Mat t=src;
	filter::Conv2Sep (prKernel, nKSize, rKFactor, &t, &src);
}
void convolve2sep(const Mat& src, Mat& dst, const int* pnKernel, int nKSize, int nKFactor)
{
	if (!src.isEqualMat (dst))
		dst.create (src);
	filter::Conv2Sep (pnKernel, nKSize, nKFactor, (Mat*)&src, &dst);
}
void convolve2sep(const Mat& src, Mat& dst, const float* prKernel, int nKSize, float rKFactor)
{
	if (!src.isEqualMat (dst))
		dst.create (src);
	filter::Conv2Sep (prKernel, nKSize, rKFactor, (Mat*)&src, &dst);
}
/*static inline Point2i normalizeAnchor( Point2i anchor, Size ksize )
{
    if( anchor.x == -1 )
        anchor.x = ksize.width/2;
    if( anchor.y == -1 )
        anchor.y = ksize.height/2;
    assert( Rect(0, 0, ksize.width, ksize.height).PtInRect(anchor) );
    return anchor;
}

void sepFilter2D( const Mat& src, Mat& dst, int ddepth, Mat& kernelX, Mat& kernelY, Point2i anchor, double delta, int borderType )
{
    if( ddepth < 0 )
 		ddepth = src.type();

	dst.create( src.size(), (TYPE)CVLIB_MAKETYPE(ddepth, src.channels()) );

	Ptr<FilterEngine> f = createSeparableLinearFilter(src.type1(),
		dst.type1(), kernelX, kernelY, anchor, delta, borderType & ~BORDER_ISOLATED );
    f->apply(src, dst, Rect(0,0,-1,-1), Point(), (borderType & BORDER_ISOLATED) != 0 );
}
void filter2D(const Mat& src, Mat& dst, int ddepth, Mat& kernel, Point2i anchor, double delta, int borderType )
{
    if( ddepth < 0 )
		ddepth = src.type();

#if CVLIB_SSE2
    int dft_filter_size = ((src.type() == MAT_Tuchar && (ddepth == MAT_Tuchar || ddepth == MAT_Tshort)) ||
        (src.type() == MAT_Tfloat && ddepth == MAT_Tfloat)) && checkHardwareSupport(CVLIB_CPU_SSE3)? 130 : 50;
#else
    int dft_filter_size = 50;
#endif
	
	dst.create( src.size(), (TYPE)CVLIB_MAKETYPE(ddepth, src.channels()) );
	normalizeAnchor(anchor, kernel.size());
    if( kernel.rows()*kernel.cols() >= dft_filter_size )
    {
        Mat temp;
        if( src.data.ptr != dst.data.ptr )
            temp = dst;
        else
            temp.create(dst.size(), dst.type1());
		crossCorr( src, kernel, temp, src.size(), CVLIB_MAKETYPE(ddepth, src.channels()),
                   anchor, delta, borderType );
        if( temp.data.ptr != dst.data.ptr )
			dst = temp;
		return;
    }

    Ptr<FilterEngine> f = createLinearFilter(src.type1(), dst.type1(), kernel,
                                             anchor, delta, borderType & ~BORDER_ISOLATED );
    f->apply(src, dst, Rect(0,0,-1,-1), Point(), (borderType & BORDER_ISOLATED) != 0 );
}*/

}}