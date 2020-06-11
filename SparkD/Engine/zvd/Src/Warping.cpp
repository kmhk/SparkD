#include "Warping.h"
#include "cvlibbase/Inc/matoperation.h"
#include "cvlibbase/Inc/singularValueDecomposition.h"
#include "cvlibbase/Inc/parallel.h"
#include "warp_avx.hpp"
#include "warp_avx2.hpp"
#include <cmath>
#include <algorithm>

#pragma warning (push)
#pragma warning (disable : 4512)

namespace cvlib{ namespace ip{

const int INTER_RESIZE_COEF_BITS=11;
const int INTER_RESIZE_COEF_SCALE=1 << INTER_RESIZE_COEF_BITS;

const int INTER_REMAP_COEF_BITS=15;
const int INTER_REMAP_COEF_SCALE=1 << INTER_REMAP_COEF_BITS;

static uchar NNDeltaTab_i[INTER_TAB_SIZE2][2];

static float BilinearTab_f[INTER_TAB_SIZE2][2][2];
static short BilinearTab_i[INTER_TAB_SIZE2][2][2];

#if CVLIB_SSE2
static short BilinearTab_iC4_buf[INTER_TAB_SIZE2+2][2][8];
static short (*BilinearTab_iC4)[2][8] = (short (*)[2][8])alignPtr(BilinearTab_iC4_buf, 16);
#endif

static float BicubicTab_f[INTER_TAB_SIZE2][4][4];
static short BicubicTab_i[INTER_TAB_SIZE2][4][4];

static float Lanczos4Tab_f[INTER_TAB_SIZE2][8][8];
static short Lanczos4Tab_i[INTER_TAB_SIZE2][8][8];

static inline void interpolateLinear( float x, float* coeffs )
{
	coeffs[0] = 1.f - x;
	coeffs[1] = x;
}

static inline void interpolateCubic( float x, float* coeffs )
{
	const float A = -0.75f;

	coeffs[0] = ((A*(x + 1) - 5*A)*(x + 1) + 8*A)*(x + 1) - 4*A;
	coeffs[1] = ((A + 2)*x - (A + 3))*x*x + 1;
	coeffs[2] = ((A + 2)*(1 - x) - (A + 3))*(1 - x)*(1 - x) + 1;
	coeffs[3] = 1.f - coeffs[0] - coeffs[1] - coeffs[2];
}

static inline void interpolateLanczos4( float x, float* coeffs )
{
	static const double s45 = 0.70710678118654752440084436210485;
	static const double cs[][2]=
	{{1, 0}, {-s45, -s45}, {0, 1}, {s45, -s45}, {-1, 0}, {s45, s45}, {0, -1}, {-s45, s45}};

	if( x < FLT_EPSILON )
	{
		for( int i = 0; i < 8; i++ )
			coeffs[i] = 0;
		coeffs[3] = 1;
		return;
	}

	float sum = 0;
	double y0=-(x+3)*CVLIB_PI*0.25, s0 = sin(y0), c0=cos(y0);
	for(int i = 0; i < 8; i++ )
	{
		double y = -(x+3-i)*CVLIB_PI*0.25;
		coeffs[i] = (float)((cs[i][0]*s0 + cs[i][1]*c0)/(y*y));
		sum += coeffs[i];
	}

	sum = 1.f/sum;
	for(int i = 0; i < 8; i++ )
		coeffs[i] *= sum;
}

static void initInterTab1D(int method, float* tab, int tabsz)
{
	float scale = 1.f/tabsz;
	if( method == INTER_LINEAR )
	{
		for( int i = 0; i < tabsz; i++, tab += 2 )
			interpolateLinear( i*scale, tab );
	}
	else if( method == INTER_CUBIC )
	{
		for( int i = 0; i < tabsz; i++, tab += 4 )
			interpolateCubic( i*scale, tab );
	}
	else if( method == INTER_LANCZOS4 )
	{
		for( int i = 0; i < tabsz; i++, tab += 8 )
			interpolateLanczos4( i*scale, tab );
	}
	else
		assert (false); //( CV_StsBadArg, "Unknown interpolation method" );
}


static const void* initInterTab2D( int method, bool fixpt )
{
	static bool inittab[INTER_MAX+1] = {false};
	float* tab = 0;
	short* itab = 0;
	int ksize = 0;
	if( method == INTER_LINEAR )
		tab = BilinearTab_f[0][0], itab = BilinearTab_i[0][0], ksize=2;
	else if( method == INTER_CUBIC )
		tab = BicubicTab_f[0][0], itab = BicubicTab_i[0][0], ksize=4;
	else if( method == INTER_LANCZOS4 )
		tab = Lanczos4Tab_f[0][0], itab = Lanczos4Tab_i[0][0], ksize=8;
	else
		assert(false); // CV_StsBadArg, "Unknown/unsupported interpolation type" );

	if( !inittab[method] )
	{
		AutoBuffer<float> _tab(8*INTER_TAB_SIZE);
		int i, j, k1, k2;
		initInterTab1D(method, _tab, INTER_TAB_SIZE);
		for( i = 0; i < INTER_TAB_SIZE; i++ )
			for( j = 0; j < INTER_TAB_SIZE; j++, tab += ksize*ksize, itab += ksize*ksize )
			{
				int isum = 0;
				NNDeltaTab_i[i*INTER_TAB_SIZE+j][0] = j < INTER_TAB_SIZE/2;
				NNDeltaTab_i[i*INTER_TAB_SIZE+j][1] = i < INTER_TAB_SIZE/2;

				for( k1 = 0; k1 < ksize; k1++ )
				{
					float vy = _tab[i*ksize + k1];
					for( k2 = 0; k2 < ksize; k2++ )
					{
						float v = vy*_tab[j*ksize + k2];
						tab[k1*ksize + k2] = v;
						isum += itab[k1*ksize + k2] = saturate_cast<short>(v*INTER_REMAP_COEF_SCALE);
					}
				}

				if( isum != INTER_REMAP_COEF_SCALE )
				{
					int diff = isum - INTER_REMAP_COEF_SCALE;
					int ksize2 = ksize/2, Mk1=ksize2, Mk2=ksize2, mk1=ksize2, mk2=ksize2;
					for( k1 = ksize2; k1 < ksize2+2; k1++ )
						for( k2 = ksize2; k2 < ksize2+2; k2++ )
						{
							if( itab[k1*ksize+k2] < itab[mk1*ksize+mk2] )
								mk1 = k1, mk2 = k2;
							else if( itab[k1*ksize+k2] > itab[Mk1*ksize+Mk2] )
								Mk1 = k1, Mk2 = k2;
						}
						if( diff < 0 )
							itab[Mk1*ksize + Mk2] = (short)(itab[Mk1*ksize + Mk2] - diff);
						else
							itab[mk1*ksize + mk2] = (short)(itab[mk1*ksize + mk2] - diff);
				}
			}
			tab -= INTER_TAB_SIZE2*ksize*ksize;
			itab -= INTER_TAB_SIZE2*ksize*ksize;
#if CVLIB_SSE2
			if( method == INTER_LINEAR )
			{
				for( i = 0; i < INTER_TAB_SIZE2; i++ )
					for( j = 0; j < 4; j++ )
					{
						BilinearTab_iC4[i][0][j*2] = BilinearTab_i[i][0][0];
						BilinearTab_iC4[i][0][j*2+1] = BilinearTab_i[i][0][1];
						BilinearTab_iC4[i][1][j*2] = BilinearTab_i[i][1][0];
						BilinearTab_iC4[i][1][j*2+1] = BilinearTab_i[i][1][1];
					}
			}
#endif
			inittab[method] = true;
	}
	return fixpt ? (const void*)itab : (const void*)tab;
}



#ifndef __MINGW32__
static bool initAllInterTab2D()
{
	return  initInterTab2D( INTER_LINEAR, false ) &&
		initInterTab2D( INTER_LINEAR, true ) &&
		initInterTab2D( INTER_CUBIC, false ) &&
		initInterTab2D( INTER_CUBIC, true ) &&
		initInterTab2D( INTER_LANCZOS4, false ) &&
		initInterTab2D( INTER_LANCZOS4, true );
}

static volatile bool doInitAllInterTab2D = initAllInterTab2D();
#endif


template<typename ST, typename DT> struct Cast
{
	typedef ST type1;
	typedef DT rtype;

	DT operator()(ST val) const { return saturate_cast<DT>(val); }
};

template<typename ST, typename DT, int bits> struct FixedPtCast
{
	typedef ST type1;
	typedef DT rtype;
	enum { SHIFT = bits, DELTA = 1 << (bits-1) };

	DT operator()(ST val) const { return saturate_cast<DT>((val + DELTA)>>SHIFT); }
};

typedef void (*BilinearInterpolationFunc)(uchar** _dst, int x, int y, uchar** const _src, float rX, float rY, int channel);

template<typename _Tp> static void
BilinearInterpolation_( uchar** _dst, int x, int y, uchar** const _src, float rX, float rY, int channel)
{
	_Tp** const src=(_Tp** const)_src;
	_Tp** dst=(_Tp**)_dst;
	int nX = (int)rX; rX -=  nX;
	int nY = (int)rY; rY -=  nY;
	int n=nX*channel;
	int dstx=x*channel;
    _Tp* prcur=(_Tp*)src [ nY ];
    _Tp* prnext=(_Tp*)src [ nY+1 ];
    _Tp* pdst=(_Tp*)dst[y];
	for (int i=0; i<channel; i++, n++)
	{
		int n1=n+channel;
		float rTemp = (float)prcur[n];
		if ( rX )
			rTemp += rX * (float)( prcur[n1] - prcur[n] );
		if ( rY )
			rTemp += rY * (float)( prnext[n] - prcur[n] );
		if ( rX && rY )
			rTemp += rX * rY * (float)(prnext[n1] + prcur[n] -prcur[n1] - prnext[n] );
		pdst[dstx++]=(_Tp)rTemp;
	}
}

BilinearInterpolationFunc getBilinearInterpolationFunc (int type)
{
	static BilinearInterpolationFunc Tab[] =
	{
		NULL, 
		BilinearInterpolation_<uchar>, 
		BilinearInterpolation_<short>, 
		BilinearInterpolation_<int>, 
		BilinearInterpolation_<float>,
		BilinearInterpolation_<double> 
	};
	return Tab[type];
}
float triangleArea (float rX1, float rY1, 
					float rX2, float rY2,
					float rX3, float rY3)
{
	float rArea = 0.0f;
	rArea = (rX3 - rX2) * (rY1 - rY3) + (rX3 - rX1) * (rY3 - rY2);
	rArea = (float)fabs (rArea);
	return rArea;
}
/*
static float BilinearInterpolation ( uchar** ppbImage, int nX, int nY, float rX, float rY )
{
	float	rTemp;
	
	rTemp = ppbImage [ nY ][ nX ];
	if ( rX )
		rTemp += rX * ( ppbImage [ nY ][ nX + 1 ] - ppbImage [ nY ][ nX ] );
	
	if ( rY )
		rTemp += rY * ( ppbImage [ nY + 1 ][ nX ] - ppbImage [ nY ][ nX ] );
	
	if ( rX && rY )
		rTemp += rX * rY * (
		ppbImage [ nY + 1 ][ nX + 1 ] + ppbImage [ nY ][ nX ] -
		ppbImage [ nY ][ nX + 1 ] - ppbImage [ nY + 1 ][ nX ] );
	
	return rTemp;
}
*/
void warping(const Mat& src, Mat& dst, Rect* pWarpingRect,
	Point2f* pSrcPoints, Point2f* pDstPoints, TriangleModel* pTriangleModel)
{
	if (!dst.isValid())
		dst = src;

	pWarpingRect->x = src.cols() - 1;
	pWarpingRect->y = src.rows() - 1;
	pWarpingRect->width = 1;
	pWarpingRect->height = 1;

	float rX1, rY1, rX2, rY2, rX3, rY3;
	float rX, rY;

	int nTriangleCount = pTriangleModel->m_TriangleList.getSize();
	int iTri;
	int nLeft, nTop, nRight, nBottom;

	for (iTri = 0; iTri < nTriangleCount; iTri++)
	{
		Triangle* pTriangle = (Triangle*)pTriangleModel->m_TriangleList.getAt(iTri);
		rX1 = (float)pDstPoints[pTriangle->n1].x, rY1 = (float)pDstPoints[pTriangle->n1].y;
		rX2 = (float)pDstPoints[pTriangle->n2].x, rY2 = (float)pDstPoints[pTriangle->n2].y;
		rX3 = (float)pDstPoints[pTriangle->n3].x, rY3 = (float)pDstPoints[pTriangle->n3].y;

		nLeft = (int)(MIN(rX1, MIN(rX2, rX3)));
		nRight = (int)(MAX(rX1, MAX(rX2, rX3)));
		nTop = (int)(MIN(rY1, MIN(rY2, rY3)));
		nBottom = (int)(MAX(rY1, MAX(rY2, rY3)));
		nTop = MAX(0, nTop);
		nLeft = MAX(0, nLeft);
		nTop = MIN(nTop, dst.rows());
		nLeft = MIN(nLeft, dst.cols());
		nBottom = MIN(nBottom, dst.rows() - 1);
		nRight = MIN(nRight, dst.cols() - 1);
		for (int iY = nTop; iY <= nBottom; iY++)
		{
			for (int iX = nLeft; iX <= nRight; iX++)
			{
				int nPtX = iX;
				int nPtY = iY;
				rX = (float)iX;
				rY = (float)iY;

				float rS = triangleArea(rX1, rY1, rX2, rY2, rX3, rY3);
				float rS1 = triangleArea(rX, rY, rX2, rY2, rX3, rY3);
				float rS2 = triangleArea(rX, rY, rX1, rY1, rX3, rY3);
				float rS3 = triangleArea(rX, rY, rX1, rY1, rX2, rY2);
				if ((rS1 + rS2 + rS3) - rS > 0.001f)
					continue;
				float rA = 0.1f, rB = 0.5f, rC = 1.0f - rA - rB;
				computeCoefficients(pDstPoints, pTriangle, (float)nPtX, (float)nPtY, &rA, &rB, &rC);
				float rX1_, rY1_, rX2_, rY2_, rX3_, rY3_;
				rX1_ = (float)pSrcPoints[pTriangle->n1].x, rY1_ = (float)pSrcPoints[pTriangle->n1].y;
				rX2_ = (float)pSrcPoints[pTriangle->n2].x, rY2_ = (float)pSrcPoints[pTriangle->n2].y;
				rX3_ = (float)pSrcPoints[pTriangle->n3].x, rY3_ = (float)pSrcPoints[pTriangle->n3].y;

				float rWarpX = rX1_ * rA + rX2_ * rB + rX3_ * rC;
				float rWarpY = rY1_ * rA + rY2_ * rB + rY3_ * rC;
				if (rWarpX < 0 || rWarpY < 0 || rWarpX >= src.cols() - 1 || rWarpY >= src.rows() - 1)
				{
					dst.data.ptr[nPtY][nPtX * 3] = 128;
					dst.data.ptr[nPtY][nPtX * 3 + 1] = 128;
					dst.data.ptr[nPtY][nPtX * 3 + 2] = 128;
				}
				else
				{
					BilinearInterpolationFunc func = getBilinearInterpolationFunc(src.type());
					func(dst.data.ptr, nPtX, nPtY, src.data.ptr, rWarpX, rWarpY, src.channels());
				}
			}
		}

		pWarpingRect->x = MIN(pWarpingRect->x, nLeft);
		pWarpingRect->y = MIN(pWarpingRect->y, nTop);
		pWarpingRect->width = MAX(pWarpingRect->width, nRight - pWarpingRect->x + 1);
		pWarpingRect->height = MAX(pWarpingRect->height, nBottom - pWarpingRect->y + 1);
	}
}

void computeCoefficients (const Point2f* pPoints, const Triangle* pTriangle, float rX, float rY, float* prA, float* prB, float* prC)
{
	if(!pPoints || ! pTriangle)
		return;
	
	float rX1, rY1, rX2, rY2, rX3, rY3;
	rX1 = (float)pPoints[pTriangle->n1].x, rY1 = (float)pPoints[pTriangle->n1].y;
	rX2 = (float)pPoints[pTriangle->n2].x, rY2 = (float)pPoints[pTriangle->n2].y;
	rX3 = (float)pPoints[pTriangle->n3].x, rY3 = (float)pPoints[pTriangle->n3].y;
	float rA1, rA2, rB1, rB2, rC1, rC2;
	rA1 = rX1 - rX3; rB1 = rX2 - rX3; rC1 = rX - rX3;
	rA2 = rY1 - rY3; rB2 = rY2 - rY3; rC2 = rY - rY3;
	float rTemp = 1.0f/(rA1 * rB2 - rA2 * rB1);
	*prA = (rC1 * rB2 - rC2 * rB1) * rTemp;
	*prB = (rA1 * rC2 - rA2 * rC1) * rTemp;
	*prC = 1.0f - (*prA) - (*prB);
	//	assert ((*prA) >= 0.0f);
	//	assert ((*prB) >= 0.0f);
	//	assert ((*prC) >= 0.0f);
}



void initWarp(Mat& W, float a, float b, float c, float d, float tx, float ty)
{
	W.create(3, 3, MAT_Tfloat);
	float data[9] = { 1.0f + a, b, tx,
					c, 1.0f + d, ty,
					0.0f, 0.0f, 1.0f };
	memcpy(W.data.fl[0], data, sizeof(float) * 9);
}

class warpAffineInvoker : public ParallelLoopBody
{
public:
    warpAffineInvoker(const Mat &_src, Mat &_dst, const float* prT) :
        ParallelLoopBody(), src(_src), dst(_dst), aT(prT)
    {
    }

	virtual void operator() (const Range& range) const
	{
		int cn = dst.channels();
		float az[2];
		int nsrcW = src.cols() - 1;
		int nsrcH = src.rows() - 1;

		if (src.type() == MAT_Tuchar)
		{
			int bit = 10;
			int s = 1024;
			int s2 = 1024 * 1024;
			int limx = (nsrcW - 1)*s;
			int limy = (nsrcH - 1)*s;
			uchar** ppsrc = src.data.ptr;
			uchar** ppdst = dst.data.ptr;
			float rx0 = aT[0] * range.start + aT[2];
			float rx3 = aT[3] * range.start + aT[5];
			for (int x = range.start; x < range.end; x++, rx0 += aT[0], rx3 += aT[3])
			{
				az[0] = rx0;
				az[1] = rx3;
				for (int y = 0; y < dst.rows(); y++)
				{
					int rx = az[0] < 0 ? 0 : az[0]>nsrcW - 1 ? limx : (int)(az[0] * s);
					int ry = az[1] < 0 ? 0 : az[1]>nsrcH - 1 ? limy : (int)(az[1] * s);
					{
						uint nX = rx >> bit; rx = rx & 0x3FF;
						uint nY = ry >> bit; ry = ry & 0x3FF;
						uint n = nX*cn;
						const uchar* prcur = (uchar*)&ppsrc[nY][n];
						const uchar* prnext = (uchar*)&ppsrc[nY + 1][n];
						uchar* pdst = (uchar*)&ppdst[y][x*cn];
						uint xy = rx*ry;
						uint x_y = rx + ry;
						for (int i = 0; i < cn; i++, prcur++, prnext++, pdst++)
						{
							int t = ((xy*(*prcur - prcur[cn] - (*prnext) + prnext[cn]) +
								(s2 - s*x_y)*(*prcur) + s*(rx*prcur[cn] + ry*(*prnext))) / s2);
							t = t < 0 ? 0 : t>255 ? 255 : t;
							*pdst = (uchar)t;
						}
					}

					az[0] += aT[1];
					az[1] += aT[4];
				}
			}
		}
		else {
			BilinearInterpolationFunc func = getBilinearInterpolationFunc((int)src.type());
			for (int x = 0; x < dst.cols(); x++)
			{
				az[0] = aT[0] * x + aT[2];
				az[1] = aT[3] * x + aT[5];
				for (int y = 0; y < dst.rows(); y++)
				{
					//			if (az[0] >= 0 && az[0] < nsrcW && az[1] >= 0 && az[1] < nsrcH)
					//				func (dst.data.ptr, x, y, src.data.ptr, az[0], az[1], cn);
					float rx = az[0] < 0 ? 0 : az[0]>nsrcW ? nsrcW : az[0];
					float ry = az[1] < 0 ? 0 : az[1]>nsrcH ? nsrcH : az[1];
					func(dst.data.ptr, x, y, src.data.ptr, rx, ry, cn);

					az[0] += aT[1];
					az[1] += aT[4];
				}
			}
		}
	}

private:
    const Mat& src;
    Mat& dst;
    const float *aT;
};

void warpAffine(const Mat& src, Mat& dst, const Mat& W, bool finvert)
{
	assert(src.channels() == dst.channels());
	dst.zero();

	float aT[9];
	if (!finvert) {
		Mat m = W;
		m.convert(MAT_Tfloat);
		int count = m.rows()*m.cols();
		memcpy(aT, m.data.fl[0], sizeof(float)*count);
	}
	else {
		if (W.rows() == W.cols()) {
			Mat mInvert = W.inverted();
			mInvert.convert(MAT_Tfloat);
			memcpy(aT, mInvert.data.fl[0], sizeof(aT));
		}
		else
		{
			for (int i = 0; i < W.rows()*W.cols(); i++) {
				aT[i] = W.value(0, i);
			}

			float* M = aT;
			double D = M[0] * M[4] - M[1] * M[3];
			D = D != 0 ? 1. / D : 0;
			double A11 = M[4] * D, A22 = M[0] * D;
			M[0] = A11; M[1] *= -D;
			M[3] *= -D; M[4] = A22;
			double b1 = -M[0] * M[2] - M[1] * M[5];
			double b2 = -M[3] * M[2] - M[4] * M[5];
			M[2] = b1; M[5] = b2;
		}
	}

	warpAffineInvoker warper(src, dst, aT);
	warper(Range(0, dst.cols()));
}

void warpPoint (const Vector<Point2i>& src, Vector<Point2i>& dst, const Mat& W)
{
	dst.removeAll();
	if (W.type() == MAT_Tfloat)
	{
		Matf X(3,1);
		Matf Z(3,1);
		for(int i=0; i<src.length(); i++)
		{
			X[0][0]=(float)src[i].x; X[1][0]=(float)src[i].y; X[2][0]=1.0f;
			MatOp::gemm(&W, &X, 1, 0, 0, &Z);
			dst.add (Point2i((int)(Z[0][0]/Z[2][0]), (int)(Z[1][0]/Z[2][0])));
		}
	}
	else if (W.type()==MAT_Tdouble)
	{
		Matd X(3,1);
		Matd Z(3,1);
		for(int i=0; i<src.length(); i++)
		{
			X[0][0]=(float)src[i].x; X[1][0]=(float)src[i].y; X[2][0]=1.0f;
			MatOp::gemm(&W, &X, 1, 0, 0, &Z);
			dst.add (Point2i((int)(Z[0][0]/Z[2][0]), (int)(Z[1][0]/Z[2][0])));
		}
	}
	else
	{
		assert (false);
	}
}
void warpPoint(const Vector<Point2f>& src, Vector<Point2f>& dst, const Mat& W)
{
	dst.removeAll();
	if (W.type() == MAT_Tfloat)
	{
		Matf X(3, 1);
		Matf Z(3, 1);
		for (int i = 0; i<src.length(); i++)
		{
			X[0][0] = (float)src[i].x; X[1][0] = (float)src[i].y; X[2][0] = 1.0f;
			MatOp::gemm(&W, &X, 1, 0, 0, &Z);
			dst.add(Point2f((float)(Z[0][0] / Z[2][0]), (float)(Z[1][0] / Z[2][0])));
		}
	}
	else if (W.type() == MAT_Tdouble)
	{
		Matd X(3, 1);
		Matd Z(3, 1);
		for (int i = 0; i<src.length(); i++)
		{
			X[0][0] = (float)src[i].x; X[1][0] = (float)src[i].y; X[2][0] = 1.0f;
			MatOp::gemm(&W, &X, 1, 0, 0, &Z);
			dst.add(Point2f((float)(Z[0][0] / Z[2][0]), (float)(Z[1][0] / Z[2][0])));
		}
	}
	else
	{
		assert(false);
	}
}
Point2f warpPoint(const Point2f& src, const Mat& W)
{
	vec2f dst;
	if (W.type() == MAT_Tfloat)
	{
		Matf X(3, 1);
		Matf Z(3, 1);
		X[0][0] = (float)src.x; X[1][0] = (float)src.y; X[2][0] = 1.0f;
		MatOp::gemm(&W, &X, 1, 0, 0, &Z);
		dst = Point2f((float)(Z[0][0] / Z[2][0]), (float)(Z[1][0] / Z[2][0]));
	}
	else if (W.type() == MAT_Tdouble)
	{
		Matd X(3, 1);
		Matd Z(3, 1);
		X[0][0] = (float)src.x; X[1][0] = (float)src.y; X[2][0] = 1.0f;
		MatOp::gemm(&W, &X, 1, 0, 0, &Z);
		dst = Point2f((float)(Z[0][0] / Z[2][0]), (float)(Z[1][0] / Z[2][0]));
	}
	else
	{
		assert(false);
	}
	return dst;
}
void drawWarpRect (Mat& image, const Rect& rect, const Mat& W)
{
	Point2i lt, lb, rt, rb;
	
	Matf X(3,1);
	Matf Z(3,1);

	// left-top point
	X[0][0]=(float)rect.x; X[1][0]=(float)rect.y; X[2][0]=1.0f;
	MatOp::gemm(&W, &X, 1, 0, 0, &Z);
	lt.x=(int)Z[0][0], lt.y=(int)Z[1][0];

	// left-bottom point
	X[0][0]=(float)rect.x; X[1][0]=(float)rect.limy(); X[2][0]=1.0f;
	MatOp::gemm(&W, &X, 1, 0, 0, &Z);
	lb.x=(int)Z[0][0], lb.y=(int)Z[1][0];

	// right-top point
	X[0][0]=(float)rect.limx(); X[1][0]=(float)rect.y; X[2][0]=1.0f;
	MatOp::gemm(&W, &X, 1, 0, 0, &Z);
	rt.x=(int)Z[0][0], rt.y=(int)Z[1][0];

	// right-bottom point
	X[0][0]=(float)rect.limx(); X[1][0]=(float)rect.limy(); X[2][0]=1.0f;
	MatOp::gemm(&W, &X, 1, 0, 0, &Z);
	rb.x=(int)Z[0][0], rb.y=(int)Z[1][0];

	// draw rectangle
	image.drawLine (lt, rt, COLOR(255,255,255));
	image.drawLine (rt, rb, COLOR(255,255,255));
	image.drawLine (rb, lb, COLOR(255,255,255));
	image.drawLine (lb, lt, COLOR(255,255,255));
}

/* Calculates coefficients of perspective transformation
 * which maps (xi,yi) to (ui,vi), (i=1,2,3,4):
 *
 *      c00*xi + c01*yi + c02
 * ui = ---------------------
 *      c20*xi + c21*yi + c22
 *
 *      c10*xi + c11*yi + c12
 * vi = ---------------------
 *      c20*xi + c21*yi + c22
 *
 * Coefficients are calculated by solving linear system:
 * / x0 y0  1  0  0  0 -x0*u0 -y0*u0 \ /c00\ /u0\
 * | x1 y1  1  0  0  0 -x1*u1 -y1*u1 | |c01| |u1|
 * | x2 y2  1  0  0  0 -x2*u2 -y2*u2 | |c02| |u2|
 * | x3 y3  1  0  0  0 -x3*u3 -y3*u3 |.|c10|=|u3|,
 * |  0  0  0 x0 y0  1 -x0*v0 -y0*v0 | |c11| |v0|
 * |  0  0  0 x1 y1  1 -x1*v1 -y1*v1 | |c12| |v1|
 * |  0  0  0 x2 y2  1 -x2*v2 -y2*v2 | |c20| |v2|
 * \  0  0  0 x3 y3  1 -x3*v3 -y3*v3 / \c21/ \v3/
 *
 * where:
 *   cij - matrix coefficients, c22 = 1
 */
Mat	getPerspectiveTransform( const Vector<Point2f>& src, const Vector<Point2f>& dst)
{
    double a[8][8], b[8];
    Mat A(a, 8, 8, MAT_Tdouble), B(b, 8, 1, MAT_Tdouble);

    for( int i = 0; i < 4; ++i )
    {
        a[i][0] = a[i+4][3] = src[i].x;
        a[i][1] = a[i+4][4] = src[i].y;
        a[i][2] = a[i+4][5] = 1;
        a[i][3] = a[i][4] = a[i][5] =
        a[i+4][0] = a[i+4][1] = a[i+4][2] = 0;
        a[i][6] = -src[i].x*dst[i].x;
        a[i][7] = -src[i].y*dst[i].x;
        a[i+4][6] = -src[i].x*dst[i].y;
        a[i+4][7] = -src[i].y*dst[i].y;
        b[i] = dst[i].x;
        b[i+4] = dst[i].y;
    }

	LUD lud(&A);
	Mat* pmR=lud.solve(&B);
	Mat M(3,3,MAT_Tdouble);
	if (pmR)
	{
		memcpy (M.data.db[0], pmR->data.db[0], sizeof(double)*8);
		M.data.db[2][2]=1.0;
		delete pmR;
	}
	else
	{
		Mat m2(8, 1, MAT_Tdouble);
		Mat mInverted;
		cvutil::pseudoInvert(A, mInverted);
		MatOp::mul(&m2, &mInverted, &B);
		memcpy(M.data.db[0], m2.data.db[0], sizeof(double) * 8);
		M.data.db[2][2] = 1.0;
	}
    return M;
}

class warpPerspectiveInvokerMask : public ParallelLoopBody
{
public:
	warpPerspectiveInvokerMask(const Mat &_src, Mat &_dst, const float* prT) :
		ParallelLoopBody(), src(_src), dst(_dst), M(prT)
	{
	}

	virtual void operator() (const Range& range) const
	{
		int width = dst.cols();
		int nsrcw = src.cols(), nsrch = src.rows();
		int x, y;

		{
			int s = 1024;
			int limx = (nsrcw - 1)*s;
			int limy = (nsrch - 1)*s;
			float y1 = M[2] + M[1] * range.start;
			float y4 = M[5] + M[4] * range.start;
			float y7 = M[8] + M[7] * range.start;
			uchar** ppdst = (uchar**)dst.data.ptr;
			for (y = range.start; y < range.end; y++)
			{
				float X0 = y1;
				float Y0 = y4;
				float W0 = y7;
				for (x = 0; x < width; x++)
				{
					float W1 = W0 ? 1.f / W0 : 0;
					int rX = (int)(X0*W1*s);
					int rY = (int)(Y0*W1*s);
					if (rX >= 0 && rX<limx && rY >= 0 && rY<limy)
						ppdst[y][x] = 0;
					else
						ppdst[y][x] = 255;
					X0 += M[0];
					Y0 += M[3];
					W0 += M[6];
				}
				y1 += M[1];
				y4 += M[4];
				y7 += M[7];
			}
		}
	}

private:
	const Mat& src;
	Mat& dst;
	const float *M;
	RETMode retMode;
};

void warpPerspectiveMask(const Mat& src, Mat& dst, const Mat& m)
{
	assert(dst.channels() == 1);
	dst.zero();

	Mat minverse = m.inverted();
	const float aT[9] = {
		(float)minverse.data.db[0][0], (float)minverse.data.db[0][1], (float)minverse.data.db[0][2],
		(float)minverse.data.db[1][0], (float)minverse.data.db[1][1], (float)minverse.data.db[1][2],
		(float)minverse.data.db[2][0], (float)minverse.data.db[2][1], (float)minverse.data.db[2][2] };

	warpPerspectiveInvokerMask warper(src, dst, aT);
	parallel_for_(Range(0, dst.rows()), warper);
}

void warpPerspective(const Mat& src, Mat& dst, const Mat& m)
{
	assert (src.channels() == dst.channels());

	BilinearInterpolationFunc func=getBilinearInterpolationFunc((int)src.type());
	//dst.zero();

	int cn=dst.channels();
	Mat minverse=m.inverted();
	const double* M=minverse.data.db[0];
	int height=dst.rows();
	int width=dst.cols();
	int nsrcw=src.cols(), nsrch=src.rows();
    int x, y;
	/*
    for( y = 0; y < height; y ++ )
    {
        for( x = 0; x < width; x ++ )
        {
            double X0 = M[0]*x + M[1]*y + M[2];
            double Y0 = M[3]*x + M[4]*y + M[5];
            double W0 = M[6]*x + M[7]*y + M[8];
            W0 = W0 ? 1./W0 : 0;
            double fX = MAX((double)INT_MIN, MIN((double)INT_MAX, X0*W0));
            double fY = MAX((double)INT_MIN, MIN((double)INT_MAX, Y0*W0));
			if (fX>=0 && fX<nsrcw && fY>=0 && fY<nsrch)
				func (dst.data.ptr, x, y, src.data.ptr, fX, fY, cn);
        }
    }*/
	if (src.type() != MAT_Tuchar)
	{
		double y1 = M[2];
		double y4 = M[5];
		double y7 = M[8];
		for (y = 0; y < height; y++)
		{
			double X0 = y1;
			double Y0 = y4;
			double W0 = y7;
			for (x = 0; x < width; x++)
			{
				double W1 = W0 ? 1. / W0 : 0;
				double fX = MAX((double)INT_MIN, MIN((double)INT_MAX, X0*W1));
				double fY = MAX((double)INT_MIN, MIN((double)INT_MAX, Y0*W1));
				if (fX >= 0 && fX < nsrcw - 1 && fY >= 0 && fY < nsrch - 1)
					func(dst.data.ptr, x, y, src.data.ptr, (float)fX, (float)fY, cn);
				X0 += M[0];
				Y0 += M[3];
				W0 += M[6];
			}
			y1 += M[1];
			y4 += M[4];
			y7 += M[7];
		}
	}
	else
	{
		int bit = 10;
		int s = 1024;
		int s2 = 1024 * 1024;
		int limx = (nsrcw - 1)*s;
		int limy = (nsrch - 1)*s;
		float y1 = (float)M[2];
		float y4 = (float)M[5];
		float y7 = (float)M[8];
		uchar** const ppsrc = (uchar** const)src.data.ptr;
		uchar** ppdst = (uchar**)dst.data.ptr;
		for (y = 0; y < height; y++)
		{
			float X0 = y1;
			float Y0 = y4;
			float W0 = y7;
			for (x = 0; x < width; x++)
			{
				float W1 = W0 ? 1.f / W0 : 0;
				int rX = (int)(X0*W1*s);
				int rY = (int)(Y0*W1*s);
				if (rX >= 0 && rX<limx && rY >= 0 && rY<limy)
				{
					int nX = rX >> bit; rX = rX & 0x3FF;
					int nY = rY >> bit; rY = rY & 0x3FF;
					int n = nX*cn;
					uchar* prcur = (uchar*)&ppsrc[nY][n];
					uchar* prnext = (uchar*)&ppsrc[nY + 1][n];
					uchar* pdst = (uchar*)&ppdst[y][x*cn];
					int xy = rX*rY;
					int x_y = rX + rY;
					for (int i = 0; i<cn; i++, prcur++, prnext++, pdst++)
					{
						*pdst = (uchar)((xy*(*prcur - prcur[cn] - (*prnext) + prnext[cn]) +
							(s2 - s*x_y)*(*prcur) + s*(rX*prcur[cn] + rY*(*prnext))) / s2);
					}
				}
				X0 += (float)M[0];
				Y0 += (float)M[3];
				W0 += (float)M[6];
			}
			y1 += (float)M[1];
			y4 += (float)M[4];
			y7 += (float)M[7];
		}
	}
}
Mat getAffineTransform ( const Point2f src[], const Point2f dst[])
{
	Vector<Point2f> v1(src, 3, false);
	Vector<Point2f> v2(dst, 3, false);
	return getAffineTransform(v1, v2);
}

Mat getRotationMatrix2D(Point2f center, double angle, double scale)
{
	angle *= CVLIB_PI / 180;
	double alpha = cos(angle)*scale;
	double beta = sin(angle)*scale;

	Mat M(2, 3, MAT_Tdouble);
	double* m = M.data.db[0];

	m[0] = alpha;
	m[1] = beta;
	m[2] = (1 - alpha)*center.x - beta*center.y;
	m[3] = -beta;
	m[4] = alpha;
	m[5] = beta*center.x + (1 - alpha)*center.y;

	return M;
}

Mat getAffineTransform ( const Vector<Point2f>& src, const Vector<Point2f>& dst)
{
	int i;
	int _nN=src.getSize();

	Mat mH(2, 3, MAT_Tdouble);
	mH.zero();

	Mat A(2 * _nN, 6, MAT_Tdouble);
	Mat B(2 * _nN, 1, MAT_Tdouble);
	Mat X(6, 1, MAT_Tdouble);

	A.zero();

	for( i = 0; i < _nN; i++ )
	{
		A.data.db[i][0] = src[i].x;
		A.data.db[i+_nN][3] = src[i].x;
		A.data.db[i][1] = src[i].y;
		A.data.db[i+_nN][4] = src[i].y;
		A.data.db[i][2] = 1.0;
		A.data.db[i+_nN][5] = 1.0;
		B.data.db[i][0] = dst[i].x ;
		B.data.db[i+_nN][0] = dst[i].y ;
	}

	SingularValueDecomposition SVDecomp(&A);

	int nRow = A.rows();
	int nCol = A.cols();
	int nMin = MIN(nRow, nCol);

	Mat PseudoIMat(nCol, nRow, MAT_Tdouble);
	Mat V(nCol, nCol,  MAT_Tdouble);
	Mat U(nMin, nRow, MAT_Tdouble);
	Mat SingularV(nCol, nMin, MAT_Tdouble);
	Mat matTemp(nCol, nMin, MAT_Tdouble);

	SingularV.zero();
	PseudoIMat.zero();

	Mat* pmU = SVDecomp.GetU();
	Mat* pmV = SVDecomp.GetV();
	for (i = 0; i < nRow; i ++)
	{
		for (int j = 0; j < nMin; j ++)
		{
			U.data.db[j][i] = pmU->data.db[i][j];
		}
	}

	for (i = 0; i < nCol; i ++)
	{
		for (int j = 0; j < nCol; j ++)
		{
			V.data.db[i][j] = pmV->data.db[i][j];
		}
	}

	for (i = 0; i < nCol; i ++)
		SingularV.data.db[i][i] = (float)1 / SVDecomp.singularValues()[i];

	delete pmU;
	delete pmV;

	MatOp::mul(&matTemp, &V, &SingularV);
	MatOp::mul(&PseudoIMat, &matTemp, &U);
	MatOp::mul(&X, &PseudoIMat, &B);

	//////////////////////////////////////////////////////////////////////////
	for (i = 0; i < 6; i++)
	{
		mH.data.db[i / 3][i % 3] = X.data.db[i][0];
	}

	Mat mAffine(3,3,MAT_Tdouble);
	mAffine.zero();
	mAffine.data.db[2][2]=1.0;
	for (i=0; i<6; i++)
		mAffine.data.db[0][i]=mH.data.db[0][i];
	return mAffine;
}

static void BilinearInterpolation2 ( uchar** const ppbImage, uchar* prTemp, int nX, int nY, float rX, float rY, int cn )
{
	for (int k=0; k<cn; k++)
	{
		float rTemp = ppbImage [ nY ][ nX*cn+k ];
		if ( rX )
			rTemp += rX * ( ppbImage [ nY ][ (nX+1)*cn+k ] - ppbImage [ nY ][ nX*cn+k ] );
	
		if ( rY )
			rTemp += rY * ( ppbImage [ nY + 1 ][ nX*cn+k ] - ppbImage [ nY ][ nX*cn+k ] );
	
		if ( rX && rY )
			rTemp += rX * rY * (
			ppbImage [ nY + 1 ][ (nX+1)*cn+k ] + ppbImage [ nY ][  nX*cn+k ] -
			ppbImage [ nY ][ (nX+1)*cn+k ] - ppbImage [ nY + 1 ][  nX*cn+k ] );
		prTemp[k]=(uchar)rTemp;
	}
}

void warpTriangle(const Mat& src, Mat& dst, const Vector<vec2f>& srcTriangle, const Vector<vec2f>& dstTriangle)
{
	vec2f v0 = dstTriangle[0];
	vec2f v1 = dstTriangle[1];
	vec2f v2 = dstTriangle[2];

	float rminx = std::min(std::min(v0.x, v1.x), v2.x);
	float rmaxx = std::max(std::max(v0.x, v1.x), v2.x);
	float rminy = std::min(std::min(v0.y, v1.y), v2.y);
	float rmaxy = std::max(std::max(v0.y, v1.y), v2.y);
	int minx = (int)rminx;
	int miny = (int)rminy;
	int maxx = (int)rmaxx + 1;
	int maxy = (int)rmaxy + 1;
	Size imgsize = src.size();

	// these will be used for barycentric weights computation
	float x1 = v0.x, y1 = v0.y;
	float x2 = v1.x, y2 = v1.y;
	float x3 = v2.x, y3 = v2.y;
	float rS = triangleArea(x1, y1, x2, y2, x3, y3);

	for (int yi = miny; yi <= maxy; yi++)
	{
		for (int xi = minx; xi <= maxx; xi++)
		{
			if (xi < 0 || yi < 0 || xi >= imgsize.width || yi >= imgsize.height)
				continue;
			const float x = static_cast<float>(xi);//+0.5f;
			const float y = static_cast<float>(yi);//+0.5f;

			float rS1 = triangleArea(x, y, x2, y2, x3, y3);
			float rS2 = triangleArea(x, y, x1, y1, x3, y3);
			float rS3 = triangleArea(x, y, x1, y1, x2, y2);

			if ((rS1 + rS2 + rS3) - rS > 0.001f)
				continue;

			// affine barycentric weights
			const float alpha = rS1 / rS;
			const float beta = rS2 / rS;
			const float gamma = rS3 / rS;

			// if pixel (x, y) is inside the triangle or on one of its edges
			if (alpha >= 0 && beta >= 0 && gamma >= 0)
			{
				vec2f vsrc = alpha * srcTriangle[0] + beta * srcTriangle[1] + gamma * srcTriangle[2];

				int srcx = (int)(vsrc.x);
				int srcy = (int)(vsrc.y);
				float rx = vsrc.x - srcx;
				float ry = vsrc.y - srcy;
				//rx = 1.0f-rx;
				//ry = 1.0f-ry;
				int  src_cn = srcx * 3;

				if (srcx < 0 || srcy < 0 || srcx + 1 >= imgsize.width || srcy + 1 >= imgsize.height)
					continue;
				for (int icn = 0; icn < 3; icn++)
				{
					dst.data.ptr[yi][xi * 3 + icn] =
						(1.0f - rx)*(1.0f - ry)*src.data.ptr[srcy][src_cn + icn] + (1.0f - rx)*ry*src.data.ptr[srcy][src_cn + 3 + icn] +
						rx*(1.0f - ry)*src.data.ptr[srcy + 1][src_cn + icn] + rx*ry*src.data.ptr[srcy + 1][src_cn + 3 + icn];
				}
			}
		}
	}

}

void warpMesh (const Mat& src, Mat& dst, Rect& warpingRect, 
	const Point2f* pSrcPoints, const Point2f* pDstPoints, const TriangleModel* pTriangleModel)
{
	int cn = src.channels();
	warpingRect.x = src.cols() - 1;
	warpingRect.y = src.rows() - 1;
	warpingRect.width = 1;
	warpingRect.height = 1;

	float rX1, rY1, rX2, rY2, rX3, rY3;
	float rX, rY;
	
	int nTriangleCount = pTriangleModel->m_TriangleList.getSize();
	int iTri;
	int nLeft, nTop, nRight, nBottom;

	for(iTri = 0; iTri < nTriangleCount; iTri ++)
	{
		Triangle* pTriangle = (Triangle*)pTriangleModel->m_TriangleList.getAt(iTri);
		rX1 = (float)pDstPoints[pTriangle->n1].x, rY1 = (float)pDstPoints[pTriangle->n1].y;
		rX2 = (float)pDstPoints[pTriangle->n2].x, rY2 = (float)pDstPoints[pTriangle->n2].y;
		rX3 = (float)pDstPoints[pTriangle->n3].x, rY3 = (float)pDstPoints[pTriangle->n3].y;
		
		nLeft = (int)(MIN(rX1, MIN(rX2, rX3)));
		nRight = (int)(MAX(rX1, MAX(rX2, rX3)));
		nTop = (int)(MIN(rY1, MIN(rY2, rY3)));
		nBottom = (int)(MAX(rY1, MAX(rY2, rY3)));
		nTop = MAX(0,nTop);
		nLeft = MAX(0,nLeft);
		nTop = MIN(nTop,dst.rows());
		nLeft = MIN(nLeft,dst.cols());
		nBottom = MIN(nBottom,dst.rows()-1);
		nRight = MIN(nRight,dst.cols()-1);
		float rS = triangleArea (rX1, rY1, rX2, rY2, rX3, rY3);
		for (int iY = nTop ; iY <= nBottom ; iY ++)
		{
			uchar* pdst = dst.data.ptr[iY];
			float rS1_1 = (rX3 - rX2) * (iY - rY3);
			float rS2_1 = (rX3 - rX1) * (iY - rY3);
			float rS3_1 = (rX2 - rX1) * (iY - rY2);
			float rS1_2 = rS1_1+(rX3 - nLeft) * (rY3 - rY2);
			float rS2_2 = rS2_1+(rX3 - nLeft) * (rY3 - rY1);
			float rS3_2 = rS3_1+(rX2 - nLeft) * (rY2 - rY1);
			for (int iX = nLeft ; iX <= nRight ; iX ++, rS1_2+=(rY2-rY3), rS2_2+=(rY1-rY3), rS3_2+=(rY1-rY2))
			{
				int nPtX = iX;
				int nPtY = iY;
				rX = (float)iX;
				rY = (float)iY;
				//float rS1 = triangleArea (rX, rY, rX2, rY2, rX3, rY3);
				//float rS2 = triangleArea (rX, rY, rX1, rY1, rX3, rY3);
				//float rS3 = triangleArea (rX, rY, rX1, rY1, rX2, rY2);

				float rS1 = fabs(rS1_2);
				float rS2 = fabs(rS2_2);
				float rS3 = fabs(rS3_2);
				if ((rS1 + rS2 + rS3) - rS > 0.001f)
					continue;
				float rA = 0.1f, rB = 0.5f, rC = 1.0f - rA - rB;
				computeCoefficients (pDstPoints, pTriangle, (float)nPtX, (float)nPtY, &rA, &rB, &rC);
				float rX1_, rY1_, rX2_, rY2_, rX3_, rY3_;
				rX1_ = (float)pSrcPoints[pTriangle->n1].x, rY1_ = (float)pSrcPoints[pTriangle->n1].y;
				rX2_ = (float)pSrcPoints[pTriangle->n2].x, rY2_ = (float)pSrcPoints[pTriangle->n2].y;
				rX3_ = (float)pSrcPoints[pTriangle->n3].x, rY3_ = (float)pSrcPoints[pTriangle->n3].y;
				
				float rWarpX = rX1_ * rA + rX2_ * rB + rX3_ * rC;
				float rWarpY = rY1_ * rA + rY2_ * rB + rY3_ * rC;
				int nWarpX = (int)rWarpX; rWarpX -=  nWarpX;
				int nWarpY = (int)rWarpY; rWarpY -=  nWarpY;

				if (nWarpX < 0 || nWarpY < 0 || nWarpX >= src.cols()-1 || nWarpY >= src.rows()-1)
				{
					pdst[nPtX*cn]=0;
					pdst[nPtX*cn+1]=0;
					pdst[nPtX*cn+2]=0;
					if (cn == 4)
						pdst[nPtX*cn+3] = 0;
				}
				else
				{
					BilinearInterpolation2 (src.data.ptr, &pdst[nPtX*cn], nWarpX, nWarpY, rWarpX, rWarpY, cn);
				}
			}
		}

		warpingRect.x = MIN(warpingRect.x, nLeft);
		warpingRect.y = MIN(warpingRect.y, nTop);
		warpingRect.width = MAX(warpingRect.width, nRight - warpingRect.x + 1);
		warpingRect.height = MAX(warpingRect.height, nBottom - 	warpingRect.y  + 1);
	}
}

static inline int clip(int x, int a, int b)
{
	return x >= a ? (x < b ? x : b-1) : a;
}

/*****************************************************************************/


struct VResizeNoVec
{
	int operator()(const uchar**, uchar*, const uchar*, int) const { return 0; }
};

struct HResizeNoVec
{
	int operator()(const uchar**, uchar**, int, const int*,
		const uchar*, int, int, int, int, int) const {
		return 0;
	}
};

#if CVLIB_SSE2

static int VResizeLinearVec_32s8u_sse2(const uchar** _src, uchar* dst, const uchar* _beta, int width)
{
	const int** src = (const int**)_src;
	const short* beta = (const short*)_beta;
	const int *S0 = src[0], *S1 = src[1];
	int x = 0;
	__m128i b0 = _mm_set1_epi16(beta[0]), b1 = _mm_set1_epi16(beta[1]);
	__m128i delta = _mm_set1_epi16(2);

	if ((((size_t)S0 | (size_t)S1) & 15) == 0)
		for (; x <= width - 16; x += 16)
		{
			__m128i x0, x1, x2, y0, y1, y2;
			x0 = _mm_load_si128((const __m128i*)(S0 + x));
			x1 = _mm_load_si128((const __m128i*)(S0 + x + 4));
			y0 = _mm_load_si128((const __m128i*)(S1 + x));
			y1 = _mm_load_si128((const __m128i*)(S1 + x + 4));
			x0 = _mm_packs_epi32(_mm_srai_epi32(x0, 4), _mm_srai_epi32(x1, 4));
			y0 = _mm_packs_epi32(_mm_srai_epi32(y0, 4), _mm_srai_epi32(y1, 4));

			x1 = _mm_load_si128((const __m128i*)(S0 + x + 8));
			x2 = _mm_load_si128((const __m128i*)(S0 + x + 12));
			y1 = _mm_load_si128((const __m128i*)(S1 + x + 8));
			y2 = _mm_load_si128((const __m128i*)(S1 + x + 12));
			x1 = _mm_packs_epi32(_mm_srai_epi32(x1, 4), _mm_srai_epi32(x2, 4));
			y1 = _mm_packs_epi32(_mm_srai_epi32(y1, 4), _mm_srai_epi32(y2, 4));

			x0 = _mm_adds_epi16(_mm_mulhi_epi16(x0, b0), _mm_mulhi_epi16(y0, b1));
			x1 = _mm_adds_epi16(_mm_mulhi_epi16(x1, b0), _mm_mulhi_epi16(y1, b1));

			x0 = _mm_srai_epi16(_mm_adds_epi16(x0, delta), 2);
			x1 = _mm_srai_epi16(_mm_adds_epi16(x1, delta), 2);
			_mm_storeu_si128((__m128i*)(dst + x), _mm_packus_epi16(x0, x1));
		}
	else
		for (; x <= width - 16; x += 16)
		{
			__m128i x0, x1, x2, y0, y1, y2;
			x0 = _mm_loadu_si128((const __m128i*)(S0 + x));
			x1 = _mm_loadu_si128((const __m128i*)(S0 + x + 4));
			y0 = _mm_loadu_si128((const __m128i*)(S1 + x));
			y1 = _mm_loadu_si128((const __m128i*)(S1 + x + 4));
			x0 = _mm_packs_epi32(_mm_srai_epi32(x0, 4), _mm_srai_epi32(x1, 4));
			y0 = _mm_packs_epi32(_mm_srai_epi32(y0, 4), _mm_srai_epi32(y1, 4));

			x1 = _mm_loadu_si128((const __m128i*)(S0 + x + 8));
			x2 = _mm_loadu_si128((const __m128i*)(S0 + x + 12));
			y1 = _mm_loadu_si128((const __m128i*)(S1 + x + 8));
			y2 = _mm_loadu_si128((const __m128i*)(S1 + x + 12));
			x1 = _mm_packs_epi32(_mm_srai_epi32(x1, 4), _mm_srai_epi32(x2, 4));
			y1 = _mm_packs_epi32(_mm_srai_epi32(y1, 4), _mm_srai_epi32(y2, 4));

			x0 = _mm_adds_epi16(_mm_mulhi_epi16(x0, b0), _mm_mulhi_epi16(y0, b1));
			x1 = _mm_adds_epi16(_mm_mulhi_epi16(x1, b0), _mm_mulhi_epi16(y1, b1));

			x0 = _mm_srai_epi16(_mm_adds_epi16(x0, delta), 2);
			x1 = _mm_srai_epi16(_mm_adds_epi16(x1, delta), 2);
			_mm_storeu_si128((__m128i*)(dst + x), _mm_packus_epi16(x0, x1));
		}

	for (; x < width - 4; x += 4)
	{
		__m128i x0, y0;
		x0 = _mm_srai_epi32(_mm_loadu_si128((const __m128i*)(S0 + x)), 4);
		y0 = _mm_srai_epi32(_mm_loadu_si128((const __m128i*)(S1 + x)), 4);
		x0 = _mm_packs_epi32(x0, x0);
		y0 = _mm_packs_epi32(y0, y0);
		x0 = _mm_adds_epi16(_mm_mulhi_epi16(x0, b0), _mm_mulhi_epi16(y0, b1));
		x0 = _mm_srai_epi16(_mm_adds_epi16(x0, delta), 2);
		x0 = _mm_packus_epi16(x0, x0);
		*(int*)(dst + x) = _mm_cvtsi128_si32(x0);
	}

	return x;
}

struct VResizeLinearVec_32s8u
{
	int operator()(const uchar** _src, uchar* dst, const uchar* _beta, int width) const
	{
		int processed = 0;

		if (checkHardwareSupport(CVLIB_CPU_AVX2))
			processed += VResizeLinearVec_32s8u_avx2(_src, dst, _beta, width);

		if (!processed && checkHardwareSupport(CVLIB_CPU_SSE2))
			processed += VResizeLinearVec_32s8u_sse2(_src, dst, _beta, width);

		return processed;
	}
};

template<int shiftval>
int VResizeLinearVec_32f16_sse2(const uchar** _src, uchar* _dst, const uchar* _beta, int width)
{
	const float** src = (const float**)_src;
	const float* beta = (const float*)_beta;
	const float *S0 = src[0], *S1 = src[1];
	ushort* dst = (ushort*)_dst;
	int x = 0;

	__m128 b0 = _mm_set1_ps(beta[0]), b1 = _mm_set1_ps(beta[1]);
	__m128i preshift = _mm_set1_epi32(shiftval);
	__m128i postshift = _mm_set1_epi16((short)shiftval);

	if ((((size_t)S0 | (size_t)S1) & 15) == 0)
		for (; x <= width - 16; x += 16)
		{
			__m128 x0, x1, y0, y1;
			__m128i t0, t1, t2;
			x0 = _mm_load_ps(S0 + x);
			x1 = _mm_load_ps(S0 + x + 4);
			y0 = _mm_load_ps(S1 + x);
			y1 = _mm_load_ps(S1 + x + 4);

			x0 = _mm_add_ps(_mm_mul_ps(x0, b0), _mm_mul_ps(y0, b1));
			x1 = _mm_add_ps(_mm_mul_ps(x1, b0), _mm_mul_ps(y1, b1));
			t0 = _mm_add_epi32(_mm_cvtps_epi32(x0), preshift);
			t2 = _mm_add_epi32(_mm_cvtps_epi32(x1), preshift);
			t0 = _mm_add_epi16(_mm_packs_epi32(t0, t2), postshift);

			x0 = _mm_load_ps(S0 + x + 8);
			x1 = _mm_load_ps(S0 + x + 12);
			y0 = _mm_load_ps(S1 + x + 8);
			y1 = _mm_load_ps(S1 + x + 12);

			x0 = _mm_add_ps(_mm_mul_ps(x0, b0), _mm_mul_ps(y0, b1));
			x1 = _mm_add_ps(_mm_mul_ps(x1, b0), _mm_mul_ps(y1, b1));
			t1 = _mm_add_epi32(_mm_cvtps_epi32(x0), preshift);
			t2 = _mm_add_epi32(_mm_cvtps_epi32(x1), preshift);
			t1 = _mm_add_epi16(_mm_packs_epi32(t1, t2), postshift);

			_mm_storeu_si128((__m128i*)(dst + x), t0);
			_mm_storeu_si128((__m128i*)(dst + x + 8), t1);
		}
	else
		for (; x <= width - 16; x += 16)
		{
			__m128 x0, x1, y0, y1;
			__m128i t0, t1, t2;
			x0 = _mm_loadu_ps(S0 + x);
			x1 = _mm_loadu_ps(S0 + x + 4);
			y0 = _mm_loadu_ps(S1 + x);
			y1 = _mm_loadu_ps(S1 + x + 4);

			x0 = _mm_add_ps(_mm_mul_ps(x0, b0), _mm_mul_ps(y0, b1));
			x1 = _mm_add_ps(_mm_mul_ps(x1, b0), _mm_mul_ps(y1, b1));
			t0 = _mm_add_epi32(_mm_cvtps_epi32(x0), preshift);
			t2 = _mm_add_epi32(_mm_cvtps_epi32(x1), preshift);
			t0 = _mm_add_epi16(_mm_packs_epi32(t0, t2), postshift);

			x0 = _mm_loadu_ps(S0 + x + 8);
			x1 = _mm_loadu_ps(S0 + x + 12);
			y0 = _mm_loadu_ps(S1 + x + 8);
			y1 = _mm_loadu_ps(S1 + x + 12);

			x0 = _mm_add_ps(_mm_mul_ps(x0, b0), _mm_mul_ps(y0, b1));
			x1 = _mm_add_ps(_mm_mul_ps(x1, b0), _mm_mul_ps(y1, b1));
			t1 = _mm_add_epi32(_mm_cvtps_epi32(x0), preshift);
			t2 = _mm_add_epi32(_mm_cvtps_epi32(x1), preshift);
			t1 = _mm_add_epi16(_mm_packs_epi32(t1, t2), postshift);

			_mm_storeu_si128((__m128i*)(dst + x), t0);
			_mm_storeu_si128((__m128i*)(dst + x + 8), t1);
		}

	for (; x < width - 4; x += 4)
	{
		__m128 x0, y0;
		__m128i t0;
		x0 = _mm_loadu_ps(S0 + x);
		y0 = _mm_loadu_ps(S1 + x);

		x0 = _mm_add_ps(_mm_mul_ps(x0, b0), _mm_mul_ps(y0, b1));
		t0 = _mm_add_epi32(_mm_cvtps_epi32(x0), preshift);
		t0 = _mm_add_epi16(_mm_packs_epi32(t0, t0), postshift);
		_mm_storel_epi64((__m128i*)(dst + x), t0);
	}

	return x;
}

template<int shiftval> struct VResizeLinearVec_32f16
{
	int operator()(const uchar** _src, uchar* _dst, const uchar* _beta, int width) const
	{
		int processed = 0;

		if (checkHardwareSupport(CVLIB_CPU_AVX2))
			processed += VResizeLinearVec_32f16_avx2<shiftval>(_src, _dst, _beta, width);

		if (!processed && checkHardwareSupport(CVLIB_CPU_SSE2))
			processed += VResizeLinearVec_32f16_sse2<shiftval>(_src, _dst, _beta, width);

		return processed;
	}
};

typedef VResizeLinearVec_32f16<SHRT_MIN> VResizeLinearVec_32f16u;
typedef VResizeLinearVec_32f16<0> VResizeLinearVec_32f16s;

static int VResizeLinearVec_32f_sse(const uchar** _src, uchar* _dst, const uchar* _beta, int width)
{
	const float** src = (const float**)_src;
	const float* beta = (const float*)_beta;
	const float *S0 = src[0], *S1 = src[1];
	float* dst = (float*)_dst;
	int x = 0;

	__m128 b0 = _mm_set1_ps(beta[0]), b1 = _mm_set1_ps(beta[1]);

	if ((((size_t)S0 | (size_t)S1) & 15) == 0)
		for (; x <= width - 8; x += 8)
		{
			__m128 x0, x1, y0, y1;
			x0 = _mm_load_ps(S0 + x);
			x1 = _mm_load_ps(S0 + x + 4);
			y0 = _mm_load_ps(S1 + x);
			y1 = _mm_load_ps(S1 + x + 4);

			x0 = _mm_add_ps(_mm_mul_ps(x0, b0), _mm_mul_ps(y0, b1));
			x1 = _mm_add_ps(_mm_mul_ps(x1, b0), _mm_mul_ps(y1, b1));

			_mm_storeu_ps(dst + x, x0);
			_mm_storeu_ps(dst + x + 4, x1);
		}
	else
		for (; x <= width - 8; x += 8)
		{
			__m128 x0, x1, y0, y1;
			x0 = _mm_loadu_ps(S0 + x);
			x1 = _mm_loadu_ps(S0 + x + 4);
			y0 = _mm_loadu_ps(S1 + x);
			y1 = _mm_loadu_ps(S1 + x + 4);

			x0 = _mm_add_ps(_mm_mul_ps(x0, b0), _mm_mul_ps(y0, b1));
			x1 = _mm_add_ps(_mm_mul_ps(x1, b0), _mm_mul_ps(y1, b1));

			_mm_storeu_ps(dst + x, x0);
			_mm_storeu_ps(dst + x + 4, x1);
		}

	return x;
}

struct VResizeLinearVec_32f
{
	int operator()(const uchar** _src, uchar* _dst, const uchar* _beta, int width) const
	{
		int processed = 0;

		if (checkHardwareSupport(CVLIB_CPU_AVX))
			processed += VResizeLinearVec_32f_avx(_src, _dst, _beta, width);

		if (!processed && checkHardwareSupport(CVLIB_CPU_SSE))
			processed += VResizeLinearVec_32f_sse(_src, _dst, _beta, width);

		return processed;
	}
};

static int VResizeCubicVec_32s8u_sse2(const uchar** _src, uchar* dst, const uchar* _beta, int width)
{
	const int** src = (const int**)_src;
	const short* beta = (const short*)_beta;
	const int *S0 = src[0], *S1 = src[1], *S2 = src[2], *S3 = src[3];
	int x = 0;
	float scale = 1.f / (INTER_RESIZE_COEF_SCALE*INTER_RESIZE_COEF_SCALE);
	__m128 b0 = _mm_set1_ps(beta[0] * scale), b1 = _mm_set1_ps(beta[1] * scale),
		b2 = _mm_set1_ps(beta[2] * scale), b3 = _mm_set1_ps(beta[3] * scale);

	if ((((size_t)S0 | (size_t)S1 | (size_t)S2 | (size_t)S3) & 15) == 0)
		for (; x <= width - 8; x += 8)
		{
			__m128i x0, x1, y0, y1;
			__m128 s0, s1, f0, f1;
			x0 = _mm_load_si128((const __m128i*)(S0 + x));
			x1 = _mm_load_si128((const __m128i*)(S0 + x + 4));
			y0 = _mm_load_si128((const __m128i*)(S1 + x));
			y1 = _mm_load_si128((const __m128i*)(S1 + x + 4));

			s0 = _mm_mul_ps(_mm_cvtepi32_ps(x0), b0);
			s1 = _mm_mul_ps(_mm_cvtepi32_ps(x1), b0);
			f0 = _mm_mul_ps(_mm_cvtepi32_ps(y0), b1);
			f1 = _mm_mul_ps(_mm_cvtepi32_ps(y1), b1);
			s0 = _mm_add_ps(s0, f0);
			s1 = _mm_add_ps(s1, f1);

			x0 = _mm_load_si128((const __m128i*)(S2 + x));
			x1 = _mm_load_si128((const __m128i*)(S2 + x + 4));
			y0 = _mm_load_si128((const __m128i*)(S3 + x));
			y1 = _mm_load_si128((const __m128i*)(S3 + x + 4));

			f0 = _mm_mul_ps(_mm_cvtepi32_ps(x0), b2);
			f1 = _mm_mul_ps(_mm_cvtepi32_ps(x1), b2);
			s0 = _mm_add_ps(s0, f0);
			s1 = _mm_add_ps(s1, f1);
			f0 = _mm_mul_ps(_mm_cvtepi32_ps(y0), b3);
			f1 = _mm_mul_ps(_mm_cvtepi32_ps(y1), b3);
			s0 = _mm_add_ps(s0, f0);
			s1 = _mm_add_ps(s1, f1);

			x0 = _mm_cvtps_epi32(s0);
			x1 = _mm_cvtps_epi32(s1);

			x0 = _mm_packs_epi32(x0, x1);
			_mm_storel_epi64((__m128i*)(dst + x), _mm_packus_epi16(x0, x0));
		}
	else
		for (; x <= width - 8; x += 8)
		{
			__m128i x0, x1, y0, y1;
			__m128 s0, s1, f0, f1;
			x0 = _mm_loadu_si128((const __m128i*)(S0 + x));
			x1 = _mm_loadu_si128((const __m128i*)(S0 + x + 4));
			y0 = _mm_loadu_si128((const __m128i*)(S1 + x));
			y1 = _mm_loadu_si128((const __m128i*)(S1 + x + 4));

			s0 = _mm_mul_ps(_mm_cvtepi32_ps(x0), b0);
			s1 = _mm_mul_ps(_mm_cvtepi32_ps(x1), b0);
			f0 = _mm_mul_ps(_mm_cvtepi32_ps(y0), b1);
			f1 = _mm_mul_ps(_mm_cvtepi32_ps(y1), b1);
			s0 = _mm_add_ps(s0, f0);
			s1 = _mm_add_ps(s1, f1);

			x0 = _mm_loadu_si128((const __m128i*)(S2 + x));
			x1 = _mm_loadu_si128((const __m128i*)(S2 + x + 4));
			y0 = _mm_loadu_si128((const __m128i*)(S3 + x));
			y1 = _mm_loadu_si128((const __m128i*)(S3 + x + 4));

			f0 = _mm_mul_ps(_mm_cvtepi32_ps(x0), b2);
			f1 = _mm_mul_ps(_mm_cvtepi32_ps(x1), b2);
			s0 = _mm_add_ps(s0, f0);
			s1 = _mm_add_ps(s1, f1);
			f0 = _mm_mul_ps(_mm_cvtepi32_ps(y0), b3);
			f1 = _mm_mul_ps(_mm_cvtepi32_ps(y1), b3);
			s0 = _mm_add_ps(s0, f0);
			s1 = _mm_add_ps(s1, f1);

			x0 = _mm_cvtps_epi32(s0);
			x1 = _mm_cvtps_epi32(s1);

			x0 = _mm_packs_epi32(x0, x1);
			_mm_storel_epi64((__m128i*)(dst + x), _mm_packus_epi16(x0, x0));
		}

	return x;
}

struct VResizeCubicVec_32s8u
{
	int operator()(const uchar** _src, uchar* dst, const uchar* _beta, int width) const
	{
		int processed = 0;

		if (checkHardwareSupport(CVLIB_CPU_AVX2))
			processed += VResizeCubicVec_32s8u_avx2(_src, dst, _beta, width);

		if (!processed && checkHardwareSupport(CVLIB_CPU_SSE2))
			processed += VResizeCubicVec_32s8u_sse2(_src, dst, _beta, width);

		return processed;
	}
};


template<int shiftval>
int VResizeCubicVec_32f16_sse2(const uchar** _src, uchar* _dst, const uchar* _beta, int width)
{
	const float** src = (const float**)_src;
	const float* beta = (const float*)_beta;
	const float *S0 = src[0], *S1 = src[1], *S2 = src[2], *S3 = src[3];
	ushort* dst = (ushort*)_dst;
	int x = 0;
	__m128 b0 = _mm_set1_ps(beta[0]), b1 = _mm_set1_ps(beta[1]),
		b2 = _mm_set1_ps(beta[2]), b3 = _mm_set1_ps(beta[3]);
	__m128i preshift = _mm_set1_epi32(shiftval);
	__m128i postshift = _mm_set1_epi16((short)shiftval);

	if ((((size_t)S0 | (size_t)S1 | (size_t)S2 | (size_t)S3) & 15) == 0)
		for (; x <= width - 8; x += 8)
		{
			__m128 x0, x1, y0, y1, s0, s1;
			__m128i t0, t1;
			x0 = _mm_load_ps(S0 + x);
			x1 = _mm_load_ps(S0 + x + 4);
			y0 = _mm_load_ps(S1 + x);
			y1 = _mm_load_ps(S1 + x + 4);

			s0 = _mm_mul_ps(x0, b0);
			s1 = _mm_mul_ps(x1, b0);
			y0 = _mm_mul_ps(y0, b1);
			y1 = _mm_mul_ps(y1, b1);
			s0 = _mm_add_ps(s0, y0);
			s1 = _mm_add_ps(s1, y1);

			x0 = _mm_load_ps(S2 + x);
			x1 = _mm_load_ps(S2 + x + 4);
			y0 = _mm_load_ps(S3 + x);
			y1 = _mm_load_ps(S3 + x + 4);

			x0 = _mm_mul_ps(x0, b2);
			x1 = _mm_mul_ps(x1, b2);
			y0 = _mm_mul_ps(y0, b3);
			y1 = _mm_mul_ps(y1, b3);
			s0 = _mm_add_ps(s0, x0);
			s1 = _mm_add_ps(s1, x1);
			s0 = _mm_add_ps(s0, y0);
			s1 = _mm_add_ps(s1, y1);

			t0 = _mm_add_epi32(_mm_cvtps_epi32(s0), preshift);
			t1 = _mm_add_epi32(_mm_cvtps_epi32(s1), preshift);

			t0 = _mm_add_epi16(_mm_packs_epi32(t0, t1), postshift);
			_mm_storeu_si128((__m128i*)(dst + x), t0);
		}
	else
		for (; x <= width - 8; x += 8)
		{
			__m128 x0, x1, y0, y1, s0, s1;
			__m128i t0, t1;
			x0 = _mm_loadu_ps(S0 + x);
			x1 = _mm_loadu_ps(S0 + x + 4);
			y0 = _mm_loadu_ps(S1 + x);
			y1 = _mm_loadu_ps(S1 + x + 4);

			s0 = _mm_mul_ps(x0, b0);
			s1 = _mm_mul_ps(x1, b0);
			y0 = _mm_mul_ps(y0, b1);
			y1 = _mm_mul_ps(y1, b1);
			s0 = _mm_add_ps(s0, y0);
			s1 = _mm_add_ps(s1, y1);

			x0 = _mm_loadu_ps(S2 + x);
			x1 = _mm_loadu_ps(S2 + x + 4);
			y0 = _mm_loadu_ps(S3 + x);
			y1 = _mm_loadu_ps(S3 + x + 4);

			x0 = _mm_mul_ps(x0, b2);
			x1 = _mm_mul_ps(x1, b2);
			y0 = _mm_mul_ps(y0, b3);
			y1 = _mm_mul_ps(y1, b3);
			s0 = _mm_add_ps(s0, x0);
			s1 = _mm_add_ps(s1, x1);
			s0 = _mm_add_ps(s0, y0);
			s1 = _mm_add_ps(s1, y1);

			t0 = _mm_add_epi32(_mm_cvtps_epi32(s0), preshift);
			t1 = _mm_add_epi32(_mm_cvtps_epi32(s1), preshift);

			t0 = _mm_add_epi16(_mm_packs_epi32(t0, t1), postshift);
			_mm_storeu_si128((__m128i*)(dst + x), t0);
		}

	return x;
}

template<int shiftval> struct VResizeCubicVec_32f16
{
	int operator()(const uchar** _src, uchar* _dst, const uchar* _beta, int width) const
	{
		int processed = 0;

		if (checkHardwareSupport(CVLIB_CPU_AVX2))
			processed += VResizeCubicVec_32f16_avx2<shiftval>(_src, _dst, _beta, width);

		if (!processed && checkHardwareSupport(CVLIB_CPU_SSE2))
			processed += VResizeCubicVec_32f16_sse2<shiftval>(_src, _dst, _beta, width);

		return processed;
	}
};

typedef VResizeCubicVec_32f16<SHRT_MIN> VResizeCubicVec_32f16u;
typedef VResizeCubicVec_32f16<0> VResizeCubicVec_32f16s;

static int VResizeCubicVec_32f_sse(const uchar** _src, uchar* _dst, const uchar* _beta, int width)
{
	const float** src = (const float**)_src;
	const float* beta = (const float*)_beta;
	const float *S0 = src[0], *S1 = src[1], *S2 = src[2], *S3 = src[3];
	float* dst = (float*)_dst;
	int x = 0;
	__m128 b0 = _mm_set1_ps(beta[0]), b1 = _mm_set1_ps(beta[1]),
		b2 = _mm_set1_ps(beta[2]), b3 = _mm_set1_ps(beta[3]);

	if ((((size_t)S0 | (size_t)S1 | (size_t)S2 | (size_t)S3) & 15) == 0)
		for (; x <= width - 8; x += 8)
		{
			__m128 x0, x1, y0, y1, s0, s1;
			x0 = _mm_load_ps(S0 + x);
			x1 = _mm_load_ps(S0 + x + 4);
			y0 = _mm_load_ps(S1 + x);
			y1 = _mm_load_ps(S1 + x + 4);

			s0 = _mm_mul_ps(x0, b0);
			s1 = _mm_mul_ps(x1, b0);
			y0 = _mm_mul_ps(y0, b1);
			y1 = _mm_mul_ps(y1, b1);
			s0 = _mm_add_ps(s0, y0);
			s1 = _mm_add_ps(s1, y1);

			x0 = _mm_load_ps(S2 + x);
			x1 = _mm_load_ps(S2 + x + 4);
			y0 = _mm_load_ps(S3 + x);
			y1 = _mm_load_ps(S3 + x + 4);

			x0 = _mm_mul_ps(x0, b2);
			x1 = _mm_mul_ps(x1, b2);
			y0 = _mm_mul_ps(y0, b3);
			y1 = _mm_mul_ps(y1, b3);
			s0 = _mm_add_ps(s0, x0);
			s1 = _mm_add_ps(s1, x1);
			s0 = _mm_add_ps(s0, y0);
			s1 = _mm_add_ps(s1, y1);

			_mm_storeu_ps(dst + x, s0);
			_mm_storeu_ps(dst + x + 4, s1);
		}
	else
		for (; x <= width - 8; x += 8)
		{
			__m128 x0, x1, y0, y1, s0, s1;
			x0 = _mm_loadu_ps(S0 + x);
			x1 = _mm_loadu_ps(S0 + x + 4);
			y0 = _mm_loadu_ps(S1 + x);
			y1 = _mm_loadu_ps(S1 + x + 4);

			s0 = _mm_mul_ps(x0, b0);
			s1 = _mm_mul_ps(x1, b0);
			y0 = _mm_mul_ps(y0, b1);
			y1 = _mm_mul_ps(y1, b1);
			s0 = _mm_add_ps(s0, y0);
			s1 = _mm_add_ps(s1, y1);

			x0 = _mm_loadu_ps(S2 + x);
			x1 = _mm_loadu_ps(S2 + x + 4);
			y0 = _mm_loadu_ps(S3 + x);
			y1 = _mm_loadu_ps(S3 + x + 4);

			x0 = _mm_mul_ps(x0, b2);
			x1 = _mm_mul_ps(x1, b2);
			y0 = _mm_mul_ps(y0, b3);
			y1 = _mm_mul_ps(y1, b3);
			s0 = _mm_add_ps(s0, x0);
			s1 = _mm_add_ps(s1, x1);
			s0 = _mm_add_ps(s0, y0);
			s1 = _mm_add_ps(s1, y1);

			_mm_storeu_ps(dst + x, s0);
			_mm_storeu_ps(dst + x + 4, s1);
		}

	return x;
}

struct VResizeCubicVec_32f
{
	int operator()(const uchar** _src, uchar* _dst, const uchar* _beta, int width) const
	{
		int processed = 0;

		if (checkHardwareSupport(CVLIB_CPU_AVX))
			processed += VResizeCubicVec_32f_avx(_src, _dst, _beta, width);

		if (!processed && checkHardwareSupport(CVLIB_CPU_SSE))
			processed += VResizeCubicVec_32f_sse(_src, _dst, _beta, width);

		return processed;
	}
};

#else

typedef VResizeNoVec VResizeLinearVec_32s8u;
typedef VResizeNoVec VResizeLinearVec_32f16u;
typedef VResizeNoVec VResizeLinearVec_32f16s;
typedef VResizeNoVec VResizeLinearVec_32f;

typedef VResizeNoVec VResizeCubicVec_32s8u;
typedef VResizeNoVec VResizeCubicVec_32f16u;
typedef VResizeNoVec VResizeCubicVec_32f16s;
typedef VResizeNoVec VResizeCubicVec_32f;

#endif

typedef HResizeNoVec HResizeLinearVec_8u32s;
typedef HResizeNoVec HResizeLinearVec_16u32f;
typedef HResizeNoVec HResizeLinearVec_16s32f;
typedef HResizeNoVec HResizeLinearVec_32f;
typedef HResizeNoVec HResizeLinearVec_64f;


template<typename T, typename WT, typename AT, int ONE, class VecOp>
struct HResizeLinear
{
	typedef T value_type;
	typedef WT buf_type;
	typedef AT alpha_type;

	void operator()(const T** src, WT** dst, int count,
		const int* xofs, const AT* alpha,
		int swidth, int dwidth, int cn, int xmin, int xmax) const
	{
		int dx, k;
		VecOp vecOp;

		int dx0 = vecOp((const uchar**)src, (uchar**)dst, count,
			xofs, (const uchar*)alpha, swidth, dwidth, cn, xmin, xmax);

		for (k = 0; k <= count - 2; k++)
		{
			const T *S0 = src[k], *S1 = src[k + 1];
			WT *D0 = dst[k], *D1 = dst[k + 1];
			for (dx = dx0; dx < xmax; dx++)
			{
				int sx = xofs[dx];
				WT a0 = alpha[dx * 2], a1 = alpha[dx * 2 + 1];
				WT t0 = S0[sx] * a0 + S0[sx + cn] * a1;
				WT t1 = S1[sx] * a0 + S1[sx + cn] * a1;
				D0[dx] = t0; D1[dx] = t1;
			}

			for (; dx < dwidth; dx++)
			{
				int sx = xofs[dx];
				D0[dx] = WT(S0[sx] * ONE); D1[dx] = WT(S1[sx] * ONE);
			}
		}

		for (; k < count; k++)
		{
			const T *S = src[k];
			WT *D = dst[k];
			for (dx = 0; dx < xmax; dx++)
			{
				int sx = xofs[dx];
				D[dx] = S[sx] * alpha[dx * 2] + S[sx + cn] * alpha[dx * 2 + 1];
			}

			for (; dx < dwidth; dx++)
				D[dx] = WT(S[xofs[dx]] * ONE);
		}
	}
};


template<typename T, typename WT, typename AT, class CastOp, class VecOp>
struct VResizeLinear
{
	typedef T value_type;
	typedef WT buf_type;
	typedef AT alpha_type;

	void operator()(const WT** src, T* dst, const AT* beta, int width) const
	{
		WT b0 = beta[0], b1 = beta[1];
		const WT *S0 = src[0], *S1 = src[1];
		CastOp castOp;
		VecOp vecOp;

		int x = vecOp((const uchar**)src, (uchar*)dst, (const uchar*)beta, width);
#if CVLIB_ENABLE_UNROLLED
		for (; x <= width - 4; x += 4)
		{
			WT t0, t1;
			t0 = S0[x] * b0 + S1[x] * b1;
			t1 = S0[x + 1] * b0 + S1[x + 1] * b1;
			dst[x] = castOp(t0); dst[x + 1] = castOp(t1);
			t0 = S0[x + 2] * b0 + S1[x + 2] * b1;
			t1 = S0[x + 3] * b0 + S1[x + 3] * b1;
			dst[x + 2] = castOp(t0); dst[x + 3] = castOp(t1);
		}
#endif
		for (; x < width; x++)
			dst[x] = castOp(S0[x] * b0 + S1[x] * b1);
	}
};

template<>
struct VResizeLinear<uchar, int, short, FixedPtCast<int, uchar, INTER_RESIZE_COEF_BITS * 2>, VResizeLinearVec_32s8u>
{
	typedef uchar value_type;
	typedef int buf_type;
	typedef short alpha_type;

	void operator()(const buf_type** src, value_type* dst, const alpha_type* beta, int width) const
	{
		alpha_type b0 = beta[0], b1 = beta[1];
		const buf_type *S0 = src[0], *S1 = src[1];
		VResizeLinearVec_32s8u vecOp;

		int x = vecOp((const uchar**)src, (uchar*)dst, (const uchar*)beta, width);
#if CVLIB_ENABLE_UNROLLED
		for (; x <= width - 4; x += 4)
		{
			dst[x + 0] = uchar((((b0 * (S0[x + 0] >> 4)) >> 16) + ((b1 * (S1[x + 0] >> 4)) >> 16) + 2) >> 2);
			dst[x + 1] = uchar((((b0 * (S0[x + 1] >> 4)) >> 16) + ((b1 * (S1[x + 1] >> 4)) >> 16) + 2) >> 2);
			dst[x + 2] = uchar((((b0 * (S0[x + 2] >> 4)) >> 16) + ((b1 * (S1[x + 2] >> 4)) >> 16) + 2) >> 2);
			dst[x + 3] = uchar((((b0 * (S0[x + 3] >> 4)) >> 16) + ((b1 * (S1[x + 3] >> 4)) >> 16) + 2) >> 2);
		}
#endif
		for (; x < width; x++)
			dst[x] = uchar((((b0 * (S0[x] >> 4)) >> 16) + ((b1 * (S1[x] >> 4)) >> 16) + 2) >> 2);
	}
};


template<typename T, typename WT, typename AT>
struct HResizeCubic
{
	typedef T value_type;
	typedef WT buf_type;
	typedef AT alpha_type;

	void operator()(const T** src, WT** dst, int count,
		const int* xofs, const AT* alpha,
		int swidth, int dwidth, int cn, int xmin, int xmax) const
	{
		for (int k = 0; k < count; k++)
		{
			const T *S = src[k];
			WT *D = dst[k];
			int dx = 0, limit = xmin;
			for (;;)
			{
				for (; dx < limit; dx++, alpha += 4)
				{
					int j, sx = xofs[dx] - cn;
					WT v = 0;
					for (j = 0; j < 4; j++)
					{
						int sxj = sx + j*cn;
						if ((unsigned)sxj >= (unsigned)swidth)
						{
							while (sxj < 0)
								sxj += cn;
							while (sxj >= swidth)
								sxj -= cn;
						}
						v += S[sxj] * alpha[j];
					}
					D[dx] = v;
				}
				if (limit == dwidth)
					break;
				for (; dx < xmax; dx++, alpha += 4)
				{
					int sx = xofs[dx];
					D[dx] = S[sx - cn] * alpha[0] + S[sx] * alpha[1] +
						S[sx + cn] * alpha[2] + S[sx + cn * 2] * alpha[3];
				}
				limit = dwidth;
			}
			alpha -= dwidth * 4;
		}
	}
};


template<typename T, typename WT, typename AT, class CastOp, class VecOp>
struct VResizeCubic
{
	typedef T value_type;
	typedef WT buf_type;
	typedef AT alpha_type;

	void operator()(const WT** src, T* dst, const AT* beta, int width) const
	{
		WT b0 = beta[0], b1 = beta[1], b2 = beta[2], b3 = beta[3];
		const WT *S0 = src[0], *S1 = src[1], *S2 = src[2], *S3 = src[3];
		CastOp castOp;
		VecOp vecOp;

		int x = vecOp((const uchar**)src, (uchar*)dst, (const uchar*)beta, width);
		for (; x < width; x++)
			dst[x] = castOp(S0[x] * b0 + S1[x] * b1 + S2[x] * b2 + S3[x] * b3);
	}
};


template<typename T, typename WT, typename AT>
struct HResizeLanczos4
{
	typedef T value_type;
	typedef WT buf_type;
	typedef AT alpha_type;

	void operator()(const T** src, WT** dst, int count,
		const int* xofs, const AT* alpha,
		int swidth, int dwidth, int cn, int xmin, int xmax) const
	{
		for (int k = 0; k < count; k++)
		{
			const T *S = src[k];
			WT *D = dst[k];
			int dx = 0, limit = xmin;
			for (;;)
			{
				for (; dx < limit; dx++, alpha += 8)
				{
					int j, sx = xofs[dx] - cn * 3;
					WT v = 0;
					for (j = 0; j < 8; j++)
					{
						int sxj = sx + j*cn;
						if ((unsigned)sxj >= (unsigned)swidth)
						{
							while (sxj < 0)
								sxj += cn;
							while (sxj >= swidth)
								sxj -= cn;
						}
						v += S[sxj] * alpha[j];
					}
					D[dx] = v;
				}
				if (limit == dwidth)
					break;
				for (; dx < xmax; dx++, alpha += 8)
				{
					int sx = xofs[dx];
					D[dx] = S[sx - cn * 3] * alpha[0] + S[sx - cn * 2] * alpha[1] +
						S[sx - cn] * alpha[2] + S[sx] * alpha[3] +
						S[sx + cn] * alpha[4] + S[sx + cn * 2] * alpha[5] +
						S[sx + cn * 3] * alpha[6] + S[sx + cn * 4] * alpha[7];
				}
				limit = dwidth;
			}
			alpha -= dwidth * 8;
		}
	}
};


template<typename T, typename WT, typename AT, class CastOp, class VecOp>
struct VResizeLanczos4
{
	typedef T value_type;
	typedef WT buf_type;
	typedef AT alpha_type;

	void operator()(const WT** src, T* dst, const AT* beta, int width) const
	{
		CastOp castOp;
		VecOp vecOp;
		int k, x = vecOp((const uchar**)src, (uchar*)dst, (const uchar*)beta, width);
#if CVLIB_ENABLE_UNROLLED
		for (; x <= width - 4; x += 4)
		{
			WT b = beta[0];
			const WT* S = src[0];
			WT s0 = S[x] * b, s1 = S[x + 1] * b, s2 = S[x + 2] * b, s3 = S[x + 3] * b;

			for (k = 1; k < 8; k++)
			{
				b = beta[k]; S = src[k];
				s0 += S[x] * b; s1 += S[x + 1] * b;
				s2 += S[x + 2] * b; s3 += S[x + 3] * b;
			}

			dst[x] = castOp(s0); dst[x + 1] = castOp(s1);
			dst[x + 2] = castOp(s2); dst[x + 3] = castOp(s3);
		}
#endif
		for (; x < width; x++)
		{
			dst[x] = castOp(src[0][x] * beta[0] + src[1][x] * beta[1] +
				src[2][x] * beta[2] + src[3][x] * beta[3] + src[4][x] * beta[4] +
				src[5][x] * beta[5] + src[6][x] * beta[6] + src[7][x] * beta[7]);
		}
	}
};


static const int MAX_ESIZE = 16;

template <typename HResize, typename VResize>
class resizeGeneric_Invoker :
	public ParallelLoopBody
{
public:
	typedef typename HResize::value_type T;
	typedef typename HResize::buf_type WT;
	typedef typename HResize::alpha_type AT;

	resizeGeneric_Invoker(const Mat& _src, Mat &_dst, const int *_xofs, const int *_yofs,
		const AT* _alpha, const AT* __beta, const Size& _ssize, const Size &_dsize,
		int _ksize, int _xmin, int _xmax) :
		ParallelLoopBody(), src(_src), dst(_dst), xofs(_xofs), yofs(_yofs),
		alpha(_alpha), _beta(__beta), ssize(_ssize), dsize(_dsize),
		ksize(_ksize), xmin(_xmin), xmax(_xmax)
	{
		assert(ksize <= MAX_ESIZE);
	}

#if defined(__GNUC__) && (__GNUC__ == 4) && (__GNUC_MINOR__ == 8)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Warray-bounds"
#endif
	virtual void operator() (const Range& range) const
	{
		int dy, cn = src.channels();
		HResize hresize;
		VResize vresize;

		int bufstep = (int)alignSize(dsize.width, 16);
		AutoBuffer<WT> _buffer(bufstep*ksize);
		const T* srows[MAX_ESIZE] = { 0 };
		WT* rows[MAX_ESIZE] = { 0 };
		int prev_sy[MAX_ESIZE];

		for (int k = 0; k < ksize; k++)
		{
			prev_sy[k] = -1;
			rows[k] = (WT*)_buffer + bufstep*k;
		}

		const AT* beta = _beta + ksize * range.start;

		for (dy = range.start; dy < range.end; dy++, beta += ksize)
		{
			int sy0 = yofs[dy], k0 = ksize, k1 = 0, ksize2 = ksize / 2;

			for (int k = 0; k < ksize; k++)
			{
				int sy = clip(sy0 - ksize2 + 1 + k, 0, ssize.height);
				for (k1 = std::max(k1, k); k1 < ksize; k1++)
				{
					if (sy == prev_sy[k1]) // if the sy-th row has been computed already, reuse it.
					{
						if (k1 > k)
							memcpy(rows[k], rows[k1], bufstep * sizeof(rows[0][0]));
						break;
					}
				}
				if (k1 == ksize)
					k0 = std::min(k0, k); // remember the first row that needs to be computed
				srows[k] = (T*)(src.data.ptr[sy]);
				prev_sy[k] = sy;
			}

			if (k0 < ksize)
				hresize((const T**)(srows + k0), (WT**)(rows + k0), ksize - k0, xofs, (const AT*)(alpha),
					ssize.width, dsize.width, cn, xmin, xmax);
			vresize((const WT**)rows, (T*)(dst.data.ptr[dy]), beta, dsize.width);
		}
	}
#if defined(__GNUC__) && (__GNUC__ == 4) && (__GNUC_MINOR__ == 8)
# pragma GCC diagnostic pop
#endif

private:
	const Mat& src;
	Mat& dst;
	const int* xofs, *yofs;
	const AT* alpha, *_beta;
	Size ssize, dsize;
	const int ksize, xmin, xmax;

	resizeGeneric_Invoker& operator = (const resizeGeneric_Invoker&);
};

template<class HResize, class VResize>
static void resizeGeneric_(const Mat& src, Mat& dst,
	const int* xofs, const void* _alpha,
	const int* yofs, const void* _beta,
	int xmin, int xmax, int ksize)
{
	typedef typename HResize::alpha_type AT;

	const AT* beta = (const AT*)_beta;
	Size ssize = src.size(), dsize = dst.size();
	int cn = src.channels();
	ssize.width *= cn;
	dsize.width *= cn;
	xmin *= cn;
	xmax *= cn;
	// image resize is a separable operation. In case of not too strong

	Range range(0, dsize.height);
	resizeGeneric_Invoker<HResize, VResize> invoker(src, dst, xofs, yofs, (const AT*)_alpha, beta,
		ssize, dsize, ksize, xmin, xmax);
	parallel_for_(range, invoker, dst.total() / (double)(1 << 16));
}

template <typename T, typename WT>
struct ResizeAreaFastNoVec
{
	ResizeAreaFastNoVec(int /*_scale_x*/, int /*_scale_y*/,
		int /*_cn*/, int /*_step*//*, const int**/ /*_ofs*/) { }
	int operator() (const T* /*S*/, T* /*D*/, int /*w*/) const { return 0; }
};

template<typename T>
struct ResizeAreaFastVec
{
	ResizeAreaFastVec(int _scale_x, int _scale_y, int _cn, int _step/*, const int* _ofs*/) :
		scale_x(_scale_x), scale_y(_scale_y), cn(_cn), step(_step)/*, ofs(_ofs)*/
	{
		fast_mode = scale_x == 2 && scale_y == 2 && (cn == 1 || cn == 3 || cn == 4);
	}

	int operator() (const T* S, T* D, int w) const
	{
		if (!fast_mode)
			return 0;

		const T* nextS = (const T*)((const uchar*)S + step);
		int dx = 0;

		if (cn == 1)
			for (; dx < w; ++dx)
			{
				int index = dx * 2;
				D[dx] = (T)((S[index] + S[index + 1] + nextS[index] + nextS[index + 1] + 2) >> 2);
			}
		else if (cn == 3)
			for (; dx < w; dx += 3)
			{
				int index = dx * 2;
				D[dx] = (T)((S[index] + S[index + 3] + nextS[index] + nextS[index + 3] + 2) >> 2);
				D[dx + 1] = (T)((S[index + 1] + S[index + 4] + nextS[index + 1] + nextS[index + 4] + 2) >> 2);
				D[dx + 2] = (T)((S[index + 2] + S[index + 5] + nextS[index + 2] + nextS[index + 5] + 2) >> 2);
			}
		else
		{
			assert(cn == 4);
			for (; dx < w; dx += 4)
			{
				int index = dx * 2;
				D[dx] = (T)((S[index] + S[index + 4] + nextS[index] + nextS[index + 4] + 2) >> 2);
				D[dx + 1] = (T)((S[index + 1] + S[index + 5] + nextS[index + 1] + nextS[index + 5] + 2) >> 2);
				D[dx + 2] = (T)((S[index + 2] + S[index + 6] + nextS[index + 2] + nextS[index + 6] + 2) >> 2);
				D[dx + 3] = (T)((S[index + 3] + S[index + 7] + nextS[index + 3] + nextS[index + 7] + 2) >> 2);
			}
		}

		return dx;
	}

private:
	int scale_x, scale_y;
	int cn;
	bool fast_mode;
	int step;
};

template <typename T, typename WT, typename VecOp>
class resizeAreaFast_Invoker :
	public ParallelLoopBody
{
public:
	resizeAreaFast_Invoker(const Mat &_src, Mat &_dst,
		int _scale_x, int _scale_y, const int* _ofs, const int* _xofs) :
		ParallelLoopBody(), src(_src), dst(_dst), scale_x(_scale_x),
		scale_y(_scale_y), ofs(_ofs), xofs(_xofs)
	{
	}

	virtual void operator() (const Range& range) const
	{
		Size ssize = src.size(), dsize = dst.size();
		int cn = src.channels();
		int area = scale_x*scale_y;
		float scale = 1.f / (area);
		int dwidth1 = (ssize.width / scale_x)*cn;
		dsize.width *= cn;
		ssize.width *= cn;
		int dy, dx, k = 0;

		VecOp vop(scale_x, scale_y, src.channels(), (int)(src.data.ptr[1] - src.data.ptr[0])/*, area_ofs*/);

		for (dy = range.start; dy < range.end; dy++)
		{
			T* D = (T*)(dst.data.ptr[dy]);
			int sy0 = dy*scale_y;
			int w = sy0 + scale_y <= ssize.height ? dwidth1 : 0;

			if (sy0 >= ssize.height)
			{
				for (dx = 0; dx < dsize.width; dx++)
					D[dx] = 0;
				continue;
			}

			dx = vop((const T*)(src.data.ptr[sy0]), D, w);
			for (; dx < w; dx++)
			{
				const T* S = (const T*)(src.data.ptr[sy0]) + xofs[dx];
				WT sum = 0;
				k = 0;
#if CVLIB_ENABLE_UNROLLED
				for (; k <= area - 4; k += 4)
					sum += S[ofs[k]] + S[ofs[k + 1]] + S[ofs[k + 2]] + S[ofs[k + 3]];
#endif
				for (; k < area; k++)
					sum += S[ofs[k]];

				D[dx] = saturate_cast<T>(sum * scale);
			}

			for (; dx < dsize.width; dx++)
			{
				WT sum = 0;
				int count = 0, sx0 = xofs[dx];
				if (sx0 >= ssize.width)
					D[dx] = 0;

				for (int sy = 0; sy < scale_y; sy++)
				{
					if (sy0 + sy >= ssize.height)
						break;
					const T* S = (const T*)(src.data.ptr[sy0 + sy]) + sx0;
					for (int sx = 0; sx < scale_x*cn; sx += cn)
					{
						if (sx0 + sx >= ssize.width)
							break;
						sum += S[sx];
						count++;
					}
				}

				D[dx] = saturate_cast<T>((float)sum / count);
			}
		}
	}

private:
	const Mat& src;
	Mat& dst;
	int scale_x, scale_y;
	const int *ofs, *xofs;
};

template<typename T, typename WT, typename VecOp>
static void resizeAreaFast_(const Mat& src, Mat& dst, const int* ofs, const int* xofs,
	int scale_x, int scale_y)
{
	Range range(0, dst.rows());
	resizeAreaFast_Invoker<T, WT, VecOp> invoker(src, dst, scale_x,
		scale_y, ofs, xofs);
	parallel_for_(range, invoker, dst.total() / (double)(1 << 16));
}

struct DecimateAlpha
{
	int si, di;
	float alpha;
};


template<typename T, typename WT> class ResizeArea_Invoker :
	public ParallelLoopBody
{
public:
	ResizeArea_Invoker(const Mat& _src, Mat& _dst,
		const DecimateAlpha* _xtab, int _xtab_size,
		const DecimateAlpha* _ytab, int _ytab_size,
		const int* _tabofs)
	{
		src = &_src;
		dst = &_dst;
		xtab0 = _xtab;
		xtab_size0 = _xtab_size;
		ytab = _ytab;
		ytab_size = _ytab_size;
		tabofs = _tabofs;
	}

	virtual void operator() (const Range& range) const
	{
		Size dsize = dst->size();
		int cn = dst->channels();
		dsize.width *= cn;
		AutoBuffer<WT> _buffer(dsize.width * 2);
		const DecimateAlpha* xtab = xtab0;
		int xtab_size = xtab_size0;
		WT *buf = _buffer, *sum = buf + dsize.width;
		int j_start = tabofs[range.start], j_end = tabofs[range.end], j, k, dx, prev_dy = ytab[j_start].di;

		for (dx = 0; dx < dsize.width; dx++)
			sum[dx] = (WT)0;

		for (j = j_start; j < j_end; j++)
		{
			WT beta = ytab[j].alpha;
			int dy = ytab[j].di;
			int sy = ytab[j].si;

			{
				const T* S = (const T*)(src->data.ptr[sy]);
				for (dx = 0; dx < dsize.width; dx++)
					buf[dx] = (WT)0;

				if (cn == 1)
					for (k = 0; k < xtab_size; k++)
					{
						int dxn = xtab[k].di;
						WT alpha = xtab[k].alpha;
						buf[dxn] += S[xtab[k].si] * alpha;
					}
				else if (cn == 2)
					for (k = 0; k < xtab_size; k++)
					{
						int sxn = xtab[k].si;
						int dxn = xtab[k].di;
						WT alpha = xtab[k].alpha;
						WT t0 = buf[dxn] + S[sxn] * alpha;
						WT t1 = buf[dxn + 1] + S[sxn + 1] * alpha;
						buf[dxn] = t0; buf[dxn + 1] = t1;
					}
				else if (cn == 3)
					for (k = 0; k < xtab_size; k++)
					{
						int sxn = xtab[k].si;
						int dxn = xtab[k].di;
						WT alpha = xtab[k].alpha;
						WT t0 = buf[dxn] + S[sxn] * alpha;
						WT t1 = buf[dxn + 1] + S[sxn + 1] * alpha;
						WT t2 = buf[dxn + 2] + S[sxn + 2] * alpha;
						buf[dxn] = t0; buf[dxn + 1] = t1; buf[dxn + 2] = t2;
					}
				else if (cn == 4)
				{
					for (k = 0; k < xtab_size; k++)
					{
						int sxn = xtab[k].si;
						int dxn = xtab[k].di;
						WT alpha = xtab[k].alpha;
						WT t0 = buf[dxn] + S[sxn] * alpha;
						WT t1 = buf[dxn + 1] + S[sxn + 1] * alpha;
						buf[dxn] = t0; buf[dxn + 1] = t1;
						t0 = buf[dxn + 2] + S[sxn + 2] * alpha;
						t1 = buf[dxn + 3] + S[sxn + 3] * alpha;
						buf[dxn + 2] = t0; buf[dxn + 3] = t1;
					}
				}
				else
				{
					for (k = 0; k < xtab_size; k++)
					{
						int sxn = xtab[k].si;
						int dxn = xtab[k].di;
						WT alpha = xtab[k].alpha;
						for (int c = 0; c < cn; c++)
							buf[dxn + c] += S[sxn + c] * alpha;
					}
				}
			}

			if (dy != prev_dy)
			{
				T* D = (T*)(dst->data.ptr[prev_dy]);

				for (dx = 0; dx < dsize.width; dx++)
				{
					D[dx] = saturate_cast<T>(sum[dx]);
					sum[dx] = beta*buf[dx];
				}
				prev_dy = dy;
			}
			else
			{
				for (dx = 0; dx < dsize.width; dx++)
					sum[dx] += beta*buf[dx];
			}
		}

		{
			T* D = (T*)(dst->data.ptr[prev_dy]);
			for (dx = 0; dx < dsize.width; dx++)
				D[dx] = saturate_cast<T>(sum[dx]);
		}
	}

private:
	const Mat* src;
	Mat* dst;
	const DecimateAlpha* xtab0;
	const DecimateAlpha* ytab;
	int xtab_size0, ytab_size;
	const int* tabofs;
};


template <typename T, typename WT>
static void resizeArea_(const Mat& src, Mat& dst,
	const DecimateAlpha* xtab, int xtab_size,
	const DecimateAlpha* ytab, int ytab_size,
	const int* tabofs)
{
	parallel_for_(Range(0, dst.rows()),
		ResizeArea_Invoker<T, WT>(src, dst, xtab, xtab_size, ytab, ytab_size, tabofs),
		dst.total() / ((double)(1 << 16)));
}


typedef void(*ResizeFunc)(const Mat& src, Mat& dst,
	const int* xofs, const void* alpha,
	const int* yofs, const void* beta,
	int xmin, int xmax, int ksize);

typedef void(*ResizeAreaFastFunc)(const Mat& src, Mat& dst,
	const int* ofs, const int *xofs,
	int scale_x, int scale_y);

typedef void(*ResizeAreaFunc)(const Mat& src, Mat& dst,
	const DecimateAlpha* xtab, int xtab_size,
	const DecimateAlpha* ytab, int ytab_size,
	const int* yofs);


static int computeResizeAreaTab(int ssize, int dsize, int cn, double scale, DecimateAlpha* tab)
{
	int k = 0;
	for (int dx = 0; dx < dsize; dx++)
	{
		double fsx1 = dx * scale;
		double fsx2 = fsx1 + scale;
		double cellWidth = std::min(scale, ssize - fsx1);

		int sx1 = cvutil::ceil(fsx1), sx2 = cvutil::floor(fsx2);

		sx2 = std::min(sx2, ssize - 1);
		sx1 = std::min(sx1, sx2);

		if (sx1 - fsx1 > 1e-3)
		{
			assert(k < ssize * 2);
			tab[k].di = dx * cn;
			tab[k].si = (sx1 - 1) * cn;
			tab[k++].alpha = (float)((sx1 - fsx1) / cellWidth);
		}

		for (int sx = sx1; sx < sx2; sx++)
		{
			assert(k < ssize * 2);
			tab[k].di = dx * cn;
			tab[k].si = sx * cn;
			tab[k++].alpha = float(1.0 / cellWidth);
		}

		if (fsx2 - sx2 > 1e-3)
		{
			assert(k < ssize * 2);
			tab[k].di = dx * cn;
			tab[k].si = sx2 * cn;
			tab[k++].alpha = (float)(std::min(std::min(fsx2 - sx2, 1.), cellWidth) / cellWidth);
		}
	}
	return k;
}

#if defined (HAVE_IPP) && (IPP_VERSION_MAJOR >= 7)
class IPPresizeInvoker :
	public ParallelLoopBody
{
public:
	IPPresizeInvoker(Mat &_src, Mat &_dst, double &_inv_scale_x, double &_inv_scale_y, int _mode, ippiResizeSqrPixelFunc _func, bool *_ok) :
		ParallelLoopBody(), src(_src), dst(_dst), inv_scale_x(_inv_scale_x), inv_scale_y(_inv_scale_y), mode(_mode), func(_func), ok(_ok)
	{
		*ok = true;
	}

	virtual void operator() (const Range& range) const
	{
		int cn = src.channels();
		IppiRect srcroi = { 0, range.start, src.cols, range.end - range.start };
		int dsty = CV_IMIN(cvRound(range.start * inv_scale_y), dst.rows);
		int dstwidth = CV_IMIN(cvRound(src.cols * inv_scale_x), dst.cols);
		int dstheight = CV_IMIN(cvRound(range.end * inv_scale_y), dst.rows);
		IppiRect dstroi = { 0, dsty, dstwidth, dstheight - dsty };
		int bufsize;
		ippiResizeGetBufSize(srcroi, dstroi, cn, mode, &bufsize);
		AutoBuffer<uchar> buf(bufsize + 64);
		uchar* bufptr = alignPtr((uchar*)buf, 32);
		if (func(src.data, ippiSize(src.cols, src.rows), (int)src.step[0], srcroi, dst.data, (int)dst.step[0], dstroi, inv_scale_x, inv_scale_y, 0, 0, mode, bufptr) < 0)
			*ok = false;
	}
private:
	Mat &src;
	Mat &dst;
	double inv_scale_x;
	double inv_scale_y;
	int mode;
	ippiResizeSqrPixelFunc func;
	bool *ok;
	const IPPresizeInvoker& operator= (const IPPresizeInvoker&);
};
#endif


typedef void(*ResizeFunc)(const Mat& src, Mat& dst, const int* xofs, const void* alpha,
	const int* yofs, const void* beta, int xmin, int xmax, int ksize);

void image_resize(const Mat& src, Mat& dst, Size dsize, double inv_scale_x, double inv_scale_y, int interpolation)
{
	static ResizeFunc linear_tab[] =
	{
		resizeGeneric_<
		HResizeLinear<uchar, int, short, INTER_RESIZE_COEF_SCALE, HResizeLinearVec_8u32s>,
		VResizeLinear<uchar, int, short, FixedPtCast<int, uchar, INTER_RESIZE_COEF_BITS * 2>, VResizeLinearVec_32s8u> >,
		resizeGeneric_<
		HResizeLinear<uchar, int, short, INTER_RESIZE_COEF_SCALE, HResizeLinearVec_8u32s>,
		VResizeLinear<uchar, int, short, FixedPtCast<int, uchar, INTER_RESIZE_COEF_BITS * 2>, VResizeLinearVec_32s8u> >,
		resizeGeneric_<
		HResizeLinear<short, float, float, 1, HResizeLinearVec_16s32f>,
		VResizeLinear<short, float, float, Cast<float, short>, VResizeLinearVec_32f16s> >,
		0,
		resizeGeneric_<
		HResizeLinear<float, float, float, 1, HResizeLinearVec_32f>,
		VResizeLinear<float, float, float, Cast<float, float>, VResizeLinearVec_32f> >,
		resizeGeneric_<
		HResizeLinear<double, double, float, 1, HResizeNoVec>,
		VResizeLinear<double, double, float, Cast<double, double>, VResizeNoVec> >,
		0
	};

	static ResizeFunc cubic_tab[] =
	{
		resizeGeneric_<
		HResizeCubic<uchar, int, short>,
		VResizeCubic<uchar, int, short, FixedPtCast<int, uchar, INTER_RESIZE_COEF_BITS * 2>, VResizeCubicVec_32s8u> >,
		resizeGeneric_<
		HResizeCubic<uchar, int, short>,
		VResizeCubic<uchar, int, short, FixedPtCast<int, uchar, INTER_RESIZE_COEF_BITS * 2>, VResizeCubicVec_32s8u> >,
		resizeGeneric_<
		HResizeCubic<short, float, float>,
		VResizeCubic<short, float, float, Cast<float, short>, VResizeCubicVec_32f16s> >,
		0,
		resizeGeneric_<
		HResizeCubic<float, float, float>,
		VResizeCubic<float, float, float, Cast<float, float>, VResizeCubicVec_32f> >,
		resizeGeneric_<
		HResizeCubic<double, double, float>,
		VResizeCubic<double, double, float, Cast<double, double>, VResizeNoVec> >,
		0
	};

	static ResizeFunc lanczos4_tab[] =
	{
		resizeGeneric_<HResizeLanczos4<uchar, int, short>,
		VResizeLanczos4<uchar, int, short, FixedPtCast<int, uchar, INTER_RESIZE_COEF_BITS * 2>, VResizeNoVec> >,
		resizeGeneric_<HResizeLanczos4<uchar, int, short>,
		VResizeLanczos4<uchar, int, short, FixedPtCast<int, uchar, INTER_RESIZE_COEF_BITS * 2>, VResizeNoVec> >,
		resizeGeneric_<HResizeLanczos4<short, float, float>,
		VResizeLanczos4<short, float, float, Cast<float, short>, VResizeNoVec> >,
		0,
		resizeGeneric_<HResizeLanczos4<float, float, float>,
		VResizeLanczos4<float, float, float, Cast<float, float>, VResizeNoVec> >,
		resizeGeneric_<HResizeLanczos4<double, double, float>,
		VResizeLanczos4<double, double, float, Cast<double, double>, VResizeNoVec> >,
		0
	};

	static ResizeAreaFastFunc areafast_tab[] =
	{
		resizeAreaFast_<uchar, int, ResizeAreaFastVec<uchar> >,
		resizeAreaFast_<uchar, int, ResizeAreaFastVec<uchar> >,
		resizeAreaFast_<short, float, ResizeAreaFastVec<short> >,
		0,
		resizeAreaFast_<float, float, ResizeAreaFastNoVec<float, float> >,
		resizeAreaFast_<double, double, ResizeAreaFastNoVec<double, double> >,
		0
	};

	static ResizeAreaFunc area_tab[] =
	{
		resizeArea_<uchar, float>, resizeArea_<uchar, float>, 
		resizeArea_<short, float>, 0, resizeArea_<float, float>,
		resizeArea_<double, double>, 0
	};

	Size ssize = src.size();

	assert(ssize.area() > 0);
	assert(dsize.area() || (inv_scale_x > 0 && inv_scale_y > 0));
	if (!dsize.area())
	{
		dsize = Size(saturate_cast<int>(src.cols()*inv_scale_x),
			saturate_cast<int>(src.rows()*inv_scale_y));
		assert(dsize.area());
	}
	else
	{
		inv_scale_x = (double)dsize.width / src.cols();
		inv_scale_y = (double)dsize.height / src.rows();
	}
	dst.create(dsize, src.type1());

#ifdef HAVE_TEGRA_OPTIMIZATION
	if (tegra::resize(src, dst, (float)inv_scale_x, (float)inv_scale_y, interpolation))
		return;
#endif

	TYPE depth = src.type();
	int cn = src.channels();
	double scale_x = 1. / inv_scale_x, scale_y = 1. / inv_scale_y;
	int k, sx, sy, dx, dy;
	/*
	#if defined (HAVE_IPP) && (IPP_VERSION_MAJOR >= 7)
	int mode = interpolation == INTER_LINEAR ? IPPI_INTER_LINEAR : 0;
	int type = src.type();
	ippiResizeSqrPixelFunc ippFunc =
	type == CV_8UC1 ? (ippiResizeSqrPixelFunc)ippiResizeSqrPixel_8u_C1R :
	type == CV_8UC3 ? (ippiResizeSqrPixelFunc)ippiResizeSqrPixel_8u_C3R :
	type == CV_8UC4 ? (ippiResizeSqrPixelFunc)ippiResizeSqrPixel_8u_C4R :
	type == CV_16UC1 ? (ippiResizeSqrPixelFunc)ippiResizeSqrPixel_16u_C1R :
	type == CV_16UC3 ? (ippiResizeSqrPixelFunc)ippiResizeSqrPixel_16u_C3R :
	type == CV_16UC4 ? (ippiResizeSqrPixelFunc)ippiResizeSqrPixel_16u_C4R :
	type == CV_16SC1 ? (ippiResizeSqrPixelFunc)ippiResizeSqrPixel_16s_C1R :
	type == CV_16SC3 ? (ippiResizeSqrPixelFunc)ippiResizeSqrPixel_16s_C3R :
	type == CV_16SC4 ? (ippiResizeSqrPixelFunc)ippiResizeSqrPixel_16s_C4R :
	type == CV_32FC1 ? (ippiResizeSqrPixelFunc)ippiResizeSqrPixel_32f_C1R :
	type == CV_32FC3 ? (ippiResizeSqrPixelFunc)ippiResizeSqrPixel_32f_C3R :
	type == CV_32FC4 ? (ippiResizeSqrPixelFunc)ippiResizeSqrPixel_32f_C4R :
	0;
	if( ippFunc && mode != 0 )
	{
	bool ok;
	Range range(0, src.rows);
	IPPresizeInvoker invoker(src, dst, inv_scale_x, inv_scale_y, mode, ippFunc, &ok);
	parallel_for_(range, invoker, dst.total()/(double)(1<<16));
	if( ok )
	return;
	}
	#endif
	*/
	if (interpolation == INTER_NEAREST)
	{
		//resizeNN(src, dst, inv_scale_x, inv_scale_y);
		return;
	}

	{
		int iscale_x = saturate_cast<int>(scale_x);
		int iscale_y = saturate_cast<int>(scale_y);

		bool is_area_fast = std::abs(scale_x - iscale_x) < DBL_EPSILON &&
			std::abs(scale_y - iscale_y) < DBL_EPSILON;

		// in case of scale_x && scale_y is equal to 2
		// INTER_AREA (fast) also is equal to INTER_LINEAR
		if (interpolation == INTER_LINEAR && is_area_fast && iscale_x == 2 && iscale_y == 2)
		{
			interpolation = INTER_AREA;
		}

		// true "area" interpolation is only implemented for the case (scale_x <= 1 && scale_y <= 1).
		// In other cases it is emulated using some variant of bilinear interpolation
		if (interpolation == INTER_AREA && scale_x >= 1 && scale_y >= 1)
		{
			if (is_area_fast)
			{
				int area = iscale_x*iscale_y;
				size_t srcstep = src.cols()*cn;
				AutoBuffer<int> _ofs(area + dsize.width*cn);
				int* ofs = _ofs;
				int* xofs = ofs + area;
				ResizeAreaFastFunc func = areafast_tab[depth];
				assert(func != 0);

				for (sy = 0, k = 0; sy < iscale_y; sy++)
					for (sx = 0; sx < iscale_x; sx++)
						ofs[k++] = (int)(sy*srcstep + sx*cn);

				for (dx = 0; dx < dsize.width; dx++)
				{
					int j = dx * cn;
					sx = iscale_x * j;
					for (k = 0; k < cn; k++)
						xofs[j + k] = sx + k;
				}

				func(src, dst, ofs, xofs, iscale_x, iscale_y);
				return;
			}

			ResizeAreaFunc func = area_tab[depth];
			assert(func != 0 && cn <= 4);

			AutoBuffer<DecimateAlpha> _xytab((ssize.width + ssize.height) * 2);
			DecimateAlpha* xtab = _xytab, *ytab = xtab + ssize.width * 2;

			int xtab_size = computeResizeAreaTab(ssize.width, dsize.width, cn, scale_x, xtab);
			int ytab_size = computeResizeAreaTab(ssize.height, dsize.height, 1, scale_y, ytab);

			AutoBuffer<int> _tabofs(dsize.height + 1);
			int* tabofs = _tabofs;
			for (k = 0, dy = 0; k < ytab_size; k++)
			{
				if (k == 0 || ytab[k].di != ytab[k - 1].di)
				{
					assert(ytab[k].di == dy);
					tabofs[dy++] = k;
				}
			}
			tabofs[dy] = ytab_size;

			func(src, dst, xtab, xtab_size, ytab, ytab_size, tabofs);
			return;
		}
	}

	int xmin = 0, xmax = dsize.width, width = dsize.width*cn;
	bool area_mode = interpolation == INTER_AREA;
	bool fixpt = depth == MAT_Tuchar;
	float fx, fy;
	ResizeFunc func = 0;
	int ksize = 0, ksize2;
	if (interpolation == INTER_CUBIC)
		ksize = 4, func = cubic_tab[depth];
	else if (interpolation == INTER_LANCZOS4)
		ksize = 8, func = lanczos4_tab[depth];
	else if (interpolation == INTER_LINEAR || interpolation == INTER_AREA)
		ksize = 2, func = linear_tab[depth];
	else
		assert(false);
	ksize2 = ksize / 2;

	assert(func != 0);

	AutoBuffer<uchar> _buffer((width + dsize.height)*(sizeof(int) + sizeof(float)*ksize));
	int* xofs = (int*)(uchar*)_buffer;
	int* yofs = xofs + width;
	float* alpha = (float*)(yofs + dsize.height);
	short* ialpha = (short*)alpha;
	float* beta = alpha + width*ksize;
	short* ibeta = ialpha + width*ksize;
	float cbuf[MAX_ESIZE];

	for (dx = 0; dx < dsize.width; dx++)
	{
		if (!area_mode)
		{
			fx = (float)((dx + 0.5)*scale_x - 0.5);
			sx = cvutil::floor(fx);
			fx -= sx;
		}
		else
		{
			sx = cvutil::floor(dx*scale_x);
			fx = (float)((dx + 1) - (sx + 1)*inv_scale_x);
			fx = fx <= 0 ? 0.f : fx - cvutil::floor(fx);
		}

		if (sx < ksize2 - 1)
		{
			xmin = dx + 1;
			if (sx < 0)
				fx = 0, sx = 0;
		}

		if (sx + ksize2 >= ssize.width)
		{
			xmax = std::min(xmax, dx);
			if (sx >= ssize.width - 1)
				fx = 0, sx = ssize.width - 1;
		}

		for (k = 0, sx *= cn; k < cn; k++)
			xofs[dx*cn + k] = sx + k;

		if (interpolation == INTER_CUBIC)
			interpolateCubic(fx, cbuf);
		else if (interpolation == INTER_LANCZOS4)
			interpolateLanczos4(fx, cbuf);
		else
		{
			cbuf[0] = 1.f - fx;
			cbuf[1] = fx;
		}
		if (fixpt)
		{
			for (k = 0; k < ksize; k++)
				ialpha[dx*cn*ksize + k] = saturate_cast<short>(cbuf[k] * INTER_RESIZE_COEF_SCALE);
			for (; k < cn*ksize; k++)
				ialpha[dx*cn*ksize + k] = ialpha[dx*cn*ksize + k - ksize];
		}
		else
		{
			for (k = 0; k < ksize; k++)
				alpha[dx*cn*ksize + k] = cbuf[k];
			for (; k < cn*ksize; k++)
				alpha[dx*cn*ksize + k] = alpha[dx*cn*ksize + k - ksize];
		}
	}

	for (dy = 0; dy < dsize.height; dy++)
	{
		if (!area_mode)
		{
			fy = (float)((dy + 0.5)*scale_y - 0.5);
			sy = cvutil::floor(fy);
			fy -= sy;
		}
		else
		{
			sy = cvutil::floor(dy*scale_y);
			fy = (float)((dy + 1) - (sy + 1)*inv_scale_y);
			fy = fy <= 0 ? 0.f : fy - cvutil::floor(fy);
		}

		yofs[dy] = sy;
		if (interpolation == INTER_CUBIC)
			interpolateCubic(fy, cbuf);
		else if (interpolation == INTER_LANCZOS4)
			interpolateLanczos4(fy, cbuf);
		else
		{
			cbuf[0] = 1.f - fy;
			cbuf[1] = fy;
		}

		if (fixpt)
		{
			for (k = 0; k < ksize; k++)
				ibeta[dy*ksize + k] = saturate_cast<short>(cbuf[k] * INTER_RESIZE_COEF_SCALE);
		}
		else
		{
			for (k = 0; k < ksize; k++)
				beta[dy*ksize + k] = cbuf[k];
		}
	}

	func(src, dst, xofs, fixpt ? (void*)ialpha : (void*)alpha, yofs,
		fixpt ? (void*)ibeta : (void*)beta, xmin, xmax, ksize);
}


/*****************************************************************************/
template<typename _Tp>
static void remapNearest( const Mat& _src, Mat& _dst, const Mat& _xy,
	int borderType, const Scalar& _borderValue )
{
	Size ssize = _src.size(), dsize = _dst.size();
	int cn = _src.channels();
	const _Tp* S0 = (const _Tp*)_src.data.ptr[0];
	size_t sstep = _src.cols()*_src.channels();
	Scalar_<_Tp> cval(saturate_cast<_Tp>(_borderValue[0]),
		saturate_cast<_Tp>(_borderValue[1]),
		saturate_cast<_Tp>(_borderValue[2]),
		saturate_cast<_Tp>(_borderValue[3]));
	int dx, dy;

	unsigned width1 = ssize.width, height1 = ssize.height;

	if( _dst.isContinuous() && _xy.isContinuous() )
	{
		dsize.width *= dsize.height;
		dsize.height = 1;
	}

	for( dy = 0; dy < dsize.height; dy++ )
	{
		_Tp* D = (_Tp*)(_dst.data.ptr[dy]);
		const short* XY = (const short*)(_xy.data.ptr[dy]);

		if( cn == 1 )
		{
			for( dx = 0; dx < dsize.width; dx++ )
			{
				int sx = XY[dx*2], sy = XY[dx*2+1];
				if( (unsigned)sx < width1 && (unsigned)sy < height1 )
					D[dx] = S0[sy*sstep + sx];
				else
				{
					if( borderType == BORDER_REPLICATE )
					{
						sx = clip(sx, 0, ssize.width);
						sy = clip(sy, 0, ssize.height);
						D[dx] = S0[sy*sstep + sx];
					}
					else if( borderType == BORDER_CONSTANT )
						D[dx] = cval[0];
					else if( borderType != BORDER_TRANSPARENT )
					{
						sx = borderInterpolate(sx, ssize.width, borderType);
						sy = borderInterpolate(sy, ssize.height, borderType);
						D[dx] = S0[sy*sstep + sx];
					}
				}
			}
		}
		else
		{
			for( dx = 0; dx < dsize.width; dx++, D += cn )
			{
				int sx = XY[dx*2], sy = XY[dx*2+1], k;
				const _Tp *S;
				if( (unsigned)sx < width1 && (unsigned)sy < height1 )
				{
					if( cn == 3 )
					{
						S = S0 + sy*sstep + sx*3;
						D[0] = S[0], D[1] = S[1], D[2] = S[2];
					}
					else if( cn == 4 )
					{
						S = S0 + sy*sstep + sx*4;
						D[0] = S[0], D[1] = S[1], D[2] = S[2], D[3] = S[3];
					}
					else
					{
						S = S0 + sy*sstep + sx*cn;
						for( k = 0; k < cn; k++ )
							D[k] = S[k];
					}
				}
				else if( borderType != BORDER_TRANSPARENT )
				{
					if( borderType == BORDER_REPLICATE )
					{
						sx = clip(sx, 0, ssize.width);
						sy = clip(sy, 0, ssize.height);
						S = S0 + sy*sstep + sx*cn;
					}
					else if( borderType == BORDER_CONSTANT )
						S = &cval[0];
					else
					{
						sx = borderInterpolate(sx, ssize.width, borderType);
						sy = borderInterpolate(sy, ssize.height, borderType);
						S = S0 + sy*sstep + sx*cn;
					}
					for( k = 0; k < cn; k++ )
						D[k] = S[k];
				}
			}
		}
	}
}


struct RemapNoVec
{
	int operator()( const Mat&, void*, const short*, const ushort*,
		const void*, int ) const { return 0; }
};

#if CVLIB_SSE2

struct RemapVec_8u
{
	int operator()( const Mat& _src, void* _dst, const short* XY,
		const ushort* FXY, const void* _wtab, int width ) const
	{
		int cn = _src.channels(), x = 0, sstep = (int)_src.cols()*_src.step()*_src.channels();

		if( (cn != 1 && cn != 3 && cn != 4) || !checkHardwareSupport(CVLIB_CPU_SSE2) ||
			sstep > 0x8000 )
			return 0;

		const uchar *S0 = _src.data.ptr[0], *S1 = _src.data.ptr[1];
		const short* wtab = cn == 1 ? (const short*)_wtab : &BilinearTab_iC4[0][0][0];
		uchar* D = (uchar*)_dst;
		__m128i delta = _mm_set1_epi32(INTER_REMAP_COEF_SCALE/2);
		__m128i xy2ofs = _mm_set1_epi32(cn + (sstep << 16));
		__m128i z = _mm_setzero_si128();
		int CVLIB_DECL_ALIGNED(16) iofs0[4], iofs1[4];

		if( cn == 1 )
		{
			for( ; x <= width - 8; x += 8 )
			{
				__m128i xy0 = _mm_loadu_si128( (const __m128i*)(XY + x*2));
				__m128i xy1 = _mm_loadu_si128( (const __m128i*)(XY + x*2 + 8));
				__m128i v0, v1, v2, v3, a0, a1, b0, b1;
				unsigned i0, i1;

				xy0 = _mm_madd_epi16( xy0, xy2ofs );
				xy1 = _mm_madd_epi16( xy1, xy2ofs );
				_mm_store_si128( (__m128i*)iofs0, xy0 );
				_mm_store_si128( (__m128i*)iofs1, xy1 );

				i0 = *(ushort*)(S0 + iofs0[0]) + (*(ushort*)(S0 + iofs0[1]) << 16);
				i1 = *(ushort*)(S0 + iofs0[2]) + (*(ushort*)(S0 + iofs0[3]) << 16);
				v0 = _mm_unpacklo_epi32(_mm_cvtsi32_si128(i0), _mm_cvtsi32_si128(i1));
				i0 = *(ushort*)(S1 + iofs0[0]) + (*(ushort*)(S1 + iofs0[1]) << 16);
				i1 = *(ushort*)(S1 + iofs0[2]) + (*(ushort*)(S1 + iofs0[3]) << 16);
				v1 = _mm_unpacklo_epi32(_mm_cvtsi32_si128(i0), _mm_cvtsi32_si128(i1));
				v0 = _mm_unpacklo_epi8(v0, z);
				v1 = _mm_unpacklo_epi8(v1, z);

				a0 = _mm_unpacklo_epi32(_mm_loadl_epi64((__m128i*)(wtab+FXY[x]*4)),
					_mm_loadl_epi64((__m128i*)(wtab+FXY[x+1]*4)));
				a1 = _mm_unpacklo_epi32(_mm_loadl_epi64((__m128i*)(wtab+FXY[x+2]*4)),
					_mm_loadl_epi64((__m128i*)(wtab+FXY[x+3]*4)));
				b0 = _mm_unpacklo_epi64(a0, a1);
				b1 = _mm_unpackhi_epi64(a0, a1);
				v0 = _mm_madd_epi16(v0, b0);
				v1 = _mm_madd_epi16(v1, b1);
				v0 = _mm_add_epi32(_mm_add_epi32(v0, v1), delta);

				i0 = *(ushort*)(S0 + iofs1[0]) + (*(ushort*)(S0 + iofs1[1]) << 16);
				i1 = *(ushort*)(S0 + iofs1[2]) + (*(ushort*)(S0 + iofs1[3]) << 16);
				v2 = _mm_unpacklo_epi32(_mm_cvtsi32_si128(i0), _mm_cvtsi32_si128(i1));
				i0 = *(ushort*)(S1 + iofs1[0]) + (*(ushort*)(S1 + iofs1[1]) << 16);
				i1 = *(ushort*)(S1 + iofs1[2]) + (*(ushort*)(S1 + iofs1[3]) << 16);
				v3 = _mm_unpacklo_epi32(_mm_cvtsi32_si128(i0), _mm_cvtsi32_si128(i1));
				v2 = _mm_unpacklo_epi8(v2, z);
				v3 = _mm_unpacklo_epi8(v3, z);

				a0 = _mm_unpacklo_epi32(_mm_loadl_epi64((__m128i*)(wtab+FXY[x+4]*4)),
					_mm_loadl_epi64((__m128i*)(wtab+FXY[x+5]*4)));
				a1 = _mm_unpacklo_epi32(_mm_loadl_epi64((__m128i*)(wtab+FXY[x+6]*4)),
					_mm_loadl_epi64((__m128i*)(wtab+FXY[x+7]*4)));
				b0 = _mm_unpacklo_epi64(a0, a1);
				b1 = _mm_unpackhi_epi64(a0, a1);
				v2 = _mm_madd_epi16(v2, b0);
				v3 = _mm_madd_epi16(v3, b1);
				v2 = _mm_add_epi32(_mm_add_epi32(v2, v3), delta);

				v0 = _mm_srai_epi32(v0, INTER_REMAP_COEF_BITS);
				v2 = _mm_srai_epi32(v2, INTER_REMAP_COEF_BITS);
				v0 = _mm_packus_epi16(_mm_packs_epi32(v0, v2), z);
				_mm_storel_epi64( (__m128i*)(D + x), v0 );
			}
		}
		else if( cn == 3 )
		{
			for( ; x <= width - 5; x += 4, D += 12 )
			{
				__m128i xy0 = _mm_loadu_si128( (const __m128i*)(XY + x*2));
				__m128i u0, v0, u1, v1;

				xy0 = _mm_madd_epi16( xy0, xy2ofs );
				_mm_store_si128( (__m128i*)iofs0, xy0 );
				const __m128i *w0, *w1;
				w0 = (const __m128i*)(wtab + FXY[x]*16);
				w1 = (const __m128i*)(wtab + FXY[x+1]*16);

				u0 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int*)(S0 + iofs0[0])),
					_mm_cvtsi32_si128(*(int*)(S0 + iofs0[0] + 3)));
				v0 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int*)(S1 + iofs0[0])),
					_mm_cvtsi32_si128(*(int*)(S1 + iofs0[0] + 3)));
				u1 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int*)(S0 + iofs0[1])),
					_mm_cvtsi32_si128(*(int*)(S0 + iofs0[1] + 3)));
				v1 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int*)(S1 + iofs0[1])),
					_mm_cvtsi32_si128(*(int*)(S1 + iofs0[1] + 3)));
				u0 = _mm_unpacklo_epi8(u0, z);
				v0 = _mm_unpacklo_epi8(v0, z);
				u1 = _mm_unpacklo_epi8(u1, z);
				v1 = _mm_unpacklo_epi8(v1, z);
				u0 = _mm_add_epi32(_mm_madd_epi16(u0, w0[0]), _mm_madd_epi16(v0, w0[1]));
				u1 = _mm_add_epi32(_mm_madd_epi16(u1, w1[0]), _mm_madd_epi16(v1, w1[1]));
				u0 = _mm_srai_epi32(_mm_add_epi32(u0, delta), INTER_REMAP_COEF_BITS);
				u1 = _mm_srai_epi32(_mm_add_epi32(u1, delta), INTER_REMAP_COEF_BITS);
				u0 = _mm_slli_si128(u0, 4);
				u0 = _mm_packs_epi32(u0, u1);
				u0 = _mm_packus_epi16(u0, u0);
				_mm_storel_epi64((__m128i*)D, _mm_srli_si128(u0,1));

				w0 = (const __m128i*)(wtab + FXY[x+2]*16);
				w1 = (const __m128i*)(wtab + FXY[x+3]*16);

				u0 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int*)(S0 + iofs0[2])),
					_mm_cvtsi32_si128(*(int*)(S0 + iofs0[2] + 3)));
				v0 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int*)(S1 + iofs0[2])),
					_mm_cvtsi32_si128(*(int*)(S1 + iofs0[2] + 3)));
				u1 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int*)(S0 + iofs0[3])),
					_mm_cvtsi32_si128(*(int*)(S0 + iofs0[3] + 3)));
				v1 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int*)(S1 + iofs0[3])),
					_mm_cvtsi32_si128(*(int*)(S1 + iofs0[3] + 3)));
				u0 = _mm_unpacklo_epi8(u0, z);
				v0 = _mm_unpacklo_epi8(v0, z);
				u1 = _mm_unpacklo_epi8(u1, z);
				v1 = _mm_unpacklo_epi8(v1, z);
				u0 = _mm_add_epi32(_mm_madd_epi16(u0, w0[0]), _mm_madd_epi16(v0, w0[1]));
				u1 = _mm_add_epi32(_mm_madd_epi16(u1, w1[0]), _mm_madd_epi16(v1, w1[1]));
				u0 = _mm_srai_epi32(_mm_add_epi32(u0, delta), INTER_REMAP_COEF_BITS);
				u1 = _mm_srai_epi32(_mm_add_epi32(u1, delta), INTER_REMAP_COEF_BITS);
				u0 = _mm_slli_si128(u0, 4);
				u0 = _mm_packs_epi32(u0, u1);
				u0 = _mm_packus_epi16(u0, u0);
				_mm_storel_epi64((__m128i*)(D + 6), _mm_srli_si128(u0,1));
			}
		}
		else if( cn == 4 )
		{
			for( ; x <= width - 4; x += 4, D += 16 )
			{
				__m128i xy0 = _mm_loadu_si128( (const __m128i*)(XY + x*2));
				__m128i u0, v0, u1, v1;

				xy0 = _mm_madd_epi16( xy0, xy2ofs );
				_mm_store_si128( (__m128i*)iofs0, xy0 );
				const __m128i *w0, *w1;
				w0 = (const __m128i*)(wtab + FXY[x]*16);
				w1 = (const __m128i*)(wtab + FXY[x+1]*16);

				u0 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int*)(S0 + iofs0[0])),
					_mm_cvtsi32_si128(*(int*)(S0 + iofs0[0] + 4)));
				v0 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int*)(S1 + iofs0[0])),
					_mm_cvtsi32_si128(*(int*)(S1 + iofs0[0] + 4)));
				u1 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int*)(S0 + iofs0[1])),
					_mm_cvtsi32_si128(*(int*)(S0 + iofs0[1] + 4)));
				v1 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int*)(S1 + iofs0[1])),
					_mm_cvtsi32_si128(*(int*)(S1 + iofs0[1] + 4)));
				u0 = _mm_unpacklo_epi8(u0, z);
				v0 = _mm_unpacklo_epi8(v0, z);
				u1 = _mm_unpacklo_epi8(u1, z);
				v1 = _mm_unpacklo_epi8(v1, z);
				u0 = _mm_add_epi32(_mm_madd_epi16(u0, w0[0]), _mm_madd_epi16(v0, w0[1]));
				u1 = _mm_add_epi32(_mm_madd_epi16(u1, w1[0]), _mm_madd_epi16(v1, w1[1]));
				u0 = _mm_srai_epi32(_mm_add_epi32(u0, delta), INTER_REMAP_COEF_BITS);
				u1 = _mm_srai_epi32(_mm_add_epi32(u1, delta), INTER_REMAP_COEF_BITS);
				u0 = _mm_packs_epi32(u0, u1);
				u0 = _mm_packus_epi16(u0, u0);
				_mm_storel_epi64((__m128i*)D, u0);

				w0 = (const __m128i*)(wtab + FXY[x+2]*16);
				w1 = (const __m128i*)(wtab + FXY[x+3]*16);

				u0 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int*)(S0 + iofs0[2])),
					_mm_cvtsi32_si128(*(int*)(S0 + iofs0[2] + 4)));
				v0 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int*)(S1 + iofs0[2])),
					_mm_cvtsi32_si128(*(int*)(S1 + iofs0[2] + 4)));
				u1 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int*)(S0 + iofs0[3])),
					_mm_cvtsi32_si128(*(int*)(S0 + iofs0[3] + 4)));
				v1 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int*)(S1 + iofs0[3])),
					_mm_cvtsi32_si128(*(int*)(S1 + iofs0[3] + 4)));
				u0 = _mm_unpacklo_epi8(u0, z);
				v0 = _mm_unpacklo_epi8(v0, z);
				u1 = _mm_unpacklo_epi8(u1, z);
				v1 = _mm_unpacklo_epi8(v1, z);
				u0 = _mm_add_epi32(_mm_madd_epi16(u0, w0[0]), _mm_madd_epi16(v0, w0[1]));
				u1 = _mm_add_epi32(_mm_madd_epi16(u1, w1[0]), _mm_madd_epi16(v1, w1[1]));
				u0 = _mm_srai_epi32(_mm_add_epi32(u0, delta), INTER_REMAP_COEF_BITS);
				u1 = _mm_srai_epi32(_mm_add_epi32(u1, delta), INTER_REMAP_COEF_BITS);
				u0 = _mm_packs_epi32(u0, u1);
				u0 = _mm_packus_epi16(u0, u0);
				_mm_storel_epi64((__m128i*)(D + 8), u0);
			}
		}

		return x;
	}
};

#else

typedef RemapNoVec RemapVec_8u;

#endif


template<class CastOp, class VecOp, typename AT>
static void remapBilinear( const Mat& _src, Mat& _dst, const Mat& _xy,
	const Mat& _fxy, const void* _wtab,
	int borderType, const Scalar& _borderValue )
{
	typedef typename CastOp::rtype _Tp;
	typedef typename CastOp::type1 WT;
	Size ssize = _src.size(), dsize = _dst.size();
	int cn = _src.channels();
	const AT* wtab = (const AT*)_wtab;
	const _Tp* S0 = (const _Tp*)_src.data.ptr[0];
	size_t sstep = _src.cols()*_src.channels();
	Scalar_<_Tp> cval(saturate_cast<_Tp>(_borderValue[0]),
		saturate_cast<_Tp>(_borderValue[1]),
		saturate_cast<_Tp>(_borderValue[2]),
		saturate_cast<_Tp>(_borderValue[3]));
	int dx, dy;
	CastOp castOp;
	VecOp vecOp;

	unsigned width1 = MAX(ssize.width-1, 0), height1 = MAX(ssize.height-1, 0);
	assert( cn <= 4 && ssize.area() > 0 );
#if CVLIB_SSE2
	if( _src.type1() == MAT_Tuchar3 )
		width1 = MAX(ssize.width-2, 0);
#endif

	for( dy = 0; dy < dsize.height; dy++ )
	{
		_Tp* D = (_Tp*)(_dst.data.ptr[dy]);
		const short* XY = (const short*)(_xy.data.s[dy]);
		const ushort* FXY = (const ushort*)(_fxy.data.s[dy]);
		int X0 = 0;
		bool prevInlier = false;

		for( dx = 0; dx <= dsize.width; dx++ )
		{
			bool curInlier = dx < dsize.width ?
				(unsigned)XY[dx*2] < width1 &&
				(unsigned)XY[dx*2+1] < height1 : !prevInlier;
			if( curInlier == prevInlier )
				continue;

			int X1 = dx;
			dx = X0;
			X0 = X1;
			prevInlier = curInlier;

			if( !curInlier )
			{
				int len = vecOp( _src, D, XY + dx*2, FXY + dx, wtab, X1 - dx );
				D += len*cn;
				dx += len;

				if( cn == 1 )
				{
					for( ; dx < X1; dx++, D++ )
					{
						int sx = XY[dx*2], sy = XY[dx*2+1];
						const AT* w = wtab + FXY[dx]*4;
						const _Tp* S = S0 + sy*sstep + sx;
						*D = castOp(WT(S[0]*w[0] + S[1]*w[1] + S[sstep]*w[2] + S[sstep+1]*w[3]));
					}
				}
				else if( cn == 2 )
					for( ; dx < X1; dx++, D += 2 )
					{
						int sx = XY[dx*2], sy = XY[dx*2+1];
						const AT* w = wtab + FXY[dx]*4;
						const _Tp* S = S0 + sy*sstep + sx*2;
						WT t0 = S[0]*w[0] + S[2]*w[1] + S[sstep]*w[2] + S[sstep+2]*w[3];
						WT t1 = S[1]*w[0] + S[3]*w[1] + S[sstep+1]*w[2] + S[sstep+3]*w[3];
						D[0] = castOp(t0); D[1] = castOp(t1);
					}
				else if( cn == 3 )
					for( ; dx < X1; dx++, D += 3 )
					{
						int sx = XY[dx*2], sy = XY[dx*2+1];
						const AT* w = wtab + FXY[dx]*4;
						const _Tp* S = S0 + sy*sstep + sx*3;
						WT t0 = S[0]*w[0] + S[3]*w[1] + S[sstep]*w[2] + S[sstep+3]*w[3];
						WT t1 = S[1]*w[0] + S[4]*w[1] + S[sstep+1]*w[2] + S[sstep+4]*w[3];
						WT t2 = S[2]*w[0] + S[5]*w[1] + S[sstep+2]*w[2] + S[sstep+5]*w[3];
						D[0] = castOp(t0); D[1] = castOp(t1); D[2] = castOp(t2);
					}
				else
					for( ; dx < X1; dx++, D += 4 )
					{
						int sx = XY[dx*2], sy = XY[dx*2+1];
						const AT* w = wtab + FXY[dx]*4;
						const _Tp* S = S0 + sy*sstep + sx*4;
						WT t0 = S[0]*w[0] + S[4]*w[1] + S[sstep]*w[2] + S[sstep+4]*w[3];
						WT t1 = S[1]*w[0] + S[5]*w[1] + S[sstep+1]*w[2] + S[sstep+5]*w[3];
						D[0] = castOp(t0); D[1] = castOp(t1);
						t0 = S[2]*w[0] + S[6]*w[1] + S[sstep+2]*w[2] + S[sstep+6]*w[3];
						t1 = S[3]*w[0] + S[7]*w[1] + S[sstep+3]*w[2] + S[sstep+7]*w[3];
						D[2] = castOp(t0); D[3] = castOp(t1);
					}
			}
			else
			{
				if( borderType == BORDER_TRANSPARENT && cn != 3 )
				{
					D += (X1 - dx)*cn;
					dx = X1;
					continue;
				}

				if( cn == 1 )
					for( ; dx < X1; dx++, D++ )
					{
						int sx = XY[dx*2], sy = XY[dx*2+1];
						if( borderType == BORDER_CONSTANT &&
							(sx >= ssize.width || sx+1 < 0 ||
							sy >= ssize.height || sy+1 < 0) )
						{
							D[0] = cval[0];
						}
						else
						{
							int sx0, sx1, sy0, sy1;
							_Tp v0, v1, v2, v3;
							const AT* w = wtab + FXY[dx]*4;
							if( borderType == BORDER_REPLICATE )
							{
								sx0 = clip(sx, 0, ssize.width);
								sx1 = clip(sx+1, 0, ssize.width);
								sy0 = clip(sy, 0, ssize.height);
								sy1 = clip(sy+1, 0, ssize.height);
								v0 = S0[sy0*sstep + sx0];
								v1 = S0[sy0*sstep + sx1];
								v2 = S0[sy1*sstep + sx0];
								v3 = S0[sy1*sstep + sx1];
							}
							else
							{
								sx0 = borderInterpolate(sx, ssize.width, borderType);
								sx1 = borderInterpolate(sx+1, ssize.width, borderType);
								sy0 = borderInterpolate(sy, ssize.height, borderType);
								sy1 = borderInterpolate(sy+1, ssize.height, borderType);
								v0 = sx0 >= 0 && sy0 >= 0 ? S0[sy0*sstep + sx0] : cval[0];
								v1 = sx1 >= 0 && sy0 >= 0 ? S0[sy0*sstep + sx1] : cval[0];
								v2 = sx0 >= 0 && sy1 >= 0 ? S0[sy1*sstep + sx0] : cval[0];
								v3 = sx1 >= 0 && sy1 >= 0 ? S0[sy1*sstep + sx1] : cval[0];
							}
							D[0] = castOp(WT(v0*w[0] + v1*w[1] + v2*w[2] + v3*w[3]));
						}
					}
				else
					for( ; dx < X1; dx++, D += cn )
					{
						int sx = XY[dx*2], sy = XY[dx*2+1], k;
						if( borderType == BORDER_CONSTANT &&
							(sx >= ssize.width || sx+1 < 0 ||
							sy >= ssize.height || sy+1 < 0) )
						{
							for( k = 0; k < cn; k++ )
								D[k] = cval[k];
						}
						else
						{
							int sx0, sx1, sy0, sy1;
							const _Tp *v0, *v1, *v2, *v3;
							const AT* w = wtab + FXY[dx]*4;
							if( borderType == BORDER_REPLICATE )
							{
								sx0 = clip(sx, 0, ssize.width);
								sx1 = clip(sx+1, 0, ssize.width);
								sy0 = clip(sy, 0, ssize.height);
								sy1 = clip(sy+1, 0, ssize.height);
								v0 = S0 + sy0*sstep + sx0*cn;
								v1 = S0 + sy0*sstep + sx1*cn;
								v2 = S0 + sy1*sstep + sx0*cn;
								v3 = S0 + sy1*sstep + sx1*cn;
							}
							else if( borderType == BORDER_TRANSPARENT &&
								((unsigned)sx >= (unsigned)(ssize.width-1) ||
								(unsigned)sy >= (unsigned)(ssize.height-1)))
								continue;
							else
							{
								sx0 = borderInterpolate(sx, ssize.width, borderType);
								sx1 = borderInterpolate(sx+1, ssize.width, borderType);
								sy0 = borderInterpolate(sy, ssize.height, borderType);
								sy1 = borderInterpolate(sy+1, ssize.height, borderType);
								v0 = sx0 >= 0 && sy0 >= 0 ? S0 + sy0*sstep + sx0*cn : &cval[0];
								v1 = sx1 >= 0 && sy0 >= 0 ? S0 + sy0*sstep + sx1*cn : &cval[0];
								v2 = sx0 >= 0 && sy1 >= 0 ? S0 + sy1*sstep + sx0*cn : &cval[0];
								v3 = sx1 >= 0 && sy1 >= 0 ? S0 + sy1*sstep + sx1*cn : &cval[0];
							}
							for( k = 0; k < cn; k++ )
								D[k] = castOp(WT(v0[k]*w[0] + v1[k]*w[1] + v2[k]*w[2] + v3[k]*w[3]));
						}
					}
			}
		}
	}
}


template<class CastOp, typename AT, int ONE>
static void remapBicubic( const Mat& _src, Mat& _dst, const Mat& _xy,
	const Mat& _fxy, const void* _wtab,
	int borderType, const Scalar& _borderValue )
{
	typedef typename CastOp::rtype _Tp;
	typedef typename CastOp::type1 WT;
	Size ssize = _src.size(), dsize = _dst.size();
	int cn = _src.channels();
	const AT* wtab = (const AT*)_wtab;
	const _Tp* S0 = (const _Tp*)_src.data.ptr[0];
	size_t sstep = _src.cols()*_src.channels();
	Scalar_<_Tp> cval(saturate_cast<_Tp>(_borderValue[0]),
		saturate_cast<_Tp>(_borderValue[1]),
		saturate_cast<_Tp>(_borderValue[2]),
		saturate_cast<_Tp>(_borderValue[3]));
	int dx, dy;
	CastOp castOp;
	int borderType1 = borderType != BORDER_TRANSPARENT ? borderType : BORDER_REFLECT_101;

	unsigned width1 = MAX(ssize.width-3, 0), height1 = MAX(ssize.height-3, 0);

	if( _dst.isContinuous() && _xy.isContinuous() && _fxy.isContinuous() )
	{
		dsize.width *= dsize.height;
		dsize.height = 1;
	}

	for( dy = 0; dy < dsize.height; dy++ )
	{
		_Tp* D = (_Tp*)(_dst.data.ptr[dy]);
		const short* XY = (const short*)(_xy.data.s[dy]);
		const ushort* FXY = (const ushort*)(_fxy.data.s[dy]);

		for( dx = 0; dx < dsize.width; dx++, D += cn )
		{
			int sx = XY[dx*2]-1, sy = XY[dx*2+1]-1;
			const AT* w = wtab + FXY[dx]*16;
			int i, k;
			if( (unsigned)sx < width1 && (unsigned)sy < height1 )
			{
				const _Tp* S = S0 + sy*sstep + sx*cn;
				for( k = 0; k < cn; k++ )
				{
					WT sum = S[0]*w[0] + S[cn]*w[1] + S[cn*2]*w[2] + S[cn*3]*w[3];
					S += sstep;
					sum += S[0]*w[4] + S[cn]*w[5] + S[cn*2]*w[6] + S[cn*3]*w[7];
					S += sstep;
					sum += S[0]*w[8] + S[cn]*w[9] + S[cn*2]*w[10] + S[cn*3]*w[11];
					S += sstep;
					sum += S[0]*w[12] + S[cn]*w[13] + S[cn*2]*w[14] + S[cn*3]*w[15];
					S += 1 - sstep*3;
					D[k] = castOp(sum);
				}
			}
			else
			{
				int x[4], y[4];
				if( borderType == BORDER_TRANSPARENT &&
					((unsigned)(sx+1) >= (unsigned)ssize.width ||
					(unsigned)(sy+1) >= (unsigned)ssize.height) )
					continue;

				if( borderType1 == BORDER_CONSTANT &&
					(sx >= ssize.width || sx+4 <= 0 ||
					sy >= ssize.height || sy+4 <= 0))
				{
					for( k = 0; k < cn; k++ )
						D[k] = cval[k];
					continue;
				}

				for( i = 0; i < 4; i++ )
				{
					x[i] = borderInterpolate(sx + i, ssize.width, borderType1)*cn;
					y[i] = borderInterpolate(sy + i, ssize.height, borderType1);
				}

				for( k = 0; k < cn; k++, S0++, w -= 16 )
				{
					WT cv = cval[k], sum = cv*ONE;
					for( i = 0; i < 4; i++, w += 4 )
					{
						int yi = y[i];
						const _Tp* S = S0 + yi*sstep;
						if( yi < 0 )
							continue;
						if( x[0] >= 0 )
							sum += (S[x[0]] - cv)*w[0];
						if( x[1] >= 0 )
							sum += (S[x[1]] - cv)*w[1];
						if( x[2] >= 0 )
							sum += (S[x[2]] - cv)*w[2];
						if( x[3] >= 0 )
							sum += (S[x[3]] - cv)*w[3];
					}
					D[k] = castOp(sum);
				}
				S0 -= cn;
			}
		}
	}
}


template<class CastOp, typename AT, int ONE>
static void remapLanczos4( const Mat& _src, Mat& _dst, const Mat& _xy,
	const Mat& _fxy, const void* _wtab,
	int borderType, const Scalar& _borderValue )
{
	typedef typename CastOp::rtype _Tp;
	typedef typename CastOp::type1 WT;
	Size ssize = _src.size(), dsize = _dst.size();
	int cn = _src.channels();
	const AT* wtab = (const AT*)_wtab;
	const _Tp* S0 = (const _Tp*)_src.data.ptr[0];
	size_t sstep = _src.cols()*_src.channels();
	Scalar_<_Tp> cval(saturate_cast<_Tp>(_borderValue[0]),
		saturate_cast<_Tp>(_borderValue[1]),
		saturate_cast<_Tp>(_borderValue[2]),
		saturate_cast<_Tp>(_borderValue[3]));
	int dx, dy;
	CastOp castOp;
	int borderType1 = borderType != BORDER_TRANSPARENT ? borderType : BORDER_REFLECT_101;

	unsigned width1 = MAX(ssize.width-7, 0), height1 = MAX(ssize.height-7, 0);

	if( _dst.isContinuous() && _xy.isContinuous() && _fxy.isContinuous() )
	{
		dsize.width *= dsize.height;
		dsize.height = 1;
	}

	for( dy = 0; dy < dsize.height; dy++ )
	{
		_Tp* D = (_Tp*)(_dst.data.ptr[dy]);
		const short* XY = (const short*)(_xy.data.s[dy]);
		const ushort* FXY = (const ushort*)(_fxy.data.s[dy]);

		for( dx = 0; dx < dsize.width; dx++, D += cn )
		{
			int sx = XY[dx*2]-3, sy = XY[dx*2+1]-3;
			const AT* w = wtab + FXY[dx]*64;
			const _Tp* S = S0 + sy*sstep + sx*cn;
			int i, k;
			if( (unsigned)sx < width1 && (unsigned)sy < height1 )
			{
				for( k = 0; k < cn; k++ )
				{
					WT sum = 0;
					for( int r = 0; r < 8; r++, S += sstep, w += 8 )
						sum += S[0]*w[0] + S[cn]*w[1] + S[cn*2]*w[2] + S[cn*3]*w[3] +
						S[cn*4]*w[4] + S[cn*5]*w[5] + S[cn*6]*w[6] + S[cn*7]*w[7];
					w -= 64;
					S -= sstep*8 - 1;
					D[k] = castOp(sum);
				}
			}
			else
			{
				int x[8], y[8];
				if( borderType == BORDER_TRANSPARENT &&
					((unsigned)(sx+3) >= (unsigned)ssize.width ||
					(unsigned)(sy+3) >= (unsigned)ssize.height) )
					continue;

				if( borderType1 == BORDER_CONSTANT &&
					(sx >= ssize.width || sx+8 <= 0 ||
					sy >= ssize.height || sy+8 <= 0))
				{
					for( k = 0; k < cn; k++ )
						D[k] = cval[k];
					continue;
				}

				for( i = 0; i < 8; i++ )
				{
					x[i] = borderInterpolate(sx + i, ssize.width, borderType1)*cn;
					y[i] = borderInterpolate(sy + i, ssize.height, borderType1);
				}

				for( k = 0; k < cn; k++, S0++, w -= 64 )
				{
					WT cv = cval[k], sum = cv*ONE;
					for( i = 0; i < 8; i++, w += 8 )
					{
						int yi = y[i];
						const _Tp* S1 = S0 + yi*sstep;
						if( yi < 0 )
							continue;
						if( x[0] >= 0 )
							sum += (S1[x[0]] - cv)*w[0];
						if( x[1] >= 0 )
							sum += (S1[x[1]] - cv)*w[1];
						if( x[2] >= 0 )
							sum += (S1[x[2]] - cv)*w[2];
						if( x[3] >= 0 )
							sum += (S1[x[3]] - cv)*w[3];
						if( x[4] >= 0 )
							sum += (S1[x[4]] - cv)*w[4];
						if( x[5] >= 0 )
							sum += (S1[x[5]] - cv)*w[5];
						if( x[6] >= 0 )
							sum += (S1[x[6]] - cv)*w[6];
						if( x[7] >= 0 )
							sum += (S1[x[7]] - cv)*w[7];
					}
					D[k] = castOp(sum);
				}
				S0 -= cn;
			}
		}
	}
}


typedef void (*RemapNNFunc)(const Mat& _src, Mat& _dst, const Mat& _xy,
	int borderType, const Scalar& _borderValue );

typedef void (*RemapFunc)(const Mat& _src, Mat& _dst, const Mat& _xy,
	const Mat& _fxy, const void* _wtab,
	int borderType, const Scalar& _borderValue);

class RemapInvoker :
	public ParallelLoopBody
{
public:
	RemapInvoker(const Mat& _src, Mat& _dst, const Mat *_m1,
		const Mat *_m2, int _interpolation, int _borderType, const Scalar &_borderValue,
		int _planar_input, RemapNNFunc _nnfunc, RemapFunc _ifunc, const void *_ctab) :
	ParallelLoopBody(), src(&_src), dst(&_dst), m1(_m1), m2(_m2),
		interpolation(_interpolation), borderType(_borderType), borderValue(_borderValue),
		planar_input(_planar_input), nnfunc(_nnfunc), ifunc(_ifunc), ctab(_ctab)
	{
	}

	virtual void operator() (const Range& range) const
	{
		int x, y, x1, y1;
		const int buf_size = 1 << 14;
		int brows0 = MIN(128, dst->rows()), map_depth = m1->type();
		int bcols0 = MIN(buf_size/brows0, dst->cols());
		brows0 = MIN(buf_size/bcols0, dst->rows());
#if CVLIB_SSE2
		bool useSIMD = checkHardwareSupport(CVLIB_CPU_SSE2);
#endif

		Mat _bufxy(brows0, bcols0, MAT_Tshort2), _bufa;
		if( !nnfunc )
			_bufa.create(brows0, bcols0, MAT_Tshort);

		for( y = range.start; y < range.end; y += brows0 )
		{
			for( x = 0; x < dst->cols(); x += bcols0 )
			{
				int brows = MIN(brows0, range.end - y);
				int bcols = MIN(bcols0, dst->cols() - x);
				Mat dpart(*dst, Rect(x, y, bcols, brows));
				Mat bufxy(_bufxy, Rect(0, 0, bcols, brows));

				if( nnfunc )
				{
					if( m1->type1() == MAT_Tshort2 && !m2->data.ptr ) // the data is already in the right format
						m1->subRefMat(Rect(x, y, bcols, brows), bufxy);
//						bufxy = (*m1)(Rect(x, y, bcols, brows));
					else if( map_depth != MAT_Tfloat )
					{
						for( y1 = 0; y1 < brows; y1++ )
						{
							short* XY = bufxy.data.s[y1];
							const short* sXY = (const short*)(m1->data.s[y+y1]) + x*2;
							const ushort* sA = (const ushort*)(m2->data.s[y+y1]) + x;

							for( x1 = 0; x1 < bcols; x1++ )
							{
								int a = sA[x1] & (INTER_TAB_SIZE2-1);
								XY[x1*2] = sXY[x1*2] + NNDeltaTab_i[a][0];
								XY[x1*2+1] = sXY[x1*2+1] + NNDeltaTab_i[a][1];
							}
						}
					}
					else if( !planar_input )
						Mat(*m1, (Rect(x, y, bcols, brows))).convertTo(bufxy, bufxy.type1(), 1.0, 0.0);
					else
					{
						for( y1 = 0; y1 < brows; y1++ )
						{
							short* XY = (short*)(bufxy.data.s[y1]);
							const float* sX = (const float*)(m1->data.fl[y+y1]) + x;
							const float* sY = (const float*)(m2->data.fl[y+y1]) + x;
							x1 = 0;

#if CVLIB_SSE2
							if( useSIMD )
							{
								for( ; x1 <= bcols - 8; x1 += 8 )
								{
									__m128 fx0 = _mm_loadu_ps(sX + x1);
									__m128 fx1 = _mm_loadu_ps(sX + x1 + 4);
									__m128 fy0 = _mm_loadu_ps(sY + x1);
									__m128 fy1 = _mm_loadu_ps(sY + x1 + 4);
									__m128i ix0 = _mm_cvtps_epi32(fx0);
									__m128i ix1 = _mm_cvtps_epi32(fx1);
									__m128i iy0 = _mm_cvtps_epi32(fy0);
									__m128i iy1 = _mm_cvtps_epi32(fy1);
									ix0 = _mm_packs_epi32(ix0, ix1);
									iy0 = _mm_packs_epi32(iy0, iy1);
									ix1 = _mm_unpacklo_epi16(ix0, iy0);
									iy1 = _mm_unpackhi_epi16(ix0, iy0);
									_mm_storeu_si128((__m128i*)(XY + x1*2), ix1);
									_mm_storeu_si128((__m128i*)(XY + x1*2 + 8), iy1);
								}
							}
#endif

							for( ; x1 < bcols; x1++ )
							{
								XY[x1*2] = saturate_cast<short>(sX[x1]);
								XY[x1*2+1] = saturate_cast<short>(sY[x1]);
							}
						}
					}
					nnfunc( *src, dpart, bufxy, borderType, borderValue );
					continue;
				}

				Mat bufa(_bufa, Rect(0, 0, bcols, brows));
				for( y1 = 0; y1 < brows; y1++ )
				{
					short* XY = (short*)(bufxy.data.s[y1]);
					ushort* A = (ushort*)(bufa.data.s[y1]);

					if( m1->type1() == MAT_Tshort2 && (m2->type1() == MAT_Tshort || m2->type1() == MAT_Tshort) )
					{
						m1->subRefMat(Rect(x, y, bcols, brows), bufxy); //bufxy = (*m1)(Rect(x, y, bcols, brows));

						const ushort* sA = (const ushort*)(m2->data.s[y+y1]) + x;
						for( x1 = 0; x1 < bcols; x1++ )
							A[x1] = (ushort)(sA[x1] & (INTER_TAB_SIZE2-1));
					}
					else if( planar_input )
					{
						const float* sX = (const float*)(m1->data.fl[y+y1]) + x;
						const float* sY = (const float*)(m2->data.fl[y+y1]) + x;

						x1 = 0;
#if CVLIB_SSE2
						if( useSIMD )
						{
							__m128 scale = _mm_set1_ps((float)INTER_TAB_SIZE);
							__m128i mask = _mm_set1_epi32(INTER_TAB_SIZE-1);
							for( ; x1 <= bcols - 8; x1 += 8 )
							{
								__m128 fx0 = _mm_loadu_ps(sX + x1);
								__m128 fx1 = _mm_loadu_ps(sX + x1 + 4);
								__m128 fy0 = _mm_loadu_ps(sY + x1);
								__m128 fy1 = _mm_loadu_ps(sY + x1 + 4);
								__m128i ix0 = _mm_cvtps_epi32(_mm_mul_ps(fx0, scale));
								__m128i ix1 = _mm_cvtps_epi32(_mm_mul_ps(fx1, scale));
								__m128i iy0 = _mm_cvtps_epi32(_mm_mul_ps(fy0, scale));
								__m128i iy1 = _mm_cvtps_epi32(_mm_mul_ps(fy1, scale));
								__m128i mx0 = _mm_and_si128(ix0, mask);
								__m128i mx1 = _mm_and_si128(ix1, mask);
								__m128i my0 = _mm_and_si128(iy0, mask);
								__m128i my1 = _mm_and_si128(iy1, mask);
								mx0 = _mm_packs_epi32(mx0, mx1);
								my0 = _mm_packs_epi32(my0, my1);
								my0 = _mm_slli_epi16(my0, INTER_BITS);
								mx0 = _mm_or_si128(mx0, my0);
								_mm_storeu_si128((__m128i*)(A + x1), mx0);
								ix0 = _mm_srai_epi32(ix0, INTER_BITS);
								ix1 = _mm_srai_epi32(ix1, INTER_BITS);
								iy0 = _mm_srai_epi32(iy0, INTER_BITS);
								iy1 = _mm_srai_epi32(iy1, INTER_BITS);
								ix0 = _mm_packs_epi32(ix0, ix1);
								iy0 = _mm_packs_epi32(iy0, iy1);
								ix1 = _mm_unpacklo_epi16(ix0, iy0);
								iy1 = _mm_unpackhi_epi16(ix0, iy0);
								_mm_storeu_si128((__m128i*)(XY + x1*2), ix1);
								_mm_storeu_si128((__m128i*)(XY + x1*2 + 8), iy1);
							}
						}
#endif

						for( ; x1 < bcols; x1++ )
						{
							int sx = cvutil::round(sX[x1]*INTER_TAB_SIZE);
							int sy = cvutil::round(sY[x1]*INTER_TAB_SIZE);
							int v = (sy & (INTER_TAB_SIZE-1))*INTER_TAB_SIZE + (sx & (INTER_TAB_SIZE-1));
							XY[x1*2] = saturate_cast<short>(sx >> INTER_BITS);
							XY[x1*2+1] = saturate_cast<short>(sy >> INTER_BITS);
							A[x1] = (ushort)v;
						}
					}
					else
					{
						const float* sXY = (const float*)(m1->data.fl[y+y1]) + x*2;

						for( x1 = 0; x1 < bcols; x1++ )
						{
							int sx = cvutil::round(sXY[x1*2]*INTER_TAB_SIZE);
							int sy = cvutil::round(sXY[x1*2+1]*INTER_TAB_SIZE);
							int v = (sy & (INTER_TAB_SIZE-1))*INTER_TAB_SIZE + (sx & (INTER_TAB_SIZE-1));
							XY[x1*2] = saturate_cast<short>(sx >> INTER_BITS);
							XY[x1*2+1] = saturate_cast<short>(sy >> INTER_BITS);
							A[x1] = (ushort)v;
						}
					}
				}
				ifunc(*src, dpart, bufxy, bufa, ctab, borderType, borderValue);
			}
		}
	}

private:
	const Mat* src;
	Mat* dst;
	const Mat *m1, *m2;
	int interpolation, borderType;
	Scalar borderValue;
	int planar_input;
	RemapNNFunc nnfunc;
	RemapFunc ifunc;
	const void *ctab;
};

void remap( const Mat& _src, Mat& dst,
	const Mat& map1, const Mat& map2,
	int interpolation, int borderType, const Scalar& borderValue )
{
	static RemapNNFunc nn_tab[] =
	{
		remapNearest<uchar>, remapNearest<char>, remapNearest<short>,
		remapNearest<int>, remapNearest<float>, remapNearest<double>, 0
	};

	static RemapFunc linear_tab[] =
	{
		remapBilinear<FixedPtCast<int, uchar, INTER_REMAP_COEF_BITS>, RemapVec_8u, short>, 0,
		remapBilinear<Cast<float, ushort>, RemapNoVec, float>,
		remapBilinear<Cast<float, short>, RemapNoVec, float>, 0,
		remapBilinear<Cast<float, float>, RemapNoVec, float>,
		remapBilinear<Cast<double, double>, RemapNoVec, float>, 0
	};

	static RemapFunc cubic_tab[] =
	{
		remapBicubic<FixedPtCast<int, uchar, INTER_REMAP_COEF_BITS>, short, INTER_REMAP_COEF_SCALE>, 0,
		remapBicubic<Cast<float, ushort>, float, 1>,
		remapBicubic<Cast<float, short>, float, 1>, 0,
		remapBicubic<Cast<float, float>, float, 1>,
		remapBicubic<Cast<double, double>, float, 1>, 0
	};

	static RemapFunc lanczos4_tab[] =
	{
		remapLanczos4<FixedPtCast<int, uchar, INTER_REMAP_COEF_BITS>, short, INTER_REMAP_COEF_SCALE>, 0,
		remapLanczos4<Cast<float, ushort>, float, 1>,
		remapLanczos4<Cast<float, short>, float, 1>, 0,
		remapLanczos4<Cast<float, float>, float, 1>,
		remapLanczos4<Cast<double, double>, float, 1>, 0
	};

	assert( map1.size().area() > 0 );
	assert( !map2.data.ptr || (map2.size() == map1.size()));
	Mat src = _src;
	dst.create( map1.size(), src.type1() );
//	if( dst.data.ptr == src.data.ptr )
//		src = src.clone();

	int depth = src.type();
	RemapNNFunc nnfunc = 0;
	RemapFunc ifunc = 0;
	const void* ctab = 0;
	bool fixpt = depth == MAT_Tuchar;
	bool planar_input = false;

	if( interpolation == INTER_NEAREST )
	{
		nnfunc = nn_tab[depth];
		assert( nnfunc != 0 );
	}
	else
	{
		if( interpolation == INTER_AREA )
			interpolation = INTER_LINEAR;

		if( interpolation == INTER_LINEAR )
			ifunc = linear_tab[depth];
		else if( interpolation == INTER_CUBIC )
			ifunc = cubic_tab[depth];
		else if( interpolation == INTER_LANCZOS4 )
			ifunc = lanczos4_tab[depth];
		else
			assert (false);
			//CV_Error( CV_StsBadArg, "Unknown interpolation method" );
		assert( ifunc != 0 );
		ctab = initInterTab2D( interpolation, fixpt );
	}

	const Mat *m1 = &map1, *m2 = &map2;

	if( (map1.type1() == MAT_Tshort2 && (map2.type1() == MAT_Tshort || map2.type1() == MAT_Tshort || !map2.data.ptr)) ||
		(map2.type1() == MAT_Tshort2 && (map1.type1() == MAT_Tshort || map1.type1() == MAT_Tshort || !map1.data.ptr)) )
	{
		if( map1.type1() != MAT_Tshort2 )
			std::swap(m1, m2);
	}
	else
	{
		assert( ((map1.type1() == MAT_Tfloat2 || map1.type1() == MAT_Tshort2) && !map2.data.ptr) ||
			(map1.type1() == MAT_Tfloat && map2.type1() == MAT_Tfloat) );
		planar_input = map1.channels() == 1;
	}

	RemapInvoker invoker(src, dst, m1, m2, interpolation,
		borderType, borderValue, planar_input, nnfunc, ifunc,
		ctab);
	parallel_for_(Range(0, dst.rows()), invoker, dst.total()/(double)(1<<16));
}


void convertMaps( const Mat& map1, const Mat& map2,
	Mat& dstmap1, Mat& dstmap2,
	int dstm1type, bool nninterpolate )
{
	Size size = map1.size();
	const Mat *m1 = &map1, *m2 = &map2;
	int m1type = m1->type1(), m2type = m2->type1();

	assert( (m1type == MAT_Tshort2 && (nninterpolate || m2type == MAT_Tshort || m2type == MAT_Tshort)) ||
		(m2type == MAT_Tshort2 && (nninterpolate || m1type == MAT_Tshort || m1type == MAT_Tshort)) ||
		(m1type == MAT_Tfloat && m2type == MAT_Tfloat) ||
		(m1type == MAT_Tfloat2 && !m2->data.ptr) );

	if( m2type == MAT_Tshort2 )
	{
		std::swap( m1, m2 );
		std::swap( m1type, m2type );
	}

	if( dstm1type <= 0 )
		dstm1type = m1type == MAT_Tshort2 ? MAT_Tfloat2 : MAT_Tshort2;
	assert( dstm1type == MAT_Tshort2 || dstm1type == MAT_Tfloat || dstm1type == MAT_Tfloat2 );
	dstmap1.create( size, (TYPE)dstm1type );

	if( !nninterpolate && dstm1type != MAT_Tfloat2 )
	{
		dstmap2.create( size, dstm1type == MAT_Tshort2 ? MAT_Tshort : MAT_Tfloat );
	}
	else
		dstmap2.release();

	if( m1type == dstm1type || (nninterpolate &&
		((m1type == MAT_Tshort2 && dstm1type == MAT_Tfloat2) ||
		(m1type == MAT_Tfloat2 && dstm1type == MAT_Tshort2))) )
	{
		m1->convertTo( dstmap1, dstmap1.type1(), 1.0, 0.0 );
		if( dstmap2.data.ptr && dstmap2.type1() == m2->type1() )
			dstmap2 = *m2;
		return;
	}

	if( m1type == MAT_Tfloat && dstm1type == MAT_Tfloat2 )
	{
		Mat vdata[] = { *m1, *m2 };
		MatOp::merge( vdata, 2, dstmap1 );
		return;
	}

	if( m1type == MAT_Tfloat2 && dstm1type == MAT_Tfloat )
	{
		Mat mv[] = { dstmap1, dstmap2 };
		MatOp::split( *m1, mv );
		return;
	}

	if( m1->isContinuous() && (!m2->data.ptr || m2->isContinuous()) &&
		dstmap1.isContinuous() && (!dstmap2.data.ptr || dstmap2.isContinuous()) )
	{
		size.width *= size.height;
		size.height = 1;
	}

	const float scale = 1.f/INTER_TAB_SIZE;
	int x, y;
	for( y = 0; y < size.height; y++ )
	{
		const float* src1f = (const float*)(m1->data.fl[y]);
		const float* src2f = (const float*)(m2->data.fl[y]);
		const short* src1 = (const short*)src1f;
		const ushort* src2 = (const ushort*)src2f;

		float* dst1f = (float*)(dstmap1.data.fl[y]);
		float* dst2f = (float*)(dstmap2.data.fl[y]);
		short* dst1 = (short*)dst1f;
		ushort* dst2 = (ushort*)dst2f;

		if( m1type == MAT_Tfloat && dstm1type == MAT_Tshort2 )
		{
			if( nninterpolate )
				for( x = 0; x < size.width; x++ )
				{
					dst1[x*2] = saturate_cast<short>(src1f[x]);
					dst1[x*2+1] = saturate_cast<short>(src2f[x]);
				}
			else
				for( x = 0; x < size.width; x++ )
				{
					int ix = saturate_cast<int>(src1f[x]*INTER_TAB_SIZE);
					int iy = saturate_cast<int>(src2f[x]*INTER_TAB_SIZE);
					dst1[x*2] = saturate_cast<short>(ix >> INTER_BITS);
					dst1[x*2+1] = saturate_cast<short>(iy >> INTER_BITS);
					dst2[x] = (ushort)((iy & (INTER_TAB_SIZE-1))*INTER_TAB_SIZE + (ix & (INTER_TAB_SIZE-1)));
				}
		}
		else if( m1type == MAT_Tfloat2 && dstm1type == MAT_Tshort2 )
		{
			if( nninterpolate )
				for( x = 0; x < size.width; x++ )
				{
					dst1[x*2] = saturate_cast<short>(src1f[x*2]);
					dst1[x*2+1] = saturate_cast<short>(src1f[x*2+1]);
				}
			else
				for( x = 0; x < size.width; x++ )
				{
					int ix = saturate_cast<int>(src1f[x*2]*INTER_TAB_SIZE);
					int iy = saturate_cast<int>(src1f[x*2+1]*INTER_TAB_SIZE);
					dst1[x*2] = saturate_cast<short>(ix >> INTER_BITS);
					dst1[x*2+1] = saturate_cast<short>(iy >> INTER_BITS);
					dst2[x] = (ushort)((iy & (INTER_TAB_SIZE-1))*INTER_TAB_SIZE + (ix & (INTER_TAB_SIZE-1)));
				}
		}
		else if( m1type == MAT_Tshort2 && dstm1type == MAT_Tfloat )
		{
			for( x = 0; x < size.width; x++ )
			{
				int fxy = src2 ? src2[x] & (INTER_TAB_SIZE2-1) : 0;
				dst1f[x] = src1[x*2] + (fxy & (INTER_TAB_SIZE-1))*scale;
				dst2f[x] = src1[x*2+1] + (fxy >> INTER_BITS)*scale;
			}
		}
		else if( m1type == MAT_Tshort2 && dstm1type == MAT_Tfloat2 )
		{
			for( x = 0; x < size.width; x++ )
			{
				int fxy = src2 ? src2[x] & (INTER_TAB_SIZE2-1): 0;
				dst1f[x*2] = src1[x*2] + (fxy & (INTER_TAB_SIZE-1))*scale;
				dst1f[x*2+1] = src1[x*2+1] + (fxy >> INTER_BITS)*scale;
			}
		}
		else
			assert (false);//"Unsupported combination of input/output matrices"
	}
}

}}

#pragma warning (pop)


