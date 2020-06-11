/*!
 * \file
 * \brief 
 * \author
 */
#include <assert.h>
#include "ScaleXY.h"
#include "Warping.h"

namespace cvlib
{


	class ScaleXY
	{
	public:
		//! Construction and Destruction
		ScaleXY();
		virtual ~ScaleXY();

		//! Operations
		void process(Mat* pDst, int nXOrgDst, int nYOrgDst, int nWidthDst, int nHeightDst,
			const Mat* pSrc, int nXOrgSrc, int nYOrgSrc, int nWidthSrc, int nHeightSrc);
		void process(Mat* pDst, const Mat* pSrc);

		void FastProcess(const Mat* pmatSrc, Mat* pmatDest);
		double BilinearInterpolation(uchar** ppbImage, int nX, int nY, double rX, double rY);
		double BilinearInterpolation(float** pprImage, int nX, int nY, double rX, double rY);
		Mat* process(const Mat* pSrc, float rScale);
	};

	ScaleXY::ScaleXY()
	{
	}

	ScaleXY::~ScaleXY()
	{
	}

	void	ScaleXY::process(Mat* pDst, int nXOrgDst, int nYOrgDst, int nWidthDst, int nHeightDst,
		const Mat* pSrc, int nXOrgSrc, int nYOrgSrc, int nWidthSrc, int nHeightSrc)
	{
		switch (pDst->type())
		{
		case MAT_Tuchar:
		{
			uchar** ppbDst = pDst->data.ptr;
			uchar** ppbSrc = pSrc->data.ptr;

			int	iH, iW;
			double	rHRate, rWRate;

			rHRate = (double)nHeightSrc / (double)nHeightDst;
			rWRate = (double)nWidthSrc / (double)nWidthDst;

			int	nNewRow, nNewCol;
			double	rA = 0.0f, rB = 0.0f;
			for (iH = 0; iH < nHeightDst; iH++)
			{
				double rHScale = rA;
				nNewRow = (int)rA;
				if (nNewRow < nHeightSrc - 1)
					rHScale -= nNewRow;
				else
					rHScale = 0.0f;

				rB = 0.0f;
				for (iW = 0; iW < nWidthDst; iW++)
				{
					double rWScale = rB;
					nNewCol = (int)rB;
					if (nNewCol < nWidthSrc - 1)
						rWScale -= nNewCol;
					else
						rWScale = 0.0f;

					ppbDst[iH + nYOrgDst][iW + nXOrgDst] = (uchar)BilinearInterpolation(
						ppbSrc, nNewCol + nXOrgSrc, nNewRow + nYOrgSrc, rWScale, rHScale);
					rB += rWRate;
				}
				rA += rHRate;
			}
		}
		break;
		case MAT_Tfloat:
		{
			float** pprDst = pDst->data.fl;
			float** pprSrc = pSrc->data.fl;

			int	iH, iW;
			double	rHRate, rWRate;

			rHRate = (double)nHeightSrc / (double)nHeightDst;
			rWRate = (double)nWidthSrc / (double)nWidthDst;

			int	nNewRow, nNewCol;
			double	rA = 0.0f, rB = 0.0f;
			for (iH = 0; iH < nHeightDst; iH++)
			{
				double rHScale = rA;
				nNewRow = (int)rA;
				if (nNewRow < nHeightSrc - 1)
					rHScale -= nNewRow;
				else
					rHScale = 0.0f;

				rB = 0.0f;
				for (iW = 0; iW < nWidthDst; iW++)
				{
					double rWScale = rB;
					nNewCol = (int)rB;
					if (nNewCol < nWidthSrc - 1)
						rWScale -= nNewCol;
					else
						rWScale = 0.0f;

					pprDst[iH + nYOrgDst][iW + nXOrgDst] = (uchar)BilinearInterpolation(
						pprSrc, nNewCol + nXOrgSrc, nNewRow + nYOrgSrc, rWScale, rHScale);
					rB += rWRate;
				}
				rA += rHRate;
			}
		}
		break;
		default:
			assert(false);
		}
	}

	double ScaleXY::BilinearInterpolation(uchar** ppbImage, int nX, int nY, double rX, double rY)
	{
		double	rTemp;

		rTemp = ppbImage[nY][nX];
		if (rX)
			rTemp += rX * (ppbImage[nY][nX + 1] - ppbImage[nY][nX]);

		if (rY)
			rTemp += rY * (ppbImage[nY + 1][nX] - ppbImage[nY][nX]);

		if (rX && rY)
			rTemp += rX * rY * (
				ppbImage[nY + 1][nX + 1] + ppbImage[nY][nX] -
				ppbImage[nY][nX + 1] - ppbImage[nY + 1][nX]);

		return rTemp;
	}

	double ScaleXY::BilinearInterpolation(float** pprImage, int nX, int nY, double rX, double rY)
	{
		double	rTemp;

		rTemp = pprImage[nY][nX];
		if (rX)
			rTemp += rX * (pprImage[nY][nX + 1] - pprImage[nY][nX]);

		if (rY)
			rTemp += rY * (pprImage[nY + 1][nX] - pprImage[nY][nX]);

		if (rX && rY)
			rTemp += rX * rY * (
				pprImage[nY + 1][nX + 1] + pprImage[nY][nX] -
				pprImage[nY][nX + 1] - pprImage[nY + 1][nX]);

		return rTemp;
	}

	void fastProcess_8u(const Mat* pmatSrc, Mat* pmatDest)
	{
		assert(pmatSrc->type() == MAT_Tuchar);
		assert(pmatDest->type() == MAT_Tuchar);
		assert(pmatSrc->type1() == pmatDest->type1());

		int	i, j, k;
		int	ndstW = pmatDest->cols();
		int	nShrink = pmatSrc->cols() / ndstW;
		int	nRest = pmatSrc->cols() % ndstW;
		int cn = pmatSrc->channels();

		for (i = 0; i < pmatDest->rows(); i++)
		{
			int		nR, nNR;
			int		nSkip = 0;
			int		nShift = 0;
			uchar	bNext;

			nR = i * pmatSrc->rows() / pmatDest->rows();
			nNR = (i + 1) * pmatSrc->rows() / pmatDest->rows();
			if (nNR - nR > 1)
				bNext = 1;
			else
				bNext = 0;
			uchar*	pbSrc = pmatSrc->data.ptr[nR];
			uchar*	pbSrcNext = pmatSrc->data.ptr[nR + bNext];
			uchar*	pbDst = pmatDest->data.ptr[i];
			if (cn == 1)
			{
				for (j = 0; j < ndstW; j++)
				{
					int	nC;
					int	nSum;

					nSkip += nRest;
					nC = j * nShrink + nShift;
					nSum = pbSrc[nC] + pbSrcNext[nC];
					if (nSkip >= ndstW)
					{
						nShift++;
						nSkip -= ndstW;
						if (nC + 1 < pmatSrc->cols())
						{
							nSum += pbSrc[nC + 1] + pbSrcNext[nC + 1];
							nSum >>= 2;
						}
						else
						{
							nSum >>= 1;
						}
					}
					else
						nSum >>= 1;
					pbDst[j] = (uchar)nSum;
				}
			}
			else
			{
				int pixel = 0;
				for (j = 0; j < ndstW; j++)
				{
					int	nC;
					nSkip += nRest;
					nC = j * nShrink + nShift;
					if (nSkip < ndstW)
					{
						int nC_cn = nC*cn;
						for (k = 0; k < cn; k++, pixel++, nC_cn++)
						{
							pbDst[pixel] = (uchar)((pbSrc[nC_cn] + pbSrcNext[nC_cn]) >> 1);
						}
					}
					else
					{
						int nC_cn = nC*cn;
						if (nC_cn + cn < pmatSrc->cols()*cn)
						{
							for (k = 0; k < cn; k++, pixel++, nC_cn++)
							{
								pbDst[pixel] = (uchar)((pbSrc[nC_cn] + pbSrcNext[nC_cn] + pbSrc[nC_cn + cn] + pbSrcNext[nC_cn + cn]) >> 2);
							}
						}
						else
						{
							for (k = 0; k < cn; k++, pixel++, nC_cn++)
							{
								pbDst[pixel] = (uchar)(((int)(pbSrc[nC_cn] + pbSrcNext[nC_cn])) >> 1);
							}
						}
						nShift++;
						nSkip -= ndstW;
					}
				}
			}
		}
	}

	typedef void(*BilinearInterpolationFunc)(uchar** _dst, int x, int y, uchar** const _src, float rX, float rY, int channel);
	template<typename _Tp> static void
		bilinearInterpolation_(uchar** _dst, int x, int y, uchar** const _src, float rX, float rY, int channel)
	{
		_Tp** const src = (_Tp** const)_src;
		_Tp** dst = (_Tp**)_dst;
		int nX = (int)rX; rX -= nX;
		int nY = (int)rY; rY -= nY;
		int n = nX*channel;
		int dstx = x*channel;
		_Tp* prcur = (_Tp*)src[nY];
		_Tp* prnext = (_Tp*)src[nY + 1];
		_Tp* pdst = (_Tp*)dst[y];
		for (int i = 0; i < channel; i++, n++)
		{
			int n1 = n + channel;
			float rTemp = (float)prcur[n];

			if (rX)
				rTemp += rX * (float)(prcur[n1] - prcur[n]);
			if (rY)
				rTemp += rY * (float)(prnext[n] - prcur[n]);
			if (rX && rY)
				rTemp += rX * rY * (float)(prnext[n1] + prcur[n] - prcur[n1] - prnext[n]);
			pdst[dstx++] = (_Tp)rTemp;
		}
	}
	typedef void(*ResizeFunc)(uchar** const src, uchar** dst, Size srcsize, Size dstsize, int cn);
	template<typename _Tp> static void
		resizeFunc_(uchar** const _src, uchar** _dst, Size srcsize, Size dstsize, int cn)
	{
		_Tp** const src = (_Tp** const)_src;
		_Tp** dst = (_Tp**)_dst;
		double sx = (srcsize.width - 1) / (double)(dstsize.width - 1);
		double sy = (srcsize.height - 1) / (double)(dstsize.height - 1);

		int* pnx = new int[dstsize.width];
		float* prx = new float[dstsize.width];

		float xpos = 0.0f, ypos = 0.0f;
		for (int ix = 0; ix < dstsize.width; ix++)
		{
			pnx[ix] = (int)xpos;
			prx[ix] = xpos - pnx[ix];
			pnx[ix] *= cn;
			xpos += (float)sx;
		}
		for (int iy = 0; iy < dstsize.height; iy++)
		{
			_Tp* d = dst[iy];

			int ny = (int)ypos;
			float ry = ypos - ny;

			_Tp* s0 = src[ny];
			_Tp* s1 = ny == srcsize.height - 1 ? src[ny] : src[ny + 1];

			for (int ix = 0, iix = 0; ix < dstsize.width; ix++)
			{
				int n = pnx[ix];
				if (ix != dstsize.width - 1) {
					int n1 = n + cn;
					float rx = prx[ix];
					for (int ich = 0; ich < cn; ich++, n++, n1++, iix++)
					{
						float rTemp = (float)s0[n];
						if (rx)
							rTemp += rx * (float)(s0[n1] - s0[n]);
						if (ry)
							rTemp += ry * (float)(s1[n] - s0[n]);
						if (rx && ry)
							rTemp += rx * ry * (float)(s1[n1] + s0[n] - s0[n1] - s1[n]);
						d[iix] = (_Tp)rTemp;
					}
				}
				else {
					for (int ich = 0; ich < cn; ich++, n++, iix++)
						d[iix] = (_Tp)s0[n];
				}
			}
			ypos += (float)sy;
		}
		delete[]pnx;
		delete[]prx;
	}
	void ScaleXY::FastProcess(const Mat* src, Mat* dst)
	{
		if (src->type() == MAT_Tuchar || src->type() == MAT_Tchar)
		{
			fastProcess_8u(src, dst);
			return;
		}

		ResizeFunc resizeFuncs[6] =
		{ 0, 0, resizeFunc_<short>, resizeFunc_<int>, resizeFunc_<float>, resizeFunc_<double> };
		ResizeFunc func = resizeFuncs[src->type()];
		func(src->data.ptr, dst->data.ptr, src->size(), dst->size(), src->channels());
	}

	void ScaleXY::process(Mat* pDst, const Mat* pSrc)
	{
		process(pDst, 0, 0, pDst->cols(), pDst->rows(),
			pSrc, 0, 0, pSrc->cols(), pSrc->rows());
	}

	Mat* ScaleXY::process(const Mat* pSrc, float rScale)
	{
		Mat* pDst = new Mat((int)(pSrc->rows() * rScale + 0.5f), (int)(pSrc->cols() * rScale + 0.5f), MAT_Tuchar);
		process(pDst, pSrc);
		return pDst;
	}

	namespace ip
	{
		void image_resize(const Mat& src, Mat& dst, Size dsize, double inv_scale_x, double inv_scale_y, int interpolation);

		void resize(const Mat& src, Mat& dst, int newx, int newy, int interpolation)
		{
			Mat t;
			const Mat* psrc = &src;
			if (src.data.ptr == dst.data.ptr) {
				t = src;
				psrc = &t;
			}
			dst.create(newy, newx, src.type1());
			if (interpolation == INTER_NEAREST) {
				ScaleXY scaler;
				scaler.FastProcess(psrc, &dst);
			}
			else {
				image_resize(*psrc, dst, Size(newx, newy), 0, 0, interpolation);
			}
		}
		void resize(const Mat& src, Mat& dst, float rate)
		{
			int newx = (int)(src.cols()*rate + 0.5f);
			int newy = (int)(src.rows()*rate + 0.5f);
			resize(src, dst, newx, newy, INTER_NEAREST);
		}
		void resize(Mat& src, int newx, int newy)
		{
			Mat dst = src;
			src.create(newy, newx, dst.type1());
			ScaleXY scaler;
			scaler.FastProcess(&dst, &src);
		}
		void resize(Mat& src, float rate)
		{
			int newx = (int)(src.cols()*rate + 0.5f);
			int newy = (int)(src.rows()*rate + 0.5f);
			resize(src, newx, newy);
		}
	}

}
