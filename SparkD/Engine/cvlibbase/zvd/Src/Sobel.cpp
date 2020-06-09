#include "ImgProc.h"
#include "ImgFilter.h"
#include <assert.h>
#include "Sobel.h"

namespace cvlib 
{

Sobel::Sobel()
{
}

Sobel::~Sobel()
{
}

void	Sobel::process(Mat* pIn, Mat* pOut /* = NULL */)
{
	assert (pIn->type1() == MAT_Tbyte);

	int nX, nY;
	int nWidth = pIn->cols();
	int	nHeight = pIn->rows();
	uchar**	ppbIn = pIn->data.ptr;
	uchar**	ppbOut = pOut ? pOut->data.ptr: ppbIn;
	int	nEdgeX, nEdgeY;

	assert (ppbIn);
	assert (ppbOut);

	Mat		matTemp;
	matTemp.create (nHeight, nWidth, MAT_Tint);
	matTemp.zero();
	int**	ppnTemp = matTemp.data.i;

	for (nY = 1; nY < nHeight - 1; nY ++)
	{
		for (nX = 1; nX < nWidth - 1; nX ++)
		{
			nEdgeY = 
				ppbIn[nY - 1][nX - 1] + ppbIn[nY - 1][nX] * 2 + ppbIn[nY - 1][nX + 1] - 
				ppbIn[nY + 1][nX - 1] - ppbIn[nY + 1][nX] * 2 - ppbIn[nY + 1][nX + 1];
			nEdgeX = 
				ppbIn[nY - 1][nX + 1] + ppbIn[nY][nX + 1] * 2 + ppbIn[nY + 1][nX + 1] - 
				ppbIn[nY - 1][nX - 1] - ppbIn[nY][nX - 1] * 2 - ppbIn[nY + 1][nX - 1];
			ppnTemp[nY][nX] = ABS(nEdgeX) + ABS(nEdgeY);
		}
	}
	if (ppbIn == ppbOut)
	{
		matTemp.convert (MAT_Tbyte);
		for (nY = 0; nY < nHeight; nY ++)
		{
			for (nX = 0; nX < nWidth; nX ++)
			{
				ppbOut[nY][nX] = matTemp.data.ptr[nY][nX];
			}
		}
	}
	else
	{
		matTemp.convertTo (*pOut, MAT_Tbyte);
	}
}

/**
 * getIntensityY:
 *
 * @param pmatIn 
 * @param pmatOut 
 * @return void 
 */
void Sobel::getIntensityY(const Mat* pmatIn, Mat* pmatOut /* = NULL */)
{
	assert (pmatIn->type() == MAT_Tbyte);
	
	int nX, nY;
	int nWidth = pmatIn->cols();
	int	nHeight = pmatIn->rows();
	uchar**	ppbIn = pmatIn->data.ptr;
	uchar**	ppbOut = pmatOut ? pmatOut->data.ptr: ppbIn;
	int	nEdgeY;
	
	Mat		matTemp;
	matTemp.create (nHeight, nWidth, MAT_Tint);
	matTemp.zero();
	int**	ppnTemp = matTemp.data.i;

	for (nY = 1; nY < nHeight - 1; nY ++)
	{
		for (nX = 1; nX < nWidth - 1; nX ++)
		{
			nEdgeY = 
				ppbIn[nY - 1][nX - 1] + ppbIn[nY - 1][nX] * 2 + ppbIn[nY - 1][nX + 1] - 
				ppbIn[nY + 1][nX - 1] - ppbIn[nY + 1][nX] * 2 - ppbIn[nY + 1][nX + 1];
			ppnTemp[nY][nX] = ABS(nEdgeY);
		}
	}

	matTemp.convert(MAT_Tbyte);

	for (nY = 0; nY < nHeight; nY ++)
	{
		for (nX = 0; nX < nWidth; nX ++)
		{
			ppbOut[nY][nX] = matTemp.data.ptr[nY][nX];
		}
	}
}

/**
 * getIntensityX:
 *
 * @param pmatIn 
 * @param pmatOut 
 * @return void 
 */
void Sobel::getIntensityX(const Mat* pmatIn, Mat* pmatOut /* = NULL */)
{
	assert (pmatIn->type() == MAT_Tbyte);
//	assert (pmatOut->type() == MAT_Tbyte);
	
	int nX, nY;
	int nWidth = pmatIn->cols();
	int	nHeight = pmatIn->rows();
	uchar**	ppbIn = pmatIn->data.ptr;
	uchar**	ppbOut = pmatOut ? pmatOut->data.ptr: ppbIn;
	int	nEdgeX;
	
	Mat		matTemp;
	matTemp.create (nHeight, nWidth, MAT_Tint);
	matTemp.zero();
	int**	ppnTemp = matTemp.data.i;

	for (nY = 1; nY < nHeight - 1; nY ++)
	{
		for (nX = 1; nX < nWidth - 1; nX ++)
		{
			nEdgeX = 
				ppbIn[nY - 1][nX + 1] + ppbIn[nY][nX + 1] * 2 + ppbIn[nY + 1][nX + 1] - 
				ppbIn[nY - 1][nX - 1] - ppbIn[nY][nX - 1] * 2 - ppbIn[nY + 1][nX - 1];
			ppnTemp[nY][nX] = ABS(nEdgeX);
		}
	}
	matTemp.convert(MAT_Tbyte);
	for (nY = 0; nY < nHeight; nY ++)
	{
		for (nX = 0; nX < nWidth; nX ++)
		{
			ppbOut[nY][nX] = (uchar)matTemp.data.ptr[nY][nX];
		}
	}
}

/**
 * gradientMagnitude:
 *
 * @param pmIn 
 * @param pmOut 
 * @return void 
 */
void	Sobel::gradientMagnitude(const Mat* pmIn, Mat* pmOut)
{
	assert (pmIn->type() == MAT_Tbyte);
	
	int nX, nY;
	int nWidth = pmIn->cols();
	int	nHeight = pmIn->rows();
	uchar**	ppbIn = pmIn->data.ptr;
	int		nEdgeX;
	int		nEdgeY;
	
	switch(pmOut->type())
	{
	case MAT_Tbyte:
		{
			uchar**	ppbOut = pmOut->data.ptr;
			for (nY = 1; nY < nHeight - 1; nY ++)
			{
				for (nX = 1; nX < nWidth - 1; nX ++)
				{
					nEdgeX = 
						ppbIn[nY - 1][nX + 1] + ppbIn[nY][nX + 1] * 2 + ppbIn[nY + 1][nX + 1] - 
						ppbIn[nY - 1][nX - 1] - ppbIn[nY][nX - 1] * 2 - ppbIn[nY + 1][nX - 1];
					nEdgeY = 
						ppbIn[nY - 1][nX - 1] + ppbIn[nY - 1][nX] * 2 + ppbIn[nY - 1][nX + 1] - 
						ppbIn[nY + 1][nX - 1] - ppbIn[nY + 1][nX] * 2 - ppbIn[nY + 1][nX + 1];
					ppbOut[nY][nX] = (uchar)(ABS(nEdgeX) + ABS(nEdgeY));
				}
			}
		}
		break;
	case MAT_Tshort:
		{
			short**	ppsOut = pmOut->data.s;
			for (nY = 1; nY < nHeight - 1; nY ++)
			{
				for (nX = 1; nX < nWidth - 1; nX ++)
				{
					nEdgeX = 
						ppbIn[nY - 1][nX + 1] + ppbIn[nY][nX + 1] * 2 + ppbIn[nY + 1][nX + 1] - 
						ppbIn[nY - 1][nX - 1] - ppbIn[nY][nX - 1] * 2 - ppbIn[nY + 1][nX - 1];
					nEdgeY = 
						ppbIn[nY - 1][nX - 1] + ppbIn[nY - 1][nX] * 2 + ppbIn[nY - 1][nX + 1] - 
						ppbIn[nY + 1][nX - 1] - ppbIn[nY + 1][nX] * 2 - ppbIn[nY + 1][nX + 1];
					ppsOut[nY][nX] = (short)(ABS(nEdgeX) + ABS(nEdgeY));
				}
			}
		}
		break;
	case MAT_Tint:
		{
			int**	ppnOut = pmOut->data.i;
			for (nY = 1; nY < nHeight - 1; nY ++)
			{
				for (nX = 1; nX < nWidth - 1; nX ++)
				{
					nEdgeX = 
						ppbIn[nY - 1][nX + 1] + ppbIn[nY][nX + 1] * 2 + ppbIn[nY + 1][nX + 1] - 
						ppbIn[nY - 1][nX - 1] - ppbIn[nY][nX - 1] * 2 - ppbIn[nY + 1][nX - 1];
					nEdgeY = 
						ppbIn[nY - 1][nX - 1] + ppbIn[nY - 1][nX] * 2 + ppbIn[nY - 1][nX + 1] - 
						ppbIn[nY + 1][nX - 1] - ppbIn[nY + 1][nX] * 2 - ppbIn[nY + 1][nX + 1];
					ppnOut[nY][nX] = (ABS(nEdgeX) + ABS(nEdgeY));
				}
			}
		}
	    break;
	case MAT_Tfloat:
		{
			float**	pprOut = pmOut->data.fl;
			for (nY = 1; nY < nHeight - 1; nY ++)
			{
				for (nX = 1; nX < nWidth - 1; nX ++)
				{
					nEdgeX = 
						ppbIn[nY - 1][nX + 1] + ppbIn[nY][nX + 1] * 2 + ppbIn[nY + 1][nX + 1] - 
						ppbIn[nY - 1][nX - 1] - ppbIn[nY][nX - 1] * 2 - ppbIn[nY + 1][nX - 1];
					nEdgeY = 
						ppbIn[nY - 1][nX - 1] + ppbIn[nY - 1][nX] * 2 + ppbIn[nY - 1][nX + 1] - 
						ppbIn[nY + 1][nX - 1] - ppbIn[nY + 1][nX] * 2 - ppbIn[nY + 1][nX + 1];
					pprOut[nY][nX] = (float)(fabs((double)nEdgeX) + fabs((double)nEdgeY));
				}
			}
		}
	    break;
	case MAT_Tdouble:
		{
			double**	pprOut = pmOut->data.db;
			for (nY = 1; nY < nHeight - 1; nY ++)
			{
				for (nX = 1; nX < nWidth - 1; nX ++)
				{
					nEdgeX = 
						ppbIn[nY - 1][nX + 1] + ppbIn[nY][nX + 1] * 2 + ppbIn[nY + 1][nX + 1] - 
						ppbIn[nY - 1][nX - 1] - ppbIn[nY][nX - 1] * 2 - ppbIn[nY + 1][nX - 1];
					nEdgeY = 
						ppbIn[nY - 1][nX - 1] + ppbIn[nY - 1][nX] * 2 + ppbIn[nY - 1][nX + 1] - 
						ppbIn[nY + 1][nX - 1] - ppbIn[nY + 1][nX] * 2 - ppbIn[nY + 1][nX + 1];
					pprOut[nY][nX] = fabs( (double)nEdgeX) + fabs((double)nEdgeY);
				}
			}
		}
	    break;
        default:
            assert(false);
	}
}

#define I_DEF_GradientMagnitudeX( flavor, SrcType, DestType )  \
static void	iGradientMagnitudeX_##flavor						\
( SrcType** ppSrc, int nWidth, int nHeight, DestType** ppDest, bool fABS )	\
{												                       \
	SrcType**	ppSrc1 = ppSrc;											\
	DestType**	ppDest1 = ppDest;										\
	DestType		nEdgeX;													\
	int nX, nY;															\
	for (nY = 1; nY < nHeight - 1; nY ++)								\
	{																	\
		for (nX = 1; nX < nWidth - 1; nX ++)							\
		{																\
			nEdgeX = (DestType)(													\
				ppSrc1[nY - 1][nX + 1] + ppSrc1[nY][nX + 1] * 2 + ppSrc1[nY + 1][nX + 1] - \
				ppSrc1[nY - 1][nX - 1] - ppSrc1[nY][nX - 1] * 2 - ppSrc1[nY + 1][nX - 1]);	\
			if (fABS)													\
				ppDest1[nY][nX] = (DestType)fabs((double)nEdgeX);		\
			else														\
				ppDest1[nY][nX] = (DestType)nEdgeX;						\
		}																\
	}																	\
	for (nY = 0; nY < nHeight; nY ++)									\
	{																	\
		ppDest1[nY][0] = (DestType)0;									\
		ppDest1[nY][nWidth - 1] = (DestType)0;							\
	}																	\
	for (nX = 0; nX < nWidth; nX ++)									\
	{																	\
		ppDest1[0][nX] = (DestType)0;									\
		ppDest1[nHeight-1][nX] = (DestType)0;							\
	}																	\
}


I_DEF_GradientMagnitudeX( B2B, uchar, uchar )
I_DEF_GradientMagnitudeX( B2short, uchar, short )
I_DEF_GradientMagnitudeX( B2int, uchar, int )
I_DEF_GradientMagnitudeX( B2float, uchar, float )
I_DEF_GradientMagnitudeX( B2double, uchar, double )
I_DEF_GradientMagnitudeX( short2short, short, short )
I_DEF_GradientMagnitudeX( short2int, short, int )
I_DEF_GradientMagnitudeX( short2float, short, float )
I_DEF_GradientMagnitudeX( short2double, short, double )
I_DEF_GradientMagnitudeX( int2int, int, int )
I_DEF_GradientMagnitudeX( int2float, int, float )
I_DEF_GradientMagnitudeX( int2double, int, double )
I_DEF_GradientMagnitudeX( float2float, float, float )
I_DEF_GradientMagnitudeX( float2double, float, double )
I_DEF_GradientMagnitudeX( double2double, double, double )

/**
 * gradientMagnitudeX:
 *
 * @param pmIn 
 * @param pmOut 
 * @return void 
 */
void Sobel::gradientMagnitudeX(const Mat* pmIn, Mat* pmOut, bool fABS/*=true*/)
{
	int nWidth = pmIn->cols();
	int	nHeight = pmIn->rows();

	switch (pmIn->type())
	{
	case MAT_Tbyte:
		{
			switch (pmOut->type())
			{
			case MAT_Tbyte:
				iGradientMagnitudeX_B2B (pmIn->data.ptr, nWidth, nHeight, pmOut->data.ptr, fABS);
				break;
			case MAT_Tshort:
				iGradientMagnitudeX_B2short (pmIn->data.ptr, nWidth, nHeight, pmOut->data.s, fABS);
				break;
			case MAT_Tint:
				iGradientMagnitudeX_B2int (pmIn->data.ptr, nWidth, nHeight, pmOut->data.i, fABS);
				break;
			case MAT_Tfloat:
				iGradientMagnitudeX_B2float (pmIn->data.ptr, nWidth, nHeight, pmOut->data.fl, fABS);
				break;
			case MAT_Tdouble:
				iGradientMagnitudeX_B2double (pmIn->data.ptr, nWidth, nHeight, pmOut->data.db, fABS);
				break;
                    default:
                    assert(false);
			}
		}
		break;
	case MAT_Tshort:
		{
			switch (pmOut->type())
			{
			case MAT_Tshort:
				iGradientMagnitudeX_short2short (pmIn->data.s, nWidth, nHeight, pmOut->data.s, fABS);
				break;
			case MAT_Tint:
				iGradientMagnitudeX_short2int (pmIn->data.s, nWidth, nHeight, pmOut->data.i, fABS);
				break;
			case MAT_Tfloat:
				iGradientMagnitudeX_short2float (pmIn->data.s, nWidth, nHeight, pmOut->data.fl, fABS);
				break;
			case MAT_Tdouble:
				iGradientMagnitudeX_short2double (pmIn->data.s, nWidth, nHeight, pmOut->data.db, fABS);
				break;
                default:
                    assert(false);
			}
		}
		break;
	case MAT_Tint:
		{
			switch (pmOut->type())
			{
			case MAT_Tint:
				iGradientMagnitudeX_int2int (pmIn->data.i, nWidth, nHeight, pmOut->data.i, fABS);
				break;
			case MAT_Tfloat:
				iGradientMagnitudeX_int2float (pmIn->data.i, nWidth, nHeight, pmOut->data.fl, fABS);
				break;
			case MAT_Tdouble:
				iGradientMagnitudeX_int2double (pmIn->data.i, nWidth, nHeight, pmOut->data.db, fABS);
				break;
                default:
                    assert(false);
			}
		}
		break;
	case MAT_Tfloat:
		{
			switch (pmOut->type())
			{
			case MAT_Tfloat:
				iGradientMagnitudeX_float2float (pmIn->data.fl, nWidth, nHeight, pmOut->data.fl, fABS);
				break;
			case MAT_Tdouble:
				iGradientMagnitudeX_float2double (pmIn->data.fl, nWidth, nHeight, pmOut->data.db, fABS);
				break;
                default:
                    assert(false);
			}
		}
		break;
	case MAT_Tdouble:
		{
			switch (pmOut->type())
			{
			case MAT_Tdouble:
				iGradientMagnitudeX_double2double (pmIn->data.db, nWidth, nHeight, pmOut->data.db, fABS);
				break;
                default:
                    assert(false);
			}
		}
		break;
            default:
            assert (false);
	}
}

#define I_DEF_GradientMagnitudeY( flavor, SrcType, DestType)  \
static void iGradientMagnitudeY_##flavor					\
( SrcType** ppSrc, int nWidth, int nHeight, DestType** ppDest, bool fABS )	\
{												                       \
	SrcType**	ppSrc1 = ppSrc;											\
	DestType**	ppDest1 = ppDest;										\
	DestType		nEdgeX;													\
	int nX, nY;															\
	for (nY = 1; nY < nHeight - 1; nY ++)								\
	{																	\
		for (nX = 1; nX < nWidth - 1; nX ++)							\
		{																\
			nEdgeX = (DestType)(													\
			ppSrc1[nY-1][nX-1] + ppSrc1[nY-1][nX] * 2 + ppSrc1[nY-1][nX+1] - \
			ppSrc1[nY+1][nX-1] - ppSrc1[nY+1][nX] * 2 - ppSrc1[nY+1][nX+1]);	\
			if (fABS)														\
				ppDest1[nY][nX] = (DestType)fabs((double)nEdgeX);			\
			else															\
				ppDest1[nY][nX] = (DestType)nEdgeX;			\
		}																\
	}																	\
	for (nY = 0; nY < nHeight; nY ++)									\
	{																	\
		ppDest1[nY][0] = (DestType)0;												\
		ppDest1[nY][nWidth - 1] = (DestType)0;										\
	}																	\
	for (nX = 0; nX < nWidth; nX ++)									\
	{																	\
		ppDest1[0][nX] = (DestType)0;												\
		ppDest1[nHeight-1][nX] = (DestType)0;										\
	}																	\
}

I_DEF_GradientMagnitudeY( B2B, uchar, uchar )
I_DEF_GradientMagnitudeY( B2short, uchar, short )
I_DEF_GradientMagnitudeY( B2int, uchar, int )
I_DEF_GradientMagnitudeY( B2float, uchar, float )
I_DEF_GradientMagnitudeY( B2double, uchar, double )
I_DEF_GradientMagnitudeY( short2short, short, short )
I_DEF_GradientMagnitudeY( short2int, short, int )
I_DEF_GradientMagnitudeY( short2float, short, float )
I_DEF_GradientMagnitudeY( short2double, short, double )
I_DEF_GradientMagnitudeY( int2int, int, int )
I_DEF_GradientMagnitudeY( int2float, int, float )
I_DEF_GradientMagnitudeY( int2double, int, double )
I_DEF_GradientMagnitudeY( float2float, float, float )
I_DEF_GradientMagnitudeY( float2double, float, double )
I_DEF_GradientMagnitudeY( double2double, double, double )


/**
 * gradientMagnitudeY:
 *
 * @param pmIn 
 * @param pmOut 
 * @return void 
 */
void Sobel::gradientMagnitudeY(const Mat* pmIn, Mat* pmOut, bool fABS/*=true*/)
{
	int nWidth = pmIn->cols();
	int	nHeight = pmIn->rows();
	
	switch (pmIn->type())
	{
	case MAT_Tbyte:
		{
			switch (pmOut->type())
			{
			case MAT_Tbyte:
				iGradientMagnitudeY_B2B (pmIn->data.ptr, nWidth, nHeight, pmOut->data.ptr, fABS);
				break;
			case MAT_Tshort:
				iGradientMagnitudeY_B2short (pmIn->data.ptr, nWidth, nHeight, pmOut->data.s, fABS);
				break;
			case MAT_Tint:
				iGradientMagnitudeY_B2int (pmIn->data.ptr, nWidth, nHeight, pmOut->data.i, fABS);
				break;
			case MAT_Tfloat:
				iGradientMagnitudeY_B2float (pmIn->data.ptr, nWidth, nHeight, pmOut->data.fl, fABS);
				break;
			case MAT_Tdouble:
				iGradientMagnitudeY_B2double (pmIn->data.ptr, nWidth, nHeight, pmOut->data.db, fABS);
				break;
                default:
                    assert(false);
			}
		}
		break;
	case MAT_Tshort:
		{
			switch (pmOut->type())
			{
			case MAT_Tshort:
				iGradientMagnitudeY_short2short (pmIn->data.s, nWidth, nHeight, pmOut->data.s, fABS);
				break;
			case MAT_Tint:
				iGradientMagnitudeY_short2int (pmIn->data.s, nWidth, nHeight, pmOut->data.i, fABS);
				break;
			case MAT_Tfloat:
				iGradientMagnitudeY_short2float (pmIn->data.s, nWidth, nHeight, pmOut->data.fl, fABS);
				break;
			case MAT_Tdouble:
				iGradientMagnitudeY_short2double (pmIn->data.s, nWidth, nHeight, pmOut->data.db, fABS);
				break;
                default:
                    assert(false);
			}
		}
		break;
	case MAT_Tint:
		{
			switch (pmOut->type())
			{
			case MAT_Tint:
				iGradientMagnitudeY_int2int (pmIn->data.i, nWidth, nHeight, pmOut->data.i, fABS);
				break;
			case MAT_Tfloat:
				iGradientMagnitudeY_int2float (pmIn->data.i, nWidth, nHeight, pmOut->data.fl, fABS);
				break;
			case MAT_Tdouble:
				iGradientMagnitudeY_int2double (pmIn->data.i, nWidth, nHeight, pmOut->data.db, fABS);
				break;
                default:
                    assert(false);
			}
		}
		break;
	case MAT_Tfloat:
		{
			switch (pmOut->type())
			{
			case MAT_Tfloat:
				iGradientMagnitudeY_float2float (pmIn->data.fl, nWidth, nHeight, pmOut->data.fl, fABS);
				break;
			case MAT_Tdouble:
				iGradientMagnitudeY_float2double (pmIn->data.fl, nWidth, nHeight, pmOut->data.db, fABS);
				break;
                default:
                    assert(false);
			}
		}
		break;
	case MAT_Tdouble:
		{
			switch (pmOut->type())
			{
			case MAT_Tdouble:
				iGradientMagnitudeY_double2double (pmIn->data.db, nWidth, nHeight, pmOut->data.db, fABS);
				break;
                    default:
                    assert(false);
			}
		}
		break;
            default:
            assert (false);
	}
}

void Sobel::processEOF(const Mat* pmatIn, Mat* pmatOut, int nThreshold /* = 0 */)
{
	assert (pmatIn->type() == MAT_Tbyte);
	assert (pmatOut->type() == MAT_Tbyte);
	
	int nX, nY;
	int nWidth = pmatIn->cols();
	int	nHeight = pmatIn->rows();
	uchar**	ppbIn = pmatIn->data.ptr;
	uchar**	ppbOut = pmatOut->data.ptr;
	int	nEdgeX, nEdgeY;
	
	Mat		matTemp;
	matTemp.create (nHeight, nWidth, MAT_Tint);
	matTemp.zero();
	int**	ppnTemp = matTemp.data.i;
	for (nY = 1; nY < nHeight - 1; nY ++)
	{
		for (nX = 1; nX < nWidth - 1; nX ++)
		{
			nEdgeY = 
				ppbIn[nY - 1][nX - 1] + ppbIn[nY - 1][nX] * 2 + ppbIn[nY - 1][nX + 1] - 
				ppbIn[nY + 1][nX - 1] - ppbIn[nY + 1][nX] * 2 - ppbIn[nY + 1][nX + 1];
			nEdgeX = 
				ppbIn[nY - 1][nX + 1] + ppbIn[nY][nX + 1] * 2 + ppbIn[nY + 1][nX + 1] - 
				ppbIn[nY - 1][nX - 1] - ppbIn[nY][nX - 1] * 2 - ppbIn[nY + 1][nX - 1];
			if((nEdgeX == 0) && (nEdgeY == 0))
				ppnTemp[nY][nX] = 36;
			else if (ABS(nEdgeX) + ABS(nEdgeY) < nThreshold)
				ppnTemp[nY][nX] = 36;
			else
				ppnTemp[nY][nX] = ( uchar )( ( int )
				( ( atan2 ( - (double)nEdgeX, - (double)nEdgeY ) / ( 2 * CVLIB_PI ) + 1.25F ) * 36 ) % 36 );
			assert((ppnTemp[nY][nX] < 37) && (ppnTemp[nY][nX] >= 0));		
			
		}
	}
	
	for (nY = 0; nY < nHeight-2; nY ++)
	{
		for (nX = 0; nX < nWidth-2; nX ++)
		{
			ppbOut[nY][nX] = (uchar)ppnTemp[nY+1][nX+1];
		}
	}
	
	matTemp.release();
}


#define	MEAN_DIR_CODE			32				//	mean value for direction code operation
#define	DIR_CODE_NUMBER			64				//	dimension for direction code table

static uchar	gaabDirCodeTable [ DIR_CODE_NUMBER ][ DIR_CODE_NUMBER ] = 
{	//	convert table for direction code
	{
		13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17,
		18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22
	},
	{
		13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17,
		18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22
	},
	{
		13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17,
		18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22
	},
	{
		13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17,
		18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22
	},
	{
		13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17,
		18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22
	},
	{
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 17, 17, 17, 17,
		18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22
	},
	{
		12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 17, 17, 17, 17,
		18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23
	},
	{
		12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 17, 17, 17, 17,
		18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 23
	},
	{
		12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 16, 16, 16, 16, 17, 17, 17, 17,
		18, 18, 18, 18, 18, 19, 19, 19, 19, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23
	},
	{
		12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 16, 16, 16, 16, 17, 17, 17, 17,
		18, 18, 18, 18, 18, 19, 19, 19, 19, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23
	},
	{
		12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 16, 17, 17, 17,
		18, 18, 18, 18, 19, 19, 19, 19, 19, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23
	},
	{
		12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 16, 16, 16, 16, 17, 17, 17,
		18, 18, 18, 18, 19, 19, 19, 19, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23
	},
	{
		12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17, 17, 17,
		18, 18, 18, 18, 19, 19, 19, 19, 20, 20, 20, 20, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23
	},
	{
		12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 15, 15, 15, 15, 16, 16, 16, 17, 17, 17,
		18, 18, 18, 18, 19, 19, 19, 20, 20, 20, 20, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 23
	},
	{
		11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 15, 15, 15, 15, 16, 16, 16, 17, 17, 17,
		18, 18, 18, 18, 19, 19, 19, 20, 20, 20, 20, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23
	},
	{
		11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 15, 15, 15, 16, 16, 16, 16, 17, 17,
		18, 18, 18, 19, 19, 19, 19, 20, 20, 20, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 23, 24, 24
	},
	{
		11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15, 15, 16, 16, 16, 17, 17,
		18, 18, 18, 19, 19, 19, 20, 20, 20, 20, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24
	},
	{
		11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15, 16, 16, 16, 17, 17,
		18, 18, 18, 19, 19, 19, 20, 20, 20, 21, 21, 21, 21, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24
	},
	{
		11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 16, 17, 17,
		18, 18, 18, 19, 19, 19, 20, 20, 20, 21, 21, 21, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24
	},
	{
		11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 17, 17,
		18, 18, 18, 19, 19, 20, 20, 20, 21, 21, 21, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24
	},
	{
		11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 16, 16, 17, 17,
		18, 18, 18, 19, 19, 20, 20, 21, 21, 21, 21, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
	},
	{
		10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 15, 15, 16, 16, 16, 17,
		18, 18, 19, 19, 19, 20, 20, 21, 21, 21, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 25
	},
	{
		10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 16, 16, 17,
		18, 18, 19, 19, 20, 20, 21, 21, 21, 22, 22, 22, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25
	},
	{
		10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17,
		18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 22, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25
	},
	{
		10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 17,
		18, 18, 19, 20, 20, 21, 21, 22, 22, 22, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25
	},
	{
		10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 14, 15, 15, 16, 17,
		18, 18, 19, 20, 20, 21, 22, 22, 22, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25
	},
	{
		10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 12, 12, 12, 13, 13, 14, 14, 15, 16, 17,
		18, 18, 19, 20, 21, 21, 22, 22, 23, 23, 23, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25
	},
	{
		9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 12, 12, 12, 13, 14, 14, 15, 16, 18,
		19, 20, 21, 21, 22, 23, 23, 23, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26
	},
	{
		9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 13, 14, 15, 16, 18, 19,
		20, 21, 22, 23, 23, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26, 26
	},
	{
		9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 12, 12, 13, 14, 16, 18, 19,
		21, 22, 23, 23, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26
	},
	{
		9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 11, 11, 12, 13, 15, 18, 20, 22,
		23, 24, 24, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26
	},
	{
		9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 11, 13, 18, 22, 24, 25,
		25, 25, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26
	},
	{
		9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 36, 27, 27, 27,
		27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27
	},
	{
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 4, 0, 31, 29, 28,
		28, 28, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27
	},
	{
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 6, 6, 5, 4, 2, 0, 33, 31, 30,
		29, 29, 28, 28, 28, 28, 28, 28, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27
	},
	{
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 5, 5, 4, 3, 1, 0, 34, 32, 31,
		30, 30, 29, 29, 29, 28, 28, 28, 28, 28, 28, 28, 28, 28, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27
	},
	{
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 5, 5, 4, 3, 2, 1, 0, 34, 33, 32,
		31, 30, 30, 29, 29, 29, 29, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 27, 27, 27, 27, 27, 27, 27, 27, 27
	},
	{
		8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 5, 5, 5, 4, 3, 3, 2, 1, 0, 34, 33, 32,
		32, 31, 30, 30, 30, 29, 29, 29, 29, 29, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 27, 27, 27
	},
	{
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 5, 5, 5, 4, 4, 3, 3, 2, 1, 0, 0, 35, 34, 33,
		32, 32, 31, 31, 30, 30, 30, 29, 29, 29, 29, 29, 29, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28
	},
	{
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 4, 4, 4, 3, 2, 2, 1, 0, 0, 35, 34, 33,
		33, 32, 31, 31, 31, 30, 30, 30, 30, 29, 29, 29, 29, 29, 29, 29, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28
	},
	{
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 4, 4, 4, 3, 3, 2, 2, 1, 0, 0, 35, 34, 33,
		33, 32, 32, 31, 31, 31, 30, 30, 30, 30, 29, 29, 29, 29, 29, 29, 29, 29, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28
	},
	{
		7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 4, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0, 35, 34, 34,
		33, 33, 32, 32, 31, 31, 31, 30, 30, 30, 30, 30, 29, 29, 29, 29, 29, 29, 29, 29, 29, 28, 28, 28, 28, 28, 28, 28
	},
	{
		7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 1, 1, 0, 0, 35, 34, 34,
		33, 33, 32, 32, 32, 31, 31, 31, 30, 30, 30, 30, 30, 30, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 28, 28, 28, 28
	},
	{
		7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3, 2, 2, 1, 1, 1, 0, 0, 35, 34, 34,
		34, 33, 33, 32, 32, 32, 31, 31, 31, 31, 30, 30, 30, 30, 30, 30, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 28
	},
	{
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3, 3, 2, 2, 1, 1, 0, 0, 0, 35, 35, 34,
		34, 33, 33, 32, 32, 32, 32, 31, 31, 31, 31, 30, 30, 30, 30, 30, 30, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29
	},
	{
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 0, 0, 0, 35, 35, 34,
		34, 33, 33, 33, 32, 32, 32, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 30, 30, 29, 29, 29, 29, 29, 29, 29, 29, 29
	},
	{
		6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 0, 0, 0, 35, 35, 34,
		34, 34, 33, 33, 33, 32, 32, 32, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 30, 30, 30, 29, 29, 29, 29, 29, 29, 29
	},
	{
		6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 0, 0, 0, 35, 35, 34,
		34, 34, 33, 33, 33, 32, 32, 32, 32, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 30, 30, 30, 29, 29, 29, 29, 29, 29
	},
	{
		6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 0, 0, 0, 35, 35, 34,
		34, 34, 33, 33, 33, 33, 32, 32, 32, 32, 31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 30, 30, 30, 29, 29, 29, 29
	},
	{
		6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 35, 35, 34,
		34, 34, 34, 33, 33, 33, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 30, 30, 30, 30, 29, 29
	},
	{
		6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 0, 0, 0, 0, 35, 35, 35,
		34, 34, 34, 33, 33, 33, 33, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30
	},
	{
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 0, 0, 0, 0, 35, 35, 35,
		34, 34, 34, 33, 33, 33, 33, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 30, 30, 30, 30
	},
	{
		5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 35, 35, 35,
		34, 34, 34, 34, 33, 33, 33, 33, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 30, 30, 30
	},
	{
		5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 35, 35, 35,
		34, 34, 34, 34, 33, 33, 33, 33, 33, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 30
	},
	{
		5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 35, 35, 35,
		34, 34, 34, 34, 34, 33, 33, 33, 33, 32, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30
	},
	{
		5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 35, 35, 35,
		35, 34, 34, 34, 34, 33, 33, 33, 33, 33, 32, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 31, 31, 31, 30, 30, 30, 30
	},
	{
		5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 35, 35, 35,
		35, 34, 34, 34, 34, 33, 33, 33, 33, 33, 32, 32, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 31, 31, 31, 30, 30, 30
	},
	{
		5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 35, 35, 35,
		35, 34, 34, 34, 34, 34, 33, 33, 33, 33, 33, 32, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 31, 31, 31, 31, 30, 30
	},
	{
		5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 35, 35, 35,
		35, 34, 34, 34, 34, 34, 33, 33, 33, 33, 33, 33, 32, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 31, 31, 31, 31, 30
	},
	{
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 35, 35, 35,
		35, 34, 34, 34, 34, 34, 33, 33, 33, 33, 33, 33, 32, 32, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 31, 31, 31, 31
	},
	{
		4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 35, 35, 35,
		35, 34, 34, 34, 34, 34, 34, 33, 33, 33, 33, 33, 33, 32, 32, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 31, 31, 31
	},
	{
		4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 35, 35, 35,
		35, 35, 34, 34, 34, 34, 34, 33, 33, 33, 33, 33, 33, 32, 32, 32, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 31, 31
	},
	{
		4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 35, 35, 35,
		35, 35, 34, 34, 34, 34, 34, 33, 33, 33, 33, 33, 33, 33, 32, 32, 32, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 31
	},
	{
		4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 35, 35, 35,
		35, 35, 34, 34, 34, 34, 34, 34, 33, 33, 33, 33, 33, 33, 32, 32, 32, 32, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31
	}
};

void Sobel::EOField(const Mat* pmIn, Mat* pmOut, Mati* pmEdge/*=NULL*/)
{
	int nX, nY;
	int nWidth = pmOut->cols();
	int	nHeight = pmOut->rows();
	int	nEdgeX, nEdgeY;
	uchar* pbY0;
	uchar* pbY1;
	uchar* pbY2;

	for (nY = 0; nY < nHeight; nY++)
	{
		pbY0 = pmIn->data.ptr[nY];
		pbY1 = pmIn->data.ptr[nY + 1];
		pbY2 = pmIn->data.ptr[nY + 2];
		for (nX = 0; nX < nWidth; nX++)
		{
			nEdgeY =
				pbY0[nX] + (pbY0[nX + 1] << 1) + pbY0[nX + 2] - pbY2[nX] - (pbY2[nX + 1] << 1) - pbY2[nX + 2];
			nEdgeX =
				pbY0[nX + 2] + (pbY1[nX + 2] << 1) + pbY2[nX + 2] - pbY0[nX] - (pbY1[nX] << 1) - pbY2[nX];

			{
				if (nEdgeX > 0)
					nEdgeX = nEdgeX >> 5;
				else
					nEdgeX = -((-nEdgeX) >> 5);

				if (nEdgeY > 0)
					nEdgeY = nEdgeY >> 5;
				else
					nEdgeY = -((-nEdgeY) >> 5);
			}
			/*if ( nEdgeX > 0 )
				nEdgeX = (((nEdgeX) >> 4)+1)>>1;
			else
				nEdgeX =-(((-nEdgeX) >> 4)+1)>>1;

			if ( nEdgeY > 0 )
				nEdgeY = (((nEdgeY) >> 4)+1)>>1;
			else
				nEdgeY =-(((-nEdgeY) >> 4)+1)>>1;	*/

			pmOut->data.ptr[nY][nX] = gaabDirCodeTable[nEdgeX + MEAN_DIR_CODE][nEdgeY + MEAN_DIR_CODE];

			if (pmEdge)
				pmEdge->data.i[nY][nX] = ABS(nEdgeX) + ABS(nEdgeY);
		}
	}
}

void ip::edgeOrientaitonField (const Mat* pmatIn, Mat* pmatOut, Mati* pmEdge)
{
	cvlib::Sobel _sobel;
	_sobel.EOField (pmatIn, pmatOut, pmEdge);
}
void ip::edgeOrientaitonField (const Mat& in, Mat& out)
{
	out.create (in.rows()-2,in.cols()-2,MAT_Tbyte);
	cvlib::Sobel sobel;
	sobel.EOField (&in, &out, 0);
}
namespace ip
{

static void getScharrKernels( Mat& kx, Mat& ky, int dx, int dy, bool normalize, int ktype )
{
    const int ksize = 3;

    assert( ktype == MAT_Tfloat || ktype == MAT_Tdouble );
    kx.create(ksize, 1, (TYPE)ktype);
    ky.create(ksize, 1, (TYPE)ktype);

    assert( dx >= 0 && dy >= 0 && dx+dy == 1 );

    for( int k = 0; k < 2; k++ )
    {
        Mat* kernel = k == 0 ? &kx : &ky;
        int order = k == 0 ? dx : dy;
        int kerI[3];

        if( order == 0 )
            kerI[0] = 3, kerI[1] = 10, kerI[2] = 3;
        else if( order == 1 )
            kerI[0] = -1, kerI[1] = 0, kerI[2] = 1;

        Mat temp(kerI, kernel->rows(), kernel->cols(), MAT_Tint);
        double scale = !normalize || order == 1 ? 1. : 1./32;
        temp.convertTo(*kernel, (TYPE)ktype, scale, 0);
    }
}


static void getSobelKernels( Mat& kx, Mat& ky,
                             int dx, int dy, int _ksize, bool normalize, int ktype )
{
    int i, j, ksizeX = _ksize, ksizeY = _ksize;
    if( ksizeX == 1 && dx > 0 )
        ksizeX = 3;
    if( ksizeY == 1 && dy > 0 )
        ksizeY = 3;

    assert( ktype == MAT_Tfloat || ktype == MAT_Tdouble );

    kx.create(ksizeX, 1, (TYPE)ktype);
    ky.create(ksizeY, 1, (TYPE)ktype);

    if( _ksize % 2 == 0 || _ksize > 31 )
        assert(false);//( CV_StsOutOfRange, "The kernel size must be odd and not larger than 31" );
    Vector<int> kerI(MAX(ksizeX, ksizeY) + 1);

    assert( dx >= 0 && dy >= 0 && dx+dy > 0 );

    for( int k = 0; k < 2; k++ )
    {
        Mat* kernel = k == 0 ? &kx : &ky;
        int order = k == 0 ? dx : dy;
        int ksize = k == 0 ? ksizeX : ksizeY;

        assert( ksize > order );

        if( ksize == 1 )
            kerI[0] = 1;
        else if( ksize == 3 )
        {
            if( order == 0 )
                kerI[0] = 1, kerI[1] = 2, kerI[2] = 1;
            else if( order == 1 )
                kerI[0] = -1, kerI[1] = 0, kerI[2] = 1;
            else
                kerI[0] = 1, kerI[1] = -2, kerI[2] = 1;
        }
        else
        {
            int oldval, newval;
            kerI[0] = 1;
            for( i = 0; i < ksize; i++ )
                kerI[i+1] = 0;

            for( i = 0; i < ksize - order - 1; i++ )
            {
                oldval = kerI[0];
                for( j = 1; j <= ksize; j++ )
                {
                    newval = kerI[j]+kerI[j-1];
                    kerI[j-1] = oldval;
                    oldval = newval;
                }
            }

            for( i = 0; i < order; i++ )
            {
                oldval = -kerI[0];
                for( j = 1; j <= ksize; j++ )
                {
                    newval = kerI[j-1] - kerI[j];
                    kerI[j-1] = oldval;
                    oldval = newval;
                }
            }
        }

        Mat temp(&kerI[0], kernel->rows(), kernel->cols(), MAT_Tint);
        double scale = !normalize ? 1. : 1./(1 << (ksize-order-1));
        temp.convertTo(*kernel, (TYPE)ktype, scale, 0);
    }
}
void getDerivKernels( Mat& kx, Mat& ky, int dx, int dy, int ksize, bool normalize, int ktype )
{
    if( ksize <= 0 )
        getScharrKernels( kx, ky, dx, dy, normalize, ktype );
    else
        getSobelKernels( kx, ky, dx, dy, ksize, normalize, ktype );
}

void sobel(const Mat& src, Mat& dst, int dx, int dy, int aperture_size )
{
	assert( src.isEqualSize(dst) && src.channels() == dst.channels() );
	dst.create (src.size(), (TYPE)CVLIB_MAKETYPE(dst.type1(), src.channels()) );

    int ktype = MAX(MAT_Tfloat, MAX(dst.type(), src.type()));

    Mat kx, ky;
    getDerivKernels( kx, ky, dx, dy, aperture_size, false, ktype );
    /*if( scale != 1 )
    {
        // usually the smoothing part is the slowest to compute,
        // so try to scale it instead of the faster differenciating part
        if( dx == 0 )
            kx *= scale;
        else
            ky *= scale;
    }*/
    sepFilter2D( src, dst, dst.type(), kx, ky, Point2i(-1,-1), 0.0f/*delta*/, BORDER_REPLICATE/*borderType*/ );
}
void laplace(const Mat& src, Mat& dst, int aperture_size )
{
	assert( src.isEqualSize(dst) && src.channels() == dst.channels() );
    dst.create( src.size(), (TYPE)CVLIB_MAKETYPE(dst.type1(), src.channels()) );
    int ksize=aperture_size;
    if( ksize == 1 || ksize == 3 )
    {
        float K[2][9] =
        {{0, 1, 0, 1, -4, 1, 0, 1, 0},
         {2, 0, 2, 0, -8, 0, 2, 0, 2}};
        Mat kernel(K[ksize == 3], 3, 3, MAT_Tfloat);
//        if( scale != 1 )
//            kernel *= scale;
        filter2D( src, dst, dst.type(), kernel, Point2i(-1,-1), 0.0f/*delta*/, BORDER_REPLICATE/*borderType*/ );
    }
    else
    {
       /* const size_t STRIPE_SIZE = 1 << 14;

        int depth = src.depth();
        int ktype = MAX(MAT_Tfloat, MAX(ddepth, depth));
        int wdepth = depth == MAT_Tbyte && ksize <= 5 ? MAT_Tshort : depth <= MAT_Tfloat ? MAT_Tfloat : MAT_Tdouble;
        int wtype = CVLIB_MAKETYPE(wdepth, src.channels());
        Mat kd, ks;
        getSobelKernels( kd, ks, 2, 0, ksize, false, ktype );
        if( ddepth < 0 )
            ddepth = src.depth();
        int dtype = CVLIB_MAKETYPE(ddepth, src.channels());

        int dy0 = MIN(MAX((int)(STRIPE_SIZE/(getElemSize(src.type())*src.cols)), 1), src.rows);
        Ptr<FilterEngine> fx = createSeparableLinearFilter(src.type(),
            wtype, kd, ks, Point(-1,-1), 0, borderType, borderType, Scalar() ); 
        Ptr<FilterEngine> fy = createSeparableLinearFilter(src.type(),
            wtype, ks, kd, Point(-1,-1), 0, borderType, borderType, Scalar() );

        int y = fx->start(src), dsty = 0, dy = 0;
        fy->start(src);
        const uchar* sptr = src.data + y*src.step;

        Mat d2x( dy0 + kd.rows - 1, src.cols, wtype );
        Mat d2y( dy0 + kd.rows - 1, src.cols, wtype );

        for( ; dsty < src.rows; sptr += dy0*src.step, dsty += dy )
        {
            fx->proceed( sptr, (int)src.step, dy0, d2x.data, (int)d2x.step );
            dy = fy->proceed( sptr, (int)src.step, dy0, d2y.data, (int)d2y.step );
            if( dy > 0 )
            {
                Mat dstripe = dst.rowRange(dsty, dsty + dy);
                d2x.rows = d2y.rows = dy; // modify the headers, which should work
                d2x += d2y;
                d2x.convertTo( dstripe, dtype, scale, delta );
            }
        }*/
    }
}

}

}
