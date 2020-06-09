/*!
 * \file	MatOp.cpp
 * \ingroup base
 * \brief	
 * \author	
 */

#include "MatOperation.h"
#include "AutoBuffer.h"
#include <algorithm>

namespace cvlib
{

	void MatOp::copyMat(Mat& dst, const Mat& src, const Rect& srcRect, const Point2i& dstpt)
	{
		if (!src.isValid())
			return;

		if (!dst.isValid() || (dst.isValid() && (dst.rows()<srcRect.height || dst.cols()<srcRect.width)))
		{
			dst.release();
			dst.create(srcRect.height, srcRect.width, src.type1());
		}

		int nRight2, nBottom2;
		int nCopyW, nCopyH;

		int nSrcW = src.cols();
		int nSrcH = src.rows();
		uchar** ppbSrc = src.data.ptr;

		int nDstW = dst.cols();
		int nDstH = dst.rows();
		uchar** ppbDst = dst.data.ptr;

		assert(srcRect.x >= 0 && srcRect.x < nSrcW);
		assert(srcRect.limx() >= 0 && srcRect.limx() <= nSrcW);
		assert(srcRect.y >= 0 && srcRect.y < nSrcH);
		assert(srcRect.limy() >= 0 && srcRect.limy() <= nSrcH);

		nCopyW = srcRect.width;
		nCopyH = srcRect.height;
		nRight2 = dstpt.x + nCopyW - 1;
		nBottom2 = dstpt.y + nCopyH - 1;
		assert(dstpt.x >= 0 && dstpt.x < nDstW);
		assert(nRight2 >= 0 && nRight2 < nDstW);
		assert(dstpt.y >= 0 && dstpt.y < nDstH);
		assert(nBottom2 >= 0 && nBottom2 < nDstH);

		int ncopyline = dst.rowByteSize();
		int noffset = srcRect.x*src.step()*src.channels();
		for (int iH = dstpt.y; iH <= nBottom2; iH++)
		{
			uchar* pbSrcLine = ppbSrc[iH - dstpt.y + srcRect.y] + noffset;
			uchar* pbDstLine = ppbDst[iH];
			memcpy(pbDstLine, pbSrcLine, ncopyline);
		}
	}

	static void	icopyMat(Mat* pDst, const Mat* pSrc, TYPE dsttype)
	{
		assert(pDst && pSrc);
		int i, j;
		int nRows = pSrc->rows();
		int nCols = pSrc->cols();

		if (!pDst->isValid() || pDst->rows() != nRows || pDst->cols() != nCols)
		{
			pDst->release();
			pDst->create(nRows, nCols, dsttype);
		}

		if (dsttype == pSrc->type())
		{
			int nlinestep = nCols * pSrc->step()*pSrc->channels();
			for (i = 0; i < nRows; i++)
				memcpy(pDst->data.ptr[i], pSrc->data.ptr[i], nlinestep);
		}
		else
		{
			switch (pDst->type())
			{
			case MAT_Tbyte:
				for (i = 0; i < nRows; i++)
					for (j = 0; j < nCols; j++)
						pDst->data.ptr[i][j] = (uchar)pSrc->value(i, j);
				break;
			case MAT_Tshort:
				for (i = 0; i < nRows; i++)
					for (j = 0; j < nCols; j++)
						pDst->data.s[i][j] = (short)pSrc->value(i, j);
				break;
			case MAT_Tint:
				for (i = 0; i < nRows; i++)
					for (j = 0; j < nCols; j++)
						pDst->data.i[i][j] = (int)pSrc->value(i, j);
				break;
			case MAT_Tfloat:
				for (i = 0; i < nRows; i++)
					for (j = 0; j < nCols; j++)
						pDst->data.fl[i][j] = (float)pSrc->value(i, j);
				break;
			case MAT_Tdouble:
				for (i = 0; i < nRows; i++)
					for (j = 0; j < nCols; j++)
						pDst->data.db[i][j] = (double)pSrc->value(i, j);
				break;
			default:
				assert(false);
			}
		}
	}
	void	MatOp::mul(Mat* pC, const Vec* pA, const Vec* pB)
	{
		assert(pA->type() == pB->type());
		assert(pB->type() == pC->type());

		assert(pA->length() == pB->length());
		assert(pC->rows() == pC->cols());
		assert(pA->length() == pC->cols());

		switch (pA->type())
		{
		case MAT_Tbyte:
			return;
		case MAT_Tshort:
		{
			short**	pprC = pC->data.s;
			short*	prA = pA->data.s;
			short*	prB = pB->data.s;
			int nLen = pA->length();
			for (int i = 0; i < nLen; i++)
			{
				for (int j = 0; j < nLen; j++)
				{
					pprC[i][j] = prA[i] * prB[j];
				}
			}
		}
		break;
		case MAT_Tint:
		{
			int**	pprC = pC->data.i;
			int*	prA = pA->data.i;
			int*	prB = pB->data.i;
			int nLen = pA->length();
			for (int i = 0; i < nLen; i++)
			{
				for (int j = 0; j < nLen; j++)
				{
					pprC[i][j] = prA[i] * prB[j];
				}
			}
		}
		break;
		case MAT_Tfloat:
		{
			float**	pprC = pC->data.fl;
			float*	prA = pA->data.fl;
			float*	prB = pB->data.fl;
			int nLen = pA->length();
			for (int i = 0; i < nLen; i++)
			{
				for (int j = 0; j < nLen; j++)
				{
					pprC[i][j] = prA[i] * prB[j];
				}
			}
		}
		break;
		case MAT_Tdouble:
		{
			double** pprC = pC->data.db;
			double*	prA = pA->data.db;
			double*	prB = pB->data.db;
			int nLen = pA->length();
			for (int i = 0; i < nLen; i++)
			{
				for (int j = 0; j < nLen; j++)
				{
					pprC[i][j] = prA[i] * prB[j];
				}
			}
		}
		break;
		default:
			assert(false);
		}
	}

	typedef void(*addMatFunc)(uchar** pdst, uchar** const _src1, uchar** const _src2, int rows, int cols);
	template<typename _Tp> static void addMatFunc_(uchar** _dst, uchar** const _src1, uchar** const _src2, int rows, int cols)
	{
		int i, j;
		for (i = 0; i < rows; i++) {
			_Tp* const src1 = (_Tp* const)_src1[i];
			_Tp* const src2 = (_Tp* const)_src2[i];
			_Tp* dst = (_Tp*)_dst[i];
			for (j = 0; j < cols; j++) {
				dst[j] = (_Tp)(src1[j] + src2[j]);
			}
		}
	}
	void MatOp::add(Mat* pA, const Mat* pB, const Mat* pC)
	{
		assert((pA->rows() == pB->rows()) && (pA->cols() == pB->cols()));
		assert((pC->rows() == pB->rows()) && (pC->cols() == pB->cols()));
		assert((pA->type1() == pB->type1()) && (pB->type1() == pC->type1()));
		addMatFunc addfuncs[] = { addMatFunc_<char>, addMatFunc_<uchar>, addMatFunc_<short>,
			addMatFunc_<int>, addMatFunc_<float>, addMatFunc_<double> };
		int type = pA->type();
		int nrows = pA->rows(), ncols = pA->cols() * pA->channels();
		if (pA->isContinuous() && pB->isContinuous() && pC->isContinuous())
		{
			addfuncs[type](pA->data.ptr, pB->data.ptr, pC->data.ptr, 1, nrows * ncols);
		}
		else
		{
			addfuncs[type](pA->data.ptr, pB->data.ptr, pC->data.ptr, nrows, ncols);
		}
	}
	typedef void(*subMatFunc)(uchar** pdst, uchar** const _src1, uchar** const _src2, int rows, int cols);
	template<typename _Tp> static void subMatFunc_(uchar** _dst, uchar** const _src1, uchar** const _src2, int rows, int cols)
	{
		int i, j;
		for (i = 0; i < rows; i++) {
			_Tp* const src1 = (_Tp* const)_src1[i];
			_Tp* const src2 = (_Tp* const)_src2[i];
			_Tp* dst = (_Tp*)_dst[i];
			for (j = 0; j < cols; j++) {
				dst[j] = (_Tp)(src1[j] - src2[j]);
			}
		}
	}
	void MatOp::sub(Mat* pA, const Mat* pB, const Mat* pC)
	{
		assert((pA->rows() == pB->rows()) && (pA->cols() == pB->cols()));
		assert((pC->rows() == pB->rows()) && (pC->cols() == pB->cols()));
		assert((pA->type1() == pB->type1()) && (pB->type1() == pC->type1()));
		addMatFunc subfuncs[] = { subMatFunc_<char>, subMatFunc_<uchar>, subMatFunc_<short>,
			subMatFunc_<int>, subMatFunc_<float>, subMatFunc_<double> };
		int type = pA->type();
		int nrows = pA->rows(), ncols = pA->cols() * pA->channels();
		if (pA->isContinuous() && pB->isContinuous() && pC->isContinuous())
		{
			subfuncs[type](pA->data.ptr, pB->data.ptr, pC->data.ptr, 1, nrows * ncols);
		}
		else
		{
			subfuncs[type](pA->data.ptr, pB->data.ptr, pC->data.ptr, nrows, ncols);
		}
	}

	void MatOp::mul(Mat* pB, double rReal, const Mat* pA)
	{
		assert((pA->rows() == pB->rows()) && (pA->cols() == pB->cols()));
		assert(pA->type1() == pB->type1());
		if (!pA->isValid() || !pB->isValid())
			return;

		int nRows = pA->rows(), nCols = pA->cols(), len = nRows*nCols*pA->channels();
		switch (pA->type())
		{
		case MAT_Tbyte:
		{
			uchar* ppbA = pA->data.ptr[0];
			uchar* ppbB = pB->data.ptr[0];
			for (int i = 0; i < len; i++)
				ppbB[i] = (uchar)(rReal * ppbA[i]);
		}
		break;
		case MAT_Tshort:
		{
			short* ppnA = pA->data.s[0];
			short* ppnB = pB->data.s[0];
			for (int i = 0; i < len; i++)
				ppnB[i] = (short)(rReal * ppnA[i]);
		}
		break;
		case MAT_Tint:
		{
			int* ppnA = pA->data.i[0];
			int* ppnB = pB->data.i[0];
			for (int i = 0; i < len; i++)
				ppnB[i] = (int)(rReal * ppnA[i]);
		}
		break;
		case MAT_Tfloat:
		{
			float* pprA = pA->data.fl[0];
			float* pprB = pB->data.fl[0];
			for (int i = 0; i < len; i++)
				pprB[i] = (float)(rReal * pprA[i]);
		}
		break;
		case MAT_Tdouble:
		{
			double* pprA = pA->data.db[0];
			double* pprB = pB->data.db[0];
			for (int i = 0; i < len; i++)
				pprB[i] = (double)(rReal * pprA[i]);
		}
		break;
		default:
			assert(false);
		}
	}

	void MatOp::sub(Vec* pA, const Vec* pB, const Vec* pC)
	{
		assert(pA->length() == pB->length());
		assert(pA->length() == pC->length());
		assert(pA->type() == pB->type());
		assert(pA->type() == pC->type());

		int i;
		int		nLen = pA->length();
		switch (pA->type())
		{
		case MAT_Tbyte:
		{
			uchar* pA_ = pA->data.ptr;
			for (i = 0; i < nLen; i++)
				pA_[i] = (uchar)(pB->data.ptr[i] - pC->data.ptr[i]);
		}
		break;
		case MAT_Tshort:
		{
			short* pA_ = pA->data.s;
			for (i = 0; i < nLen; i++)
				pA_[i] = (pB->data.s[i] - pC->data.s[i]);
		}
		break;
		case MAT_Tint:
		{
			int* pA_ = pA->data.i;
			for (i = 0; i < nLen; i++)
				pA_[i] = (pB->data.i[i] - pC->data.i[i]);
		}
		break;
		case MAT_Tfloat:
		{
			float* pA_ = pA->data.fl;
			for (i = 0; i < nLen; i++)
				pA_[i] = pB->data.fl[i] - pC->data.fl[i];
		}
		break;
		case MAT_Tdouble:
		{
			double* pA_ = pA->data.db;
			for (i = 0; i < nLen; i++)
				pA_[i] = pB->data.db[i] - pC->data.db[i];
		}
		break;
		default:
			assert(false);
		}
	}

	void MatOp::mul(Vec* pA, const Mat* pB, const Vec* pC)
	{
		assert((pA->length() == pB->rows()) && (pB->cols() == pC->length()));
		assert((pA->type() == pB->type()));
		assert((pB->type() == pC->type()));
		assert(pA->type() != MAT_Tbyte);

		int nRows = pB->rows(), nCols = pB->cols();

		pA->zero();

		switch (pA->type())
		{
		case MAT_Tshort:
		{
			short*	prA = pA->data.s;
			short**	pprB = pB->data.s;
			short*	prC = pC->data.s;
			for (int i = 0; i < nRows; i++)
			{
				for (int j = 0; j < nCols; j++)
				{
					prA[i] += pprB[i][j] * prC[j];
				}
			}
		}
		break;
		case MAT_Tint:
		{
			int*	prA = pA->data.i;
			int**	pprB = pB->data.i;
			int*	prC = pC->data.i;
			for (int i = 0; i < nRows; i++)
			{
				for (int j = 0; j < nCols; j++)
				{
					prA[i] += pprB[i][j] * prC[j];
				}
			}
		}
		break;
		case MAT_Tfloat:
		{
			float*	prA = pA->data.fl;
			float**	pprB = pB->data.fl;
			float*	prC = pC->data.fl;
			for (int i = 0; i < nRows; i++)
			{
				for (int j = 0; j < nCols; j++)
				{
					prA[i] += pprB[i][j] * prC[j];
				}
			}
		}
		break;
		case MAT_Tdouble:
		{
			double*	prA = pA->data.db;
			double** pprB = pB->data.db;
			double*	prC = pC->data.db;
			for (int i = 0; i < nRows; i++)
			{
				for (int j = 0; j < nCols; j++)
				{
					prA[i] += pprB[i][j] * prC[j];
				}
			}
		}
		break;
		default:
			assert(false);
		}
	}

	void MatOp::mul(Vec* pA, const Vec* pB, const Mat* pC)
	{
		assert((pA->length() == pC->cols()) && (pC->rows() == pB->length()));
		assert((pA->type() == pB->type()) && (pB->type() == pC->type()));

		int nRows = pC->rows(), nCols = pC->cols();

		switch (pA->type())
		{
		case MAT_Tfloat:
		{
			float*	prA = pA->data.fl;
			float*	prB = pB->data.fl;
			float**	pprC = pC->data.fl;
			pA->zero();
			for (int i = 0; i < nCols; i++)
			{
				for (int j = 0; j < nRows; j++)
				{
					prA[i] += prB[j] * pprC[j][i];
				}
			}
		}
		break;
		case MAT_Tdouble:
		{
			double*	prA = pA->data.db;
			double* prB = pB->data.db;
			double** pprC = pC->data.db;
			pA->zero();
			for (int i = 0; i < nCols; i++)
			{
				for (int j = 0; j < nRows; j++)
				{
					prA[i] += prB[j] * pprC[j][i];
				}
			}
		}
		break;
		default:
			assert(false);
		}
	}

	void MatOp::mul(Mat* pC, const Mat* pA, const Mat* pB)
	{
		assert(pA->cols() == pB->rows());
		assert(pC->rows() >= pA->rows() && pC->cols() >= pB->cols());
		assert(pA->type() == pB->type());
		assert(pA->type() == pC->type());

		int i, j, k, r = pA->rows(), c = pB->cols(), m = pB->rows();

		pC->zero();
		switch (pA->type())
		{
		case MAT_Tbyte:
		{
			uchar** ppbA = pA->data.ptr;
			uchar** ppbB = pB->data.ptr;
			uchar** ppbC = pC->data.ptr;

			for (i = 0; i < r; i++)
				for (j = 0; j < c; j++)
				{
					for (k = 0; k < m; k++)
						ppbC[i][j] += ppbA[i][k] * ppbB[k][j];
				}
		}
		break;
		case MAT_Tshort:
		{
			short** ppsA = pA->data.s;
			short** ppsB = pB->data.s;
			short** ppsC = pC->data.s;

			for (i = 0; i < r; i++)
				for (j = 0; j < c; j++)
				{
					for (k = 0; k < m; k++)
						ppsC[i][j] += ppsA[i][k] * ppsB[k][j];
				}
		}
		break;
		case MAT_Tint:
		{
			int** ppiA = pA->data.i;
			int** ppiB = pB->data.i;
			int** ppiC = pC->data.i;

			for (i = 0; i < r; i++)
				for (j = 0; j < c; j++)
				{
					for (k = 0; k < m; k++)
						ppiC[i][j] += ppiA[i][k] * ppiB[k][j];
				}
		}
		break;
		case MAT_Tfloat:
		{
			float** pprA = pA->data.fl;
			float** pprB = pB->data.fl;
			float** pprC = pC->data.fl;

			for (i = 0; i < r; i++)
				for (j = 0; j < c; j++)
				{
					for (k = 0; k < m; k++)
						pprC[i][j] += pprA[i][k] * pprB[k][j];
				}
		}
		break;
		case MAT_Tdouble:
		{
			double** ppdA = pA->data.db;
			double** ppdB = pB->data.db;
			double** ppdC = pC->data.db;

			for (i = 0; i < r; i++)
				for (j = 0; j < c; j++)
				{
					for (k = 0; k < m; k++)
						ppdC[i][j] += ppdA[i][k] * ppdB[k][j];
				}
		}
		break;
		default:
			assert(false);
		}
	}

	void MatOp::add(Vec* pA, const Vec* pB, const Vec* pC)
	{
		assert(pB->length() == pC->length());
		assert(pA->length() == pC->length());
		assert(pA->type() == pB->type());
		assert(pA->type() == pC->type());

		int i;
		int	nLen = pA->length();
		switch (pA->type())
		{
		case MAT_Tbyte:
		{
			uchar* pA_ = pA->data.ptr;
			for (i = 0; i < nLen; i++)
				pA_[i] = (uchar)(pB->data.ptr[i] + pC->data.ptr[i]);
		}
		break;
		case MAT_Tshort:
		{
			short* pA_ = pA->data.s;
			for (i = 0; i < nLen; i++)
				pA_[i] = (short)(pB->data.s[i] + pC->data.s[i]);
		}
		break;
		case MAT_Tint:
		{
			int* pA_ = pA->data.i;
			for (i = 0; i < nLen; i++)
				pA_[i] = (int)(pB->data.i[i] + pC->data.i[i]);
		}
		break;
		case MAT_Tfloat:
		{
			float* pA_ = pA->data.fl;
			for (i = 0; i < nLen; i++)
				pA_[i] = (float)(pB->data.fl[i] + pC->data.fl[i]);
		}
		break;
		case MAT_Tdouble:
		{
			double* pA_ = pA->data.db;
			for (i = 0; i < nLen; i++)
				pA_[i] = (double)(pB->data.db[i] + pC->data.db[i]);
		}
		break;
		default:
			assert(false);
		}
	}

	void MatOp::mul(Vec* pB, double	rReal, const Vec* pA)
	{
		assert(pA->length() > 0);

		int nLen = pA->length();
		switch (pA->type())
		{
		case MAT_Tbyte:
		{
			uchar* pbA = pA->data.ptr;
			uchar* pbB = pB->data.ptr;
			for (int i = 0; i < nLen; i++)
				pbB[i] = (uchar)(rReal * pbA[i]);
			break;
		}
		case MAT_Tshort:
		{
			short* pnA = pA->data.s;
			short* pnB = pB->data.s;
			for (int i = 0; i < nLen; i++)
				pnB[i] = (short)(rReal * pnA[i]);
			break;
		}
		case MAT_Tint:
		{
			int* pnA = pA->data.i;
			int* pnB = pB->data.i;
			for (int i = 0; i < nLen; i++)
				pnB[i] = (int)(rReal * pnA[i]);
			break;
		}
		case MAT_Tfloat:
		{
			float* prA = pA->data.fl;
			float* prB = pB->data.fl;
			for (int i = 0; i < nLen; i++)
				prB[i] = (float)(rReal * prA[i]);
			break;
		}
		case MAT_Tdouble:
		{
			double* prA = pA->data.db;
			double* prB = pB->data.db;
			for (int i = 0; i < nLen; i++)
				prB[i] = rReal * prA[i];
			break;
		}
		default:
			assert(false);
		}
	}

	float MatOp::trXAX(const Vec* pvX, const Mat* pmA)
	{
		// copy x into here because it's faster accessing an array than a Vec
		int nLen = pvX->length();

		assert(pmA->rows() == nLen);
		assert(pmA->cols() == nLen);
		assert(pvX->type() == MAT_Tfloat || pvX->type() == MAT_Tdouble);
		assert(pmA->type() == MAT_Tfloat || pmA->type() == MAT_Tdouble);

		if (pmA->type() == MAT_Tfloat)
		{
			float* prTemp = new float[nLen];
			float DiagResult = 0, Result = 0;

			// init prTemp and sum diag elements

			int i;
			for (i = 0; i < nLen; i++)
			{
				prTemp[i] = pvX->data.fl[i];
				DiagResult += pmA->data.fl[i][i] * prTemp[i] * prTemp[i];
			}
			// sum upper right triangle elems

			for (i = 0; i < nLen; i++)
			{
				float rXi = prTemp[i];
				for (int j = i + 1; j < nLen; j++)
					Result += pmA->data.fl[i][j] * rXi * prTemp[j];
			}
			Result *= 2;    // incorporate lower left triangle elements

			delete[] prTemp;

			return DiagResult + Result;
		}
		else if (pmA->type() == MAT_Tdouble)
		{
			double* prTemp = new double[nLen];
			double DiagResult = 0, Result = 0;

			// init prTemp and sum diag elements

			int i;
			for (i = 0; i < nLen; i++)
			{
				prTemp[i] = pvX->data.db[i];
				DiagResult += pmA->data.db[i][i] * prTemp[i] * prTemp[i];
			}
			// sum upper right triangle elems

			for (i = 0; i < nLen; i++)
			{
				double rXi = prTemp[i];
				for (int j = i + 1; j < nLen; j++)
					Result += pmA->data.db[i][j] * rXi * prTemp[j];
			}
			Result *= 2;    // incorporate lower left triangle elements

			delete[] prTemp;

			return (float)(DiagResult + Result);
		}
		return 0.0f;
	}

	void MatOp::trABA(Mat* pmC, const Mat* pmA, const Mat* pmB)
	{
		assert(pmA->type() == MAT_Tdouble);
		assert(pmB->type() == MAT_Tdouble);
		assert(pmC->type() == MAT_Tdouble);

		if (pmA->type() == MAT_Tdouble)
		{
			double** pprA = pmA->data.db;
			double** pprB = pmB->data.db;
			double** pprC = pmC->data.db;
			Mat mTemp(pmA->cols(), pmB->cols(), MAT_Tdouble);
			double** pprT = mTemp.data.db;

			int i, j, k;

			for (i = 0; i < pmA->cols(); i++)
			{
				for (j = 0; j < pmB->cols(); j++)
				{
					pprT[i][j] = 0.0;
					for (k = 0; k < pmA->rows(); k++)
						pprT[i][j] += pprA[k][i] * pprB[k][j];
				}
			}

			for (i = 0; i < mTemp.rows(); i++)
			{
				for (j = 0; j < pmA->cols(); j++)
				{
					pprC[i][j] = 0.0;
					for (k = 0; k < pmA->rows(); k++)
						pprC[i][j] += pprT[i][k] * pprA[k][j];
				}
			}
			mTemp.release();
		}
		else if (pmA->type() == MAT_Tfloat)
		{
			float** pprA = pmA->data.fl;
			float** pprB = pmB->data.fl;
			float** pprC = pmC->data.fl;
			Mat mTemp(pmA->cols(), pmB->cols(), MAT_Tfloat);
			float** pprT = mTemp.data.fl;

			int i, j, k;

			for (i = 0; i < pmA->cols(); i++)
			{
				for (j = 0; j < pmB->cols(); j++)
				{
					pprT[i][j] = 0.0;
					for (k = 0; k < pmA->rows(); k++)
						pprT[i][j] += pprA[k][i] * pprB[k][j];
				}
			}

			for (i = 0; i < mTemp.rows(); i++)
			{
				for (j = 0; j < pmA->cols(); j++)
				{
					pprC[i][j] = 0.0;
					for (k = 0; k < pmA->rows(); k++)
						pprC[i][j] += pprT[i][k] * pprA[k][j];
				}
			}
			mTemp.release();
		}
	}

	void MatOp::ABTrA(Mat* pmC, const Mat* pmA, const Mat* pmB)
	{
		assert(pmA->type() == MAT_Tfloat || pmA->type() == MAT_Tdouble);
		assert(pmB->type() == MAT_Tfloat || pmB->type() == MAT_Tdouble);
		assert(pmC->type() == MAT_Tfloat || pmC->type() == MAT_Tdouble);

		if (pmA->type() == MAT_Tdouble)
		{
			double** pprA = pmA->data.db;
			double** pprB = pmB->data.db;
			double** pprC = pmC->data.db;
			Mat mTemp(pmA->rows(), pmB->cols(), MAT_Tdouble);
			double** pprT = mTemp.data.db;

			int i, j, k;

			for (i = 0; i < pmA->rows(); i++)
			{
				for (j = 0; j < pmB->cols(); j++)
				{
					pprT[i][j] = 0.0;
					for (k = 0; k < pmA->cols(); k++)
						pprT[i][j] += pprA[i][k] * pprB[k][j];
				}
			}

			for (i = 0; i < mTemp.rows(); i++)
			{
				for (j = 0; j < pmA->rows(); j++)
				{
					pprC[i][j] = 0.0;
					for (k = 0; k < pmA->cols(); k++)
						pprC[i][j] += pprT[i][k] * pprA[j][k];
				}
			}
			mTemp.release();
		}
		else if (pmA->type() == MAT_Tfloat)
		{
			float** pprA = pmA->data.fl;
			float** pprB = pmB->data.fl;
			float** pprC = pmC->data.fl;
			Mat mTemp(pmA->rows(), pmB->cols(), MAT_Tfloat);
			float** pprT = mTemp.data.fl;

			int i, j, k;

			for (i = 0; i < pmA->rows(); i++)
			{
				for (j = 0; j < pmB->cols(); j++)
				{
					pprT[i][j] = 0.0;
					for (k = 0; k < pmA->cols(); k++)
						pprT[i][j] += pprA[i][k] * pprB[k][j];
				}
			}

			for (i = 0; i < mTemp.rows(); i++)
			{
				for (j = 0; j < pmA->rows(); j++)
				{
					pprC[i][j] = 0.0;
					for (k = 0; k < pmA->cols(); k++)
						pprC[i][j] += pprT[i][k] * pprA[j][k];
				}
			}
			mTemp.release();
		}
	}

	double MatOp::dotProduct(const Vec* pA, const Vec* pB)
	{
		assert(pA->length() == pB->length());
		assert(pA->type() == pB->type());

		int nLen = pA->length();
		int i;
		double rVal = 0.0;
		switch (pA->type())
		{
		case MAT_Tbyte:
			for (i = 0; i < nLen; i++)
				rVal += pA->data.ptr[i] * pB->data.ptr[i];
			break;
		case MAT_Tshort:
			for (i = 0; i < nLen; i++)
				rVal += pA->data.s[i] * pB->data.s[i];
			break;
		case MAT_Tint:
			for (i = 0; i < nLen; i++)
				rVal += pA->data.i[i] * pB->data.i[i];
			break;
		case MAT_Tfloat:
			for (i = 0; i < nLen; i++)
				rVal += pA->data.fl[i] * pB->data.fl[i];
			break;
		case MAT_Tdouble:
			for (i = 0; i < nLen; i++)
				rVal += pA->data.db[i] * pB->data.db[i];
			break;
		default:
			assert(false);
		}
		return rVal;
	}

	void MatOp::trAA(Mat* pmB, const Mat* pmA)
	{
		assert(pmA->type() == MAT_Tfloat || pmA->type() == MAT_Tdouble);
		assert(pmB->type() == MAT_Tfloat || pmB->type() == MAT_Tdouble);
		assert(pmB->rows() == pmA->cols() && pmB->cols() == pmA->cols());

		int i, j, k;
		if (pmA->type() == MAT_Tdouble)
		{
			double** pprA = pmA->data.db;
			double** pprB = pmB->data.db;
			for (i = 0; i < pmB->rows(); i++)
			{
				for (j = i; j < pmB->cols(); j++)
				{
					pprB[i][j] = 0.0;
					for (k = 0; k < pmA->rows(); k++)
						pprB[i][j] += pprA[k][i] * pprA[k][j];
				}
				for (j = 0; j < i; j++)
					pprB[i][j] = pprB[j][i];
			}
		}
		else if (pmA->type() == MAT_Tfloat)
		{
			float** pprA = pmA->data.fl;
			float** pprB = pmB->data.fl;
			for (i = 0; i < pmB->rows(); i++)
			{
				for (j = i; j < pmB->cols(); j++)
				{
					pprB[i][j] = 0.0;
					for (k = 0; k < pmA->rows(); k++)
						pprB[i][j] += pprA[k][i] * pprA[k][j];
				}
				for (j = 0; j < i; j++)
					pprB[i][j] = pprB[j][i];
			}
		}
	}

	void MatOp::ATrA(Mat* pmB, const Mat* pmA)
	{
		assert(pmA->type() == MAT_Tfloat || pmA->type() == MAT_Tdouble);
		assert(pmB->type() == MAT_Tfloat || pmB->type() == MAT_Tdouble);
		assert(pmB->rows() == pmA->rows() && pmB->cols() == pmA->rows());

		int i, j, k;

		if (pmA->type() == MAT_Tdouble)
		{
			double** pprA = pmA->data.db;
			double** pprB = pmB->data.db;
			for (i = 0; i < pmB->rows(); i++)
			{
				for (j = i; j < pmB->cols(); j++)
				{
					pprB[i][j] = 0.0;
					for (k = 0; k < pmA->cols(); k++)
						pprB[i][j] += pprA[i][k] * pprA[j][k];
				}
				for (j = 0; j < i; j++)
					pprB[i][j] = pprB[j][i];
			}
		}
		else if (pmA->type() == MAT_Tfloat)
		{
			float** pprA = pmA->data.fl;
			float** pprB = pmB->data.fl;
			for (i = 0; i < pmB->rows(); i++)
			{
				for (j = i; j < pmB->cols(); j++)
				{
					pprB[i][j] = 0.0;
					for (k = 0; k < pmA->cols(); k++)
						pprB[i][j] += pprA[i][k] * pprA[j][k];
				}
				for (j = 0; j < i; j++)
					pprB[i][j] = pprB[j][i];
			}
		}
	}

	template<typename _Tp> static void inRange_(const _Tp* src, const _Tp* a, const _Tp* b, uchar* dst, int total)
	{
		int i;
		for (i = 0; i < total; i++)
		{
			_Tp val = src[i];
			dst[i] = a[i] <= val && val <= b[i] ? 255 : 0;
		}
		// 	for( c = 1; c < cn; c++ )
		// 	{
		// 		for( i = 0; i < total; i++ )
		// 		{
		// 			_Tp val = src[i*cn + c];
		// 			dst[i] = a[i*cn + c] <= val && val <= b[i*cn + c] ? dst[i] : 0;
		// 		}
		// 	}
	}


	void MatOp::inRange(const Mat& src, const Mat& lb, const Mat& rb, Mat& dst)
	{
		assert(src.isEqualMat(lb) && src.isEqualMat(rb));
		dst.release();
		dst.create(src.size(), MAT_Tbyte);

		int total = src.rows()*src.cols();
		{
			const uchar* sptr = src.data.ptr[0];
			const uchar* aptr = lb.data.ptr[0];
			const uchar* bptr = rb.data.ptr[0];
			uchar* dptr = dst.data.ptr[0];

			switch (src.type())
			{
			case MAT_Tbyte:
				inRange_((const uchar*)sptr, (const uchar*)aptr, (const uchar*)bptr, dptr, total);
				break;
			case MAT_Tshort:
				inRange_((const short*)sptr, (const short*)aptr, (const short*)bptr, dptr, total);
				break;
			case MAT_Tint:
				inRange_((const int*)sptr, (const int*)aptr, (const int*)bptr, dptr, total);
				break;
			case MAT_Tfloat:
				inRange_((const float*)sptr, (const float*)aptr, (const float*)bptr, dptr, total);
				break;
			case MAT_Tdouble:
				inRange_((const double*)sptr, (const double*)aptr, (const double*)bptr, dptr, total);
				break;
			default:
				assert(false);
			}
		}
	}

	template<typename _Tp, typename _WTp, typename _DTp> 
	static void inRangeS_(const _Tp* src, const _WTp a, const _WTp b, _DTp* dst, int total, int cn = 1)
	{
		int i, icn;
		for (i = 0, icn = 0; i < total; icn += cn, i++)
		{
			if (dst[i] > 0)
			{
				if ((a > src[icn]) || (src[icn] > b))
					dst[i] = 0;
			}
		}
	}

	void MatOp::inRange(const Mat& src, const double lb, const double rb, Mat& dst)
	{
		if (!src.isEqualSize(dst) || dst.type() != MAT_Tbyte)
		{
			dst.release();
			dst.create(src.size(), MAT_Tbyte);
		}

		int total = src.rows()*src.cols();
		{
			const uchar* sptr = src.data.ptr[0];
			uchar* dptr = dst.data.ptr[0];

			switch (src.type())
			{
			case MAT_Tbyte:
				inRangeS_((const uchar*)sptr, lb, rb, (uchar*)dptr, total);
				break;
			case MAT_Tshort:
				inRangeS_((const short*)sptr, lb, rb, (short*)dptr, total);
				break;
			case MAT_Tint:
				inRangeS_((const int*)sptr, lb, rb, (int*)dptr, total);
				break;
			case MAT_Tfloat:
				inRangeS_((const float*)sptr, lb, rb, (float*)dptr, total);
				break;
			case MAT_Tdouble:
				inRangeS_((const double*)sptr, lb, rb, (double*)dptr, total);
				break;
			default:
				assert(false);
			}
		}
	}

	void MatOp::inRange(const Mat& src, const double lb[3], const double rb[3], Mat& dst, int dims)
	{
		if (src.size() != dst.size() || dst.type() != MAT_Tbyte)
		{
			dst.release();
			dst.create(src.size(), MAT_Tbyte);
		}
		dst = 255;
		int total = src.size().area();
		int cn = src.channels();
		for (int k = 0; k < MIN(dims, src.channels()); k++)
		{
			const uchar* sptr = &src.data.ptr[0][k];
			uchar* dptr = dst.data.ptr[0];

			switch (src.type())
			{
			case MAT_Tbyte:
				inRangeS_((const uchar*)sptr, lb[k], rb[k], (uchar*)dptr, total, cn);
				break;
			case MAT_Tshort:
				inRangeS_((const short*)sptr, lb[k], rb[k], (uchar*)dptr, total, cn);
				break;
			case MAT_Tint:
				inRangeS_((const int*)sptr, lb[k], rb[k], (uchar*)dptr, total, cn);
				break;
			case MAT_Tfloat:
				inRangeS_((const float*)sptr, lb[k], rb[k], (uchar*)dptr, total, cn);
				break;
			case MAT_Tdouble:
				inRangeS_((const double*)sptr, lb[k], rb[k], (uchar*)dptr, total, cn);
				break;
			default:
				assert(false);
			}
		}
	}

	void MatOp::normalize(const Mat& src, Mat& dst, double a, double b,
		int norm_type, TYPE rtype, const Mat& mask)
	{
		double scale = 1, shift = 0;
		if (norm_type == CVLIB_MINMAX)
		{
			double smin = 0, smax = 0;
			double dmin = MIN(a, b), dmax = MAX(a, b);
			src.minMaxLoc(&smin, &smax, 0, 0, mask);
			scale = (dmax - dmin)*((smax - smin > DBL_EPSILON) ? 1. / (smax - smin) : 0);
			shift = dmin - smin*scale;
		}
		else if (norm_type == CVLIB_L2 || norm_type == CVLIB_L1 || norm_type == CVLIB_C)
		{
			scale = src.norm(norm_type, mask);
			scale = (scale > DBL_EPSILON) ? a / scale : 0.;
			shift = 0;
		}
		else
			assert(0);

		rtype = MAX(dst.type(), rtype);

		if (src.size() != dst.size() || rtype != dst.type())
		{
			dst.release();
			dst.create(src.size(), rtype);
		}

		if (!mask.isValid())
		{
			icopyMat(&dst, &src, rtype);
			dst.convert(rtype, scale, shift);
		}
		else
		{
			Mat temp;
			icopyMat(&temp, &src, rtype);
			temp.convert(rtype, scale, shift);
			temp.copyTo(dst, mask);
		}
	}

	typedef void(*addWeightedFunc)(uchar** const src1, uchar** const src2, uchar** dst, Size sz, void*);
	template<typename _Tp, typename WT> static void
		addWeighted_(uchar** const _src1, uchar** const _src2, uchar** _dst, Size size, void* _scalars)
	{
		_Tp** const src1 = (_Tp** const)_src1;
		_Tp** const src2 = (_Tp** const)_src2;
		_Tp** dst = (_Tp**)_dst;

		const double* scalars = (const double*)_scalars;
		WT alpha = (WT)scalars[0], beta = (WT)scalars[1], gamma = (WT)scalars[2];

		if (alpha == 0)
		{
			if (beta == 1)
			{
				for (int y = 0; y < size.height; y++)
				{
					const _Tp* s2 = src2[y];
					_Tp* d = dst[y];
					int x = 0;
					//#if CVLIB_ENABLE_UNROLLED
					for (; x <= size.width - 4; x += 4)
					{
						_Tp t0 = (_Tp)(s2[x] + gamma);
						_Tp t1 = (_Tp)(s2[x + 1] + gamma);
						d[x] = t0; d[x + 1] = t1;

						t0 = (_Tp)(s2[x + 2] + gamma);
						t1 = (_Tp)(s2[x + 3] + gamma);
						d[x + 2] = t0; d[x + 3] = t1;
					}
					//#endif
					for (; x < size.width; x++)
						d[x] = (_Tp)(s2[x] + gamma);
				}
			}
			else
			{
				for (int y = 0; y < size.height; y++)
				{
					const _Tp* s2 = src2[y];
					_Tp* d = dst[y];
					int x = 0;
					//#if CVLIB_ENABLE_UNROLLED
					for (; x <= size.width - 4; x += 4)
					{
						_Tp t0 = (_Tp)(s2[x] * beta + gamma);
						_Tp t1 = (_Tp)(s2[x + 1] * beta + gamma);
						d[x] = t0; d[x + 1] = t1;

						t0 = (_Tp)(s2[x + 2] * beta + gamma);
						t1 = (_Tp)(s2[x + 3] * beta + gamma);
						d[x + 2] = t0; d[x + 3] = t1;
					}
					//#endif
					for (; x < size.width; x++)
						d[x] = (_Tp)(s2[x] * beta + gamma);
				}
			}
		}
		else if (beta == 0)
		{
			if (alpha == 1)
			{
				for (int y = 0; y < size.height; y++)
				{
					const _Tp* s1 = src1[y];
					_Tp* d = dst[y];
					int x = 0;
					//#if CVLIB_ENABLE_UNROLLED
					for (; x <= size.width - 4; x += 4)
					{
						_Tp t0 = (_Tp)(s1[x] + gamma);
						_Tp t1 = (_Tp)(s1[x + 1] + gamma);
						d[x] = t0; d[x + 1] = t1;

						t0 = (_Tp)(s1[x + 2] + gamma);
						t1 = (_Tp)(s1[x + 3] + gamma);
						d[x + 2] = t0; d[x + 3] = t1;
					}
					//#endif
					for (; x < size.width; x++)
						d[x] = (_Tp)(s1[x] + gamma);
				}
			}
			else
			{
				for (int y = 0; y < size.height; y++)
				{
					const _Tp* s1 = src1[y];
					_Tp* d = dst[y];
					int x = 0;
					//#if CVLIB_ENABLE_UNROLLED
					for (; x <= size.width - 4; x += 4)
					{
						_Tp t0 = (_Tp)(s1[x] * alpha + gamma);
						_Tp t1 = (_Tp)(s1[x + 1] * alpha + gamma);
						d[x] = t0; d[x + 1] = t1;

						t0 = (_Tp)(s1[x + 2] * alpha + gamma);
						t1 = (_Tp)(s1[x + 3] * alpha + gamma);
						d[x + 2] = t0; d[x + 3] = t1;
					}
					//#endif
					for (; x < size.width; x++)
						d[x] = (_Tp)(s1[x] * alpha + gamma);
				}
			}
		}
		else if (alpha == 1 && beta == 1)
		{
			for (int y = 0; y < size.height; y++)
			{
				const _Tp* s1 = src1[y];
				const _Tp* s2 = src2[y];
				_Tp* d = dst[y];
				int x = 0;
				//#if CVLIB_ENABLE_UNROLLED
				for (; x <= size.width - 4; x += 4)
				{
					_Tp t0 = (_Tp)(s1[x] + s2[x] + gamma);
					_Tp t1 = (_Tp)(s1[x + 1] + s2[x + 1] + gamma);
					d[x] = t0; d[x + 1] = t1;

					t0 = (_Tp)(s1[x + 2] + s2[x + 2] + gamma);
					t1 = (_Tp)(s1[x + 3] + s2[x + 3] + gamma);
					d[x + 2] = t0; d[x + 3] = t1;
				}
				//#endif
				for (; x < size.width; x++)
					d[x] = (_Tp)(s1[x] + s2[x] + gamma);
			}
		}
		else
		{
			for (int y = 0; y < size.height; y++)
			{
				const _Tp* s1 = src1[y];
				const _Tp* s2 = src2[y];
				_Tp* d = dst[y];
				int x = 0;
				//#if CVLIB_ENABLE_UNROLLED
				for (; x <= size.width - 4; x += 4)
				{
					_Tp t0 = (_Tp)(s1[x] * alpha + s2[x] * beta + gamma);
					_Tp t1 = (_Tp)(s1[x + 1] * alpha + s2[x + 1] * beta + gamma);
					d[x] = t0; d[x + 1] = t1;

					t0 = (_Tp)(s1[x + 2] * alpha + s2[x + 2] * beta + gamma);
					t1 = (_Tp)(s1[x + 3] * alpha + s2[x + 3] * beta + gamma);
					d[x + 2] = t0; d[x + 3] = t1;
				}
				//#endif
				for (; x < size.width; x++)
					d[x] = (_Tp)(s1[x] * alpha + s2[x] * beta + gamma);
			}
		}
	}

	void MatOp::addWeighted(const Mat& src1, double alpha, const Mat& src2, double beta, double gamma, Mat& dst)
	{
		assert(src1.isEqualMat(src2));
		if (!src1.isEqualMat(dst))
		{
			dst.release();
			dst.create(src1, false);
		}
		addWeightedFunc addWeightedTab[] =
		{ NULL, addWeighted_<uchar, float>, addWeighted_<short, float>, addWeighted_<int, double>, addWeighted_<float, float>, addWeighted_<double, double> };
		addWeightedFunc func = addWeightedTab[(int)dst.type()];
		double scalars[] = { alpha, beta, gamma };
		func(src1.data.ptr, src2.data.ptr, dst.data.ptr, src1.size(), scalars);
	}

	void MatOp::gemm(const Mat* matA, const Mat* matB, double alpha,
		const Mat* matC, double beta, Mat* matD, int /*flags*/)
	{
		if (!matD) return;
		Mat temp; temp.create(*matD, false);
		Mat temp1; temp1.create(*matD, false);
		if (matA && matB)
		{
			MatOp::mul(&temp, matA, matB);
			if (fabs(alpha - 1.0) > DBL_EPSILON)
				MatOp::mul(&temp, alpha, &temp);
		}
		if (matC && fabs(beta) > DBL_EPSILON)
		{
			MatOp::mul(&temp1, beta, matC);
		}
		matD->zero();
		if (matA && matB)
			MatOp::add(matD, matD, &temp);
		if (matC && fabs(beta) > DBL_EPSILON)
			MatOp::add(matD, matD, &temp1);
	}

	void MatOp::matMulAdd(const Mat* src1, const Mat* src2, const Mat* src3, Mat* dst)
	{
		gemm(src1, src2, 1.0, src3, 1.0, dst, 0);
	}

	typedef double(*ScalarProductFunc)(uchar** const src1, uchar** const src2, const Size& sz);
	template<typename _Tp> static double
		ScalarProduct_(uchar** const _src1, uchar** const _src2, const Size& size)
	{
		double rsum = 0;
		int i, j;
		for (i = 0; i < size.height; i++)
		{
			const _Tp* src1 = (const _Tp*)_src1[i];
			const _Tp* src2 = (const _Tp*)_src2[i];
			for (j = 0; j < size.width; j++)
				rsum += src1[j] * src2[j];
		}
		return rsum;
	}

	double MatOp::scalarProduct(Mat* pmA, Mat *pmB)
	{
		assert(pmA->isEqualMat(*pmB));
		ScalarProductFunc funcs[] = {
			ScalarProduct_<char>,
			ScalarProduct_<uchar>,
			ScalarProduct_<short>,
			ScalarProduct_<int>,
			ScalarProduct_<float>,
			ScalarProduct_<double>
		};
		return funcs[pmA->type()](pmA->data.ptr, pmB->data.ptr, pmA->size());
	}

	static void Magnitude_32f(const float* x, const float* y, float* mag, int len)
	{
		int i = 0;
		for (; i < len; i++)
		{
			float x0 = x[i], y0 = y[i];
			mag[i] = sqrt(x0*x0 + y0*y0);
		}
	}

	static void Magnitude_64f(const double* x, const double* y, double* mag, int len)
	{
		int i = 0;

		for (; i < len; i++)
		{
			double x0 = x[i], y0 = y[i];
			mag[i] = sqrt(x0*x0 + y0*y0);
		}
	}

	static void FastAtan2_32f(const float *Y, const float *X, float *angle, int len, bool angleInDegrees = true)
	{
		int i = 0;
		float scale = angleInDegrees ? (float)(180 / CVLIB_PI) : 1.f;

		for (; i < len; i++)
		{
			float x = X[i], y = Y[i];
			float a, x2 = x*x, y2 = y*y;
			if (y2 <= x2)
				a = x*y / (x2 + 0.28f*y2 + (float)DBL_EPSILON) + (float)(x < 0 ? CVLIB_PI : y >= 0 ? 0 : CVLIB_PI * 2);
			else
				a = (float)(y >= 0 ? CVLIB_PI*0.5 : CVLIB_PI*1.5) - x*y / (y2 + 0.28f*x2 + (float)DBL_EPSILON);
			angle[i] = a*scale;
		}
	}

#define MAX_BLOCK_SIZE 1024
	void cartToPolar(const Mat& X, const Mat& Y, Mat& Mag, Mat& Angle, bool angleInDegrees)
	{
		float buf[2][MAX_BLOCK_SIZE];
		int i, j, k;
		TYPE type = X.type1();

		assert(X.size() == Y.size() && type == Y.type() && (type == MAT_Tfloat || type == MAT_Tdouble));
		Mag.create(X.size(), type);
		Angle.create(X.size(), type);

		int cn = X.channels();
		Size size = X.size();
		size.width *= cn;
		bool inplace = Mag.data.ptr == X.data.ptr || Mag.data.ptr == Y.data.ptr;
		int blockSize = MIN(size.width, ((MAX_BLOCK_SIZE + cn - 1) / cn)*cn);
		if (type == MAT_Tfloat)
		{
			for (k = 0; k < size.height; k++)
			{
				const float *x = (const float*)X.data.ptr[k], *y = (const float*)Y.data.ptr[k];
				float *mag = (float*)Mag.data.fl[k], *angle = (float*)Angle.data.fl[k];
				for (i = 0; i < size.width; i += blockSize)
				{
					int block_size = MIN(blockSize, size.width - i);
					Magnitude_32f(x + i, y + i, inplace ? buf[0] : mag + i, block_size);
					FastAtan2_32f(y + i, x + i, angle + i, block_size, angleInDegrees);
					if (inplace)
						for (j = 0; j < block_size; j++)
							mag[i + j] = buf[0][j];
				}
			}
		}
		else
		{
			for (k = 0; k < size.height; k++)
			{
				const double *x = (const double*)X.data.ptr[k], *y = (const double*)Y.data.ptr[k];
				double *mag = (double*)Mag.data.fl[k], *angle = (double*)Angle.data.fl[k];
				for (i = 0; i < size.width; i += blockSize)
				{
					int block_size = MIN(blockSize, size.width - i);
					for (j = 0; j < block_size; j++)
					{
						buf[0][j] = (float)x[i + j];
						buf[1][j] = (float)y[i + j];
					}
					FastAtan2_32f(buf[1], buf[0], buf[0], block_size, angleInDegrees);
					Magnitude_64f(x + i, y + i, mag + i, block_size);
					for (j = 0; j < block_size; j++)
						angle[i + j] = buf[0][j];
				}
			}
		}
	}

	Mat operator + (const Mat& a, const Mat& b)
	{
		Mat m;
		int newr = MAX(a.rows(), b.rows());
		int newc = MAX(a.cols(), b.cols());
		if (newr == 0 || newc == 0 || a.type1() != b.type1())
			return m;
		m.create(newr, newc, a.type1());
		MatOp::add(&m, &a, &b);
		return m;
	}
	typedef void(*addFunc)(uchar** const src1, uchar** dst, const Size& sz, int ch, void*);
	template<typename _Tp> static void
		addFunc_(uchar** const _src1, uchar** _dst, const Size& size, int ch, void* _scalars)
	{
		_Tp** const src1 = (_Tp** const)_src1;
		_Tp** dst = (_Tp**)_dst;

		const double* scalars = (const double*)_scalars;
		if (ch == 1)
		{
			const double& gamma = scalars[0];
			for (int y = 0; y < size.height; y++)
			{
				const _Tp* s1 = src1[y];
				_Tp* d = dst[y];
				int x = 0;
				//#if CVLIB_ENABLE_UNROLLED
				for (; x <= size.width - 4; x += 4)
				{
					_Tp t0 = (_Tp)(s1[x] + gamma);
					_Tp t1 = (_Tp)(s1[x + 1] + gamma);
					d[x] = t0; d[x + 1] = t1;

					t0 = (_Tp)(s1[x + 2] + gamma);
					t1 = (_Tp)(s1[x + 3] + gamma);
					d[x + 2] = t0; d[x + 3] = t1;
				}
				//#endif
				for (; x < size.width; x++)
					d[x] = (_Tp)(s1[x] + gamma);
			}
		}
		else
		{
			for (int y = 0; y < size.height; y++)
			{
				const _Tp* s1 = src1[y];
				_Tp* d = dst[y];
				int x = 0, i = 0;
				for (; x < size.width; x++)
				{
					for (int k = 0; k < ch; k++, i++)
						d[i] = (_Tp)(s1[i] + scalars[k]);
				}
			}
		}
	}

	Mat operator + (const Mat& a, const Scalar& s)
	{
		Mat m; m.create(a);

		addFunc addTab[] =
		{ NULL, addFunc_<uchar>, addFunc_<short>, addFunc_<int>, addFunc_<float>, addFunc_<double> };
		addFunc func = addTab[(int)a.type()];
		int channel = a.channels();
		double scalars[] = { s.vec_array[0],s.vec_array[1],s.vec_array[2],s.vec_array[3] };
		func(a.data.ptr, m.data.ptr, a.size(), channel, scalars);
		return m;
	}
	Mat operator + (const Scalar& s, const Mat& a)
	{
		Mat m; m.create(a);

		addFunc addTab[] =
		{ NULL, addFunc_<uchar>, addFunc_<short>, addFunc_<int>, addFunc_<float>, addFunc_<double> };
		addFunc func = addTab[(int)a.type()];
		int channel = a.channels();
		double scalars[] = { s.vec_array[0],s.vec_array[1],s.vec_array[2],s.vec_array[3] };
		func(a.data.ptr, m.data.ptr, a.size(), channel, scalars);
		return m;
	}

	Mat operator - (const Mat& a, const Mat& b)
	{
		Mat m;
		int newr = MAX(a.rows(), b.rows());
		int newc = MAX(a.cols(), b.cols());
		if (newr == 0 || newc == 0 || a.type1() != b.type1())
			return m;
		m.create(newr, newc, a.type1());
		MatOp::sub(&m, &a, &b);
		return m;
	}
	Mat operator - (const Mat& a, const Scalar& s)
	{
		Mat m; m.create(a);

		addFunc addTab[] =
		{ NULL, addFunc_<uchar>, addFunc_<short>, addFunc_<int>, addFunc_<float>, addFunc_<double> };
		addFunc func = addTab[(int)a.type()];
		int channel = a.channels();
		double scalars[] = { -s.vec_array[0], -s.vec_array[1], -s.vec_array[2], -s.vec_array[3] };
		func(a.data.ptr, m.data.ptr, a.size(), channel, scalars);
		return m;
	}

	typedef void(*subFunc)(uchar** const src1, uchar** dst, const Size& sz, int ch, void*);
	template<typename _Tp> static void
		subFunc_(uchar** const _src1, uchar** _dst, const Size& size, int ch, void* _scalars)
	{
		_Tp** const src1 = (_Tp** const)_src1;
		_Tp** dst = (_Tp**)_dst;

		const double* scalars = (const double*)_scalars;
		if (ch == 1)
		{
			const double& gamma = scalars[0];
			for (int y = 0; y < size.height; y++)
			{
				const _Tp* s1 = src1[y];
				_Tp* d = dst[y];
				int x = 0;
				//#if CVLIB_ENABLE_UNROLLED
				for (; x <= size.width - 4; x += 4)
				{
					_Tp t0 = (_Tp)(-s1[x] + gamma);
					_Tp t1 = (_Tp)(-s1[x + 1] + gamma);
					d[x] = t0; d[x + 1] = t1;

					t0 = (_Tp)(-s1[x + 2] + gamma);
					t1 = (_Tp)(-s1[x + 3] + gamma);
					d[x + 2] = t0; d[x + 3] = t1;
				}
				//#endif
				for (; x < size.width; x++)
					d[x] = (_Tp)(-s1[x] + gamma);
			}
		}
		else
		{
			for (int y = 0; y < size.height; y++)
			{
				const _Tp* s1 = src1[y];
				_Tp* d = dst[y];
				int x = 0, i = 0;
				for (; x < size.width; x++)
				{
					for (int k = 0; k < ch; k++, i++)
						d[i] = (_Tp)(-s1[i] + scalars[k]);
				}
			}
		}
	}
	Mat operator - (const Scalar& s, const Mat& a)
	{
		Mat m; m.create(a);
		subFunc subTab[] =
		{ NULL, subFunc_<uchar>, subFunc_<short>, subFunc_<int>, subFunc_<float>, subFunc_<double> };
		subFunc func = subTab[(int)a.type()];
		int channel = a.channels();
		double scalars[] = { s.vec_array[0], s.vec_array[1], s.vec_array[2], s.vec_array[3] };
		func(a.data.ptr, m.data.ptr, a.size(), channel, scalars);
		return m;
	}

	typedef void(*negFunc)(uchar** const src1, uchar** dst, const Size& sz);
	template<typename _Tp> static void
		negFunc_(uchar** const _src1, uchar** _dst, const Size& size)
	{
		_Tp** const src1 = (_Tp** const)_src1;
		_Tp** dst = (_Tp**)_dst;

		for (int y = 0; y < size.height; y++)
		{
			const _Tp* s1 = src1[y];
			_Tp* d = dst[y];
			int x = 0;
			//#if CVLIB_ENABLE_UNROLLED
			for (; x <= size.width - 4; x += 4)
			{
				d[x] = -s1[x];
				d[x + 1] = -s1[x + 1];
				d[x + 2] = -s1[x + 2];
				d[x + 3] = -s1[x + 3];
			}
			//#endif
			for (; x < size.width; x++)
				d[x] = -s1[x];
		}
	}
	Mat operator - (const Mat& a)
	{
		Mat m; m.create(a);
		negFunc negTab[] =
		{ negFunc_<char>, negFunc_<uchar>, negFunc_<short>, negFunc_<int>, negFunc_<float>, negFunc_<double> };
		negFunc func = negTab[(int)a.type()];
		Size size = a.size();
		size.width *= a.channels();
		func(a.data.ptr, m.data.ptr, size);
		return m;
	}

	Mat operator * (const Mat& a, const Mat& b)
	{
		Mat m;
		int newr = MAX(a.rows(), b.rows());
		int newc = MAX(a.cols(), b.cols());
		if (newr == 0 || newc == 0 || a.type1() != b.type1())
			return m;
		m.create(a.rows(), b.cols(), a.type1());
		MatOp::mul(&m, &a, &b);
		return m;
	}
	Mat operator * (const Mat& a, double s)
	{
		Mat m; m.create(a);
		MatOp::mul(&m, s, &a);
		return m;
	}
	Mat operator * (double s, const Mat& a)
	{
		Mat m; m.create(a);
		MatOp::mul(&m, s, &a);
		return m;
	}
	Mat operator / (const Mat& a, double s)
	{
		return a * (1.0 / s);
	}
	Mat operator / (double s, const Mat& a)
	{
		return a * (1.0 / s);
	}

	typedef void(*boolFunc)(uchar* const src1, uchar* const src2, uchar* dst, int len, const char* ch);
	template<typename _Tp> static void
		boolFunc_(uchar* const _src1, uchar* const _src2, uchar* _dst, int len, const char* ch)
	{
		_Tp* const src1 = (_Tp* const)_src1;
		_Tp* const src2 = (_Tp* const)_src2;
		uchar* dst = (uchar*)_dst;

		int i;

		if (!strcmp(ch, "<"))
		{
			for (i = 0; i < len; i++)
				dst[i] = src1[i] < src2[i];
		}
		else if (!strcmp(ch, "<="))
		{
			for (i = 0; i < len; i++)
				dst[i] = src1[i] <= src2[i];
		}
		else if (!strcmp(ch, "=="))
		{
			for (i = 0; i < len; i++)
				dst[i] = src1[i] == src2[i];
		}
		else if (!strcmp(ch, "!="))
		{
			for (i = 0; i < len; i++)
				dst[i] = src1[i] != src2[i];
		}
		else if (!strcmp(ch, ">"))
		{
			for (i = 0; i < len; i++)
				dst[i] = src1[i] > src2[i];
		}
		else if (!strcmp(ch, ">="))
		{
			for (i = 0; i < len; i++)
				dst[i] = src1[i] >= src2[i];
		}
		else
		{
			assert(false);
		}
	}
	Mat operator < (const Mat& a, const Mat& b)
	{
		Mat m; m.create(a.rows(), a.cols(), (TYPE)CVLIB_MAKETYPE(MAT_Tuchar, a.channels()));
		boolFunc boolTab[] =
		{ boolFunc_<char>, boolFunc_<uchar>, boolFunc_<short>, boolFunc_<int>, boolFunc_<float>, boolFunc_<double> };
		boolFunc func = boolTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], b.data.ptr[0], m.data.ptr[0], len, "<");
		return m;
	}
	typedef void(*boolFunc2)(uchar* const src1, double s, uchar* dst, int len, const char* ch);
	template<typename _Tp> static void
		boolFunc2_(uchar* const _src1, double s, uchar* _dst, int len, const char* _ch)
	{
		_Tp* const src1 = (_Tp* const)_src1;
		uchar* dst = (uchar*)_dst;

		int i;
		if (!strcmp(_ch, "<"))
		{
			for (i = 0; i < len; i++)
				dst[i] = src1[i] < s;
		}
		else if (!strcmp(_ch, "<="))
		{
			for (i = 0; i < len; i++)
				dst[i] = src1[i] <= s;
		}
		else if (!strcmp(_ch, "=="))
		{
			for (i = 0; i < len; i++)
				dst[i] = src1[i] == s;
		}
		else if (!strcmp(_ch, "!="))
		{
			for (i = 0; i < len; i++)
				dst[i] = src1[i] != s;
		}
		else if (!strcmp(_ch, ">"))
		{
			for (i = 0; i < len; i++)
				dst[i] = src1[i] > s;
		}
		else if (!strcmp(_ch, ">="))
		{
			for (i = 0; i < len; i++)
				dst[i] = src1[i] >= s;
		}
		else
		{
			assert(false);
		}
	}
	Mat operator < (const Mat& a, double s)
	{
		Mat m; m.create(a.rows(), a.cols(), (TYPE)CVLIB_MAKETYPE(MAT_Tuchar, a.channels()));
		boolFunc2 boolTab[] =
		{ boolFunc2_<char>, boolFunc2_<uchar>, boolFunc2_<short>, boolFunc2_<int>, boolFunc2_<float>, boolFunc2_<double> };
		boolFunc2 func = boolTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], s, m.data.ptr[0], len, "<");
		return m;
	}

	template<typename _Tp> static void
		boolFunc3_(uchar* const _src1, double s, uchar* _dst, int len, const char* ch)
	{
		_Tp* const src1 = (_Tp* const)_src1;
		uchar* dst = (uchar*)_dst;

		int i;
		if (!strcmp(ch, "<"))
		{
			for (i = 0; i < len; i++)
				dst[i] = (int)(s < src1[i]);
		}
		else if (!strcmp(ch, "<="))
		{
			for (i = 0; i < len; i++)
				dst[i] = (int)(s <= src1[i]);
		}
		else if (!strcmp(ch, "=="))
		{
			for (i = 0; i < len; i++)
				dst[i] = (int)(src1[i] == s);
		}
		else if (!strcmp(ch, "!="))
		{
			for (i = 0; i < len; i++)
				dst[i] = (int)(src1[i] != s);
		}
		else if (!strcmp(ch, ">"))
		{
			for (i = 0; i < len; i++)
				dst[i] = (int)(s > src1[i]);
		}
		else if (!strcmp(ch, ">="))
		{
			for (i = 0; i < len; i++)
				dst[i] = (int)(s >= src1[i]);
		}
		else
		{
			assert(false);
		}
	}
	Mat operator < (double s, const Mat& a)
	{
		Mat m; m.create(a.rows(), a.cols(), (TYPE)CVLIB_MAKETYPE(MAT_Tuchar, a.channels()));
		boolFunc2 boolTab[] =
		{ boolFunc3_<char>, boolFunc3_<uchar>, boolFunc3_<short>, boolFunc3_<int>, boolFunc3_<float>, boolFunc3_<double> };
		boolFunc2 func = boolTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], s, m.data.ptr[0], len, "<");
		return m;
	}

	Mat operator <= (const Mat& a, const Mat& b)
	{
		Mat m; m.create(a.rows(), a.cols(), (TYPE)CVLIB_MAKETYPE(MAT_Tuchar, a.channels()));
		boolFunc boolTab[] =
		{ boolFunc_<char>, boolFunc_<uchar>, boolFunc_<short>, boolFunc_<int>, boolFunc_<float>, boolFunc_<double> };
		boolFunc func = boolTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], b.data.ptr[0], m.data.ptr[0], len, "<=");
		return m;
	}
	Mat operator <= (const Mat& a, double s)
	{
		Mat m; m.create(a.rows(), a.cols(), (TYPE)CVLIB_MAKETYPE(MAT_Tuchar, a.channels()));
		boolFunc2 boolTab[] =
		{ boolFunc2_<char>, boolFunc2_<uchar>, boolFunc2_<short>, boolFunc2_<int>, boolFunc2_<float>, boolFunc2_<double> };
		boolFunc2 func = boolTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], s, m.data.ptr[0], len, "<=");
		return m;
	}
	Mat operator <= (double s, const Mat& a)
	{
		Mat m; m.create(a.rows(), a.cols(), (TYPE)CVLIB_MAKETYPE(MAT_Tuchar, a.channels()));
		boolFunc2 boolTab[] =
		{ boolFunc3_<char>, boolFunc3_<uchar>, boolFunc3_<short>, boolFunc3_<int>, boolFunc3_<float>, boolFunc3_<double> };
		boolFunc2 func = boolTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], s, m.data.ptr[0], len, "<=");
		return m;
	}

	Mat operator == (const Mat& a, const Mat& b)
	{
		Mat m; m.create(a.rows(), a.cols(), (TYPE)CVLIB_MAKETYPE(MAT_Tuchar, a.channels()));
		boolFunc boolTab[] =
		{ boolFunc_<char>, boolFunc_<uchar>, boolFunc_<short>, boolFunc_<int>, boolFunc_<float>, boolFunc_<double> };
		boolFunc func = boolTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], b.data.ptr[0], m.data.ptr[0], len, "==");
		return m;
	}
	Mat operator == (const Mat& a, double s)
	{
		Mat m; m.create(a.rows(), a.cols(), (TYPE)CVLIB_MAKETYPE(MAT_Tuchar, a.channels()));
		boolFunc2 boolTab[] =
		{ boolFunc2_<char>, boolFunc2_<uchar>, boolFunc2_<short>, boolFunc2_<int>, boolFunc2_<float>, boolFunc2_<double> };
		boolFunc2 func = boolTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], s, m.data.ptr[0], len, "==");
		return m;
	}
	Mat operator == (double s, const Mat& a)
	{
		Mat m; m.create(a.rows(), a.cols(), (TYPE)CVLIB_MAKETYPE(MAT_Tuchar, a.channels()));
		boolFunc2 boolTab[] =
		{ boolFunc3_<char>, boolFunc3_<uchar>, boolFunc3_<short>, boolFunc3_<int>, boolFunc3_<float>, boolFunc3_<double> };
		boolFunc2 func = boolTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], s, m.data.ptr[0], len, "==");
		return m;
	}

	Mat operator != (const Mat& a, const Mat& b)
	{
		Mat m; m.create(a.rows(), a.cols(), (TYPE)CVLIB_MAKETYPE(MAT_Tuchar, a.channels()));
		boolFunc boolTab[] =
		{ boolFunc_<char>, boolFunc_<uchar>, boolFunc_<short>, boolFunc_<int>, boolFunc_<float>, boolFunc_<double> };
		boolFunc func = boolTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], b.data.ptr[0], m.data.ptr[0], len, "!=");
		return m;
	}
	Mat operator != (const Mat& a, double s)
	{
		Mat m; m.create(a.rows(), a.cols(), (TYPE)CVLIB_MAKETYPE(MAT_Tuchar, a.channels()));
		boolFunc2 boolTab[] =
		{ boolFunc2_<char>, boolFunc2_<uchar>, boolFunc2_<short>, boolFunc2_<int>, boolFunc2_<float>, boolFunc2_<double> };
		boolFunc2 func = boolTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], s, m.data.ptr[0], len, "!=");
		return m;
	}
	Mat operator != (double s, const Mat& a)
	{
		Mat m; m.create(a.rows(), a.cols(), (TYPE)CVLIB_MAKETYPE(MAT_Tuchar, a.channels()));
		boolFunc2 boolTab[] =
		{ boolFunc3_<char>, boolFunc3_<uchar>, boolFunc3_<short>, boolFunc3_<int>, boolFunc3_<float>, boolFunc3_<double> };
		boolFunc2 func = boolTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], s, m.data.ptr[0], len, "!=");
		return m;
	}

	Mat operator >= (const Mat& a, const Mat& b)
	{
		Mat m; m.create(a.rows(), a.cols(), (TYPE)CVLIB_MAKETYPE(MAT_Tuchar, a.channels()));
		boolFunc boolTab[] =
		{ boolFunc_<char>, boolFunc_<uchar>, boolFunc_<short>, boolFunc_<int>, boolFunc_<float>, boolFunc_<double> };
		boolFunc func = boolTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], b.data.ptr[0], m.data.ptr[0], len, ">=");
		return m;
	}
	Mat operator >= (const Mat& a, double s)
	{
		Mat m; m.create(a.rows(), a.cols(), (TYPE)CVLIB_MAKETYPE(MAT_Tuchar, a.channels()));
		boolFunc2 boolTab[] =
		{ boolFunc2_<char>, boolFunc2_<uchar>, boolFunc2_<short>, boolFunc2_<int>, boolFunc2_<float>, boolFunc2_<double> };
		boolFunc2 func = boolTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], s, m.data.ptr[0], len, ">=");
		return m;
	}
	Mat operator >= (double s, const Mat& a)
	{
		Mat m; m.create(a.rows(), a.cols(), (TYPE)CVLIB_MAKETYPE(MAT_Tuchar, a.channels()));
		boolFunc2 boolTab[] =
		{ boolFunc3_<char>, boolFunc3_<uchar>, boolFunc3_<short>, boolFunc3_<int>, boolFunc3_<float>, boolFunc3_<double> };
		boolFunc2 func = boolTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], s, m.data.ptr[0], len, ">=");
		return m;
	}

	Mat operator > (const Mat& a, const Mat& b)
	{
		Mat m; m.create(a.rows(), a.cols(), (TYPE)CVLIB_MAKETYPE(MAT_Tuchar, a.channels()));
		boolFunc boolTab[] =
		{ boolFunc_<char>, boolFunc_<uchar>, boolFunc_<short>, boolFunc_<int>, boolFunc_<float>, boolFunc_<double> };
		boolFunc func = boolTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], b.data.ptr[0], m.data.ptr[0], len, ">");
		return m;
	}
	Mat operator > (const Mat& a, double s)
	{
		Mat m; m.create(a.rows(), a.cols(), (TYPE)CVLIB_MAKETYPE(MAT_Tuchar, a.channels()));
		boolFunc2 boolTab[] =
		{ boolFunc2_<char>, boolFunc2_<uchar>, boolFunc2_<short>, boolFunc2_<int>, boolFunc2_<float>, boolFunc2_<double> };
		boolFunc2 func = boolTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], s, m.data.ptr[0], len, ">");
		return m;
	}
	Mat operator > (double s, const Mat& a)
	{
		Mat m; m.create(a.rows(), a.cols(), (TYPE)CVLIB_MAKETYPE(MAT_Tuchar, a.channels()));
		boolFunc2 boolTab[] =
		{ boolFunc3_<char>, boolFunc3_<uchar>, boolFunc3_<short>, boolFunc3_<int>, boolFunc3_<float>, boolFunc3_<double> };
		boolFunc2 func = boolTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], s, m.data.ptr[0], len, ">");
		return m;
	}

	typedef void(*bitFunc)(uchar* const src1, uchar* const src2, uchar* dst, int len, char ch);
	template<typename _Tp> static void
		bitFunc_(uchar* const _src1, uchar* const _src2, uchar* _dst, int len, char ch)
	{
		_Tp* const src1 = (_Tp* const)_src1;
		_Tp* const src2 = (_Tp* const)_src2;
		_Tp* dst = (_Tp*)_dst;

		int i;
		switch (ch)
		{
		case '&':
			for (i = 0; i < len; i++)
				dst[i] = src1[i] & src2[i];
			break;
		case '|':
			for (i = 0; i < len; i++)
				dst[i] = src1[i] | src2[i];
			break;
		case '^':
			for (i = 0; i < len; i++)
				dst[i] = src1[i] ^ src2[i];
			break;
		default:
			assert(false);
		}
	}
	Mat operator & (const Mat& a, const Mat& b)
	{
		assert(a.type() <= MAT_Tint && b.type() <= MAT_Tint);
		Mat m = a;
		bitFunc bitTab[] =
		{ bitFunc_<char>, bitFunc_<uchar>, bitFunc_<short>, bitFunc_<int> };
		bitFunc func = bitTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], b.data.ptr[0], m.data.ptr[0], len, '&');
		return m;
	}
	typedef void(*bitFunc2)(uchar** const src1, double* const scalr, uchar** dst, const Size& size, int channel, char ch);
	template<typename _Tp> static void
		bitFunc2_(uchar** const _src, double* const scalr, uchar** _dst, const Size& size, int channel, char ch)
	{
		_Tp** const src1 = (_Tp** const)_src;
		_Tp** dst = (_Tp**)_dst;

		int h = size.height, w = size.width, k, j;
		int x, y;
		switch (ch)
		{
		case '&':
			for (y = 0; y < h; y++)
			{
				j = 0;
				_Tp* d = dst[y];
				_Tp* const s = src1[y];
				for (x = 0; x < w; x++)
				{
					for (k = 0; k < channel; k++, j++)
						d[j] = s[j] & (int)scalr[k];
				}
			}
			break;
		case '|':
			for (y = 0; y < h; y++)
			{
				j = 0;
				_Tp* d = dst[y];
				_Tp* const s = src1[y];
				for (x = 0; x < w; x++)
				{
					for (k = 0; k < channel; k++, j++)
						d[j] = (_Tp)(s[j] | (int)scalr[k]);
				}
			}
			break;
		case '^':
			for (y = 0; y < h; y++)
			{
				j = 0;
				_Tp* d = dst[y];
				_Tp* const s = src1[y];
				for (x = 0; x < w; x++)
				{
					for (k = 0; k < channel; k++, j++)
						d[j] = (_Tp)(s[j] ^ (int)scalr[k]);
				}
			}
			break;
		default:
			assert(false);
		}
	}
	Mat operator & (const Mat& a, const Scalar& s)
	{
		assert(a.type() <= MAT_Tint);
		Mat m = a;
		bitFunc2 bitTab[] =
		{ bitFunc2_<char>, bitFunc2_<uchar>, bitFunc2_<short>, bitFunc2_<int> };
		bitFunc2 func = bitTab[(int)a.type()];
		double scalar[] = { s.vec_array[0],s.vec_array[1],s.vec_array[2],s.vec_array[3] };
		func(a.data.ptr, scalar, m.data.ptr, a.size(), a.channels(), '&');
		return m;
	}
	Mat operator & (const Scalar& s, const Mat& a)
	{
		return operator & (a, s);
	}

	Mat operator | (const Mat& a, const Mat& b)
	{
		assert(a.type() <= MAT_Tint && b.type() <= MAT_Tint);
		Mat m = a;
		bitFunc bitTab[] =
		{ bitFunc_<char>, bitFunc_<uchar>, bitFunc_<short>, bitFunc_<int> };
		bitFunc func = bitTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], b.data.ptr[0], m.data.ptr[0], len, '|');
		return m;
	}
	Mat operator | (const Mat& a, const Scalar& s)
	{
		assert(a.type() <= MAT_Tint);
		Mat m = a;
		bitFunc2 bitTab[] =
		{ bitFunc2_<char>, bitFunc2_<uchar>, bitFunc2_<short>, bitFunc2_<int> };
		bitFunc2 func = bitTab[(int)a.type()];
		double scalar[] = { s.vec_array[0],s.vec_array[1],s.vec_array[2],s.vec_array[3] };
		func(a.data.ptr, scalar, m.data.ptr, a.size(), a.channels(), '|');
		return m;
	}
	Mat operator | (const Scalar& s, const Mat& a)
	{
		return operator | (a, s);
	}

	Mat operator ^ (const Mat& a, const Mat& b)
	{
		assert(a.type() <= MAT_Tint && b.type() <= MAT_Tint);
		Mat m = a;
		bitFunc bitTab[] =
		{ bitFunc_<char>, bitFunc_<uchar>, bitFunc_<short>, bitFunc_<int> };
		bitFunc func = bitTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], b.data.ptr[0], m.data.ptr[0], len, '^');
		return m;
	}
	Mat operator ^ (const Mat& a, const Scalar& s)
	{
		assert(a.type() <= MAT_Tint);
		Mat m = a;
		bitFunc2 bitTab[] =
		{ bitFunc2_<char>, bitFunc2_<uchar>, bitFunc2_<short>, bitFunc2_<int> };
		bitFunc2 func = bitTab[(int)a.type()];
		double scalar[] = { s.vec_array[0],s.vec_array[1],s.vec_array[2],s.vec_array[3] };
		func(a.data.ptr, scalar, m.data.ptr, a.size(), a.channels(), '^');
		return m;
	}
	Mat operator ^ (const Scalar& s, const Mat& a)
	{
		return operator ^ (a, s);
	}

	typedef void(*bitFunc4)(uchar* const src, uchar* dst, int len);
	template<typename _Tp> static void
		bitFunc4_(uchar* const _src, uchar* _dst, int len)
	{
		_Tp* const src1 = (_Tp* const)_src;
		_Tp* dst = (_Tp*)_dst;

		for (int i = 0; i < len; i++)
			dst[i] = ~src1[i];
	}
	Mat operator ~(const Mat& a)
	{
		assert(a.type() <= MAT_Tint);
		Mat m = a;
		bitFunc4 bitTab[] =
		{ bitFunc4_<char>, bitFunc4_<uchar>, bitFunc4_<short>, bitFunc4_<int> };
		bitFunc4 func = bitTab[(int)a.type()];
		int len = a.rows()*a.cols()*a.channels();
		func(a.data.ptr[0], m.data.ptr[0], len);
		return m;
	}

	template<typename _Tp> static void
		mixChannels_(const _Tp** src, int srcidx,
			_Tp** dst, const int dstidx,
			int rows, int cols, int sdelta, int ddelta)
	{
		int y;
		for (y = 0; y < rows; y++) {
			const _Tp* psrc = src[y];
			_Tp* pdst = dst[y];
			int xsrc = srcidx, xdst = dstidx;
			for (int i = 0; i < cols; i++, xsrc += sdelta, xdst += ddelta)
			{
				pdst[xdst] = psrc[xsrc];
			}
		}
	}
	static void mixChannels8u(const uchar** src, int srcidx, uchar** dst, int dstidx, int rows, int cols, int sdelta, int ddelta)
	{
		mixChannels_(src, srcidx, dst, dstidx, rows, cols, sdelta, ddelta);
	}

	static void mixChannels16u(const short** src, int srcidx, short** dst, int dstidx, int rows, int cols, int sdelta, int ddelta)
	{
		mixChannels_(src, srcidx, dst, dstidx, rows, cols, sdelta, ddelta);
	}

	static void mixChannels32s(const int** src, int srcidx, int** dst, int dstidx, int rows, int cols, int sdelta, int ddelta)
	{
		mixChannels_(src, srcidx, dst, dstidx, rows, cols, sdelta, ddelta);
	}

	static void mixChannels64s(const int64** src, int srcidx, int64** dst, int dstidx, int rows, int cols, int sdelta, int ddelta)
	{
		mixChannels_(src, srcidx, dst, dstidx, rows, cols, sdelta, ddelta);
	}

	typedef void(*MixChannelsFunc)(const uchar** src, int srcidx, uchar** dst, int dstidx, int rows, int cols, int sdelta, int ddelta);

	static MixChannelsFunc mixchTab[] =
	{
		(MixChannelsFunc)mixChannels8u, (MixChannelsFunc)mixChannels8u, (MixChannelsFunc)mixChannels16u,
		(MixChannelsFunc)mixChannels32s, (MixChannelsFunc)mixChannels32s,
		(MixChannelsFunc)mixChannels64s, 0
	};

#define BLOCK_SIZE 1024

	void mixChannels(const Mat* src, size_t nsrcs, Mat* dst, size_t ndsts, const int* fromTo, size_t npairs)
	{
		if (npairs == 0)
			return;
		assert(src && nsrcs > 0 && dst && ndsts > 0 && fromTo && npairs > 0);

		size_t i, j, k = 0/*, esz1 = dst[0].step()*/;
		int depth = dst[0].type();

		AutoBuffer<uchar> buf((nsrcs + ndsts + 1)*(sizeof(Mat*) + sizeof(uchar*)) + npairs*(sizeof(uchar*) * 2 + sizeof(int) * 6));
		const Mat** arrays = (const Mat**)(uchar*)buf;
		uchar** ptrs = (uchar**)(arrays + nsrcs + ndsts);
		const uchar** srcs = (const uchar**)(ptrs + nsrcs + ndsts + 1);
		uchar** dsts = (uchar**)(srcs + npairs);
		int* tab = (int*)(dsts + npairs);
		int *sdelta = (int*)(tab + npairs * 4), *ddelta = sdelta + npairs;

		for (i = 0; i < nsrcs; i++)
			arrays[i] = &src[i];
		for (i = 0; i < ndsts; i++)
			arrays[i + nsrcs] = &dst[i];
		ptrs[nsrcs + ndsts] = 0;

		for (i = 0; i < npairs; i++)
		{
			int i0 = fromTo[i * 2], i1 = fromTo[i * 2 + 1];
			if (i0 >= 0)
			{
				for (j = 0; j < nsrcs; i0 -= src[j].channels(), j++)
					if (i0 < src[j].channels())
						break;
				assert(j < nsrcs && src[j].type() == depth);
				tab[i * 4] = (int)j; tab[i * 4 + 1] = (int)i0;//(i0*esz1);
				sdelta[i] = src[j].channels();
			}
			else
			{
				tab[i * 4] = (int)(nsrcs + ndsts); tab[i * 4 + 1] = 0;
				sdelta[i] = 0;
			}

			for (j = 0; j < ndsts; i1 -= dst[j].channels(), j++)
				if (i1 < dst[j].channels())
					break;
			assert(i1 >= 0 && j < ndsts && dst[j].type() == depth);
			tab[i * 4 + 2] = (int)(j + nsrcs); tab[i * 4 + 3] = (int)i1;//(i1*esz1);
			ddelta[i] = dst[j].channels();
		}

		//    int total = (int)npairs, blocksize = std::min(total, (int)((BLOCK_SIZE + esz1-1)/esz1));
		MixChannelsFunc func = mixchTab[depth];

		int rows = src[0].rows();
		int cols = src[0].cols();
		for (i = 0; i < npairs; i++)
		{
			//      for( k = 0; k < npairs; k++ )
			const Mat* srcmat = arrays[tab[k * 4]];
			Mat* dstmat = (Mat*)arrays[tab[k * 4 + 2]];
			{
				//           srcs[k] = arrays[tab[k*4]] + tab[k*4+1];
				//           dsts[k] = ptrs[tab[k*4+2]] + tab[k*4+3];
			}
			func((const uchar**)srcmat->data.ptr, tab[k * 4 + 1], dstmat->data.ptr, tab[k * 4 + 3], rows, cols, sdelta[i], ddelta[i]);
			/*
			for( int t = 0; t < total; t += blocksize )
			{
				int bsz = std::min(total - t, blocksize);
				func( srcs, sdelta, dsts, ddelta, bsz, (int)npairs );

				if( t + blocksize < total )
					for( k = 0; k < npairs; k++ )
					{
						srcs[k] += blocksize*sdelta[k]*esz1;
						dsts[k] += blocksize*ddelta[k]*esz1;
					}
			}*/
		}
	}

/****************************************************************************************\
*                                       split & merge                                    *
\****************************************************************************************/

	template<typename _Tp> static void
		split_(const _Tp* src, _Tp** dst, int len, int cn)
	{
		int k = cn % 4 ? cn % 4 : 4;
		int i, j;
		if (k == 1)
		{
			_Tp* dst0 = dst[0];
			for (i = j = 0; i < len; i++, j += cn)
				dst0[i] = src[j];
		}
		else if (k == 2)
		{
			_Tp *dst0 = dst[0], *dst1 = dst[1];
			for (i = j = 0; i < len; i++, j += cn)
			{
				dst0[i] = src[j];
				dst1[i] = src[j + 1];
			}
		}
		else if (k == 3)
		{
			_Tp *dst0 = dst[0], *dst1 = dst[1], *dst2 = dst[2];
			for (i = j = 0; i < len; i++, j += cn)
			{
				dst0[i] = src[j];
				dst1[i] = src[j + 1];
				dst2[i] = src[j + 2];
			}
		}
		else
		{
			_Tp *dst0 = dst[0], *dst1 = dst[1], *dst2 = dst[2], *dst3 = dst[3];
			for (i = j = 0; i < len; i++, j += cn)
			{
				dst0[i] = src[j]; dst1[i] = src[j + 1];
				dst2[i] = src[j + 2]; dst3[i] = src[j + 3];
			}
		}

		for (; k < cn; k += 4)
		{
			_Tp *dst0 = dst[k], *dst1 = dst[k + 1], *dst2 = dst[k + 2], *dst3 = dst[k + 3];
			for (i = 0, j = k; i < len; i++, j += cn)
			{
				dst0[i] = src[j]; dst1[i] = src[j + 1];
				dst2[i] = src[j + 2]; dst3[i] = src[j + 3];
			}
		}
	}

	template<typename _Tp> static void
		merge_(const _Tp** src, _Tp* dst, int len, int cn)
	{
		int k = cn % 4 ? cn % 4 : 4;
		int i, j;
		if (k == 1)
		{
			const _Tp* src0 = src[0];
			for (i = j = 0; i < len; i++, j += cn)
				dst[j] = src0[i];
		}
		else if (k == 2)
		{
			const _Tp *src0 = src[0], *src1 = src[1];
			for (i = j = 0; i < len; i++, j += cn)
			{
				dst[j] = src0[i];
				dst[j + 1] = src1[i];
			}
		}
		else if (k == 3)
		{
			const _Tp *src0 = src[0], *src1 = src[1], *src2 = src[2];
			for (i = j = 0; i < len; i++, j += cn)
			{
				dst[j] = src0[i];
				dst[j + 1] = src1[i];
				dst[j + 2] = src2[i];
			}
		}
		else
		{
			const _Tp *src0 = src[0], *src1 = src[1], *src2 = src[2], *src3 = src[3];
			for (i = j = 0; i < len; i++, j += cn)
			{
				dst[j] = src0[i]; dst[j + 1] = src1[i];
				dst[j + 2] = src2[i]; dst[j + 3] = src3[i];
			}
		}

		for (; k < cn; k += 4)
		{
			const _Tp *src0 = src[k], *src1 = src[k + 1], *src2 = src[k + 2], *src3 = src[k + 3];
			for (i = 0, j = k; i < len; i++, j += cn)
			{
				dst[j] = src0[i]; dst[j + 1] = src1[i];
				dst[j + 2] = src2[i]; dst[j + 3] = src3[i];
			}
		}
	}

	static void split8u(const uchar* src, uchar** dst, int len, int cn)
	{
		split_(src, dst, len, cn);
	}

	static void split16u(const ushort* src, ushort** dst, int len, int cn)
	{
		split_(src, dst, len, cn);
	}

	static void split32s(const int* src, int** dst, int len, int cn)
	{
		split_(src, dst, len, cn);
	}

	static void split64s(const int64* src, int64** dst, int len, int cn)
	{
		split_(src, dst, len, cn);
	}

	static void merge8u(const uchar** src, uchar* dst, int len, int cn)
	{
		merge_(src, dst, len, cn);
	}

	static void merge16u(const ushort** src, ushort* dst, int len, int cn)
	{
		merge_(src, dst, len, cn);
	}

	static void merge32s(const int** src, int* dst, int len, int cn)
	{
		merge_(src, dst, len, cn);
	}

	static void merge64s(const int64** src, int64* dst, int len, int cn)
	{
		merge_(src, dst, len, cn);
	}

	typedef void(*SplitFunc)(const uchar* src, uchar** dst, int len, int cn);
	typedef void(*MergeFunc)(const uchar** src, uchar* dst, int len, int cn);

	static SplitFunc getSplitFunc(int depth)
	{
		static SplitFunc splitTab[] =
		{
			(SplitFunc)(split8u), (SplitFunc)(split8u), (SplitFunc)(split16u),
			(SplitFunc)(split32s), (SplitFunc)(split32s), (SplitFunc)(split64s), 0
		};

		return splitTab[depth];
	}

	static MergeFunc getMergeFunc(int depth)
	{
		static MergeFunc mergeTab[] =
		{
			(MergeFunc)(merge8u), (MergeFunc)(merge8u), (MergeFunc)(merge16u), (MergeFunc)(merge16u),
			(MergeFunc)(merge32s), (MergeFunc)(merge32s), (MergeFunc)(merge64s), 0
		};

		return mergeTab[depth];
	}

	void MatOp::split(const Mat& src, Mat* mv)
	{
		TYPE depth = src.type();
		int k, cn = src.channels();
		if (cn == 1)
		{
			mv[0] = src;
			return;
		}
		SplitFunc func = getSplitFunc(depth);
		assert(func != 0);

		int total = src.total();
		uchar** dsts = (uchar**)new uchar*[cn];
		for (k = 0; k < cn; k++)
		{
			mv[k].create(src.size(), depth);
			dsts[k] = mv[k].data.ptr[0];
		}

		func(src.data.ptr[0], dsts, total, cn);
		delete[]dsts;
	}

	void MatOp::merge(const Mat* mv, size_t n, Mat& dst)
	{
		assert(mv && n > 0);

		TYPE depth = mv[0].type();
		bool allch1 = true;
		int k, cn = 0;
		size_t i;

		for (i = 0; i < n; i++)
		{
			assert(mv[i].size() == mv[0].size() && mv[i].type() == depth);
			allch1 = allch1 && mv[i].channels() == 1;
			cn += mv[i].channels();
		}

		assert(0 < cn && cn <= CVLIB_CN_MAX);
		dst.create(mv[0].size(), (TYPE)CVLIB_MAKETYPE(depth, cn));

		if (n == 1)
		{
			dst = mv[0];
			return;
		}

		if (!allch1)
		{
			AutoBuffer<int> pairs(cn * 2);
			int j, ni = 0;

			for (i = 0, j = 0; i < n; i++, j += ni)
			{
				ni = mv[i].channels();
				for (k = 0; k < ni; k++)
				{
					pairs[(j + k) * 2] = j + k;
					pairs[(j + k) * 2 + 1] = j + k;
				}
			}
			mixChannels(mv, n, &dst, 1, &pairs[0], cn);
			return;
		}

		MergeFunc func = getMergeFunc(depth);
		int total = mv[0].total();
		uchar* dstptr = dst.data.ptr[0];
		const uchar** srcptrs = new const uchar*[n];
		for (i = 0; i < n; i++)
		{
			srcptrs[i] = mv[i].data.ptr[0];
		}
		func(srcptrs, dstptr, total, cn);
		delete[]srcptrs;
	}
	void MatOp::split(const Mat& src, Vector<Mat>& dsts)
	{
		int cn = src.channels();
		dsts.resize(cn);
		split(src, dsts.getData());
	}

}