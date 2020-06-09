/*!
 * \file mat.cpp
 * \ingroup base
 * \brief 
 * \author 
 */

#include "Mat.h"
#include "MatOperation.h"
#include "imagecodec/CoImage.h"
#include "IniFile.h"
#include "Exception.h"
#include "LUDecomposition.h"
#include "SingularValueDecomposition.h"
#include "cvlibutil.h"
//#include "XFileDisku.h"
#include <cmath>

namespace cvlib
{

	enum enMatMemType {
		MM_alloc_all, MM_share_all, MM_alloc_rows, MM_share_vec, MM_share_all2, MM_alloc_rows2
	};

	Mat::Mat(int rows, int cols, TYPE type)
	{
		this->m_rows = this->m_cols = 0; data.ptr = NULL;
		create(rows, cols, type);
	}
	Mat::Mat(int _rows, int _cols, TYPE _type, const Scalar& _s)
		: m_rows(0), m_cols(0)
	{
		data.ptr = 0;
		if (_rows > 0 && _cols > 0)
		{
			create(_rows, _cols, _type);
			*this = _s;
		}
	}
	Mat::Mat(void** ptr, int rows, int cols, TYPE type)
	{
		this->m_rows = this->m_cols = 0; data.ptr = NULL;
		create(ptr, rows, cols, type);
	}
	Mat::Mat(void* ptr, int rows, int cols, TYPE type, bool fAlloc /*= true*/)
	{
		this->m_rows = this->m_cols = 0; data.ptr = NULL;
		create(ptr, rows, cols, type, fAlloc);
	}
	Mat::Mat(const Mat& mat)
	{
		if (this == &mat)
			return;
		m_rows = m_cols = 0; data.ptr = 0;
		if (mat.m_depend == MM_share_all2) {
			this->m_rows = mat.m_rows;
			this->m_cols = mat.m_cols;
			this->m_type = mat.m_type;
			m_step = mat.m_step;
			data.ptr = mat.data.ptr;
			m_depend = MM_share_all2;
			return;
		}
		create(mat, true);
	}
	Mat::Mat(const Mat& mat, const Rect& roi)
	{
		m_rows = m_cols = 0; data.ptr = NULL; m_type = MAT_Tbyte;
		m_depend = MM_alloc_all; m_step = 0;
		mat.subRefMat(roi, *this);
	}
	Mat::Mat(const Mat& mat, const Range& range, bool frows)
	{
		m_rows = m_cols = 0; data.ptr = NULL; m_type = MAT_Tbyte;
		m_depend = MM_alloc_all; m_step = 0;
		if (frows)
			mat.subRefMat(Rect(0, range.start, mat.cols(), range.size()), *this);
		else
			mat.subRefMat(Rect(range.start, 0, range.size(), mat.rows()), *this);
	}
	Mat::Mat(const Size& size, TYPE type)
	{
		m_rows = m_cols = 0; data.ptr = NULL;
		create(size, type);
	}

	Mat::~Mat()
	{
		release();
	}
	int	Mat::disableRelease() { 
		int d = m_depend;
		m_depend = MM_share_all2;
		return d;
	}
	void Mat::enableRelease(int depend)
	{
		m_depend = depend;
	}

	int Mat::create(int _rows, int _cols, TYPE _type)
	{
		if (this->m_rows == _rows && this->m_cols == _cols && this->m_type == _type && data.ptr)
			return 1;
		release();
		int i;
		this->m_rows = _rows;
		this->m_cols = _cols;
		this->m_type = _type;
		int cn = channels();
		m_step = CVLIB_ELEM_SIZE(type());
		int nlineStep = m_cols * m_step* cn;
		if (m_rows)
		{
			data.ptr = (uchar**)new uchar*[m_rows];
			try {
				data.ptr[0] = (uchar*)new uchar[m_rows * nlineStep];
			}
			catch (std::bad_alloc&)
			{
				delete[]data.ptr;
				data.ptr = 0;
				return 0;
			}
			if (data.ptr[0] == 0) {
				this->m_rows = 0;
				this->m_cols = 0;
				delete[]data.ptr;
				data.ptr = 0;
				return 0;
			}
			for (i = 1; i < m_rows; i++)
				data.ptr[i] = data.ptr[i - 1] + nlineStep;
		}
		m_depend = MM_alloc_all;
		return 1;
	}
	int Mat::create(const Size& size, TYPE _type)
	{
		return create(size.height, size.width, _type);
	}
	int Mat::create(void** ptr, int _rows, int _cols, TYPE _type)
	{
		release();
		this->m_rows = _rows;
		this->m_cols = _cols;
		this->m_type = _type;
		m_step = CVLIB_ELEM_SIZE(type());
		data.ptr = (uchar**)ptr;
		m_depend = MM_share_all;
		return 1;
	}
	int Mat::create(void* ptr, int _rows, int _cols, TYPE _type, bool fAlloc /*= true*/)
	{
		release();
		int i;
		this->m_rows = _rows;
		this->m_cols = _cols;
		this->m_type = _type;
		int cn = channels();

		if (fAlloc)
		{
			m_step = CVLIB_ELEM_SIZE(type());
			data.ptr = (uchar**)new uchar*[m_rows];
			data.ptr[0] = (uchar*)ptr;
			int nlinestep = m_step*m_cols*cn;
			for (i = 1; i < m_rows; i++)
				data.ptr[i] = data.ptr[i - 1] + nlinestep;
			m_depend = MM_alloc_rows;
		}
		else
		{
			m_step = CVLIB_ELEM_SIZE(type());
			int nlinestep = m_cols * cn * m_step;
			data.ptr = (uchar**)(((uchar*)ptr) + m_rows * nlinestep);
			data.ptr[0] = (uchar*)ptr;
			for (i = 1; i < m_rows; i++)
				data.ptr[i] = data.ptr[i - 1] + nlinestep;
			m_depend = MM_share_all2;
		}
		return 1;
	}
	int	Mat::create(Vec& vec)
	{
		release();
		data.ptr = (uchar**)new uchar*[1];//	MALLOC (data.ptr, sizeof(uchar*), uchar*);
		data.ptr[0] = vec.data.ptr;
		m_rows = 1;
		m_cols = vec.length();
		this->m_type = vec.m_type;

		m_depend = MM_share_vec;
		return 1;
	}
	int	Mat::create(const Mat& mA, bool fCopy /*= false*/)
	{
		int nRet = create(mA.m_rows, mA.m_cols, mA.m_type);
		if (nRet)
		{
			if (fCopy)
			{
				Size sz;
				if (this->isContinuous() && mA.isContinuous())
					sz = Size(m_rows*m_cols*mA.channels(), 1);
				else
					sz = Size(m_cols*mA.channels(), m_rows);
				int nlinestep = step() * sz.width;
				for (int i = 0; i < sz.height; i++)
					memcpy(data.ptr[i], mA.data.ptr[i], nlinestep);
			}
			return 1;
		}
		return 0;
	}

	void Mat::release()
	{
		if (m_depend == MM_share_all2 || m_depend == MM_share_all)
			return;

		if (data.ptr)
		{
			if (m_depend == MM_alloc_all)
			{
				delete[]data.ptr[0];
				delete[]data.ptr;
			}
			else if (m_depend == MM_alloc_rows)
			{
				delete[]data.ptr;
			}
			else if (m_depend == MM_share_vec)
			{
				delete[]data.ptr;
			}
			else if (m_depend == MM_share_all2)
			{
			}
			else if (m_depend == MM_alloc_rows2)
			{
				delete[]data.ptr;
			}
			data.ptr = 0;
			m_rows = 0;
			m_cols = 0;
		}
	}
	void Mat::sharedTo(Mat& a) const
	{
		a.release();
		a.m_rows = this->m_rows;
		a.m_cols = this->m_cols;
		a.m_cols = this->m_cols;
		a.m_type = this->m_type;
		a.m_step = this->m_step;
		a.data = this->data;
		a.m_depend = MM_share_all2;
	}

	Mat& Mat::operator=(const Mat& matSrc)
	{
		if (this == &matSrc)
			return *this;
		if (matSrc.m_depend == MM_share_all2) {
			release();
			this->m_rows = matSrc.m_rows;
			this->m_cols = matSrc.m_cols;
			this->m_type = matSrc.m_type;
			m_step = matSrc.m_step;
			data.ptr = matSrc.data.ptr;
			m_depend = MM_share_all2;
			return *this;
		}
		if (m_rows == matSrc.rows() && m_cols == matSrc.cols() && m_type == matSrc.m_type)
		{
			if (isValid())
			{
				int nlinestep = m_cols * step() * channels();
				if (isContinuous() && matSrc.isContinuous())
				{
					nlinestep *= m_rows;
					memcpy(data.ptr[0], matSrc.data.ptr[0], nlinestep);
				}
				else
				{
					for (int i = 0; i < m_rows; i++)
						memcpy(data.ptr[i], matSrc.data.ptr[i], nlinestep);
				}
			}
		}
		else
		{
			create(matSrc, true);
		}
		return *this;
	}

	void scalarToRawData(const Scalar& s, void* buf, int m_type, int unroll_to)
	{
		int depth = CVLIB_MAT_DEPTH(m_type), cn = CVLIB_MAT_CN(m_type);
		switch (depth)
		{
		case MAT_Tchar:
			s.convertTo((char*)buf, cn, unroll_to);
			break;
		case MAT_Tbyte:
			s.convertTo((uchar*)buf, cn, unroll_to);
			break;
		case MAT_Tshort:
			s.convertTo((short*)buf, cn, unroll_to);
			break;
		case MAT_Tint:
			s.convertTo((int*)buf, cn, unroll_to);
			break;
		case MAT_Tfloat:
			s.convertTo((float*)buf, cn, unroll_to);
			break;
		case MAT_Tdouble:
			s.convertTo((double*)buf, cn, unroll_to);
			break;
		default:
			assert(false);
		}
	}
	Mat& Mat::operator=(const Scalar& s)
	{
		if (!isValid())
			return *this;

		Size sz = size();
		uchar** dst = data.ptr;

		sz.width *= (m_step*channels());
		if (isContinuous())
		{
			sz.width *= sz.height;
			sz.height = 1;
		}

		if (s[0] == 0 && s[1] == 0 && s[2] == 0 && s[3] == 0)
		{
			for (int i = 0; i < sz.height; i++)
				memset(dst[i], 0, sz.width);
		}
		else
		{
			int t = m_type;
			int esz1 = (int)step();
			double scalar[12];
			scalarToRawData(s, scalar, t, 12);
			int copy_len = 12 * esz1;
			if (isContinuous()) {
				uchar* dst2 = dst[0];
				uchar* dst_limit = dst2 + sz.width;
				if (sz.height--)
				{
					while (dst2 + copy_len <= dst_limit)
					{
						memcpy(dst2, scalar, copy_len);
						dst2 += copy_len;
					}
					memcpy(dst2, scalar, dst_limit - dst2);
				}

				if (sz.height > 0)
				{
					dst2 = dst_limit;
					for (; sz.height--; dst2 += sz.width)
						memcpy(dst2, data.ptr[0], sz.width);
				}
			}
			else {
				for (int y = 0; y < sz.height; y++) {
					uchar* dst2 = dst[y];
					uchar* dst_limit = dst2 + sz.width;
					while (dst2 + copy_len <= dst_limit)
					{
						memcpy(dst2, scalar, copy_len);
						dst2 += copy_len;
					}
					memcpy(dst2, scalar, dst_limit - dst2);
				}
			}
		}
		return *this;
	}

	void Mat::zero()
	{
		if (data.ptr == 0)
			return;
		if (isContinuous())
		{
			int nsize = m_step * m_rows * m_cols * channels();
			memset(data.ptr[0], 0, nsize);
		}
		else
		{
			int nsize = m_step * m_cols * channels();
			for (int i = 0; i < m_rows; i++)
				memset(data.ptr[i], 0, nsize);
		}
	}

	double	Mat::value(int i, int j, int cn) const
	{
		switch (type())
		{
		case MAT_Tbyte:
			return data.ptr[i][j*channels() + cn];
		case MAT_Tshort:
			return data.s[i][j*channels() + cn];
		case MAT_Tint:
			return data.i[i][j*channels() + cn];
		case MAT_Tfloat:
			return data.fl[i][j*channels() + cn];
		case MAT_Tdouble:
			return data.db[i][j*channels() + cn];
		default:
			return 0.0f;
		}
	}

	void Mat::subRefMat(const Rect& region, Mat& mRef) const
	{
		if (mRef.size() != region.size() || mRef.m_depend != MM_alloc_rows2)
		{
			mRef.release();
			mRef.data.ptr = (uchar**)new uchar*[region.height];
			mRef.resizeRows(region.height);
			mRef.resizeCols(region.width);
			mRef.m_depend = MM_alloc_rows2;
			mRef.m_type = m_type;
			mRef.m_step = m_step;
		}
		int offset = region.x*step()*channels();
		for (int i = 0; i < region.height; i++)
			mRef.data.ptr[i] = &data.ptr[i + region.y][offset];
	}
	void Mat::subMat(const Rect& region, Mat& mRef) const
	{
		if (mRef.size() != region.size() || mRef.type1() != this->type1())
		{
			mRef.release();
			mRef.create(region.size(), m_type);
			mRef.zero();
		}
		int nlinestep = step()*channels()*region.width;
		int offset = region.x*step()*channels();
		for (int i = 0, k = region.y; i < region.height; i++, k++) {
			if (k < 0 || k >= rows())
				continue;
			memcpy(mRef.data.ptr[i], &data.ptr[k][offset], nlinestep);
		}
	}

	int Mat::fromFile(const char* szFilename)
	{
		return Object::fromFile(szFilename);
	}
	int Mat::toFile(const char* szFilename) const
	{
		return Object::toFile(szFilename);
	}
	int Mat::fromFile(XFile* pFile)
	{
		if (isValid())
			release();
		if (pFile == NULL)
			return 0;
		int nRows, nCols, nType;
		pFile->read(&nRows, sizeof(int), 1);
		pFile->read(&nCols, sizeof(int), 1);
		pFile->read(&nType, sizeof(int), 1);
		if (nRows != 0 && nCols != 0)
		{
			create(nRows, nCols, (TYPE)nType);
			int nlinestep = nCols * step() * channels();
			for (int i = 0; i < nRows; i++)
				pFile->read(data.ptr[i], nlinestep, 1);
		}
		return 1;
	}
	int Mat::toFile(XFile* pFile) const
	{
		if (pFile == 0)
			return 0;

		int nType;
		pFile->write(&m_rows, sizeof(int), 1);
		pFile->write(&m_cols, sizeof(int), 1);
		nType = m_type;
		pFile->write(&nType, sizeof(int), 1);
		if (data.ptr) {
			int nlinestep = m_cols * step() * channels();
			for (int i = 0; i < m_rows; i++)
				pFile->write(data.ptr[i], nlinestep, 1);
		}

		return 1;
	}

	void Mat::identity(double val)
	{
		zero();
		int ii, k;
		int t = MIN(m_rows, m_cols);
		int cn = channels();
		if (cn == 1)
		{
			switch (type())
			{
			case MAT_Tbyte:
				for (ii = 0; ii < t; ii++)
					data.ptr[ii][ii] = (uchar)val;
				break;
			case MAT_Tshort:
				for (ii = 0; ii < t; ii++)
					data.s[ii][ii] = (short)val;
				break;
			case MAT_Tint:
				for (ii = 0; ii < t; ii++)
					data.i[ii][ii] = (int)val;
				break;
			case MAT_Tfloat:
				for (ii = 0; ii < t; ii++)
					data.fl[ii][ii] = (float)val;
				break;
			case MAT_Tdouble:
				for (ii = 0; ii < t; ii++)
					data.db[ii][ii] = (double)val;
				break;
			default:
				return;
			}
		}
		else
		{
			switch (type())
			{
			case MAT_Tbyte:
				for (ii = 0; ii < t; ii++) for (k = 0; k < cn; k++)
					data.ptr[ii][ii*cn + k] = (uchar)val;
				break;
			case MAT_Tshort:
				for (ii = 0; ii < t; ii++) for (k = 0; k < cn; k++)
					data.s[ii][ii*cn + k] = (short)val;
				break;
			case MAT_Tint:
				for (ii = 0; ii < t; ii++) for (k = 0; k < cn; k++)
					data.i[ii][ii*cn + k] = (int)val;
				break;
			case MAT_Tfloat:
				for (ii = 0; ii < t; ii++) for (k = 0; k < cn; k++)
					data.fl[ii][ii*cn + k] = (float)val;
				break;
			case MAT_Tdouble:
				for (ii = 0; ii < t; ii++) for (k = 0; k < cn; k++)
					data.db[ii][ii*cn + k] = (double)val;
				break;
			default:
				return;
			}
		}
	}
	void Mat::identity()
	{
		zero();
		int ii, k;
		int t = MIN(m_rows, m_cols);
		int cn = channels();

		if (cn == 1)
		{
			switch (type())
			{
			case MAT_Tbyte:
				for (ii = 0; ii < t; ii++)
					data.ptr[ii][ii] = (uchar)1;
				break;
			case MAT_Tshort:
				for (ii = 0; ii < t; ii++)
					data.s[ii][ii] = (short)1;
				break;
			case MAT_Tint:
				for (ii = 0; ii < t; ii++)
					data.i[ii][ii] = (int)1;
				break;
			case MAT_Tfloat:
				for (ii = 0; ii < t; ii++)
					data.fl[ii][ii] = (float)1;
				break;
			case MAT_Tdouble:
				for (ii = 0; ii < t; ii++)
					data.db[ii][ii] = (double)1;
				break;
			default:
				return;
			}
		}
		else
		{
			switch (type())
			{
			case MAT_Tbyte:
				for (ii = 0; ii < t; ii++) for (k = 0; k < cn; k++)
					data.ptr[ii][ii*cn + k] = (uchar)1;
				break;
			case MAT_Tshort:
				for (ii = 0; ii < t; ii++) for (k = 0; k < cn; k++)
					data.s[ii][ii*cn + k] = (short)1;
				break;
			case MAT_Tint:
				for (ii = 0; ii < t; ii++) for (k = 0; k < cn; k++)
					data.i[ii][ii*cn + k] = (int)1;
				break;
			case MAT_Tfloat:
				for (ii = 0; ii < t; ii++) for (k = 0; k < cn; k++)
					data.fl[ii][ii*cn + k] = (float)1;
				break;
			case MAT_Tdouble:
				for (ii = 0; ii < t; ii++) for (k = 0; k < cn; k++)
					data.db[ii][ii*cn + k] = (double)1;
				break;
			default:
				return;
			}
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	typedef double(*MinFunc)(uchar** const _src, int rows, int cols);
	template<typename _Tp> static double minFunc_(uchar** const _src, int rows, int cols)
	{
		_Tp** const src = (_Tp** const)_src;
		int i, j;
		_Tp minvalue = 0;
		if (rows && cols)
			minvalue = src[0][0];
		for (i = 0; i < rows; i++)
		{
			_Tp* const src_i = src[i];
			for (j = 0; j < cols; j++)
			{
				if (minvalue > src_i[j])
					minvalue = src_i[j];
			}
		}
		return (double)minvalue;
	}
	typedef double(*MinFuncCh)(uchar** const _src, int rows, int cols, int cnsize, int cn);
	template<typename _Tp> static double minFuncCh_(uchar** const _src, int rows, int cols, int cnsize, int cn)
	{
		_Tp** const src = (_Tp** const)_src;
		int i, j, icn;
		_Tp minvalue = 0;
		if (rows && cols)
			minvalue = src[0][cn];
		for (i = 0; i < rows; i++)
		{
			_Tp* const src_i = src[i];
			for (j = 0, icn = cn; j < cols; j++, icn += cnsize)
			{
				if (minvalue > src_i[icn])
					minvalue = src_i[icn];
			}
		}
		return (double)minvalue;
	}
	double Mat::min(int cn) const
	{
		int cnsize = channels();
		if (cnsize == 1)
		{
			MinFunc minFuncs[] = { minFunc_<char>, minFunc_<uchar>, minFunc_<short>, minFunc_<int>, minFunc_<float>, minFunc_<double> };
			if (this->isContinuous())
				return minFuncs[type()](data.ptr, 1, m_rows*m_cols);
			else
				return minFuncs[type()](data.ptr, m_rows, m_cols);
		}
		else
		{
			MinFuncCh minFuncsCh[] = { minFuncCh_<char>, minFuncCh_<uchar>, minFuncCh_<short>, minFuncCh_<int>, minFuncCh_<float>, minFuncCh_<double> };
			if (this->isContinuous())
				return minFuncsCh[type()](data.ptr, 1, m_rows*m_cols, cnsize, cn);
			else
				return minFuncsCh[type()](data.ptr, m_rows, m_cols, cnsize, cn);
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	typedef double(*MaxFunc)(uchar** const _src, int rows, int cols);
	template<typename _Tp> static double maxFunc_(uchar** const _src, int rows, int cols)
	{
		_Tp** const src = (_Tp** const)_src;
		int i, j;
		_Tp maxvalue = 0;
		if (rows && cols)
			maxvalue = src[0][0];
		for (i = 0; i < rows; i++)
		{
			_Tp* const src_i = src[i];
			for (j = 0; j < cols; j++)
			{
				if (maxvalue < src_i[j])
					maxvalue = src_i[j];
			}
		}
		return (double)maxvalue;
	}
	typedef double(*MaxFuncCh)(uchar** const _src, int rows, int cols, int cnsize, int cn);
	template<typename _Tp> static double maxFuncCh_(uchar** const _src, int rows, int cols, int cnsize, int cn)
	{
		_Tp** const src = (_Tp** const)_src;
		int i, j, icn;
		_Tp maxvalue = 0;
		if (rows && cols)
			maxvalue = src[0][cn];
		for (i = 0; i < rows; i++)
		{
			_Tp* const src_i = src[i];
			for (j = 0, icn = cn; j < cols; j++, icn += cnsize)
			{
				if (maxvalue < src_i[icn])
					maxvalue = src_i[icn];
			}
		}
		return (double)maxvalue;
	}
	double Mat::max(int cn) const
	{
		int cnsize = channels();
		if (cnsize == 1)
		{
			MaxFunc maxFuncs[] = { maxFunc_<char>, maxFunc_<uchar>, maxFunc_<short>, maxFunc_<int>, maxFunc_<float>, maxFunc_<double> };
			if (this->isContinuous())
				return maxFuncs[type()](data.ptr, 1, m_rows*m_cols);
			else
				return maxFuncs[type()](data.ptr, m_rows, m_cols);
		}
		else
		{
			MaxFuncCh maxFuncsCh[] = { maxFuncCh_<char>, maxFuncCh_<uchar>, maxFuncCh_<short>, maxFuncCh_<int>, maxFuncCh_<float>, maxFuncCh_<double> };
			if (this->isContinuous())
				return maxFuncsCh[type()](data.ptr, 1, m_rows*m_cols, cnsize, cn);
			else
				return maxFuncsCh[type()](data.ptr, m_rows, m_cols, cnsize, cn);
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename _Tp> static void minElementFunc_(_Tp** const src, int rows, int cols, _Tp& minvalue, int& minrow, int& mincol)
	{
		minrow = mincol = 0;
		int i, j;
		for (i = 0; i < rows; i++)
		{
			_Tp* const src_i = src[i];
			for (j = 0; j < cols; j++)
			{
				if (minvalue > src_i[j]) {
					minvalue = src_i[j];
					minrow = i;
					mincol = j;
				}
			}

		}
	}
	double Mat::min(int& row, int& col) const
	{
		Size size(m_cols, m_rows);
		row = col = 0;
		switch (type())
		{
		case MAT_Tbyte: {
			uchar minvalue = 255;
			minElementFunc_<uchar>(data.ptr, size.height, size.width, minvalue, row, col);
			return (double)minvalue;
		}
		case MAT_Tshort: {
			short minvalue = SHRT_MAX;
			minElementFunc_<short>(data.s, size.height, size.width, minvalue, row, col);
			return (double)minvalue;
		}
		case MAT_Tint: {
			int minvalue = INT_MAX;
			minElementFunc_<int>(data.i, size.height, size.width, minvalue, row, col);
			return (double)minvalue;
		}
		case MAT_Tfloat: {
			float minvalue = FLT_MAX;
			minElementFunc_<float>(data.fl, size.height, size.width, minvalue, row, col);
			return (double)minvalue;
		}
		case MAT_Tdouble: {
			double minvalue = DBL_MAX;
			minElementFunc_<double>(data.db, size.height, size.width, minvalue, row, col);
			return (double)minvalue;
		}
		default:
			return 0;
		}
	}
	template<typename _Tp> static void maxElementFunc_(_Tp** const src, int rows, int cols, _Tp& maxvalue, int& maxrow, int& maxcol)
	{
		maxrow = maxcol = 0;
		int i, j;
		for (i = 0; i < rows; i++)
		{
			_Tp* const src_i = src[i];
			for (j = 0; j < cols; j++)
			{
				if (maxvalue < src_i[j]) {
					maxvalue = src_i[j];
					maxrow = i;
					maxcol = j;
				}
			}

		}
	}

	double Mat::max(int& row, int& col) const
	{
		Size size(m_cols, m_rows);
		row = col = 0;
		switch (type())
		{
		case MAT_Tbyte: {
			uchar maxvalue = 0;
			maxElementFunc_<uchar>(data.ptr, size.height, size.width, maxvalue, row, col);
			return (double)maxvalue;
		}
		case MAT_Tshort: {
			short maxvalue = SHRT_MIN;
			maxElementFunc_<short>(data.s, size.height, size.width, maxvalue, row, col);
			return (double)maxvalue;
		}
		case MAT_Tint: {
			int maxvalue = INT_MIN;
			maxElementFunc_<int>(data.i, size.height, size.width, maxvalue, row, col);
			return (double)maxvalue;
		}
		case MAT_Tfloat: {
			float maxvalue = -FLT_MAX;
			maxElementFunc_<float>(data.fl, size.height, size.width, maxvalue, row, col);
			return (double)maxvalue;
		}
		case MAT_Tdouble: {
			double maxvalue = -DBL_MAX;
			maxElementFunc_<double>(data.db, size.height, size.width, maxvalue, row, col);
			return (double)maxvalue;
		}
		default:
			return 0;
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	typedef double(*SumFunc)(uchar** const _src, int rows, int cols);
	template<typename _Tp> static double sumValue_(uchar** const _src, int rows, int cols)
	{
		_Tp** const src = (_Tp** const)_src;
		double rsum = 0;
		for (int i = 0; i < rows; i++) {
			const _Tp* src_i = src[i];
			for (int j = 0; j < cols; j++)
				rsum += *src_i++;
		}
		return rsum;
	}
	double Mat::mean() const
	{
		if (m_rows == 0 || m_cols == 0)
			return 0;
		SumFunc sumFuncs[] = { sumValue_<char>, sumValue_<uchar>, sumValue_<short>, sumValue_<int>, sumValue_<float>, sumValue_<double> };
		Size size(m_cols, m_rows);
		if (isContinuous())
			size = Size(m_cols*m_rows, 1);
		return sumFuncs[type()](data.ptr, size.height, size.width) / (double)(m_rows * m_cols);
	}

	Mat& Mat::operator=(double value)
	{
		if (!isValid())
			return *this;

		if (fabs(value) < 1e-10)
			zero();
		else
			setValue(value);
		return *this;
	}
	Mat& Mat::operator*=(double value)
	{
		MatOp::mul(this, value, this);
		return *this;
	}
	Mat& Mat::operator+=(const Mat& A)
	{
		MatOp::add(this, this, &A);
		return *this;
	}
	Mat& Mat::operator-=(const Mat& A)
	{
		MatOp::sub(this, this, &A);
		return *this;
	}
	Mat& Mat::operator|=(const Mat& A)
	{
		int cn = channels();
		int nline = m_cols*cn*step();
		for (int i = 0; i < m_rows; i++)
		{
			const uchar* psrc = A.data.ptr[i];
			uchar* pdst = data.ptr[i];
			for (int j = 0; j < nline; j++, psrc++, pdst++)
			{
				*pdst |= *psrc;
			}
		}
		return *this;
	}
	Mat& Mat::operator&=(const Mat& A)
	{
		int cn = channels();
		int nline = m_cols*cn*step();
		for (int i = 0; i < m_rows; i++)
		{
			const uchar* psrc = A.data.ptr[i];
			uchar* pdst = data.ptr[i];
			for (int j = 0; j < nline; j++, psrc++, pdst++)
			{
				*pdst &= *psrc;
			}
		}
		return *this;
	}
	void Mat::setValue(double rVal)
	{
		if (isContinuous())
		{
			int i;
			int nelems = m_rows*m_cols*channels();
			switch (type())
			{
			case MAT_Tbyte:
			{
				uchar* pbuf = data.ptr[0];
				uchar b = (uchar)rVal;
				for (i = 0; i < nelems; i++)
					*pbuf++ = b;
			}
			break;
			case MAT_Tshort:
			{
				short* pbuf = data.s[0];
				short b = (short)rVal;
				for (i = 0; i < nelems; i++)
					*pbuf++ = b;
			}
			break;
			case MAT_Tint:
			{
				int* pbuf = data.i[0];
				int b = (int)rVal;
				for (i = 0; i < nelems; i++)
					*pbuf++ = b;
			}
			break;
			case MAT_Tfloat:
			{
				float* pbuf = data.fl[0];
				float b = (float)rVal;
				for (i = 0; i < nelems; i++)
					*pbuf++ = b;
			}
			break;
			case MAT_Tdouble:
			{
				double* pbuf = data.db[0];
				double b = (double)rVal;
				for (i = 0; i < nelems; i++)
					*pbuf = b;
			}
			break;
			default:
				return;
			}
		}
		else
		{
			int i, k;
			int nelems = cols()*channels()*step();
			if (rVal == 0)
			{
				for (i = 0; i < m_rows; i++)
					memset(data.ptr[i], 0, nelems);
			}
			else
			{
				int nelem_count = m_cols * channels();
				switch (type())
				{
				case MAT_Tbyte:
				{
					for (k = 0; k < nelem_count; k++)
						data.ptr[0][k] = (uchar)rVal;
				}
				break;
				case MAT_Tshort:
				{
					for (k = 0; k < nelem_count; k++)
						data.s[0][k] = (short)rVal;
				}
				break;
				case MAT_Tint:
				{
					for (k = 0; k < nelem_count; k++)
						data.i[0][k] = (int)rVal;
				}
				break;
				case MAT_Tfloat:
				{
					for (k = 0; k < nelem_count; k++)
						data.fl[0][k] = (float)rVal;
				}
				break;
				case MAT_Tdouble:
				{
					for (k = 0; k < nelem_count; k++)
						data.db[0][k] = (double)rVal;
				}
				break;
				default:
					return;
				}
				for (i = 1; i < m_rows; i++)
					memcpy(data.ptr[i], data.ptr[0], nelems);
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////////
	bool Mat::fromImage(uchar* buffer, int size, ulong nImageType)
	{
		XFileMem memfile(buffer, size);
		return fromImage(&memfile, nImageType);
	}
	bool	Mat::fromImage(XFile* fp, ulong nImageType)
	{
		CoImage image;
		if (!image.Decode(*this, fp, nImageType))
			return false;
		return true;
	}
	bool Mat::fromImage(const char* szfilename, ulong nImageType)
	{
		XFileDisk xfile;
		if (!xfile.open(szfilename, "rb"))
			return false;
		CoImage image;
		return image.Decode(*this, &xfile, nImageType);
	}
	/*bool Mat::fromImage(const wchar_t* szfilename, ulong nImageType)
	{
		XFileDisku xfile;
		if (!xfile.open(szfilename, L"rb"))
			return false;
		CoImage image;
		return image.Decode(*this, &xfile, nImageType);
	}*/
	bool Mat::toImage(const char* szfilename, ulong nImageType) const
	{
		XFileDisk xfile;
		if (!xfile.open(szfilename, "wb"))
			return false;
		CoImage image;
		return image.Encode(*this, &xfile, nImageType);
	}
	/*bool Mat::toImage(const wchar_t* szfilename, ulong nImageType) const
	{
		XFileDisku xfile;
		if (!xfile.open(szfilename, L"wb"))
			return false;
		CoImage image;
		return image.Encode(*this, &xfile, nImageType);
        return false;
	}*/
	bool Mat::toImage(XFile* fp, ulong nImageType) const
	{
		CoImage image;
		return image.Encode(*this, fp, nImageType);
	}
	bool Mat::toImage(const char* szFilename, ulong nImageType, bool fExpand, float rMin, float rMax) const
	{
		Mat t(m_rows, m_cols, MAT_Tuchar3);
		int i, j;
		double rMin1;
		double rMax1;
		if (fExpand)
		{
			rMin1 = min();
			rMax1 = max();
		}
		else
		{
			rMin1 = rMin;
			rMax1 = rMax;
		}
		float rExpand = 255.0f / (float)(rMax1 - rMin1);

		int idx = 0;
		uchar* pdst = t.data.ptr[0];
		for (i = 0; i < m_rows; i++)
		{
			for (j = 0; j < m_cols; j++)
			{
				double rV = (value(i, j) - rMin1) * rExpand;
				uchar d = (uchar)MIN(MAX(rV, 0), 255);
				pdst[idx++] = d;
				pdst[idx++] = d;
				pdst[idx++] = d;
			}
		}
		XFileDisk xfile;
		if (!xfile.open(szFilename, "wb"))
			return false;
		CoImage image;
		return image.Encode(t, &xfile, nImageType);
	}
	/*bool Mat::toImage(const wchar_t* szFilename, ulong nImageType, bool fExpand, float rMin, float rMax) const
	{
		Mat t(m_rows, m_cols, MAT_Tuchar3);
		int i, j;
		double rMin1;
		double rMax1;
		if (fExpand)
		{
			rMin1 = min();
			rMax1 = max();
		}
		else
		{
			rMin1 = rMin;
			rMax1 = rMax;
		}
		float rExpand = 255.0f / (float)(rMax1 - rMin1);

		int idx = 0;
		uchar* pdst = t.data.ptr[0];
		for (i = 0; i < m_rows; i++)
		{
			for (j = 0; j < m_cols; j++)
			{
				double rV = (value(i, j) - rMin1) * rExpand;
				uchar d = (uchar)MIN(MAX(rV, 0), 255);
				pdst[idx++] = d;
				pdst[idx++] = d;
				pdst[idx++] = d;
			}
		}
		XFileDisku xfile;
		if (!xfile.open(szFilename, L"wb"))
			return false;
		CoImage image;
		return image.Encode(t, &xfile, nImageType);
	}*/
	bool Mat::toImage(const char* szFilename, ulong nImageType, bool fExpand, float rMin[], float rMax[]) const
	{
		int i, j, k;
		int cn = channels();
		TYPE type2 = (TYPE)CVLIB_MAKETYPE(MAT_Tbyte, cn);
		Mat image(m_rows, m_cols, type2);
		for (k = 0; k < cn; k++)
		{
			double rMin1;
			double rMax1;
			if (fExpand)
			{
				rMin1 = min(k);
				rMax1 = max(k);
			}
			else
			{
				rMin1 = rMin[k];
				rMax1 = rMax[k];
			}
			float rExpand = 255.0f / (float)(rMax1 - rMin1);
			for (i = 0; i < m_rows; i++)
			{
				uchar* pdst = image.data.ptr[i];
				for (j = 0; j < m_cols; j++)
				{
					double rV = (value(i, j, k) - rMin1) * rExpand;
					pdst[j*cn + k] = (uchar)MIN(MAX(rV, 0), 255);
				}
			}
		}
		XFileDisk xfile;
		if (!xfile.open(szFilename, "wb"))
			return false;
		CoImage encoder;
		return encoder.Encode(image, &xfile, nImageType);
	}
	/*bool Mat::toImage(const wchar_t* szFilename, ulong nImageType, bool fExpand, float rMin[], float rMax[]) const
	{
		int i, j, k;
		int cn = channels();
		TYPE type2 = (TYPE)CVLIB_MAKETYPE(MAT_Tbyte, cn);
		Mat image(m_rows, m_cols, type2);
		for (k = 0; k < cn; k++)
		{
			double rMin1;
			double rMax1;
			if (fExpand)
			{
				rMin1 = min(k);
				rMax1 = max(k);
			}
			else
			{
				rMin1 = rMin[k];
				rMax1 = rMax[k];
			}
			float rExpand = 255.0f / (float)(rMax1 - rMin1);
			for (i = 0; i < m_rows; i++)
			{
				uchar* pdst = image.data.ptr[i];
				for (j = 0; j < m_cols; j++)
				{
					double rV = (value(i, j, k) - rMin1) * rExpand;
					pdst[j*cn + k] = (uchar)MIN(MAX(rV, 0), 255);
				}
			}
		}
		XFileDisku xfile;
		if (!xfile.open(szFilename, L"wb"))
			return false;
		CoImage encoder;
		return encoder.Encode(image, &xfile, nImageType);
	}*/
	void Mat::toMatlab(const String& sFilename, const String& sName, const String& sComment, bool fAppend) const
	{
		char		strTmp[1024];
		FILE* pFile = NULL;
		if (fAppend)
			pFile = fopen(sFilename, "a++");
		else
			pFile = fopen(sFilename, "w");

		if (sComment != "")
			fprintf(pFile, "%%%s\n", (const char*)sComment);
		fprintf(pFile, "%s=[\n", (const char*)sName);
		for (int i = 0; i < rows(); i++)
		{
			for (int j = 0; j < cols(); j++)
			{
				double rValue = value(i, j);
				sprintf(strTmp, "%6.6f ", rValue);
				fprintf(pFile, "%s", strTmp);
			}
			fprintf(pFile, ";\n");
		}
		fprintf(pFile, "];\n");
		fclose(pFile);
	}
	void Mat::fromMatlab(const String& sFilename, const String& sName)
	{
		TokenFile	DIVABuf;					//Buffer or stream to the file.
		String		sTokenString;				//String for retriving tokings, or element entities from DIVAbuf.
		Array<double, double>	Entry;			//Container for the entries or numbers untile size om matrix is known.
		int			cCols = 0;					//Number of columns in the matrix.
		int			cRows = 0;					//Number of m_rows in the matrix.
		int			cEntry = 0;					//Counter for wich entry is to be writen to the matrix.

		//open stream/buffer.
		if (!DIVABuf.open(sFilename, "w"))
			throw Exception("The requested file does not exist.", 0, "fromMatlab", "Mat.cpp", __LINE__);


		//find the sought after matrix or fail.
		do {
			if (!DIVABuf.nextToken(sTokenString))
			{
				throw Exception("The requested variable is not in the requested file.", 0, "fromMatlab", "Mat.cpp", __LINE__);
			}
		} while (sTokenString != sName + "=[");


		//Parse the first row of the matrix, and there by retrive the number of columns.
		//Get a token from the stream.
		if (!DIVABuf.nextToken(sTokenString))
		{
			throw Exception("Error reading from file.", 0, "fromMatlab", "Mat.cpp", __LINE__);
		}
		while (sTokenString != String(";"))				//";" marks the end of a line.
		{
			Entry.add(atof((const char*)(sTokenString)));
			//Get a token from the stream.
			if (!DIVABuf.nextToken(sTokenString))
			{
				throw Exception("Error reading from file.", 0, "fromMatlab", "Mat.cpp", __LINE__);
			}
			cCols++;
		}

		//Parse the rest of the matrix 
		while (sTokenString != String("];"))				//"];" marks the end of a matrix.
		{
			if (sTokenString == String(";"))				//";" marks the end of a line.
			{
				cRows++;
			}
			else
			{
				Entry.add(atof((const char*)(sTokenString)));
			}
			//Get a token from the stream.
			if (!DIVABuf.nextToken(sTokenString))
			{
				throw Exception("Error reading from file.", 0, "fromMatlab", "Mat.cpp", __LINE__);
			}
		}

		//Adjust the size of the (output)matrix.
		create(cRows, cCols, MAT_Tdouble);

		//Transfere the values.
		for (int cI = 0; cI < cRows; cI++)
		{
			for (int cJ = 0; cJ < cCols; cJ++)
			{
				data.db[cI][cJ] = Entry[cEntry];
				cEntry++;
			}
		}
	}
	void	Mat::toC(const String& szFilename, const String& szVarName, bool fAppend) const
	{
		if (rows() == 0)
			return;
		FILE* pFile = NULL;
		int* pan = new int[cols()];
		double* pan_double = new double[cols()];
		if (fAppend)
			pFile = fopen(szFilename.str(), "a+");
		else
			pFile = fopen(szFilename.str(), "w");

		char sztype[256];
		switch (type())
		{
		case MAT_Tbyte: strcpy(sztype, "uchar"); break;
		case MAT_Tshort: strcpy(sztype, "short"); break;
		case MAT_Tint: strcpy(sztype, "int"); break;
		case MAT_Tfloat: strcpy(sztype, "float"); break;
		case MAT_Tdouble: strcpy(sztype, "double"); break;
		default:  strcpy(sztype, "usrtype");
		}
		fprintf(pFile, "%s %s[%d][%d]={", sztype, szVarName.str(), m_rows, m_cols);

		int i;
		for (i = 0; i < m_rows; i++)
		{
			fprintf(pFile, "\n\t{");
			int j;
			for (j = 0; j < m_cols; j++)
			{
				switch (type())
				{
				case MAT_Tchar:
				case MAT_Tbyte:
					pan[j] = data.ptr[i][j];
					break;
				case MAT_Tshort:
					pan[j] = data.s[i][j];
					break;
				case MAT_Tint:
					pan[j] = data.i[i][j];
					break;
				case MAT_Tfloat:
					pan_double[j] = data.fl[i][j];
					break;
				case MAT_Tdouble:
					pan_double[j] = data.db[i][j];
					break;
				default:
					assert(false);
				}
			}
			switch (type())
			{
			case MAT_Tchar: case MAT_Tbyte:
				for (j = 0; j < cols() - 1; j++)
					fprintf(pFile, "%03d,", pan[j]);
				fprintf(pFile, "%03d},", pan[cols() - 1]);
				break;
			case MAT_Tshort: case MAT_Tint:
				for (j = 0; j < cols() - 1; j++)
					fprintf(pFile, "%d,", pan[j]);
				fprintf(pFile, "%d},", pan[cols() - 1]);
				break; 
			case MAT_Tfloat:
				for (j = 0; j < cols() - 1; j++)
					fprintf(pFile, "%.6f,", pan_double[j]);
				fprintf(pFile, "%.6f},", pan_double[cols() - 1]);
				break;
			case MAT_Tdouble:
				for (j = 0; j < cols() - 1; j++)
					fprintf(pFile, "%.9f,", pan_double[j]);
				fprintf(pFile, "%.9f},", pan_double[cols() - 1]);
				break;
			default:
				assert(false);
			}

		}
		fprintf(pFile, "};\n");
		fclose(pFile);
		delete[]pan;
		delete[]pan_double;
	}
	String Mat::toString() const
	{
		String strOut = "";

		String strTmp;
		int i, k;
		switch (type())
		{
		case MAT_Tbyte:
			for (i = 0; i<rows(); i++)
			{
				for (k = 0; k<cols(); k++)
				{
					char temp[20];
					sprintf(temp, "%d", data.ptr[i][k]);
					strOut += temp;
					size_t len = strlen(temp);
					if (len == 0)
						strOut += "    ";
					else if (len == 1)
						strOut += "   ";
					else if (len == 2)
						strOut += "  ";
					else
						strOut += " ";
				}
				strOut += "\r\n";
			}
			break;
		case MAT_Tshort:
			for (i = 0; i<rows(); i++)
			{
				for (k = 0; k<cols(); k++)
				{
					char temp[20];
					sprintf(temp, "%d ", data.s[i][k]);
					strOut += temp;
				}
				strOut += "\r\n";
			}
			break;
		case MAT_Tint:
			for (i = 0; i<rows(); i++)
			{
				for (k = 0; k<cols(); k++)
				{
					char temp[20];
					sprintf(temp, "%d ", data.i[i][k]);
					strOut += temp;
				}
				strOut += "\r\n";
			}
			break;
		case MAT_Tfloat:
			for (i = 0; i<rows(); i++)
			{
				for (k = 0; k<cols(); k++)
				{
					char temp[20];
					sprintf(temp, "%.6f ", data.fl[i][k]);
					strOut += temp;
				}
				strOut += "\r\n";
			}
			break;
		case MAT_Tdouble:
			for (i = 0; i<rows(); i++)
			{
				for (k = 0; k<cols(); k++)
				{
					char temp[20];
					sprintf(temp, "%.6f ", data.db[i][k]);
					strOut += temp;
				}
				strOut += "\r\n";
			}
			break;
		default:
			assert(false);
		}

		return strOut;
	}

	/////////////////////////////////// Mat Operations ///////////////////////////////////
	Mat Mat::inverted(void) const
	{
		assert(type() == MAT_Tfloat || type() == MAT_Tdouble);
		assert(rows() == cols());
		int nDim = rows();
		Mat mInverted;
		if (type() == MAT_Tdouble)
		{
			LUDecomposition lu(this);
			if (lu.isNonSingular())
			{
				Mat mB(nDim, nDim, MAT_Tdouble);
				mB.identity();
				Mat* pmI = lu.solve(&mB);
				mInverted = *pmI;
				delete pmI;
			}
			else
			{
				cvutil::pseudoInvert(*this, mInverted);
				mInverted.convert(MAT_Tdouble);
			}
		}
		else
		{
			Mat mtemp;
			this->convertTo(mtemp, MAT_Tdouble);
			LUDecomposition lu(&mtemp);
			if (lu.isNonSingular())
			{
				Mat mB(nDim, nDim, MAT_Tdouble);
				mB.identity();
				Mat* pmI = lu.solve(&mB);
				pmI->convertTo(mInverted, MAT_Tfloat);
				delete pmI;
			}
			else
			{
				cvutil::pseudoInvert(*this, mInverted);
				mInverted.convert(MAT_Tfloat);
			}
		}

		return mInverted;
	}
	Mat& Mat::invert(void)
	{
		assert(type() == MAT_Tfloat || type() == MAT_Tdouble);
		*this = inverted();
		return *this;
	}
	void Mat::diag(const Vec& vec)
	{
		int stop = MIN3(m_rows, m_cols, vec.length());

		int i;
		zero();
		switch (type())
		{
		case MAT_Tchar:
		case MAT_Tbyte:
			for (i = 0; i < stop; i++)
				data.ptr[i][i] = vec.data.ptr[i];
			break;
		case MAT_Tshort:
			for (i = 0; i < stop; i++)
				data.s[i][i] = vec.data.s[i];
			break;
		case MAT_Tint:
			for (i = 0; i < stop; i++)
				data.i[i][i] = vec.data.i[i];
			break;
		case MAT_Tfloat:
			for (i = 0; i < stop; i++)
				data.fl[i][i] = vec.data.fl[i];
			break;
		case MAT_Tdouble:
			for (i = 0; i < stop; i++)
				data.db[i][i] = vec.data.db[i];
			break;
		default:
			return;
		}
	}

	void Mat::diag(double rVal)
	{
		int i, iEnd;

		iEnd = MIN(m_rows, m_cols);
		switch (type())
		{
		case MAT_Tbyte:
			for (i = 0; i < iEnd; i++)
				data.ptr[i][i] = (unsigned char)rVal;
			break;
		case MAT_Tshort:
			for (i = 0; i < iEnd; i++)
				data.s[i][i] = (short)rVal;
			break;
		case MAT_Tint:
			for (i = 0; i < iEnd; i++)
				data.i[i][i] = (int)rVal;
			break;
		case MAT_Tfloat:
			for (i = 0; i < iEnd; i++)
				data.fl[i][i] = (float)rVal;
			break;
		case MAT_Tdouble:
			for (i = 0; i < iEnd; i++)
				data.db[i][i] = (double)rVal;
			break;
		default:
			assert(false);
		}
	}

	static void itranspose(Mat* pA, const Mat* pB)
	{
		assert((pA->rows() == pB->cols()) && (pA->cols() == pB->rows()));
		assert(pA->type1() == pB->type1());

		int i, j, r = pB->cols(), c = pB->rows();
		int cn = pB->channels();
		if (cn == 1)
		{
			switch (pA->type())
			{
			case MAT_Tbyte:
			{
				uchar ** ppbA = pA->data.ptr;
				uchar ** ppbB = pB->data.ptr;
				for (i = 0; i < r; i++)
					for (j = 0; j < c; j++)
						ppbA[i][j] = ppbB[j][i];
			}
			break;
			case MAT_Tshort:
			{
				short ** ppsA = pA->data.s;
				short ** ppsB = pB->data.s;
				for (i = 0; i < r; i++)
					for (j = 0; j < c; j++)
						ppsA[i][j] = ppsB[j][i];
			}
			break;
			case MAT_Tint:
			{
				int ** ppnA = pA->data.i;
				int ** ppnB = pB->data.i;
				for (i = 0; i < r; i++)
					for (j = 0; j < c; j++)
						ppnA[i][j] = ppnB[j][i];
			}
			break;
			case MAT_Tfloat:
			{
				float ** pprA = pA->data.fl;
				float ** pprB = pB->data.fl;
				for (i = 0; i < r; i++)
					for (j = 0; j < c; j++)
						pprA[i][j] = pprB[j][i];
			}
			break;
			case MAT_Tdouble:
			{
				double ** ppdA = pA->data.db;
				double ** ppdB = pB->data.db;
				for (i = 0; i < r; i++)
					for (j = 0; j < c; j++)
						ppdA[i][j] = ppdB[j][i];
			}
			break;
			default:
				assert(false);
			}
		}
		else
		{
			switch (pA->type())
			{
			case MAT_Tbyte:
			{
				uchar ** ppbA = pA->data.ptr;
				uchar ** ppbB = pB->data.ptr;
				for (i = 0; i < r; i++)
				{
					for (j = 0; j < c; j++)
					{
						for (int k = 0; k < cn; k++)
							ppbA[i][j*cn + k] = ppbB[j][i*cn + k];
					}
				}
			}
			break;
			case MAT_Tshort:
			{
				short ** ppsA = pA->data.s;
				short ** ppsB = pB->data.s;
				for (i = 0; i < r; i++)
				{
					for (j = 0; j < c; j++)
					{
						for (int k = 0; k < cn; k++)
							ppsA[i][j*cn + k] = ppsB[j][i*cn + k];
					}
				}
			}
			break;
			case MAT_Tint:
			{
				int ** ppnA = pA->data.i;
				int ** ppnB = pB->data.i;
				for (i = 0; i < r; i++)
					for (j = 0; j < c; j++)
						ppnA[i][j] = ppnB[j][i];
			}
			break;
			case MAT_Tfloat:
			{
				float ** pprA = pA->data.fl;
				float ** pprB = pB->data.fl;
				for (i = 0; i < r; i++)
				{
					for (j = 0; j < c; j++)
					{
						for (int k = 0; k < cn; k++)
							pprA[i][j*cn + k] = pprB[j][i*cn + k];
					}
				}
			}
			break;
			case MAT_Tdouble:
			{
				double ** ppdA = pA->data.db;
				double ** ppdB = pB->data.db;
				for (i = 0; i < r; i++)
				{
					for (j = 0; j < c; j++)
					{
						for (int k = 0; k < cn; k++)
							ppdA[i][j*cn + k] = ppdB[j][i*cn + k];
					}
				}
			}
			break;
			default:
				assert(false);
			}
		}
	}

	static void itranspose(Mat* pA)
	{
		if (pA->rows() != pA->cols())
		{
			Mat temp(*pA); pA->release();
			pA->create(temp.cols(), temp.rows(), temp.type1());
			itranspose(pA, &temp);
			return;
		}
		int i, j, r = pA->rows(), c = pA->cols();

		switch (pA->type())
		{
		case MAT_Tbyte:
		{
			uchar bTemp;
			uchar ** ppbA = pA->data.ptr;
			for (i = 0; i < r; i++)
				for (j = i + 1; j < c; j++)
				{
					bTemp = ppbA[i][j];
					ppbA[i][j] = ppbA[j][i];
					ppbA[j][i] = bTemp;
				}
		}
		break;
		case MAT_Tshort:
		{
			short bTemp;
			short ** ppbA = pA->data.s;
			for (i = 0; i < r; i++)
				for (j = i + 1; j < c; j++)
				{
					bTemp = ppbA[i][j];
					ppbA[i][j] = ppbA[j][i];
					ppbA[j][i] = bTemp;
				}
		}
		break;
		case MAT_Tint:
		{
			int bTemp;
			int** ppbA = pA->data.i;
			for (i = 0; i < r; i++)
				for (j = i + 1; j < c; j++)
				{
					bTemp = ppbA[i][j];
					ppbA[i][j] = ppbA[j][i];
					ppbA[j][i] = bTemp;
				}
		}
		break;
		case MAT_Tfloat:
		{
			float bTemp;
			float** ppbA = pA->data.fl;
			for (i = 0; i < r; i++)
				for (j = i + 1; j < c; j++)
				{
					bTemp = ppbA[i][j];
					ppbA[i][j] = ppbA[j][i];
					ppbA[j][i] = bTemp;
				}
		}
		break;
		case MAT_Tdouble:
		{
			double bTemp;
			double** ppbA = pA->data.db;
			for (i = 0; i < r; i++)
				for (j = i + 1; j < c; j++)
				{
					bTemp = ppbA[i][j];
					ppbA[i][j] = ppbA[j][i];
					ppbA[j][i] = bTemp;
				}
		}
		break;
		default:
			assert(false);
		}
	}
	Mat Mat::transposed(void) const
	{
		Mat mRet;
		mRet.create(m_cols, m_rows, type1());
		itranspose(&mRet, this);
		return mRet;
	}
	Mat& Mat::transpose(void)
	{
		itranspose(this);
		return *this;
	}
	bool Mat::isSymmetric(void) const
	{
		if (m_rows != m_cols)
			return false;
		int i, j;
		switch (type())
		{
		case MAT_Tchar:
		case MAT_Tbyte:
			for (i = 0; i < m_rows; i++)
				for (j = i + 1; j < m_cols; j++)
					if (data.ptr[i][j] != data.ptr[j][i])
						return false;
			break;
		case MAT_Tshort:
			for (i = 0; i < m_rows; i++)
				for (j = i + 1; j < m_cols; j++)
					if (data.s[i][j] != data.s[j][i])
						return false;
			break;
		case MAT_Tint:
			for (i = 0; i < m_rows; i++)
				for (j = i + 1; j < m_cols; j++)
					if (data.i[i][j] != data.i[j][i])
						return false;
			break;
		case MAT_Tfloat:
			for (i = 0; i < m_rows; i++)
				for (j = i + 1; j < m_cols; j++)
					if (data.fl[i][j] != data.fl[j][i])
						return false;
			break;
		case MAT_Tdouble:
			for (i = 0; i < m_rows; i++)
				for (j = i + 1; j < m_cols; j++)
					if (data.db[i][j] != data.db[j][i])
						return false;
			break;
		default:
			assert(false);
		}
		return true;
	}

	Scalar Mat::sum() const
	{
		Scalar s;
		int i, j, ch;
		int  cn = channels();
		if (isContinuous())
		{
			int totalsize = m_rows*m_cols*cn;
			switch (type())
			{
			case MAT_Tchar:
			case MAT_Tbyte:
				for (ch = 0; ch < cn; ch++)
				{
					double dSum = 0;
					for (i = ch; i < totalsize; i += cn)
						dSum += (double)data.ptr[0][i];
					s.vec_array[ch] = dSum;
				}
				break;
			case MAT_Tshort:
				for (ch = 0; ch < cn; ch++)
				{
					double dSum = 0;
					for (i = ch; i < totalsize; i += cn)
						dSum += (double)data.s[0][i];
					s.vec_array[ch] = dSum;
				}
				break;
			case MAT_Tint:
				for (ch = 0; ch < cn; ch++)
				{
					double dSum = 0;
					for (i = ch; i < totalsize; i += cn)
						dSum += (double)data.i[0][i];
					s.vec_array[ch] = dSum;
				}
				break;
			case MAT_Tfloat:
				for (ch = 0; ch < cn; ch++)
				{
					double dSum = 0;
					for (i = ch; i < totalsize; i += cn)
						dSum += (double)data.fl[0][i];
					s.vec_array[ch] = dSum;
				}
				break;
			case MAT_Tdouble:
				for (ch = 0; ch < cn; ch++)
				{
					double dSum = 0;
					for (i = ch; i < totalsize; i += cn)
						dSum += (double)data.db[0][i];
					s.vec_array[ch] = dSum;
				}
				break;
			default:
				assert(false);
			}
		}
		else
		{
			int totalsize = m_cols*cn;
			switch (type())
			{
			case MAT_Tbyte:
				for (ch = 0; ch < cn; ch++)
				{
					double dSum = 0;
					for (i = 0; i < m_rows; i++)
					{
						for (j = ch; j < totalsize; j += cn)
							dSum += (double)data.ptr[i][j];
					}
					s.vec_array[ch] = dSum;
				}
				break;
			case MAT_Tshort:
				for (ch = 0; ch < cn; ch++)
				{
					double dSum = 0;
					for (i = 0; i < m_rows; i++)
					{
						for (j = ch; j < totalsize; j += cn)
							dSum += (double)data.s[i][j];
					}
					s.vec_array[ch] = dSum;
				}
				break;
			case MAT_Tint:
				for (ch = 0; ch < cn; ch++)
				{
					double dSum = 0;
					for (i = 0; i < m_rows; i++)
					{
						for (j = ch; j < totalsize; j += cn)
							dSum += (double)data.i[i][j];
					}
					s.vec_array[ch] = dSum;
				}
				break;
			case MAT_Tfloat:
				for (ch = 0; ch < cn; ch++)
				{
					double dSum = 0;
					for (i = 0; i < m_rows; i++)
					{
						for (j = ch; j < totalsize; j += cn)
							dSum += (double)data.fl[i][j];
					}
					s.vec_array[ch] = dSum;
				}
				break;
			case MAT_Tdouble:
				for (ch = 0; ch < cn; ch++)
				{
					double dSum = 0;
					for (i = 0; i < m_rows; i++)
					{
						for (j = ch; j < totalsize; j += cn)
							dSum += (double)data.db[i][j];
					}
					s.vec_array[ch] = dSum;
				}
				break;
			default:
				assert(false);
			}
		}
		return s;
	}

	double Mat::var() const
	{
		int i, j;
		double dVar = 0, dMean = mean();
		int elems = rows()*cols();
		if (isContinuous())
		{
			switch (type())
			{
			case MAT_Tbyte:
				for (i = 0; i < elems; i++)
					dVar += SQR((double)data.ptr[0][i] - dMean);
				break;
			case MAT_Tshort:
				for (i = 0; i < elems; i++)
					dVar += SQR((double)data.s[0][i] - dMean);
				break;
			case MAT_Tint:
				for (i = 0; i < elems; i++)
					dVar += SQR((double)data.i[0][i] - dMean);
				break;
			case MAT_Tfloat:
				for (i = 0; i < elems; i++)
					dVar += SQR((double)data.fl[0][i] - dMean);
				break;
			case MAT_Tdouble:
				for (i = 0; i < elems; i++)
					dVar += SQR((double)data.db[0][i] - dMean);
				break;
			default:
				assert(false);
			}
		}
		else
		{
			switch (type())
			{
			case MAT_Tbyte:
				for (i = 0; i < rows(); i++) for (j = 0; j < cols(); j++)
					dVar += SQR((double)data.ptr[i][j] - dMean);
				break;
			case MAT_Tshort:
				for (i = 0; i < rows(); i++) for (j = 0; j < cols(); j++)
					dVar += SQR((double)data.ptr[i][j] - dMean);
				break;
			case MAT_Tint:
				for (i = 0; i < rows(); i++) for (j = 0; j < cols(); j++)
					dVar += SQR((double)data.ptr[i][j] - dMean);
				break;
			case MAT_Tfloat:
				for (i = 0; i < rows(); i++) for (j = 0; j < cols(); j++)
					dVar += SQR((double)data.ptr[i][j] - dMean);
				break;
			case MAT_Tdouble:
				for (i = 0; i < rows(); i++) for (j = 0; j < cols(); j++)
					dVar += SQR((double)data.ptr[i][j] - dMean);
				break;
			default:
				assert(false);
			}
		}
		if (elems != 0)
			dVar /= elems;
		return dVar;
	}


// math functions
	void Mat::sqr()
	{
		int j;
		int elems = rows()*cols();
		switch (type())
		{
		case MAT_Tbyte:
			for (j = 0; j < elems; j++)
				data.ptr[0][j] = (uchar)SQR(data.ptr[0][j]);
			break;
		case MAT_Tshort:
			for (j = 0; j < elems; j++)
				data.s[0][j] = (short)SQR(data.s[0][j]);
			break;
		case MAT_Tint:
			for (j = 0; j < elems; j++)
				data.i[0][j] = (int)SQR(data.i[0][j]);
			break;
		case MAT_Tfloat:
			for (j = 0; j < elems; j++)
				data.fl[0][j] = (float)SQR(data.fl[0][j]);
			break;
		case MAT_Tdouble:
			for (j = 0; j < elems; j++)
				data.db[0][j] = (double)SQR(data.db[0][j]);
			break;
		default:
			assert(false);
		}
	}

	void Mat::sqrt()
	{
		int j;
		int elems = rows()*cols();
		switch (type())
		{
		case MAT_Tbyte:
			for (j = 0; j < elems; j++)
				data.ptr[0][j] = (uchar)sqrtf(data.ptr[0][j]);
			break;
		case MAT_Tshort:
			for (j = 0; j < elems; j++)
				data.s[0][j] = (short)sqrtf(data.s[0][j]);
			break;
		case MAT_Tint:
			for (j = 0; j < elems; j++)
				data.i[0][j] = (int)sqrtf((float)data.i[0][j]);
			break;
		case MAT_Tfloat:
			for (j = 0; j < elems; j++)
				data.fl[0][j] = (float)sqrtf(data.fl[0][j]);
			break;
		case MAT_Tdouble:
			for (j = 0; j < elems; j++)
				data.db[0][j] = (double)std::sqrt(data.db[0][j]);
			break;
		default:
			assert(false);
		}
	}

	void Mat::log()
	{
		int j;
		int elems = rows()*cols();
		switch (type())
		{
		case MAT_Tbyte:
			for (j = 0; j < elems; j++)
				data.ptr[0][j] = (uchar)logf(data.ptr[0][j]);
			break;
		case MAT_Tshort:
			for (j = 0; j < elems; j++)
				data.s[0][j] = (short)logf(data.s[0][j]);
			break;
		case MAT_Tint:
			for (j = 0; j < elems; j++)
				data.i[0][j] = (int)logf((float)data.i[0][j]);
			break;
		case MAT_Tfloat:
			for (j = 0; j < elems; j++)
				data.fl[0][j] = (float)logf(data.fl[0][j]);
			break;
		case MAT_Tdouble:
			for (j = 0; j < elems; j++)
				data.db[0][j] = (double)std::log(data.db[0][j]);
			break;
		default:
			assert(false);
		}
	}
	void Mat::abs()
	{
		int j;
		int elemcount = rows()*cols()*channels();

		switch (type())
		{
		case MAT_Tbyte:
		{
			uchar* pndata = data.ptr[0];
			for (j = 0; j < elemcount; j++)
				pndata[j] = (uchar)ABS(pndata[j]);
		}
		break;
		case MAT_Tshort:
		{
			short* ps = data.s[0];
			for (j = 0; j < elemcount; j++)
				ps[j] = (short)ABS(ps[j]);
		}
		break;
		case MAT_Tint:
		{
			int* pn = data.i[0];
			for (j = 0; j < elemcount; j++)
				pn[j] = (int)ABS(pn[j]);
		}
		break;
		case MAT_Tfloat:
		{
			float* pr = data.fl[0];
			for (j = 0; j < elemcount; j++)
				pr[j] = (float)fabsf(pr[j]);
		}
		break;
		case MAT_Tdouble:
		{
			double* pr = data.db[0];
			for (j = 0; j < elemcount; j++)
				pr[j] = (double)fabs(pr[j]);
		}
		break;
		default:
			assert(false);
		}
	}

	template<typename _Tp> static void flipLR_(_Tp** ptr, int m_rows, int m_cols, int cn)
	{
		int i, k;
		int cols_2 = m_cols / 2;
		_Tp temp;
		if (cn == 1)
		{
			for (i = 0; i < m_rows; i++)
			{
				_Tp* pdata = ptr[i];
				for (k = 0; k < cols_2; k++)
				{
					int t1 = m_cols - k - 1;
					temp = pdata[k]; pdata[k] = pdata[t1]; pdata[t1] = temp;
				}
			}
		}
		else
		{
			for (i = 0; i < m_rows; i++)
			{
				_Tp* pdata = ptr[i];
				int t = 0, t1 = (m_cols - 1)*cn;
				if (cn == 2) {
					for (k = 0; k < cols_2; k++, t += cn, t1 -= cn)
					{
						temp = pdata[t]; pdata[t] = pdata[t1]; pdata[t1] = temp;
						temp = pdata[t + 1]; pdata[t + 1] = pdata[t1 + 1]; pdata[t1 + 1] = temp;
					}
				}
				else if (cn == 3) {
					for (k = 0; k < cols_2; k++, t += cn, t1 -= cn)
					{
						temp = pdata[t]; pdata[t] = pdata[t1]; pdata[t1] = temp;
						temp = pdata[t + 1]; pdata[t + 1] = pdata[t1 + 1]; pdata[t1 + 1] = temp;
						temp = pdata[t + 2]; pdata[t + 2] = pdata[t1 + 2]; pdata[t1 + 2] = temp;
					}
				}
				else {
					for (k = 0; k < cols_2; k++, t += cn, t1 -= cn)
					{
						temp = pdata[t]; pdata[t] = pdata[t1]; pdata[t1] = temp;
						temp = pdata[t + 1]; pdata[t + 1] = pdata[t1 + 1]; pdata[t1 + 1] = temp;
						temp = pdata[t + 2]; pdata[t + 2] = pdata[t1 + 2]; pdata[t1 + 2] = temp;
						temp = pdata[t + 3]; pdata[t + 3] = pdata[t1 + 3]; pdata[t1 + 3] = temp;
					}
				}
			}
		}
	}

// Flips
	void Mat::flipLR()
	{
		int cn = channels();
		switch (type())
		{
		case MAT_Tbyte:
			flipLR_<uchar>(data.ptr, m_rows, m_cols, cn);
			break;
		case MAT_Tshort:
			flipLR_<short>(data.s, m_rows, m_cols, cn);
			break;
		case MAT_Tint:
			flipLR_<int>(data.i, m_rows, m_cols, cn);
			break;
		case MAT_Tfloat:
			flipLR_<float>(data.fl, m_rows, m_cols, cn);
			break;
		case MAT_Tdouble:
			flipLR_<double>(data.db, m_rows, m_cols, cn);
			break;
		default:
			assert(false);
		}
	}

	void Mat::flipUD()
	{
		int nlinestep = step()*cols()*channels();
		char* buffer = new char[nlinestep];
		for (int i = 0; i < rows() / 2; i++)
		{
			memcpy(buffer, data.ptr[rows() - i - 1], nlinestep);
			memcpy(data.ptr[rows() - i - 1], data.ptr[i], nlinestep);
			memcpy(data.ptr[i], buffer, nlinestep);
		}
		delete[]buffer;
	}

// Conversions and manipulations
	void Mat::toRefVec(Vec &v) const
	{
		v.create(data.ptr[0], m_rows*m_cols*channels(), type());
	}

	void Mat::toVec(Vec& v) const
	{
		if (!v.isValid())
			v.create(m_rows*m_cols*channels(), type());
		memcpy(v.data.ptr, data.ptr[0], m_step*m_rows*m_cols*channels());
	}

	template<typename _Tp> static void CompareEq(int w, int h, _Tp** A, const double B, _Tp** C)
	{
		for (int i = 0; i < h; i++) for (int j = 0; j < w; j++)
			C[i][j] = (_Tp)(uchar)(-(A[i][j] == B));
	}
// Elementwise equal to
	void Mat::eq(const double B, Mat& C) const
	{
		if (!isEqualMat(C))
		{
			C.release();
			C.create(rows(), cols(), m_type);
		}
		switch (type())
		{
		case MAT_Tchar:
		case MAT_Tbyte:
			CompareEq<uchar>(cols(), rows(), this->data.ptr, B, C.data.ptr); break;
		case MAT_Tshort:
			CompareEq<short>(cols(), rows(), this->data.s, B, C.data.s); break;
		case MAT_Tint:
			CompareEq<int>(cols(), rows(), this->data.i, B, C.data.i); break;
		case MAT_Tfloat:
			CompareEq<float>(cols(), rows(), this->data.fl, B, C.data.fl); break;
		case MAT_Tdouble:
			CompareEq<double>(cols(), rows(), this->data.db, B, C.data.db); break;
		default:
			assert(false);
		}
	}
	template<typename _Tp> static void CompareEq(int w, int h, _Tp** A, _Tp** B, _Tp** C)
	{
		for (int i = 0; i < h; i++) for (int j = 0; j < w; j++)
			C[i][j] = (_Tp)(uchar)(-(A[i][j] == B[i][j]));
	}
	void Mat::eq(const Mat& B, Mat& C) const
	{
		if (!isEqualMat(C))
		{
			C.release();
			C.create(rows(), cols(), m_type);
		}
		switch (type())
		{
		case MAT_Tchar:
		case MAT_Tbyte:
			CompareEq<uchar>(cols(), rows(), this->data.ptr, B.data.ptr, C.data.ptr); break;
		case MAT_Tshort:
			CompareEq<short>(cols(), rows(), this->data.s, B.data.s, C.data.s); break;
		case MAT_Tint:
			CompareEq<int>(cols(), rows(), this->data.i, B.data.i, C.data.i); break;
		case MAT_Tfloat:
			CompareEq<float>(cols(), rows(), this->data.fl, B.data.fl, C.data.fl); break;
		case MAT_Tdouble:
			CompareEq<double>(cols(), rows(), this->data.db, B.data.db, C.data.db); break;
		default:
			return;
		}
	}
	template<typename _Tp> static void CompareNe(int w, int h, _Tp** A, const double B, _Tp** C)
	{
		for (int i = 0; i < h; i++) for (int j = 0; j < w; j++)
			C[i][j] = (_Tp)(uchar)(-(A[i][j] != B));
	}
	void Mat::ne(const double B, Mat& C) const
	{
		if (!isEqualMat(C))
		{
			C.release();
			C.create(rows(), cols(), m_type);
		}
		switch (type())
		{
		case MAT_Tchar:
		case MAT_Tbyte:
			CompareNe<uchar>(cols(), rows(), this->data.ptr, B, C.data.ptr); break;
		case MAT_Tshort:
			CompareNe<short>(cols(), rows(), this->data.s, B, C.data.s); break;
		case MAT_Tint:
			CompareNe<int>(cols(), rows(), this->data.i, B, C.data.i); break;
		case MAT_Tfloat:
			CompareNe<float>(cols(), rows(), this->data.fl, B, C.data.fl); break;
		case MAT_Tdouble:
			CompareNe<double>(cols(), rows(), this->data.db, B, C.data.db); break;
		default:
			return;
		}
	}
	template<typename _Tp> static void CompareNe(int w, int h, _Tp** A, _Tp** B, _Tp** C)
	{
		for (int i = 0; i < h; i++) for (int j = 0; j < w; j++)
			C[i][j] = (_Tp)(uchar)(-(A[i][j] != B[i][j]));
	}
	void Mat::ne(const Mat& B, Mat& C) const
	{
		if (!isEqualMat(C))
		{
			C.release();
			C.create(rows(), cols(), m_type);
		}
		switch (type())
		{
		case MAT_Tchar:
		case MAT_Tbyte:
			CompareNe<uchar>(cols(), rows(), this->data.ptr, B.data.ptr, C.data.ptr); break;
		case MAT_Tshort:
			CompareNe<short>(cols(), rows(), this->data.s, B.data.s, C.data.s); break;
		case MAT_Tint:
			CompareNe<int>(cols(), rows(), this->data.i, B.data.i, C.data.i); break;
		case MAT_Tfloat:
			CompareNe<float>(cols(), rows(), this->data.fl, B.data.fl, C.data.fl); break;
		case MAT_Tdouble:
			CompareNe<double>(cols(), rows(), this->data.db, B.data.db, C.data.db); break;
		default:
			return;
		}
	}
// Elementwise less than
	template<typename _Tp> static void CompareLt(int w, int h, _Tp** A, const double B, _Tp** C)
	{
		for (int i = 0; i < h; i++) for (int j = 0; j < w; j++)
			C[i][j] = (_Tp)(uchar)(-(A[i][j] < B));
	}
	void Mat::lt(const double B, Mat& C) const
	{
		if (!isEqualMat(C))
		{
			C.release();
			C.create(rows(), cols(), m_type);
		}
		switch (type())
		{
		case MAT_Tchar:
		case MAT_Tbyte:
			CompareLt<uchar>(cols(), rows(), this->data.ptr, B, C.data.ptr); break;
		case MAT_Tshort:
			CompareLt<short>(cols(), rows(), this->data.s, B, C.data.s); break;
		case MAT_Tint:
			CompareLt<int>(cols(), rows(), this->data.i, B, C.data.i); break;
		case MAT_Tfloat:
			CompareLt<float>(cols(), rows(), this->data.fl, B, C.data.fl); break;
		case MAT_Tdouble:
			CompareLt<double>(cols(), rows(), this->data.db, B, C.data.db); break;
		default:
			return;
		}
	}
	template<typename _Tp> static void CompareLt(int w, int h, _Tp** A, _Tp** B, _Tp** C)
	{
		for (int i = 0; i < h; i++) for (int j = 0; j < w; j++)
			C[i][j] = (_Tp)(uchar)(-(A[i][j] < B[i][j]));
	}
	void Mat::lt(const Mat& B, Mat& C) const
	{
		if (!isEqualMat(C))
		{
			C.release();
			C.create(rows(), cols(), m_type);
		}
		switch (type())
		{
		case MAT_Tchar:
		case MAT_Tbyte:
			CompareLt<uchar>(cols(), rows(), this->data.ptr, B.data.ptr, C.data.ptr); break;
		case MAT_Tshort:
			CompareLt<short>(cols(), rows(), this->data.s, B.data.s, C.data.s); break;
		case MAT_Tint:
			CompareLt<int>(cols(), rows(), this->data.i, B.data.i, C.data.i); break;
		case MAT_Tfloat:
			CompareLt<float>(cols(), rows(), this->data.fl, B.data.fl, C.data.fl); break;
		case MAT_Tdouble:
			CompareLt<double>(cols(), rows(), this->data.db, B.data.db, C.data.db); break;
		default:
			return;
		}
	}
// Elementwise less than or equal
	template<typename _Tp> static void CompareLe(int w, int h, _Tp** A, const double B, _Tp** C)
	{
		for (int i = 0; i < h; i++) for (int j = 0; j < w; j++)
			C[i][j] = (_Tp)(uchar)(-(A[i][j] <= B));
	}
	void Mat::le(const double B, Mat& C) const
	{
		if (!isEqualMat(C))
		{
			C.release();
			C.create(rows(), cols(), m_type);
		}
		switch (type())
		{
		case MAT_Tchar:
		case MAT_Tbyte:
			CompareLe<uchar>(cols(), rows(), this->data.ptr, B, C.data.ptr); break;
		case MAT_Tshort:
			CompareLe<short>(cols(), rows(), this->data.s, B, C.data.s); break;
		case MAT_Tint:
			CompareLe<int>(cols(), rows(), this->data.i, B, C.data.i); break;
		case MAT_Tfloat:
			CompareLe<float>(cols(), rows(), this->data.fl, B, C.data.fl); break;
		case MAT_Tdouble:
			CompareLe<double>(cols(), rows(), this->data.db, B, C.data.db); break;
		default:
			return;
		}
	}
	template<typename _Tp> static void CompareLe(int w, int h, _Tp** A, _Tp** B, _Tp** C)
	{
		for (int i = 0; i < h; i++) for (int j = 0; j < w; j++)
			C[i][j] = (_Tp)(uchar)(-(A[i][j] <= B[i][j]));
	}
	void Mat::le(const Mat& B, Mat& C) const
	{
		if (!isEqualMat(C))
		{
			C.release();
			C.create(rows(), cols(), m_type);
		}
		switch (type())
		{
		case MAT_Tchar:
		case MAT_Tbyte:
			CompareLe<uchar>(cols(), rows(), this->data.ptr, B.data.ptr, C.data.ptr); break;
		case MAT_Tshort:
			CompareLe<short>(cols(), rows(), this->data.s, B.data.s, C.data.s); break;
		case MAT_Tint:
			CompareLe<int>(cols(), rows(), this->data.i, B.data.i, C.data.i); break;
		case MAT_Tfloat:
			CompareLe<float>(cols(), rows(), this->data.fl, B.data.fl, C.data.fl); break;
		case MAT_Tdouble:
			CompareLe<double>(cols(), rows(), this->data.db, B.data.db, C.data.db); break;
		default:
			return;
		}
	}
// Elementwise greater than
	template<typename _Tp> static void CompareGt(int w, int h, _Tp** A, const double B, _Tp** C)
	{
		for (int i = 0; i < h; i++) for (int j = 0; j < w; j++)
			C[i][j] = (_Tp)(uchar)(-(A[i][j] > B));
	}
	void Mat::gt(const double B, Mat& C) const
	{
		if (!isEqualMat(C))
		{
			C.release();
			C.create(rows(), cols(), m_type);
		}
		switch (type())
		{
		case MAT_Tbyte:
			CompareGt<uchar>(cols(), rows(), this->data.ptr, B, C.data.ptr); break;
		case MAT_Tshort:
			CompareGt<short>(cols(), rows(), this->data.s, B, C.data.s); break;
		case MAT_Tint:
			CompareGt<int>(cols(), rows(), this->data.i, B, C.data.i); break;
		case MAT_Tfloat:
			CompareGt<float>(cols(), rows(), this->data.fl, B, C.data.fl); break;
		case MAT_Tdouble:
			CompareGt<double>(cols(), rows(), this->data.db, B, C.data.db); break;
		default:
			return;
		}
	}
	template<typename _Tp> static void CompareGt(int w, int h, _Tp** A, _Tp** B, _Tp** C)
	{
		for (int i = 0; i < h; i++) for (int j = 0; j < w; j++)
			C[i][j] = (_Tp)(uchar)(-(A[i][j] > B[i][j]));
	}
	void Mat::gt(const Mat& B, Mat& C) const
	{
		if (!isEqualMat(C))
		{
			C.release();
			C.create(rows(), cols(), m_type);
		}
		switch (type())
		{
		case MAT_Tbyte:
			CompareGt<uchar>(cols(), rows(), this->data.ptr, B.data.ptr, C.data.ptr); break;
		case MAT_Tshort:
			CompareGt<short>(cols(), rows(), this->data.s, B.data.s, C.data.s); break;
		case MAT_Tint:
			CompareGt<int>(cols(), rows(), this->data.i, B.data.i, C.data.i); break;
		case MAT_Tfloat:
			CompareGt<float>(cols(), rows(), this->data.fl, B.data.fl, C.data.fl); break;
		case MAT_Tdouble:
			CompareGt<double>(cols(), rows(), this->data.db, B.data.db, C.data.db); break;
		default:
			return;
		}
	}
// Elementwise greater than or equal
	template<typename _Tp> static void CompareGe(int w, int h, _Tp** A, const double B, _Tp** C)
	{
		for (int i = 0; i < h; i++) for (int j = 0; j < w; j++)
			C[i][j] = (_Tp)(uchar)(-(A[i][j] >= B));
	}
	void Mat::ge(const double B, Mat& C) const
	{
		if (!isEqualMat(C))
		{
			C.release();
			C.create(rows(), cols(), m_type);
		}
		switch (type())
		{
		case MAT_Tbyte:
			CompareGe<uchar>(cols(), rows(), this->data.ptr, B, C.data.ptr); break;
		case MAT_Tshort:
			CompareGe<short>(cols(), rows(), this->data.s, B, C.data.s); break;
		case MAT_Tint:
			CompareGe<int>(cols(), rows(), this->data.i, B, C.data.i); break;
		case MAT_Tfloat:
			CompareGe<float>(cols(), rows(), this->data.fl, B, C.data.fl); break;
		case MAT_Tdouble:
			CompareGe<double>(cols(), rows(), this->data.db, B, C.data.db); break;
		default:
			return;
		}
	}
	template<typename _Tp> static void CompareGe(int w, int h, _Tp** A, _Tp** B, _Tp** C)
	{
		for (int i = 0; i < h; i++) for (int j = 0; j < w; j++)
			C[i][j] = (_Tp)(uchar)(-(A[i][j] >= B[i][j]));
	}
	void Mat::ge(const Mat& B, Mat& C) const
	{
		if (!isEqualMat(C))
		{
			C.release();
			C.create(rows(), cols(), m_type);
		}
		switch (type())
		{
		case MAT_Tbyte:
			CompareGe<uchar>(cols(), rows(), this->data.ptr, B.data.ptr, C.data.ptr); break;
		case MAT_Tshort:
			CompareGe<short>(cols(), rows(), this->data.s, B.data.s, C.data.s); break;
		case MAT_Tint:
			CompareGe<int>(cols(), rows(), this->data.i, B.data.i, C.data.i); break;
		case MAT_Tfloat:
			CompareGe<float>(cols(), rows(), this->data.fl, B.data.fl, C.data.fl); break;
		case MAT_Tdouble:
			CompareGe<double>(cols(), rows(), this->data.db, B.data.db, C.data.db); break;
		default:
			return;
		}
	}

// determinant, trace
	template<typename _Tp> static double itrace_(_Tp** ptr, int len)
	{
		double rsum = 0;
		for (int i = 0; i < len; i++)
			rsum += ptr[i][i];
		return rsum;
	}
	double Mat::trace() const
	{
		double rsum = 0;
		int len = MIN(m_rows, m_cols);
		switch (type())
		{
		case MAT_Tdouble:
			rsum = itrace_<double>(data.db, len);
			break;
		case MAT_Tfloat:
			rsum = itrace_<float>(data.fl, len);
			break;
		case MAT_Tint:
			rsum = itrace_<int>(data.i, len);
			break;
		case MAT_Tshort:
			rsum = itrace_<short>(data.s, len);
			break;
		case MAT_Tbyte:
			rsum = itrace_<uchar>(data.ptr, len);
			break;
		default:
			assert(false);
		}
		return rsum;
	}
	void Mat::swap(Mat& other)
	{
		int ntemp;
		uchar** pbtemp;
		TYPE ntype;
		SWAP(m_step, other.m_step, ntemp);
		SWAP(m_depend, other.m_depend, ntemp);
		SWAP(m_rows, other.m_rows, ntemp);
		SWAP(m_cols, other.m_cols, ntemp);
		SWAP(m_type, other.m_type, ntype);
		SWAP(data.ptr, other.data.ptr, pbtemp);
	}

	template<typename ST, typename DT> static void matConvertFunc_(const Mat& mFrom, Mat& mTo)
	{
		int cn = mFrom.channels();
		int type = CVLIB_MAKETYPE(DataType<DT>::type, cn);
		if (mTo.size() != mFrom.size() || mTo.type1() != type)
		{
			mTo.release();
			mTo.create(mFrom.rows(), mFrom.cols(), (TYPE)type);
		}
		int i;
		if (mFrom.isContinuous() && mTo.isContinuous()) {
			DT* todata = ((DT*)(mTo.data.ptr[0]));
			ST* fromdata = ((ST*)(mFrom.data.ptr[0]));
			int len = mFrom.rows()*mFrom.cols()*cn;
			for (i = 0; i < len; i++)
			{
				todata[i] = (DT)fromdata[i];
			}
		}
		else {
			for (int irow = 0; irow < mFrom.rows(); irow++)
			{
				DT* todata = ((DT*)(mTo.data.ptr[irow]));
				ST* fromdata = ((ST*)(mFrom.data.ptr[irow]));
				int len = mFrom.cols()*cn;
				for (i = 0; i < len; i++)
				{
					todata[i] = (DT)fromdata[i];
				}
			}
		}
	}

	template<typename ST> static void matconvertfunc_byte_(const Mat& mFrom, Mat& mTo)
	{
		double rMin = mFrom.min();
		double rMax = mFrom.max();
		int i;
		mTo.create(mFrom.rows(), mFrom.cols(), MAT_Tbyte);

		uchar* todata = mTo.data.ptr[0];
		const ST* fromdata = ((const ST*)(mFrom.data.ptr[0]));
		float mul = 255.0f / (float)(rMax - rMin);
		for (i = 0; i < mFrom.rows()*mFrom.cols(); i++)
			todata[i] = (uchar)((fromdata[i] - rMin) *mul);
	}

	typedef void(*matconvertfunc)(const Mat&, Mat&);
	static void MatConvert(const Mat& src, Mat& dst, TYPE type, CASTTYPE casttype)
	{
		if (src.type() == type)
		{
			dst = src;
			return;
		}
		matconvertfunc expandFuncs[] = { NULL, NULL, matconvertfunc_byte_<short>, matconvertfunc_byte_<int>, matconvertfunc_byte_<float>, matconvertfunc_byte_<double> };
		matconvertfunc castFuncs[6][6] =
		{
			{NULL, NULL, NULL, NULL, NULL, NULL},
			{NULL, NULL, matConvertFunc_<uchar, short>, matConvertFunc_<uchar, int>, matConvertFunc_<uchar, float>, matConvertFunc_<uchar, double>},
			{NULL, matConvertFunc_<short, uchar>, NULL, matConvertFunc_<short, int>, matConvertFunc_<short, float>, matConvertFunc_<short, double>},
			{NULL, matConvertFunc_<int, uchar>, matConvertFunc_<int, short>, NULL, matConvertFunc_<int, float>, matConvertFunc_<int, double>},
			{NULL, matConvertFunc_<float, uchar>, matConvertFunc_<float, short>, matConvertFunc_<float, int>, NULL, matConvertFunc_<float, double>},
			{NULL, matConvertFunc_<double, uchar>, matConvertFunc_<double, short>, matConvertFunc_<double, int>, matConvertFunc_<double, float>, NULL},
		};

		if (casttype == CT_Default)
		{
			if (type == MAT_Tbyte)
			{
				expandFuncs[CVLIB_MAT_DEPTH(src.type())](src, dst);
			}
			else
			{
				castFuncs[CVLIB_MAT_DEPTH(src.type())][CVLIB_MAT_DEPTH(type)](src, dst);
			}
		}
		else if (casttype == CT_Cast)
		{
			castFuncs[CVLIB_MAT_DEPTH(src.type())][CVLIB_MAT_DEPTH(type)](src, dst);
		}
	}
	void Mat::convert(TYPE type_, CASTTYPE casttype/*=CT_Default*/)
	{
		if (!this->isValid())
			return;
		if (type() == type_)
			return;
		matconvertfunc expandFuncs[] = { NULL, NULL, matconvertfunc_byte_<short>, matconvertfunc_byte_<int>, matconvertfunc_byte_<float>, matconvertfunc_byte_<double> };
		matconvertfunc castFuncs[6][6] =
		{
			{NULL, NULL, NULL, NULL, NULL, NULL},
			{NULL, NULL, matConvertFunc_<uchar, short>, matConvertFunc_<uchar, int>, matConvertFunc_<uchar, float>, matConvertFunc_<uchar, double>},
			{NULL, matConvertFunc_<short, uchar>, NULL, matConvertFunc_<short, int>, matConvertFunc_<short, float>, matConvertFunc_<short, double>},
			{NULL, matConvertFunc_<int, uchar>, matConvertFunc_<int, short>, NULL, matConvertFunc_<int, float>, matConvertFunc_<int, double>},
			{NULL, matConvertFunc_<float, uchar>, matConvertFunc_<float, short>, matConvertFunc_<float, int>, NULL, matConvertFunc_<float, double>},
			{NULL, matConvertFunc_<double, uchar>, matConvertFunc_<double, short>, matConvertFunc_<double, int>, matConvertFunc_<double, float>, NULL},
		};

		Mat mClone(*this);
		this->release();
		if (casttype == CT_Default)
		{
			if (type_ == MAT_Tbyte)
			{
				expandFuncs[CVLIB_MAT_DEPTH(mClone.type())](mClone, *this);
			}
			else
			{
				castFuncs[CVLIB_MAT_DEPTH(mClone.type())][CVLIB_MAT_DEPTH(type_)](mClone, *this);
			}
		}
		else if (casttype == CT_Cast)
		{
			castFuncs[CVLIB_MAT_DEPTH(mClone.type())][CVLIB_MAT_DEPTH(type_)](mClone, *this);
		}
	}

	typedef void(*BinaryFunc)(const uchar** src, uchar** dst, Size size, double scale, double shift);

	template<typename _Tp, typename DT, typename WT> static void cvtScale_(const uchar** _src, uchar** _dst, Size size, WT scale, WT shift)
	{
		const _Tp** src = (const _Tp**)_src;
		DT** dst = (DT**)_dst;
		for (int y = 0; y < size.height; y++)
		{
			const _Tp* s = src[y];
			DT* d = dst[y];
			int x = 0;
			// #if CVLIB_ENABLE_UNROLLED
			for (; x <= size.width - 4; x += 4)
			{
				DT t0, t1;
				t0 = (DT)(s[x] * scale + shift);
				t1 = (DT)(s[x + 1] * scale + shift);
				d[x] = t0; d[x + 1] = t1;
				t0 = (DT)(s[x + 2] * scale + shift);
				t1 = (DT)(s[x + 3] * scale + shift);
				d[x + 2] = t0; d[x + 3] = t1;
			}
			// #endif

			for (; x < size.width; x++)
				d[x] = (DT)(s[x] * scale + shift);
		}
	}
	void Mat::convert(TYPE _type, double alpha, double beta)
	{
		bool noScale = fabs(alpha - 1) < DBL_EPSILON && fabs(beta) < DBL_EPSILON;

		if (_type < 0)
			_type = type1();

		int typeto = CVLIB_MAT_DEPTH(_type);
		if (noScale && typeto == type())
			return;

		if (noScale)
		{
			this->convert(_type, CT_Cast);
			return;
		}

		Mat src = *this;

#define CoefT double
		BinaryFunc cvtFuncs[6][6] =
		{
			{NULL, NULL, NULL, NULL, NULL, NULL},
			{NULL, cvtScale_<uchar,uchar,CoefT>, cvtScale_<uchar,short,CoefT>, cvtScale_<uchar,int,CoefT>, cvtScale_<uchar,float,CoefT>, cvtScale_<uchar,double,CoefT>},
			{NULL, cvtScale_<short,uchar,CoefT>, cvtScale_<short,short,CoefT>, cvtScale_<short,int,double>, cvtScale_<short,float,CoefT>, cvtScale_<short,double,CoefT>},
			{NULL, cvtScale_<int,uchar,CoefT>, cvtScale_<int,short,CoefT>, cvtScale_<int,int,CoefT>, cvtScale_<int,float,CoefT>, cvtScale_<int,double,CoefT>},
			{NULL, cvtScale_<float,uchar,CoefT>, cvtScale_<float,short,CoefT>, cvtScale_<float,int,CoefT>, cvtScale_<float,float,CoefT>, cvtScale_<float,double,CoefT>},
			{NULL, cvtScale_<double,uchar,CoefT>, cvtScale_<double,short,CoefT>, cvtScale_<double,int,CoefT>, cvtScale_<double,float,CoefT>, cvtScale_<double,double,CoefT>},
		};

		BinaryFunc func = cvtFuncs[(int)src.type()][typeto];
		this->release();
		this->create(src.size(), _type);
		Size size = src.size();
		size.width *= channels();
		func((const uchar**)src.data.ptr, (uchar**)this->data.ptr, size, alpha, beta);
	}
	void Mat::convertTo(Mat& dst, TYPE type_, CASTTYPE casttype) const
	{
		if (type_ < 0)
			type_ = type1();
		MatConvert(*this, dst, type_, casttype);
		return;
	}
	void Mat::convertTo(Mat& dst, TYPE _type, double alpha, double beta) const
	{
		bool noScale = fabs(alpha - 1) < DBL_EPSILON && fabs(beta) < DBL_EPSILON;

		if (_type < 0)
			_type = type1();

		if (noScale)
		{
			MatConvert(*this, dst, _type, CT_Cast);
			return;
		}

		dst.create(m_rows, m_cols, _type);

#define CoefT double
		BinaryFunc cvtFuncs[6][6] =
		{
			{NULL, NULL, NULL, NULL, NULL, NULL},
			{NULL, cvtScale_<uchar,uchar,CoefT>, cvtScale_<uchar,short,CoefT>, cvtScale_<uchar,int,CoefT>, cvtScale_<uchar,float,CoefT>, cvtScale_<uchar,double,CoefT>},
			{NULL, cvtScale_<short,uchar,CoefT>, cvtScale_<short,short,CoefT>, cvtScale_<short,int,double>, cvtScale_<short,float,CoefT>, cvtScale_<short,double,CoefT>},
			{NULL, cvtScale_<int,uchar,CoefT>, cvtScale_<int,short,CoefT>, cvtScale_<int,int,CoefT>, cvtScale_<int,float,CoefT>, cvtScale_<int,double,CoefT>},
			{NULL, cvtScale_<float,uchar,CoefT>, cvtScale_<float,short,CoefT>, cvtScale_<float,int,CoefT>, cvtScale_<float,float,CoefT>, cvtScale_<float,double,CoefT>},
			{NULL, cvtScale_<double,uchar,CoefT>, cvtScale_<double,short,CoefT>, cvtScale_<double,int,CoefT>, cvtScale_<double,float,CoefT>, cvtScale_<double,double,CoefT>},
		};

		BinaryFunc func = cvtFuncs[(int)type()][CVLIB_MAT_DEPTH(_type)];
		Size sz = size();
		sz.width *= channels();
		func((const uchar**)data.ptr, (uchar**)dst.data.ptr, sz, alpha, beta);
	}
	typedef double(*NormFunc)(uchar** const src1, uchar** const src2, uchar** const mask, Size size);
	template<typename _Tp> static double iNormDiffL1_(uchar** const _src1, uchar** const _src2, uchar** const mask, Size size)
	{
		double result = 0;
		_Tp** const src1 = (_Tp** const)_src1;
		_Tp** const src2 = (_Tp** const)_src2;
		if (src2)
		{
			if (!mask)
			{
				for (int y = 0; y < size.height; y++)
				{
					const _Tp* s1 = src1[y];
					const _Tp* s2 = src2[y];
					int x = 0;
					// #if CVLIB_ENABLE_UNROLLED
					for (; x <= size.width - 4; x += 4)
					{
						result += (double)fabs((double)(s1[x] - s2[x]));
						result += (double)fabs((double)(s1[x + 1] - s2[x + 1]));
						result += (double)fabs((double)(s1[x + 2] - s2[x + 2]));
						result += (double)fabs((double)(s1[x + 3] - s2[x + 3]));
					}
					// #endif

					for (; x < size.width; x++)
						result += (double)fabs((double)(s1[x] - s2[x]));
				}
			}
			else
			{
				for (int y = 0; y < size.height; y++)
				{
					const _Tp* s1 = src1[y];
					const _Tp* s2 = src2[y];
					for (int x = 0; x < size.width; x++)
						if (mask[y][x])
							result += (double)fabs((double)(s1[x] - s2[x]));
				}
			}
		}
		else
		{
			if (!mask)
			{
				for (int y = 0; y < size.height; y++)
				{
					const _Tp* s1 = src1[y];
					int x = 0;
					// #if CVLIB_ENABLE_UNROLLED
					for (; x <= size.width - 4; x += 4)
					{
						result += (double)fabs((double)(s1[x]));
						result += (double)fabs((double)(s1[x + 1]));
						result += (double)fabs((double)(s1[x + 2]));
						result += (double)fabs((double)(s1[x + 3]));
					}
					// #endif

					for (; x < size.width; x++)
						result += (double)fabs((double)(s1[x]));
				}
			}
			else
			{
				for (int y = 0; y < size.height; y++)
				{
					const _Tp* s1 = src1[y];
					for (int x = 0; x < size.width; x++)
						if (mask[y][x])
							result += (double)fabs((double)(s1[x]));
				}
			}
		}
		return result;
	}
	template<typename _Tp> static double  iNormDiffL2_(uchar** const _src1, uchar** const _src2, uchar** const mask, Size size)
	{
		double result = 0;
		_Tp** const src1 = (_Tp** const)_src1;
		_Tp** const src2 = (_Tp** const)_src2;
		if (src2)
		{
			if (!mask)
			{
				for (int y = 0; y < size.height; y++)
				{
					const _Tp* s1 = src1[y];
					const _Tp* s2 = src2[y];
					int x = 0;
					// #if CVLIB_ENABLE_UNROLLED
					for (; x <= size.width - 4; x += 4)
					{
						result += (double)SQR(s1[x] - s2[x]);
						result += (double)SQR(s1[x + 1] - s2[x + 1]);
						result += (double)SQR(s1[x + 2] - s2[x + 2]);
						result += (double)SQR(s1[x + 3] - s2[x + 3]);
					}
					// #endif

					for (; x < size.width; x++)
						result += (double)SQR(s1[x] - s2[x]);
				}
			}
			else
			{
				for (int y = 0; y < size.height; y++)
				{
					const _Tp* s1 = src1[y];
					const _Tp* s2 = src2[y];
					for (int x = 0; x < size.width; x++)
						if (mask[y][x])
							result += (double)SQR(s1[x] - s2[x]);
				}
			}
		}
		else
		{
			if (!mask)
			{
				for (int y = 0; y < size.height; y++)
				{
					const _Tp* s1 = src1[y];
					int x = 0;
					// #if CVLIB_ENABLE_UNROLLED
					for (; x <= size.width - 4; x += 4)
					{
						result += (double)SQR(s1[x]);
						result += (double)SQR(s1[x + 1]);
						result += (double)SQR(s1[x + 2]);
						result += (double)SQR(s1[x + 3]);
					}
					// #endif

					for (; x < size.width; x++)
						result += (double)SQR(s1[x]);
				}
			}
			else
			{
				for (int y = 0; y < size.height; y++)
				{
					const _Tp* s1 = src1[y];
					for (int x = 0; x < size.width; x++)
						if (mask[y][x])
							result += (double)SQR(s1[x]);
				}
			}
		}
		return sqrt(result);
	}
	template<typename _Tp> static double iNormDiffInf_(uchar** const _src1, uchar** const _src2, uchar** const mask, Size size)
	{
		double result = 0;
		_Tp** const src1 = (_Tp** const)_src1;
		_Tp** const src2 = (_Tp** const)_src2;
		if (src2)
		{
			if (!mask)
			{
				for (int y = 0; y < size.height; y++)
				{
					const _Tp* s1 = src1[y];
					const _Tp* s2 = src2[y];
					int x = 0;
					// #if CVLIB_ENABLE_UNROLLED
					for (; x <= size.width - 4; x += 4)
					{
						result = MAX(result, fabs((double)(s1[x] - s2[x])));
						result = MAX(result, fabs((double)(s1[x + 1] - s2[x + 1])));
						result = MAX(result, fabs((double)(s1[x + 2] - s2[x + 2])));
						result = MAX(result, fabs((double)(s1[x + 3] - s2[x + 3])));
					}
					// #endif

					for (; x < size.width; x++)
						if (mask[y][x])
							result = MAX(result, fabs((double)(s1[x] - s2[x])));
				}
			}
			else
			{
				for (int y = 0; y < size.height; y++)
				{
					const _Tp* s1 = src1[y];
					const _Tp* s2 = src2[y];
					for (int x = 0; x < size.width; x++)
						if (mask[y][x])
							result = MAX(result, fabs((double)(s1[x] - s2[x])));
				}
			}
		}
		else
		{
			if (!mask)
			{
				for (int y = 0; y < size.height; y++)
				{
					const _Tp* s1 = src1[y];
					int x = 0;
					// #if CVLIB_ENABLE_UNROLLED
					for (; x <= size.width - 4; x += 4)
					{
						result = MAX(result, fabs((double)(s1[x])));
						result = MAX(result, fabs((double)(s1[x + 1])));
						result = MAX(result, fabs((double)(s1[x + 2])));
						result = MAX(result, fabs((double)(s1[x + 3])));
					}
					// #endif

					for (; x < size.width; x++)
						result = MAX(result, fabs((double)(s1[x])));
				}
			}
			else
			{
				for (int y = 0; y < size.height; y++)
				{
					const _Tp* s1 = src1[y];
					for (int x = 0; x < size.width; x++)
						if (mask[y][x])
							result = MAX(result, fabs((double)(s1[x])));
				}
			}
		}
		return result;
	}
	double Mat::norm(int norm_type, const Mat& _mask) const
	{
		uchar** mask = _mask.isValid() ? _mask.data.ptr : NULL;
		Size sz = size();
		NormFunc func;
		NormFunc normFuncs[5][6] =
		{
			{NULL, NULL, NULL, NULL, NULL, NULL},
			{NULL, iNormDiffInf_<uchar>, iNormDiffInf_<short>, iNormDiffInf_<int>, iNormDiffInf_<float>, iNormDiffInf_<double>},
			{NULL, iNormDiffL1_<uchar>, iNormDiffL1_<short>, iNormDiffL1_<int>, iNormDiffL1_<float>, iNormDiffL1_<double>},
			{NULL, NULL, NULL, NULL, NULL, NULL},
			{NULL, iNormDiffL2_<uchar>, iNormDiffL2_<short>, iNormDiffL2_<int>, iNormDiffL2_<float>, iNormDiffL2_<double>},
		};
		func = normFuncs[norm_type][(int)type()];
		return func(data.ptr, 0, mask, sz);
	}

	double Mat::normDiff(int norm_type, const Mat& other, const Mat& _mask) const
	{
		assert(isEqualMat(other));
		uchar** mask = _mask.isValid() ? _mask.data.ptr : NULL;
		Size sz = size();
		NormFunc func;
		NormFunc normFuncs[5][6] =
		{
			{NULL, NULL, NULL, NULL, NULL, NULL},
			{NULL, iNormDiffInf_<uchar>, iNormDiffInf_<short>, iNormDiffInf_<int>, iNormDiffInf_<float>, iNormDiffInf_<double>},
			{NULL, iNormDiffL1_<uchar>, iNormDiffL1_<short>, iNormDiffL1_<int>, iNormDiffL1_<float>, iNormDiffL1_<double>},
			{NULL, NULL, NULL, NULL, NULL, NULL},
			{NULL, iNormDiffL2_<uchar>, iNormDiffL2_<short>, iNormDiffL2_<int>, iNormDiffL2_<float>, iNormDiffL2_<double>},
		};
		func = normFuncs[norm_type][(int)type()];
		return func(data.ptr, other.data.ptr, mask, sz);
	}

	typedef void(*CopyToFunc)(uchar** const src, uchar** dst, uchar** const mask, Size size, int cn);
	template<typename _Tp> static void iCopyTo(uchar** const _src, uchar** _dst, uchar** const mask, Size size, int cn)
	{
		_Tp** const src = (_Tp** const)_src;
		_Tp** dst = (_Tp**)_dst;
		if (!mask)
		{
			for (int y = 0; y < size.height; y++)
				memcpy(dst[y], src[y], sizeof(_Tp)*size.width*cn);
		}
		else
		{
			for (int y = 0; y < size.height; y++) for (int x = 0; x < size.width; x++)
			{
				if (mask[y][x]) {
					for (int icn = 0; icn < cn; icn++)
						dst[y][x*cn + icn] = src[y][x*cn + icn];
				}
			}
		}
	}
	void Mat::copyTo(Mat& dst, const Mat& _mask) const
	{
		uchar** const mask = _mask.data.ptr;
		Size sz = size();
		CopyToFunc func;
		CopyToFunc copytoFuncs[6] = { NULL,iCopyTo<uchar>,iCopyTo<short>,iCopyTo<int>,iCopyTo<float>,iCopyTo<double> };
		func = copytoFuncs[(int)type()];
		if (!isEqualMat(dst))
		{
			dst.release();
			dst.create(*this, false);
			dst = 0;
		}
		int cn = channels();
		func(data.ptr, dst.data.ptr, mask, sz, cn);
	}

	typedef void(*minmaxLocFunc)(uchar** const src, double* minVal, double* maxVal, Point2i* minLoc, Point2i* maxLoc, uchar** const mask, Size size);
	template<typename _Tp> static void iminmaxLoc(uchar** const _src, double* minVal, double* maxVal, Point2i* minLoc, Point2i* maxLoc, uchar** const mask, Size size)
	{
		_Tp** const src = (_Tp** const)_src;
		double rMin = DBL_MAX, rMax = -DBL_MAX;
		int iminX = 0, iminY = 0, imaxX = 0, imaxY = 0;
		if (!mask)
		{
			for (int y = 0; y < size.height; y++) for (int x = 0; x < size.width; x++)
			{
				if (rMin > src[y][x])
				{
					rMin = src[y][x];
					iminX = x;
					iminY = y;
				}
				if (rMax < src[y][x])
				{
					rMax = src[y][x];
					imaxX = x;
					imaxY = y;
				}
			}
		}
		else
		{
			for (int y = 0; y < size.height; y++) for (int x = 0; x < size.width; x++)
			{
				if (!mask[y][x])
					continue;
				if (rMin > src[y][x])
				{
					rMin = src[y][x];
					iminX = x;
					iminY = y;
				}
				if (rMax < src[y][x])
				{
					rMax = src[y][x];
					imaxX = x;
					imaxY = y;
				}
			}
		}
		if (minVal) *minVal = rMin;
		if (maxVal) *maxVal = rMax;
		if (minLoc) *minLoc = Point2i(iminX, iminY);
		if (maxLoc) *maxLoc = Point2i(imaxX, imaxY);
	}
	void Mat::minMaxLoc(double* minVal, double* maxVal, Point2i* minLoc, Point2i* maxLoc, const Mat& mask) const
	{
		minmaxLocFunc func;
		minmaxLocFunc minmaxFuncs[6] = { NULL,iminmaxLoc<uchar>,iminmaxLoc<short>,iminmaxLoc<int>,iminmaxLoc<float>,iminmaxLoc<double> };
		func = minmaxFuncs[(int)type()];
		func(data.ptr, minVal, maxVal, minLoc, maxLoc, mask.data.ptr, size());
	}
	int Mat::createFromArray(uchar* pbData, int nRows, int nCols, int nBitsperpixel, int nBytesperline, bool bFlipImage)
	{
		uchar* pBMPData = pbData;
		int nH = nRows;
		int nW = nCols;

		if ((nCols == 0) || (nRows == 0))
			return 0;

		if (nBitsperpixel <= 24)
			create(nRows, nCols, MAT_Tbyte3);
		else
			create(nRows, nCols, MAT_Tbyte4);


		uchar* ptr = data.ptr[0];
		for (int i = 0; i < nH; i++)
		{
			int nDst = (bFlipImage ? (nH - i - 1) : i) * nBytesperline;
			for (int j = 0; j < nW; j++)
			{
				int nTemp;
				ushort wRGB;
				switch (nBitsperpixel)
				{
				case enARGB:
					nTemp = nDst + j * 4;
					*ptr++ = pBMPData[nTemp];
					*ptr++ = pBMPData[nTemp + 1];
					*ptr++ = pBMPData[nTemp + 2];
					*ptr++ = pBMPData[nTemp + 3];
					break;
				case enBGRA:
					nTemp = nDst + j * 4;
					*ptr++ = pBMPData[nTemp + 2];
					*ptr++ = pBMPData[nTemp + 1];
					*ptr++ = pBMPData[nTemp + 0];
					*ptr++ = pBMPData[nTemp + 3];
					break;
				case enBGR888:
					nTemp = nDst + j * 3;
					*ptr++ = pBMPData[nTemp];
					*ptr++ = pBMPData[nTemp + 1];
					*ptr++ = pBMPData[nTemp + 2];
					break;
				case enBGR555:
					nTemp = nDst + j * 2;
					wRGB = (ushort)(pBMPData[nTemp] + (pBMPData[nTemp + 1] << 8));
					*ptr++ = (wRGB & 0x1F) << 3;
					*ptr++ = ((wRGB >> 5) & 0x1F) << 3;
					*ptr++ = ((wRGB >> 10) & 0x1F) << 3;
					break;
				case enBGR565:
					nTemp = nDst + j * 2;
					wRGB = (ushort)(pBMPData[nTemp] + (pBMPData[nTemp + 1] << 8));
					*ptr++ = (wRGB & 0x1F) << 3;
					*ptr++ = ((wRGB >> 5) & 0x3F) << 2;
					*ptr++ = ((wRGB >> 11) & 0x1F) << 3;
					break;
				case 12:
					nTemp = nDst + j * 2;
					wRGB = (ushort)(pBMPData[nTemp] + (pBMPData[nTemp + 1] << 8));
					*ptr++ = (wRGB & 0x1F) << 3;
					*ptr++ = ((wRGB >> 5) & 0x1F) << 3;
					*ptr++ = ((wRGB >> 10) & 0x1F) << 3;
					break;
				case 8:
					assert(false);
					break;
				case 1:
					assert(false);
					break;
				default:
					break;
				}
			}
		}
		return 1;
	}
	void Mat::reverseChannel()
	{
		if (channels() == 1)
			return;
		//#pragma omp parallel for
		int cn = channels();
		for (int ih = 0; ih < m_rows; ih++)
		{
			uchar temp;
			uchar* pbtemp = data.ptr[ih];
			for (int iw = 0; iw < m_cols; iw++, pbtemp += cn)
			{
				temp = *pbtemp;
				*pbtemp = *(pbtemp + 2);
				*(pbtemp + 2) = temp;
			}
		}
	}

	Mat Mat::diag(const Mat& d)
	{
		assert(d.m_cols == 1 || d.m_rows == 1);
		int len = d.m_rows + d.m_cols - 1;
		Mat m(len, len, d.type()); m.zero();
		if (d.m_cols == 1 || d.m_rows == 1)
		{
			switch (d.type())
			{
			case MAT_Tbyte:
				for (int i = 0; i < len; i++)
					m.data.ptr[i][i] = d.data.ptr[0][i];
				break;
			case MAT_Tshort:
				for (int i = 0; i < len; i++)
					m.data.s[i][i] = d.data.s[0][i];
				break;
			case MAT_Tint:
				for (int i = 0; i < len; i++)
					m.data.i[i][i] = d.data.i[0][i];
				break;
			case MAT_Tfloat:
				for (int i = 0; i < len; i++)
					m.data.fl[i][i] = d.data.fl[0][i];
				break;
			case MAT_Tdouble:
				for (int i = 0; i < len; i++)
					m.data.db[i][i] = d.data.db[0][i];
				break;
			default:
				assert(false);
			}
		}
		else
		{
			assert(false);
		}
		return m;
	}
	Mat Mat::zeros(int m_rows, int m_cols, TYPE m_type)
	{
		Mat ret(m_rows, m_cols, m_type); ret.zero();
		return ret;
	}
	Mat Mat::zeros(Size size, TYPE m_type)
	{
		Mat ret(size, m_type); ret.zero();
		return ret;
	}
	Mat Mat::ones(int m_rows, int m_cols, TYPE m_type)
	{
		Mat m(m_rows, m_cols, m_type); m.setValue(1);
		return m;
	}
	Mat Mat::ones(Size size, TYPE m_type)
	{
		Mat m(size, m_type);
		m.setValue(1);
		return m;
	}
	Mat Mat::eye(int m_rows, int m_cols, TYPE m_type)
	{
		Mat m(m_rows, m_cols, m_type);
		m.identity();
		return m;
	}
	Mat Mat::eye(Size size, TYPE m_type)
	{
		Mat m(size, m_type);
		m.identity();
		return m;
	}

	double Mat::std() const
	{
		return std::sqrt(var());
	}

	typedef void(*meanColFunc)(uchar** const _src, float* dst, const Size& size);
	template<typename _Tp> static void imeanColFunc(uchar** const _src, float* dst, const Size& size)
	{
		for (int y = 0; y < size.height; y++)
		{
			_Tp* const src = ((_Tp** const)_src)[y];
			dst[y] = 0.0f;
			for (int x = 0; x < size.width; x++)
				dst[y] += (float)src[x];
			dst[y] /= (float)(size.width);
		}
	}
	Mat Mat::meanCol() const
	{
		Mat m(1, m_rows, MAT_Tfloat);
		meanColFunc func;
		meanColFunc meanColFuncs[6] = { NULL,imeanColFunc<uchar>,imeanColFunc<short>,imeanColFunc<int>,
			imeanColFunc<float>,imeanColFunc<double> };
		func = meanColFuncs[(int)type()];
		func(data.ptr, m.data.fl[0], size());
		return m;
	}
	typedef void(*meanRowFunc)(uchar** const _src, float* dst, const Size& size);
	template<typename _Tp> static void imeanRowFunc(uchar** const _src, float* dst, const Size& size)
	{
		_Tp** const src = (_Tp** const)_src;
		float m = 1.0f / (float)size.height;
		for (int x = 0; x < size.width; x++)
		{
			dst[x] = 0.0f;
			for (int y = 0; y < size.height; y++)
				dst[x] += (float)src[y][x];
			dst[x] *= m;
		}
	}
	Mat Mat::meanRow() const
	{
		Mat m(1, m_cols, MAT_Tfloat);
		meanRowFunc func;
		meanRowFunc meanRowFuncs[6] = { NULL,imeanRowFunc<uchar>,imeanRowFunc<short>,imeanRowFunc<int>,
			imeanRowFunc<float>,imeanRowFunc<double> };
		func = meanRowFuncs[(int)type()];
		func(data.ptr, m.data.fl[0], size());
		return m;
	}
	void Mat::stdCol(Vec& /*vStd*/) const
	{
		assert(false);
	}
	typedef void(*sumRowFunc)(uchar** const _src, float* dst, const Size& size);
	template<typename _Tp> static void isumRowFunc(uchar** const _src, float* dst, const Size& size)
	{
		_Tp** const src = (_Tp** const)_src;
		for (int x = 0; x < size.width; x++)
		{
			dst[x] = 0.0f;
			for (int y = 0; y < size.height; y++)
				dst[x] += (float)src[y][x];
		}
	}
	Mat Mat::sumRows() const
	{
		Mat m(1, m_cols, MAT_Tfloat);
		sumRowFunc func;
		sumRowFunc sumColFuncs[6] = { NULL,isumRowFunc<uchar>,isumRowFunc<short>,isumRowFunc<int>,
			isumRowFunc<float>,isumRowFunc<double> };
		func = sumColFuncs[(int)type()];
		func(data.ptr, m.data.fl[0], size());
		return m;
	}


	void Mat::sumRows(Vec& vB) const
	{
		const Mat* pmA = this;

		vB.create(this->cols(), this->type());

		assert(pmA->cols() == vB.length());
		assert(pmA->type() == vB.type());

		double rSum = 0;

		int i = 0;
		switch (pmA->type())
		{
		case MAT_Tbyte:
			for (i = 0; i < pmA->cols(); i++)
			{
				rSum = 0;
				for (int j = 0; j < pmA->rows(); j++)
					rSum += pmA->data.ptr[j][i];
				vB.data.ptr[i] = (uchar)rSum;
			}
			break;
		case MAT_Tshort:
			for (i = 0; i < pmA->cols(); i++)
			{
				rSum = 0;
				for (int j = 0; j < pmA->rows(); j++)
					rSum += pmA->data.s[j][i];
				vB.data.s[i] = (short)rSum;
			}
			break;
		case MAT_Tint:
			for (i = 0; i < pmA->cols(); i++)
			{
				rSum = 0;
				for (int j = 0; j < pmA->rows(); j++)
					rSum += pmA->data.i[j][i];
				vB.data.i[i] = (int)rSum;
			}
			break;
		case MAT_Tfloat:
			for (i = 0; i < pmA->cols(); i++)
			{
				rSum = 0;
				for (int j = 0; j < pmA->rows(); j++)
					rSum += pmA->data.fl[j][i];
				vB.data.fl[i] = (float)rSum;
			}
			break;
		case MAT_Tdouble:
			for (i = 0; i < pmA->cols(); i++)
			{
				rSum = 0;
				for (int j = 0; j < pmA->rows(); j++)
					rSum += pmA->data.db[j][i];
				vB.data.db[i] = (double)rSum;
			}
			break;
		default:
			assert(false);
		}
	}

	void Mat::sumCols(Vec& vB) const
	{
		vB.create(this->rows(), this->type());

		double rSum = 0;
		const Mat* pmA = this;
		int i = 0;
		switch (pmA->type())
		{
		case MAT_Tbyte:
			for (i = 0; i < pmA->rows(); i++)
			{
				rSum = 0;
				for (int j = 0; j < pmA->cols(); j++)
					rSum += pmA->data.ptr[i][j];
				vB.data.ptr[i] = (uchar)rSum;
			}
			break;
		case MAT_Tshort:
			for (i = 0; i < pmA->rows(); i++)
			{
				rSum = 0;
				for (int j = 0; j < pmA->cols(); j++)
					rSum += pmA->data.s[i][j];
				vB.data.s[i] = (short)rSum;
			}
			break;
		case MAT_Tint:
			for (i = 0; i < pmA->rows(); i++)
			{
				rSum = 0;
				for (int j = 0; j < pmA->cols(); j++)
					rSum += pmA->data.i[i][j];
				vB.data.i[i] = (int)rSum;
			}
			break;
		case MAT_Tfloat:
			for (i = 0; i < pmA->rows(); i++)
			{
				rSum = 0;
				for (int j = 0; j < pmA->cols(); j++)
					rSum += pmA->data.fl[i][j];
				vB.data.fl[i] = (float)rSum;
			}
			break;
		case MAT_Tdouble:
			for (i = 0; i < pmA->rows(); i++)
			{
				rSum = 0;
				for (int j = 0; j < pmA->cols(); j++)
					rSum += pmA->data.db[i][j];
				vB.data.db[i] = (double)rSum;
			}
			break;
		default:
			assert(false);
		}
	}


	Mat Mat::sqrtm() const
	{
		Mat m = *this; m.convert(MAT_Tdouble);
		SVD svd(&m);
		Mat* pms = svd.GetS();
		Mat* pmu = svd.GetU();
		Mat* pmv = svd.GetV();
		for (int i = 0; i < pms->rows(); i++)
			pms->data.db[i][i] = std::sqrt(pms->data.db[i][i]);
		Mat msvd = (*pmu)*(*pms)*(*pmv).transposed();
		delete pms;
		delete pmu;
		delete pmv;
		return msvd;
	}

	int	Mat::checkVector(int _elemChannels, int _depth, bool _requireContinuous) const
	{
		return (type() == _depth || _depth <= 0) &&
			(isContinuous() || !_requireContinuous) &&
			(((((m_rows == 1 || m_cols == 1) && channels() == _elemChannels) ||
			(m_cols == _elemChannels && channels() == 1))))
			? (int)(total()*channels() / _elemChannels) : -1;
	}

	void	Mat::setElement(int i0, int i1, const void* p)
	{
		int cn = channels();
		memcpy(&data.ptr[i0][i1*cn], p, cn*m_step);
	}

	template<typename _Tp> static int countNonZero_(const _Tp* src, int len)
	{
		int i = 0, nz = 0;
		//    #if CVLIB_ENABLE_UNROLLED
		for (; i <= len - 4; i += 4)
			nz += (src[i] != 0) + (src[i + 1] != 0) + (src[i + 2] != 0) + (src[i + 3] != 0);
		//    #endif
		for (; i < len; i++)
			nz += src[i] != 0;
		return nz;
	}

	static int countNonZero8u(const uchar* src, int len)
	{
		int i = 0, nz = 0;
#if CVLIB_SSE2
		if (USE_SSE2)//5x-6x
		{
			__m128i pattern = _mm_setzero_si128();
			static uchar tab[256];
			static volatile bool initialized = false;
			if (!initialized)
			{
				// we compute inverse popcount table,
				// since we pass (img[x] == 0) mask as index in the table.
				for (int j = 0; j < 256; j++)
				{
					int val = 0;
					for (int mask = 1; mask < 256; mask += mask)
						val += (j & mask) == 0;
					tab[j] = (uchar)val;
				}
				initialized = true;
			}

			for (; i <= len - 16; i += 16)
			{
				__m128i r0 = _mm_loadu_si128((const __m128i*)(src + i));
				int val = _mm_movemask_epi8(_mm_cmpeq_epi8(r0, pattern));
				nz += tab[val & 255] + tab[val >> 8];
			}
		}
#endif
		for (; i < len; i++)
			nz += src[i] != 0;
		return nz;
	}

	static int countNonZero16u(const ushort* src, int len)
	{
		return countNonZero_(src, len);
	}

	static int countNonZero32s(const int* src, int len)
	{
		return countNonZero_(src, len);
	}

	static int countNonZero32f(const float* src, int len)
	{
		return countNonZero_(src, len);
	}

	static int countNonZero64f(const double* src, int len)
	{
		return countNonZero_(src, len);
	}

	typedef int(*CountNonZeroFunc)(const uchar*, int);

	static CountNonZeroFunc countNonZeroTab[] =
	{
		(CountNonZeroFunc)countNonZero8u,
		(CountNonZeroFunc)countNonZero8u,
		(CountNonZeroFunc)countNonZero16u,
		(CountNonZeroFunc)countNonZero32s,
		(CountNonZeroFunc)countNonZero32f,
		(CountNonZeroFunc)countNonZero64f,
		0
	};

	int Mat::countNonZero() const
	{
		CountNonZeroFunc func = countNonZeroTab[type()];
		int len = m_rows*m_cols*channels();
		return func(data.ptr[0], len);
	}

	static float ideterminant(float** pparMat, int nDim)
	{
		int i, j, k;
		float rRet, rMul;
		for (k = 0; k < nDim - 1; k++)
		{
			for (i = k + 1; i < nDim; i++)
			{
				rMul = pparMat[i][k] / pparMat[k][k];
				for (j = k; j < nDim; j++)
					pparMat[i][j] = pparMat[i][j] - rMul * pparMat[k][j];
			}
		}
		rRet = 1.0f;
		for (k = 0; k < nDim; k++)
			rRet = rRet * pparMat[k][k];
		return rRet;
	}

	double Mat::determinant() const
	{
		assert(m_rows == m_cols && channels() == 1);
		Mat t = *this;
		TYPE type_ = type1();
		if (type_ == MAT_Tfloat)
		{
			return ideterminant(t.data.fl, m_rows);
		}
		else if (type_ == MAT_Tdouble)
		{
			LUDecomposition lu(this);
			return lu.determinant();
		}
		return 0;
	}

}
