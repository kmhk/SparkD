/*!
 * \file
 * \brief
 * \author
 */

#include "GaussianFilter.h"
#include "cvlibbase/Inc/cvlibbaseDef.h"
#include "cvlibbase/Inc/IPDebug.h"
#include <math.h>

namespace cvlib
{

class GaussianFilter
{
public:
	GaussianFilter();
	virtual ~GaussianFilter();

	int process(Mat* pmIn, Mat* pmOut);
	int ProcessPixel(const Mat* pmIn, Mat* pmOut, int row, int col);
	int ProcessXPixel(const Mat* pmIn, Mat* pmOut, int row, int col);
	int ProcessYPixel(const Mat* pmIn, Mat* pmOut, int row, int col);

	int ProcessCircle(const Mat* pImgSrc, Mat* pImgDst, int nCircleRadius, int nSigma,  int nSmoothRaduis, int row, int col);
	int ProcessXCircle(const Mat* pImgSrc, Mat* pImgDst, int nCircleRadius, int nSigma,  int nSmoothRaduis, int row, int col);
	int ProcessYCircle(const Mat* pImgSrc, Mat* pImgDst, int nCircleRadius, int nSigma,  int nSmoothRaduis, int row, int col);
	void setParam (float rSigma, float rRadius);
	static void GetKernel(float rSigma, float rRadius, Vecd& vKernel);
	static void GetKernel2D(float rSigma1, float rSigma2, float rRadius, Matd& mKernel);
protected:
	Mat		m_mTemp;
private:
	float m_rSigma;
	float m_rRadius;
	float* m_prKernel;

private:
	void GetXBlur(const Mat* pmIn, Mat* pmOut, float rSigma);
	void GetXBlurPixel(const Mat* pmIn, Mat* pmOut, float rSigma, int row, int col);
	void GetYBlur(const Mat* pmIn, Mat* pmOut, float rSigma);
	void GetYBlurPixel(const Mat* pmIn, Mat* pmOut, float rSigma, int row, int col);
};

class DoGFilter
{
public:
	DoGFilter();
	virtual ~DoGFilter();

public:
	float m_rSigmaInner;
	float m_rSigmaOuter;
	float m_rRadius;

public:
	int process(Mat* pmIn, Mat* pmOut);
};

class GaussianDiffFilter : public GaussianFilter
{
public:
	GaussianDiffFilter();
	virtual ~GaussianDiffFilter();

	void setParam (float rSigma, float rRadius);
	static void GetDiffKernel(float rSigma, float rRadius, Vecd& vKernel);
	static void GetKernel(float rSigma, float rRadius, Vecd& vKernel);

	int ProcessX(const Mat* pmIn, Mat* pmOut);
	int ProcessY(const Mat* pmIn, Mat* pmOut);
private:
	float m_rSigma;
	float m_rRadius;
	float* m_prKernel;
	float* m_prDiffKernel;

	void GetXProcess(const Mat* pmIn, Mat* pmOut, float* prKernel);
	void GetYProcess(const Mat* pmIn, Mat* pmOut, float* prKernel);
};

#ifndef DEFAULT_GAUSSIAN_RADIUS
	#define DEFAULT_GAUSSIAN_RADIUS		3
#endif

#ifndef DEFAULT_GAUSSIAN_SIGMA1
	#define DEFAULT_GAUSSIAN_SIGMA1		1
#endif

#ifndef DEFAULT_GAUSSIAN_SIGMA2
	#define DEFAULT_GAUSSIAN_SIGMA2		2
#endif

#ifndef COLORNUM_8BIT
	#define COLORNUM_8BIT	256
#endif

#ifndef K
	#define K(i, sigma)		exp( - (i*i) / ( 2 * sigma * sigma ) )
#endif

GaussianFilter::GaussianFilter()
{
	m_prKernel = NULL;
	setParam(DEFAULT_GAUSSIAN_SIGMA1, DEFAULT_GAUSSIAN_RADIUS);
}

GaussianFilter::~GaussianFilter()
{
	if (m_prKernel)
		delete []m_prKernel;
}

typedef void (*GetYFunc)( uchar** const _src, uchar** _dst, int cn, const Size& size, float* prkernel, int nR);
template<typename _Tp> static void getYBlur_( uchar** const _src, uchar** _dst, int cn, const Size& size, float* prkernel, int nR)
{
	_Tp** const src = (_Tp** const)_src;
	_Tp** dst = (_Tp**)_dst;

	int nRows=size.height;
	int nCols=size.width;
	int i, j, k;
	if (cn == 1)
	{
		for ( i = 0; i < nRows; i ++)
		{
			_Tp* const src_i=src[i];
			_Tp* dst_i=dst[i];
			for (j=0; j<nR; j++)
			{
				float sum = 0.0;
				k = -nR;
				while ( k <= nR )
				{
					if (j + k < 0)
						sum += (float)( prkernel[k + nR] * src_i[-(j+k)]);
					else
						sum += (float)( prkernel[k + nR] * src_i[j + k]);
					k ++;
				}
				dst_i[j] = (_Tp)sum;
			}
			for (j=nR; j<nCols-nR; j++)
			{
				float sum = prkernel[nR]*(float)src_i[j];
				k = 0;
				for (int m=j-nR, n=j+nR; k<nR; k++, m++, n--)
					sum += prkernel[k]*(float)(src_i[m]+src_i[n]);
				dst_i[j] = (_Tp)sum;
			}
			for (j=nCols-nR; j<nCols; j++)
			{
				float sum = 0.0;
				k = -nR;
				while ( k <= nR )
				{
					if (j + k >= nCols)
						sum += (float)( prkernel[k + nR] * src_i[(nCols-1)*2-(j+k)]);
					else
						sum += (float)( prkernel[k + nR] * src_i[j + k]);
					k ++;
				}
				dst_i[j] = (_Tp)sum;
			}
		}
	}
	else
	{
		/*Matf m(nR*2+1, 512);
		float** pplut = m.data.fl;
		for (k=0; k<nR*2+1; k++) for (i=0; i<512; i++)
		{
			pplut[k][i] = (float)(i*prkernel[k]);
		}*/
		for ( i = 0; i < nRows; i ++)
		{
			_Tp* const src_i=src[i];
			_Tp* dst_i=dst[i];
			for (int icn = 0; icn < cn; icn ++)
			{
				for (j=0; j<nR; j++)
				{
					float sum = 0.0;
					k = -nR;
					while ( k <= nR )
					{
						if (j + k < 0)
							sum += (float)( prkernel[k + nR] * src_i[-(j+k)*cn+icn]);
						else
							sum += (float)( prkernel[k + nR] * src_i[(j + k)*cn+icn]);
						k ++;
					}
					dst_i[j*cn+icn] = (_Tp)sum;
				}
				for (j=nR; j<nCols-nR; j++)
				{
					float sum = prkernel[nR]*(float)src_i[j*cn+icn];
					k = 0;
					for (int m=j-nR, n=j+nR; k<nR; k++, m++, n--)
//						sum += pplut[k][(int)src_i[m*cn+icn]+(int)src_i[n*cn+icn]];
						sum += prkernel[k]*(float)(src_i[m*cn+icn]+src_i[n*cn+icn]);
					
					dst_i[j*cn+icn] = (_Tp)sum;
				}
				for (j=nCols-nR; j<nCols; j++)
				{
					float sum = 0.0;
					k = -nR;
					while ( k <= nR )
					{
						if (j + k >= nCols)
							sum += (float)( prkernel[k + nR] * src_i[((nCols-1)*2-(j+k))*cn+icn]);
						else
							sum += (float)( prkernel[k + nR] * src_i[(j + k)*cn+icn]);
						k ++;
					}
					dst_i[j*cn+icn] = (_Tp)sum;
				}
			}
		}
	}
}
void GaussianFilter::GetYBlur(const Mat* pmIn, Mat* pmOut, float /*rSigma*/)
{
	if (!pmIn || !pmOut || !pmIn->isValid())
		return;
	int nCols = pmIn->cols();
	int nR = (int)(m_rRadius + 0.5f);
	if (nR *2 > nCols)
		return;
	int cn = pmIn->channels();
	GetYFunc getFuncs[]= { getYBlur_<char>, getYBlur_<uchar>, getYBlur_<short>, getYBlur_<int>, getYBlur_<float>, getYBlur_<double> };
	getFuncs[pmIn->type()](pmIn->data.ptr, pmOut->data.ptr, cn, pmIn->size(), m_prKernel, nR);
}
typedef void (*GetXFunc)( uchar** const _src, uchar** _dst, int cn, const Size& size, float* prkernel, int nR);
template<typename _Tp> static void getXBlur_( uchar** const _src, uchar** _dst, int cn, const Size& size, float* prkernel, int side)
{
	_Tp** const src = (_Tp** const)_src;
	_Tp** dst = (_Tp**)_dst;

	int rows=size.height;
	int cols=size.width;
	int i, k;
	_Tp* srcdata=new _Tp[rows];
	if (cn == 1)
	{
		for ( i = 0; i < cols; i ++)
		{
			for (k = 0; k < rows; k ++)
				srcdata[k] = src[k][i];
			for (k = 0; k < side; k ++)
			{
				float sum=0.0f;
				for (int j=0, m=k-side; j<=side*2; j++, m++)
				{
					if (m<0)
						sum += (float)(prkernel[j] * srcdata[-m]);
					else
						sum += (float)(prkernel[j] * srcdata[m]);
				}
				dst[k][i] = (_Tp)sum;
			}
			for (k = rows-side; k<rows; k++)
			{
				float sum=0.0f;
				for (int j=0, m=k-side; j<=side*2; j++, m++)
				{
					if (m>=rows)
						sum += (float)(prkernel[j] * srcdata[rows-1-(m-rows)]);
					else
						sum += (float)(prkernel[j] * srcdata[m]);
				}
				dst[k][i] = (_Tp)sum;
			}
			for (k=side; k<rows-side; k++)
			{
				float sum=prkernel[side]*(float)srcdata[k];
				for (int j=0, m=k-side, n=k+side; j<side; j++, m++, n--)
					sum += prkernel[j] * (float)(srcdata[m]+srcdata[n]);
				dst[k][i] = (_Tp)sum;
			}
		}
	}
	else
	{
		for ( i = 0; i < cols*cn; i ++)
		{
			for (k = 0; k < rows; k ++)
				srcdata[k] = src[k][i];
			for (k = 0; k < side; k ++)
			{
				float sum=0.0f;
				for (int j=0, m=k-side; j<=side*2; j++, m++)
				{
					if (m<0)
						sum += (float)(prkernel[j] * srcdata[-m]);
					else
						sum += (float)(prkernel[j] * srcdata[m]);
				}
				dst[k][i] = (_Tp)sum;
			}
			for (k = rows-side; k<rows; k++)
			{
				float sum=0.0f;
				for (int j=0, m=k-side; j<=side*2; j++, m++)
				{
					if (m>=rows)
						sum += (float)(prkernel[j] * srcdata[rows-1-(m-rows)]);
					else
						sum += (float)(prkernel[j] * srcdata[m]);
				}
				dst[k][i] = (_Tp)sum;
			}
			for (k=side; k<rows-side; k++)
			{
				float sum=prkernel[side]*(float)srcdata[k];
				for (int j=0, m=k-side, n=k+side; j<side; j++, m++, n--)
					sum += prkernel[j] * (float)(srcdata[m]+srcdata[n]);
				dst[k][i] = (_Tp)sum;
			}
		}
	}
	delete []srcdata;
}
void GaussianFilter::GetXBlur(const Mat* pmIn, Mat* pmOut, float /*rSigma*/)
{
	if (!pmIn || !pmOut || !pmIn->isValid())
		return;
	int nCols = pmIn->cols();
	int nR = (int)(m_rRadius + 0.5f);
	if (nR *2 > nCols)
		return;
	int cn = pmIn->channels();
	GetXFunc getFuncs[]= { getXBlur_<char>, getXBlur_<uchar>, getXBlur_<short>, getXBlur_<int>, getXBlur_<float>, getXBlur_<double> };
	getFuncs[pmIn->type()](pmIn->data.ptr, pmOut->data.ptr, cn, pmIn->size(), m_prKernel, nR);
}

int GaussianFilter::process(Mat* pmIn, Mat* pmOut)
{
	if (!pmIn || !pmOut)
		return -1;
	if (!pmIn->isValid())
		return -1;
	if (!pmIn->isEqualMat(m_mTemp))
	{
		m_mTemp.release();
		m_mTemp.create (*pmIn);
	}
	if (fabs (m_rSigma) > 0.00001)
	{
		GetYBlur(pmIn, &m_mTemp, m_rSigma);
		GetXBlur(&m_mTemp, pmOut, m_rSigma);
	}
	else
	{
		if (pmOut != pmIn)
			*pmOut = *pmIn;
	}
	return 0;
}

int GaussianFilter::ProcessPixel(const Mat* pmIn, Mat* pmOut, int row, int col)
{
	if (!pmIn || !pmOut)
		return -1;
	if (!pmIn->isValid())
		return -1;
	
	if (!pmIn->isEqualMat(m_mTemp))
	{
		m_mTemp.release();
		m_mTemp.create (*pmIn);
	}
	GetYBlurPixel(pmIn, &m_mTemp, m_rSigma, row, col);
	GetXBlurPixel(&m_mTemp, pmOut, m_rSigma, row, col);
	return 0;
}

int GaussianFilter::ProcessXPixel(const Mat* pmIn, Mat* pmOut, int row, int col)
{
	if (!pmIn || !pmOut)
		return -1;
	if (!pmIn->isValid())
		return -1;
	GetXBlurPixel(pmIn, pmOut, m_rSigma, row, col);
	return 0;
}
int GaussianFilter::ProcessYPixel(const Mat* pmIn, Mat* pmOut, int row, int col)
{
	if (!pmIn || !pmOut)
		return -1;
	if (!pmIn->isValid())
		return -1;
	
	GetYBlurPixel(pmIn, pmOut, m_rSigma, row, col);
	
	return 0;
}

void GaussianFilter::setParam(float rSigma, float rRadius)
{
	m_rSigma = rSigma;
	m_rRadius = rRadius;
	if (m_prKernel)
		delete []m_prKernel;
	int nSideR = (int)(rRadius + 0.5f);
	int nCntK = nSideR * 2 + 1;
	m_prKernel = new float[nCntK];
	float sum = 0.0;
	int idxK;
	for (idxK = 0; idxK < nCntK; idxK ++)
	{
		float rBoy = (float)(idxK - nSideR) * (idxK - nSideR);
		float rMom = 2 * rSigma * rSigma;
		float rShoulder = - rBoy / rMom;
		m_prKernel[idxK] = expf(rShoulder);
		sum += m_prKernel[idxK];
	}
	for (idxK = 0; idxK < nCntK; idxK ++)
		m_prKernel[idxK] /= sum;
}

void GaussianFilter::GetKernel(float rSigma, float rRadius, Vecd& vKernel)
{
	vKernel.release();
	int nSideR = (int)(rSigma * rRadius + 0.5f);
	int nCntK = nSideR * 2 + 1;
	vKernel.create (nCntK);
	double* prKernel = vKernel.data.db;
	float sum = 0.0;
	int idxK;
	for (idxK = 0; idxK < nCntK; idxK ++)
	{
		float rBoy = (float)((idxK - nSideR) * (idxK - nSideR));
		float rMom = 2 * rSigma * rSigma;
		float rShoulder = - rBoy / rMom;
		prKernel[idxK] = exp(rShoulder);
		sum += (float)prKernel[idxK];
	}
	for (idxK = 0; idxK < nCntK; idxK ++)
		prKernel[idxK] /= sum;
}

void GaussianFilter::GetKernel2D(float rSigma1, float rSigma2, float rRadius, Matd& mKernel)
{
	Vecd vK1, vK2;
	GetKernel(rSigma1, rRadius, vK1);
	GetKernel(rSigma2, rRadius, vK2);
	mKernel.release();
	mKernel.create(vK2.length(), vK1.length());
	for (int i=0; i<mKernel.rows(); i++)
	{
		for (int j=0; j<mKernel.cols(); j++)
		{
			mKernel[i][j] = vK1[j]*vK2[i];
		}
	}
}
typedef void (*GetYBlurPixelFunc)( uchar** const _src, uchar** _dst, int nCols, float m_rRadius, int channels, float* prkernel, int row, int col);
template<typename _Tp> static void getYBlurPixel_( uchar** const _src, uchar** _dst, int nCols, float m_rRadius, int cn, float* prkernel, int row, int col)
{
	_Tp** const ppbIn = (_Tp** const)_src;
	_Tp** ppbOut = (_Tp**)_dst;

	int i, j, k;

	int nR = (int)(m_rRadius + 0.5f);
	i = row;
	j = col;
	for (int icn=0; icn<cn; icn++)
	{
		double sum = 0.0;
		k = -nR;
		while ( k <= nR )
		{
			if (j + k < 0)
				sum += (double)( prkernel[k + nR] * ppbIn[i][(-(j+k))*cn+icn]);
			else if (j + k >= nCols)
				sum += (double)( prkernel[k + nR] * ppbIn[i][((nCols-1)*2-(j+k))*cn+icn]);
			else
				sum += (double)( prkernel[k + nR] * ppbIn[i][(j+k)*cn+icn]);
			k ++;
		}
		if (ppbOut[i][j*cn+icn] < (_Tp)sum)
		{
			ppbOut[i][j*cn+icn] = (_Tp)sum;
		}

	}
}

void GaussianFilter::GetYBlurPixel(const Mat* pmIn, Mat* pmOut, float /*rSigma*/, int row, int col)
{
	if (!pmIn || !pmOut || !pmIn->isValid())
		return;
	
	int cn = pmIn->channels();
	uchar** const ppsrc = pmIn->data.ptr;
	uchar** ppdst = pmOut->data.ptr;
	int nCols = pmIn->cols();
	
	if (pmIn->type() == MAT_Tuchar)
	{
		getYBlurPixel_<uchar> (ppsrc, ppdst, nCols, m_rRadius, cn, m_prKernel, row, col);
	}
	if (pmIn->type() == MAT_Tdouble)
	{
		getYBlurPixel_<double> (ppsrc, ppdst, nCols, m_rRadius, cn, m_prKernel, row, col);
	}
	if (pmIn->type() == MAT_Tfloat)
	{
		getYBlurPixel_<float> (ppsrc, ppdst, nCols, m_rRadius, cn, m_prKernel, row, col);
	}
}
typedef void (*GetXBlurPixelFunc)( uchar** const _src, uchar** _dst, int nRows, float m_rRadius, int channels, float* prkernel, int row, int col);
template<typename _Tp> static void getXBlurPixel_( uchar** const _src, uchar** _dst, int nRows, float m_rRadius, int cn, float* prkernel, int row, int col)
{
	_Tp** const pprIn = (_Tp** const)_src;
	_Tp** pprOut = (_Tp**)_dst;

	int i, j, k;
	int nR = (int)(m_rRadius + 0.5f);
	i = row;
	j = col;

	for (int icn=0; icn<cn; icn++)
	{
		double sum = 0.0;
		k = -nR;
		while ( k <= nR )
		{
			if (i + k < 0)
				sum += (double)( prkernel[k + nR] * pprIn[-(i+k)][j*cn+icn]);
			else if (i + k >= nRows)
				sum += (double)( prkernel[k + nR] * pprIn[(nRows-1)*2 - (i+k)][j*cn+icn]);
			else
				sum += (double)( prkernel[k + nR] * pprIn[i + k][j*cn+icn]);
			k ++;
		}
		if (pprOut[i][j*cn+icn] < (_Tp)sum)
		{
			pprOut[i][j*cn+icn] = (_Tp)sum;
		}
	}
}
void GaussianFilter::GetXBlurPixel(const Mat* pmIn, Mat* pmOut, float /*rSigma*/, int row, int col)
{
	if (!pmIn || !pmOut || !pmIn->isValid())
		return;
	
	int nRows = pmIn->rows();
	int cn = pmIn->channels();
	uchar** const ppsrc = pmIn->data.ptr;
	uchar** ppdst = pmOut->data.ptr;
	
	if (pmIn->type() == MAT_Tuchar)
	{
		getXBlurPixel_<uchar> (ppsrc, ppdst, nRows, m_rRadius, cn, m_prKernel, row, col);
	}
	if (pmIn->type() == MAT_Tdouble)
	{
		getXBlurPixel_<double> (ppsrc, ppdst, nRows, m_rRadius, cn, m_prKernel, row, col);
	}
	if (pmIn->type() == MAT_Tfloat)
	{
		getXBlurPixel_<float> (ppsrc, ppdst, nRows, m_rRadius, cn, m_prKernel, row, col);
	}
}

int GaussianFilter::ProcessCircle(const Mat* pImgSrc, Mat* pImgDst, int nCircleRadius, int nSigma,  int nSmoothRaduis, int row, int col)
{
	int nCurrentrow, nCurrentcol;
	for (int noffsetY = -nCircleRadius; noffsetY <= nCircleRadius; noffsetY++)
	{
		float dLimitX = sqrt((float)SQR(nCircleRadius) - SQR(noffsetY) );
		for (int noffsetX = - (int)dLimitX; noffsetX < (int)dLimitX; noffsetX++)
		{
			nCurrentrow = row + noffsetY;
			nCurrentcol = col + noffsetX;
			float dRadius = sqrt((float)SQR(noffsetY)+SQR(noffsetX));
			float dCurrentSmoothRaduis = (float)nSmoothRaduis * (nCircleRadius - dRadius) / nCircleRadius;
			float dCurrentSmoothSigma = (float)nSigma * (nCircleRadius - dRadius) / nCircleRadius +0.1f ;
			setParam(dCurrentSmoothSigma, dCurrentSmoothRaduis);
			ProcessPixel(pImgSrc, pImgDst,nCurrentrow, nCurrentcol);
		}
	}
	return 1;
}

int GaussianFilter::ProcessXCircle(const Mat* pImgSrc, Mat* pImgDst, int nCircleRadius, int nSigma,  int nSmoothRaduis, int row, int col)
{
	int nCurrentrow, nCurrentcol;
	for (int noffsetY = -nCircleRadius; noffsetY <= nCircleRadius; noffsetY++)
	{
		float dLimitX = sqrt((float)SQR(nCircleRadius) - SQR(noffsetY) );
		for (int noffsetX = - (int)dLimitX; noffsetX < (int)dLimitX; noffsetX++)
		{
			nCurrentrow = row + noffsetY;
			nCurrentcol = col + noffsetX;
			float dRadius = sqrt((float)SQR(noffsetY)+SQR(noffsetX));
			float dCurrentSmoothRaduis = (float)nSmoothRaduis * (nCircleRadius - dRadius) / nCircleRadius;
			setParam((float)nSigma, dCurrentSmoothRaduis);
			ProcessXPixel(pImgSrc, pImgDst,nCurrentrow, nCurrentcol);
		}
	}
	return 1;
}

int GaussianFilter::ProcessYCircle(const Mat* pImgSrc, Mat* pImgDst, int nCircleRadius, int nSigma,  int nSmoothRaduis, int row, int col)
{
	int nCurrentrow, nCurrentcol;
	for (int noffsetY = -nCircleRadius; noffsetY <= nCircleRadius; noffsetY++)
	{
		float dLimitX = sqrt((float)SQR(nCircleRadius) - SQR(noffsetY) );
		for (int noffsetX = - (int)dLimitX; noffsetX < (int)dLimitX; noffsetX++)
		{
			nCurrentrow = row + noffsetY;
			nCurrentcol = col + noffsetX;
			float dRadius = sqrt((float)SQR(noffsetY)+SQR(noffsetX));
			float dCurrentSmoothRaduis = (float)nSmoothRaduis * (nCircleRadius - dRadius) / nCircleRadius;
			float dCurrentSmoothSigma = (float)nSigma * (nCircleRadius - dRadius) / nCircleRadius +0.1f ;
			setParam(dCurrentSmoothSigma, dCurrentSmoothRaduis);
			ProcessYPixel(pImgSrc, pImgDst,nCurrentrow, nCurrentcol);
		}
	}
	return 1;
}

GaussianDiffFilter::GaussianDiffFilter()
{
	m_prKernel=NULL;
	m_prDiffKernel=NULL;
}

GaussianDiffFilter::~GaussianDiffFilter()
{
	if (m_prKernel)
		delete []m_prKernel;
	if (m_prDiffKernel)
		delete []m_prDiffKernel;
}
	
void GaussianDiffFilter::setParam (float rSigma, float rRadius)
{
	m_rSigma = rSigma;
	m_rRadius = rRadius;

	if (m_prKernel)
		delete []m_prKernel;
	if (m_prDiffKernel)
		delete []m_prDiffKernel;
	
	int nSideR = (int)(rSigma * rRadius + 0.5f);
	int nCntK = nSideR * 2 + 1;
	m_prKernel = new float[nCntK];
	m_prDiffKernel = new float[nCntK];
	float sum = 0.0;
	float rk=-1.0f/(float)sqrt(2*CVLIB_PI) / powf(rSigma, 3.0);
	int idxK;
	for (idxK = 0; idxK < nCntK; idxK ++)
	{
		float rBoy = (float)((idxK - nSideR) * (idxK - nSideR));
		float rMom = 2 * rSigma * rSigma;
		float rShoulder = - rBoy / rMom;
		m_prKernel[idxK] = exp(rShoulder);
		m_prDiffKernel[idxK] = rk * (idxK-nSideR) * m_prKernel[idxK];
		sum += m_prKernel[idxK];
	}
	for (idxK = 0; idxK < nCntK; idxK ++)
		m_prKernel[idxK] /= sum;
}

void GaussianDiffFilter::GetKernel(float rSigma, float rRadius, Vecd& vKernel)
{
	vKernel.release();
	int nSideR = (int)(rSigma * rRadius + 0.5f);
	int nCntK = nSideR * 2 + 1;
	vKernel.create (nCntK);
	double* prKernel = vKernel.data.db;
	float sum = 0.0;
	int idxK;
	for (idxK = 0; idxK < nCntK; idxK ++)
	{
		float rBoy = (float)((idxK - nSideR) * (idxK - nSideR));
		float rMom = 2 * rSigma * rSigma;
		float rShoulder = - rBoy / rMom;
		prKernel[idxK] = exp(rShoulder);
		sum += (float)prKernel[idxK];
	}
	for (idxK = 0; idxK < nCntK; idxK ++)
		prKernel[idxK] /= sum;
}
	
void GaussianDiffFilter::GetDiffKernel(float rSigma, float rRadius, Vecd& vKernel)
{
	vKernel.release();
	int nSideR = (int)(rSigma * rRadius + 0.5f);
	int nCntK = nSideR * 2 + 1;
	vKernel.create (nCntK);
	double* prKernel = vKernel.data.db;
	float rk=1.0f/(float)sqrt(2*CVLIB_PI) / powf(rSigma, 3.0);
	int idxK;
	for (idxK = 0; idxK < nCntK; idxK ++)
	{
		float rBoy = (float)((idxK - nSideR) * (idxK - nSideR));
		float rMom = 2 * rSigma * rSigma;
		float rShoulder = - rBoy / rMom;
		prKernel[idxK] = rk * (idxK-nSideR) * exp(rShoulder);
	}
}

int GaussianDiffFilter::ProcessX(const Mat* pmIn, Mat* pmOut)
{
	if (!pmIn || !pmOut)
		return -1;
	if (!pmIn->isValid())
		return -1;
	
	if (!pmIn->isEqualMat(m_mTemp))
	{
		m_mTemp.release();
		m_mTemp.create (*pmIn);
	}
	GetXProcess(pmIn, &m_mTemp, m_prDiffKernel);
	GetYProcess(&m_mTemp, pmOut, m_prKernel);
	return 0;
}

int GaussianDiffFilter::ProcessY(const Mat* pmIn, Mat* pmOut)
{
	if (!pmIn || !pmOut)
		return -1;
	if (!pmIn->isValid())
		return -1;
	
	if (!pmIn->isEqualMat(m_mTemp))
	{
		m_mTemp.release();
		m_mTemp.create (*pmIn);
	}
	GetYProcess(pmIn, &m_mTemp, m_prDiffKernel);
	GetXProcess(&m_mTemp, pmOut, m_prKernel);
	return 0;
}

void GaussianDiffFilter::GetXProcess(const Mat* pmIn, Mat* pmOut, float* prKernel)
{
	if (!pmIn || !pmOut || !pmIn->isValid())
		return;
	
	int nCols = pmIn->cols();
	int nRows = pmIn->rows();
	
	if (pmIn->type() == MAT_Tuchar)
	{
		unsigned char** ppbIn = pmIn->data.ptr;
		unsigned char** ppbOut = pmOut->data.ptr;

		int i, j, k;
		int nR = (int)(m_rRadius * m_rSigma + 0.5f);
		for ( i = 0; i < nRows; i ++)
		{
			for ( j = 0; j < nCols; j ++)
			{
				float sum = 0.0;
				k = -nR;
				while ( k <= nR )
				{
					if (j + k < 0)
						sum += ( prKernel[k + nR] * ppbIn[i][-(j+k)]);
					else if (j + k >= nCols)
						sum += ( prKernel[k + nR] * ppbIn[i][(nCols-1)*2-(j+k)]);
					else
						sum += ( prKernel[k + nR] * ppbIn[i][j + k]);
					k ++;
				}

				ppbOut[i][j] = (uchar)sum;
			}
		}
	}
	else if (pmIn->type() == MAT_Tfloat)
	{
		float** ppbIn = pmIn->data.fl;
		float** ppbOut = pmOut->data.fl;
		
		int i, j, k;
		int nR = (int)(m_rRadius * m_rSigma + 0.5f);
		for ( i = 0; i < nRows; i ++)
		{
			for ( j = 0; j < nCols; j ++)
			{
				float sum = 0.0;
				k = -nR;
				while ( k <= nR )
				{
					if (j + k < 0)
						sum += ( prKernel[k + nR] * ppbIn[i][-(j+k)]);
					else if (j + k >= nCols)
						sum += ( prKernel[k + nR] * ppbIn[i][(nCols-1)*2-(j+k)]);
					else
						sum += ( prKernel[k + nR] * ppbIn[i][j + k]);
					k ++;
				}
				
				ppbOut[i][j] = (float)sum;
			}
		}
	}
}

void GaussianDiffFilter::GetYProcess(const Mat* pmIn, Mat* pmOut, float* prKernel)
{
	if (!pmIn || !pmOut || !pmIn->isValid())
		return;
	
	int nCols = pmIn->cols();
	int nRows = pmIn->rows();
	
	if (pmIn->type() == MAT_Tuchar)
	{
		unsigned char** ppbIn = pmIn->data.ptr;
		unsigned char** ppbOut = pmOut->data.ptr;
		
		int i, j, k;
		int nR = (int)(m_rRadius * m_rSigma + 0.5f);
		for ( j = 0; j < nCols; j ++)
		{
			for ( i = 0; i < nRows; i ++)
			{
				float sum = 0.0;
				k = -nR;
				while ( k <= nR )
				{
					if (i + k < 0)
						sum += ( prKernel[k + nR] * ppbIn[-(i+k)][j]);
					else if (i + k >= nRows)
						sum += ( prKernel[k + nR] * ppbIn[(nRows-1)*2 - (i+k)][j]);
					else
						sum += ( prKernel[k + nR] * ppbIn[i + k][j]);
					k ++;
				}
				
				ppbOut[i][j] = (uchar)sum;
			}
		}
	}
	else if (pmIn->type() == MAT_Tfloat)
	{
		float** ppbIn = pmIn->data.fl;
		float** ppbOut = pmOut->data.fl;
		
		int i, j, k;
		int nR = (int)(m_rRadius * m_rSigma + 0.5f);
		for ( j = 0; j < nCols; j ++)
		{
			for ( i = 0; i < nRows; i ++)
			{
				float sum = 0.0;
				k = -nR;
				while ( k <= nR )
				{
					if (i + k < 0)
						sum += ( prKernel[k + nR] * ppbIn[-(i+k)][j]);
					else if (i + k >= nRows)
						sum += ( prKernel[k + nR] * ppbIn[(nRows-1)*2 - (i+k)][j]);
					else
						sum += ( prKernel[k + nR] * ppbIn[i + k][j]);
					k ++;
				}
				
				ppbOut[i][j] = (float)sum;
			}
		}
	}

}

DoGFilter::DoGFilter()
{
	m_rSigmaInner	= DEFAULT_GAUSSIAN_SIGMA1;
	m_rSigmaOuter	= DEFAULT_GAUSSIAN_SIGMA2;
	m_rRadius		= DEFAULT_GAUSSIAN_RADIUS;
}

DoGFilter::~DoGFilter()
{
}

int DoGFilter::process(Mat* pmIn, Mat* pmOut)
{
	if (!pmIn || !pmOut)
		return -1;
	if (!pmIn->isValid())
		return -1;

	GaussianFilter gaussianInner, gaussianOuter;
	gaussianInner.setParam(m_rSigmaInner, m_rRadius);
	gaussianOuter.setParam(m_rSigmaOuter, m_rRadius);

	Mat* pmOutInner = new Mat(*pmOut);
	Mat* pmOutOuter = new Mat(*pmOut);
	gaussianInner.process(pmIn, pmOutInner);
	gaussianOuter.process(pmIn, pmOutOuter);
	
	if (pmIn->type() == MAT_Tuchar)
	{
		unsigned char** ppbInner = pmOutInner->data.ptr;
		unsigned char** ppbOuter = pmOutOuter->data.ptr;
		unsigned char** ppbOut = pmOut->data.ptr;

		int nRows = pmOut->rows();
		int nCols = pmOut->cols();
		int i, j;
		for ( i = 0; i < nRows; i ++)
			for ( j = 0; j < nCols; j ++)
			{
				int nDiff = ppbInner[i][j] - ppbOuter[i][j];
				ppbOut[i][j] = (uchar)ABS(nDiff);
			}
	}
	else if (pmIn->type() == MAT_Tdouble)
	{
		double** pprInner = pmOutInner->data.db;
		double** pprOuter = pmOutOuter->data.db;
		double** pprOut = pmOut->data.db;
		int nRows = pmOut->rows();
		int nCols = pmOut->cols();
		int i, j;
		for ( i = 0; i < nRows; i ++)
			for ( j = 0; j < nCols; j ++)
			{
				pprOut[i][j] = pprInner[i][j] - pprOuter[i][j];
			}
	}
	else if (pmIn->type() == MAT_Tfloat)
	{
		float** pprInner = pmOutInner->data.fl;
		float** pprOuter = pmOutOuter->data.fl;
		float** pprOut = pmOut->data.fl;
		int nRows = pmOut->rows();
		int nCols = pmOut->cols();
		int i, j;
		for ( i = 0; i < nRows; i ++)
			for ( j = 0; j < nCols; j ++)
			{
				pprOut[i][j] = pprInner[i][j] - pprOuter[i][j];
			}
	}
	else 
		return -1;
	delete pmOutInner;
	delete pmOutOuter;
	return 0;
}

namespace ip
{

void enhanceEdgeX (const Mat& src, Mat& dst, const Veci& xdir, const Veci& ydir, bool fnormal)
{
	int* anfilter=xdir.data.i;
	int nfilterlen=xdir.length();
	int K=0;
	for (int i=0; i<nfilterlen; i++)
		K += anfilter[i];
	Mat src2(src.size(), MAT_Tuchar); src2.zero();
	// smoothing according x direction
	bool fsysmetric=true;
	for (int i=0; i<nfilterlen/2; i++)
		if (anfilter[i]!=anfilter[nfilterlen-i-1]) fsysmetric=false;
	if (fsysmetric)
	{
		int side=nfilterlen/2;
		for (int i=0; i<src.rows(); i++)
		{
			uchar* data=src2.data.ptr[i];
			uchar* const srcdata=src.data.ptr[i];
			for (int k=side; k<src.cols()-side; k++)
			{
				int sum=anfilter[side]*srcdata[k];
				for (int j=0, m=k-side, n=k+side; j<side; j++, m++, n--)
					sum += anfilter[j]*(srcdata[m]+srcdata[n]);
				data[k]= (uchar)(sum / K);
			}
		}
	}
	else
	{
		for (int i=0; i<src.rows(); i++)
		{
			uchar* data=src2.data.ptr[i];
			uchar* const srcdata=src.data.ptr[i];
			memset(data, 0, src.cols());
			for (int k=nfilterlen/2; k<src.cols()-nfilterlen/2; k++)
			{
				int sum=0;
				for (int j=0, m=k-nfilterlen/2; j<nfilterlen; j++, m++)
					sum += anfilter[j]*srcdata[m];
				data[k]= (uchar)(sum / K);
			}
		}
	}
	// differntiation according y direction

/*	int K=0;
	for (int i=0; i<nyfilterlen; i++)
		K += anyfilter[i];*/
	// smoothing according x direction

	int* anDiffFilter=ydir.data.i;
	int nside=ydir.length();
	bool fasysmetric=true;
	for (int i=0; i<nside/2; i++)
		if (anDiffFilter[i]!=-anDiffFilter[nside-i-1]) fasysmetric=false;
	if (fasysmetric)
	{
		uchar** data=src2.data.ptr;
		if (nside == 3 && anDiffFilter[0]==-1 && anDiffFilter[1]==0 && anDiffFilter[2]==1)
		{
			if (fnormal)
			{
				Mati temp(src.rows(), src.cols()); temp.zero();
				int nmin=0, nmax=0;
				for (int i=1; i<src.rows()-1; i++)
				{
					uchar* data_i_=data[i+1];
					uchar* data_i=data[i-1];
					int* temp_i=temp[i];
					for (int k = 0; k < src.cols(); k++)
					{
						int sum=data_i_[k]-data_i[k];
						temp_i[k]=sum;
						nmin = MIN(nmin, sum);
						nmax = MAX(nmax, sum);
					}
				}
				dst.create(src.rows(), src.cols(), src.type1());
				int denom=nmax==nmin ? 255 : nmax-nmin;
				for (int i=0; i<temp.rows(); i++) 
				{
					uchar* dst_i=dst.data.ptr[i];
					int* temp_i=temp[i];
					for (int k=0; k<temp.cols(); k++)
					{
						dst_i[k]=(uchar)((temp_i[k]-nmin)*255/denom);
					}
				}
			}
			else
			{
				dst.create(src.rows(), src.cols(), MAT_Tint); dst.zero();
				int** ppdst=dst.data.i;
				for (int i=1; i<src.rows()-1; i++)
				{
					int* dst_i=ppdst[i];
					uchar* data_i=data[i+1];
					uchar* data_i_=data[i-1];
					for (int k = 0; k < src.cols(); k++)
						dst_i[k]=data_i[k]-data_i_[k];
				}
			}
		}
		else
		{
			int nside2=nside/2;
			if (fnormal)
			{
				Mati temp(src.rows(), src.cols()); temp.zero();
				int nmin=0, nmax=0;
				for (int k = 0; k < src.cols(); k++)
				{
					for (int i=nside2; i<src.rows()-nside2; i++)
					{
						int sum=data[i][k];
						for (int j=0, m=i-nside2, n=i+nside2; j<nside2; j++, m++, n--)
							sum += anDiffFilter[j]*(data[m][k]-data[n][k]);
						temp[i][k]=sum;
						nmin = MIN(nmin, sum);
						nmax = MAX(nmax, sum);
					}
				}
				dst.create(src.rows(), src.cols(), src.type1());
				int denom=nmax==nmin ? 255 : nmax-nmin;
				for (int i=0; i<temp.rows(); i++) 
				{
					uchar* dst_i=dst.data.ptr[i];
					int* temp_i=temp[i];
					for (int k=0; k<temp.cols(); k++)
					{
						dst_i[k]=(uchar)((temp_i[k]-nmin)*255/denom);
					}
				}
			}
			else
			{
				dst.create(src.rows(), src.cols(), MAT_Tint); dst.zero();
				int** ppdst=dst.data.i;
				for (int k = 0; k < src.cols(); k++)
				{
					for (int i=nside2; i<src.rows()-nside2; i++)
					{
						int sum=data[i][k];
						for (int j=0, m=i-nside2, n=i+nside2; j<nside2; j++, m++, n--)
							sum += anDiffFilter[j]*(data[m][k]-data[n][k]);
						ppdst[i][k]=sum;
					}
				}
			}
		}
	}
	else
	{
		uchar** data=src2.data.ptr;
		int nside2=nside/2;
		if (fnormal)
		{
			Mati temp(src.rows(), src.cols()); temp.zero();
			int nmin=0, nmax=0;
			for (int k = 0; k < src.cols(); k++)
			{
				for (int i=nside2; i<src.rows()-nside2; i++)
				{
					int sum=0;
					for (int j=0, m=i-nside2; j<nside; j++, m++)
						sum += anDiffFilter[j]*data[m][k];
					temp[i][k]=sum;
					nmin = MIN(nmin, sum);
					nmax = MAX(nmax, sum);
				}
			}
			dst.create(src.rows(), src.cols(), src.type1());
			int denom=nmax==nmin ? 255 : nmax-nmin;
			for (int i=0; i<temp.rows(); i++) 
			{
				uchar* dst_i=dst.data.ptr[i];
				int* temp_i=temp[i];
				for (int k=0; k<temp.cols(); k++)
				{
					dst_i[k]=(uchar)((temp_i[k]-nmin)*255/denom);
				}
			}
		}
		else
		{
			dst.create(src.rows(), src.cols(), MAT_Tint); dst.zero();
			int** ppdst=dst.data.i;
			for (int k = 0; k < src.cols(); k++)
			{
				for (int i=nside2; i<src.rows()-nside2; i++)
				{
					int sum=0;
					for (int j=0, m=i-nside2; j<nside; j++, m++)
						sum += anDiffFilter[j]*data[m][k];
					ppdst[i][k]=sum;
				}
			}
		}
	}
}

void enhanceEdgeY(const Mat& src, Mat& dst, const Veci& xdir, const Veci& ydir, bool fnormal)
{
	int *anfilter = ydir.data.i;
	int nfilterlen = ydir.length();
	int K = 0;
	for (int i = 0; i < nfilterlen; i++)
		K += anfilter[i];
	Mat src2(src.size(), MAT_Tuchar); src2.zero();
	// smoothing according y direction
	bool fsysmetric = true;
	for (int i = 0; i < nfilterlen / 2; i++)
		if (anfilter[i] != anfilter[nfilterlen - i - 1]) fsysmetric = false;
	Vecb srcdata(src.rows());
	if (fsysmetric)
	{
		int side = nfilterlen / 2;
		for (int i = 0; i < src.cols(); i++)
		{
			for (int k = 0; k < src.rows(); k++)
				srcdata[k] = src.data.ptr[k][i];
			for (int k = side; k < src.rows() - side; k++)
			{
				int sum = anfilter[side] * srcdata[k];
				for (int j = 0, m = k - side, n = k + side; j < side; j++, m++, n--)
					sum += anfilter[j] * (srcdata[m] + srcdata[n]);
				src2.data.ptr[k][i] = (uchar)(sum / K);
			}
		}
	}
	else
	{
		for (int i = 0; i < src.cols(); i++)
		{
			for (int k = 0; k < src.rows(); k++)
				srcdata[k] = src.data.ptr[k][i];
			for (int k = nfilterlen / 2; k < src.rows() - nfilterlen / 2; k++)
			{
				int sum = 0;
				for (int j = 0, m = k - nfilterlen / 2; j < nfilterlen; j++, m++)
					sum += anfilter[j] * srcdata[m];
				src2.data.ptr[k][i] = (uchar)(sum / K);
			}
		}
	}
	// differntiation according x direction

	int *anDiffFilter = xdir.data.i;
	int nside = xdir.length();
	uchar** data = src2.data.ptr;

	if (nside == 3 && anDiffFilter[0] == -1 && anDiffFilter[1] == 0 && anDiffFilter[2] == 1)
	{
		if (fnormal)
		{
			Mati temp(src.rows(), src.cols()); temp.zero();
			int nmin = 0, nmax = 0;
			for (int k = 0; k < src.rows(); k++)
			{
				for (int i = 1; i < src.cols() - 1; i++)
				{
					int sum = data[k][i + 1] - data[k][i - 1];
					temp[k][i] = sum;
					nmin = MIN(nmin, sum);
					nmax = MAX(nmax, sum);
				}
			}
			dst.create(src.rows(), src.cols(), src.type1());
			int denom = nmax == nmin ? 255 : nmax - nmin;
			for (int i = 0; i < temp.rows(); i++)
			{
				uchar* dst_i = dst.data.ptr[i];
				int* temp_i = temp[i];
				for (int k = 0; k < temp.cols(); k++)
					dst_i[k] = (uchar)((temp_i[k] - nmin) * 255 / denom);
			}
		}
		else
		{
			dst.create(src.size(), MAT_Tint); dst.zero();
			for (int k = 0; k < src.rows(); k++)
			{
				int* dst_k = dst.data.i[k];
				uchar* data_k = data[k];
				for (int i = 1; i < src.cols() - 1; i++)
					dst_k[i] = data_k[i + 1] - data_k[i - 1];
			}
		}
	}
	else
	{
		int nside2 = nside / 2;
		if (fnormal)
		{
			Mati temp(src.rows(), src.cols()); temp.zero();
			int nmin = 0, nmax = 0;
			for (int k = 0; k < src.rows(); k++)
			{
				for (int i = nside2; i < src.cols() - nside2; i++)
				{
					int sum = 0;
					for (int j = 0, m = i - nside2; j < nside; j++, m++)
						sum += anDiffFilter[j] * data[k][m];
					temp[k][i] = sum;
					nmin = MIN(nmin, sum);
					nmax = MAX(nmax, sum);
				}
			}
			dst.create(src.rows(), src.cols(), src.type1());
			int denom = nmax == nmin ? 255 : nmax - nmin;
			for (int i = 0; i < temp.rows(); i++)
			{
				uchar* dst_i = dst.data.ptr[i];
				int* temp_i = temp[i];
				for (int k = 0; k < temp.cols(); k++)
					dst_i[k] = (uchar)((temp_i[k] - nmin) * 255 / denom);
			}
		}
		else
		{
			dst.create(src.size(), MAT_Tint); dst.zero();
			for (int k = 0; k < src.rows(); k++)
			{
				int* dst_k = dst.data.i[k];
				for (int i = nside2; i < src.cols() - nside2; i++)
				{
					int sum = 0;
					for (int j = 0, m = i - nside2; j < nside; j++, m++)
						sum += anDiffFilter[j] * data[k][m];
					dst_k[i] = sum;
				}
			}
		}
	}
}

void sepFilter2D(const Mat& src, Mat& dstx, Mat& dsty, const Veci& kx, const Veci& ky, bool fnormal)
{
	enhanceEdgeX(src, dstx, kx, ky, fnormal);
	enhanceEdgeY(src, dsty, ky, kx, fnormal);
}
void enhanceEdge(const Mat& _src, Mat& dst)
{
	int cols = _src.cols();
	int rows = _src.rows();

	const Mat& src = _src;
	int anx[] = { 1,2,1 };
	int any[] = { -1,0,1 };
	Veci vxdir(anx, sizeof(anx) / sizeof(anx[0]));
	Veci vydir(any, 3);
	Mat enhancedX, enhancedY;
	ip::enhanceEdgeX(src, enhancedX, vxdir, vydir, false);
	ip::enhanceEdgeY(src, enhancedY, vydir, vxdir, false);

	int elemcount = enhancedX.rows()*enhancedX.cols()*enhancedX.channels();

	dst.create(rows, cols, MAT_Tint);

	int* pn = dst.data.i[0];
	int* const pedgex = enhancedX.data.i[0];
	int* const pedgey = enhancedY.data.i[0];
	for (int j = 0; j < elemcount; j++)
		pn[j] = std::abs(pedgex[j]) + std::abs(pedgey[j]);

	dst.convert(MAT_Tuchar);
}
void gaussian (const Mat& src, Mat& dst, float sigma)
{
	dst.create (src.rows(), src.cols(), src.type1());
	GaussianFilter filter;
	filter.setParam (sigma, sigma*2.0f);
	filter.process ((Mat*)&src, &dst);
}
void gaussian (Mat& src, float sigma)
{
	Mat t=src;
	gaussian (t, src, sigma);
}
void dogfilter (const Mat& src, Mat& dst, float sigma_in, float sigma_out, float radius)
{
	DoGFilter dog;
	dog.m_rRadius = radius;
	dog.m_rSigmaInner = sigma_in;
	dog.m_rSigmaOuter = sigma_out;
	dog.process ((Mat*)&src, &dst);
}
int blurringCircle(const Mat& src, Mat& dst, int nCircleRadius, int nSigma,  int nSmoothRaduis, const Point& pt)
{
	GaussianFilter filter;
	return filter.ProcessCircle(&src, &dst, nCircleRadius, nSigma, nSmoothRaduis, pt.y, pt.x);
}

void gaussiankernel(float rSigma, float rRadius, Vecd& vKernel)
{
	GaussianFilter::GetKernel(rSigma, rRadius, vKernel);
}
void gaussiankernel2D(float rSigma1, float rSigma2, float rRadius, Matd& mKernel)
{
	GaussianFilter::GetKernel2D(rSigma1, rSigma2, rRadius, mKernel);
}
void gaussian (const Mat& src, Mat& dst, float sigma, float radius)
{
	dst.create (src.rows(), src.cols(), src.type1());
	GaussianFilter filter;
	filter.setParam (sigma, radius);
	filter.process ((Mat*)&src, &dst);
}
void gaussian (Mat& src, float sigma, float radius)
{
	Mat t=src;
	gaussian (t, src, sigma, radius);
}
void gaussianDiffX(const Mat& src, Mat& dst, float sigma, float radius)
{
	dst.create(src, true);
	GaussianDiffFilter gdFilter;
	gdFilter.setParam (sigma, radius);
	gdFilter.ProcessX(&src, &dst);
}
void gaussianDiffY(const Mat& src, Mat& dst, float sigma, float radius)
{
	dst.create(src, true);
	GaussianDiffFilter gdFilter;
	gdFilter.setParam (sigma, radius);
	gdFilter.ProcessY(&src, &dst);
}

// void blurringPoint (const Mat& src, Mat& dst, )
// 
}

}