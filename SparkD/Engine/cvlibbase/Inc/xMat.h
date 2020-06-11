/*!
 * \file    xMat
 * \ingroup base
 * \brief   
 * \author  
 */

#pragma once

namespace cvlib
{

	template<typename _Tp> Mat::Mat(Vector<Point2_<_Tp> >& points) {
		this->m_rows = this->m_cols = 0; data.ptr = NULL;
		create((void*)&points[0], points.getSize(), 2, (TYPE)DataDepth<_Tp>::value, true);
	}
	template<typename _Tp> Mat::Mat(Vector<Point3_<_Tp> >& points) {
		this->m_rows = this->m_cols = 0; data.ptr = NULL;
		create((void*)&points[0], points.getSize(), 3, (TYPE)DataDepth<_Tp>::value, true);
	}
	template<typename _Tp> Mat::Mat(Vector<Point4_<_Tp> >& points) {
		this->m_rows = this->m_cols = 0; data.ptr = NULL;
		create((void*)&points[0], points.getSize(), 4, (TYPE)DataDepth<_Tp>::value, true);
	}
	template<typename _Tp> Mat::Mat(std::vector<Point2_<_Tp> >& points) {
		this->m_rows = this->m_cols = 0; data.ptr = NULL;
		create((void*)&points[0], (int)points.size(), 2, (TYPE)DataDepth<_Tp>::value, true);
	}
	template<typename _Tp> Mat::Mat(std::vector<Point3_<_Tp> >& points) {
		this->m_rows = this->m_cols = 0; data.ptr = NULL;
		create((void*)&points[0], (int)points.size(), 3, (TYPE)DataDepth<_Tp>::value, true);
	}
	template<typename _Tp> Mat::Mat(std::vector<Point4_<_Tp> >& points) {
		this->m_rows = this->m_cols = 0; data.ptr = NULL;
		create((void*)&points[0], (int)points.size(), 4, (TYPE)DataDepth<_Tp>::value, true);
	}
	template<typename _Tp> Mat::Mat(std::vector<_Tp>& values) {
		this->m_rows = this->m_cols = 0; data.ptr = NULL;
		create((void*)&values[0], 1, (int)values.size(), (TYPE)DataDepth<_Tp>::value, true);
	}

	template<typename _Tp>
	Mat::Mat(const Point2_<_Tp>& point, bool copyData) {
		this->m_rows = this->m_cols = 0; data.ptr = NULL;
		if (copyData) {
			create(2, 1, (TYPE)DataDepth<_Tp>::value);
			memcpy(this->data.ptr[0], &point, sizeof(_Tp) * 2);
		}
		else {
			create((void*)&point, 2, 1, (TYPE)DataDepth<_Tp>::value, true);
		}
	}
	template<typename _Tp>
	Mat::Mat(const Point3_<_Tp>& point, bool copyData) {
		this->m_rows = this->m_cols = 0; data.ptr = NULL;
		if (copyData) {
			create(3, 1, (TYPE)DataDepth<_Tp>::value);
			memcpy(this->data.ptr[0], &point, sizeof(_Tp) * 3);
		}
		else {
			create((void*)&point, 3, 1, (TYPE)DataDepth<_Tp>::value, true);
		}
	}
	template<typename _Tp>
	Mat::Mat(const Point4_<_Tp>& point, bool copyData) {
		this->m_rows = this->m_cols = 0; data.ptr = NULL;
		if (copyData) {
			create(4, 1, (TYPE)DataDepth<_Tp>::value);
			memcpy(this->data.ptr[0], &point, sizeof(_Tp) * 4);
		}
		else {
			create((void*)&point, 4, 1, (TYPE)DataDepth<_Tp>::value, true);
		}
	}
	template<typename _Tp> inline Mat::operator std::vector<_Tp>() const {
		if (isContinuous()) {
			int size = m_rows*m_cols*channels();
			std::vector<_Tp> v((_Tp*)data.ptr[0], &((_Tp*)data.ptr[0])[size]);
			return v;
		}
		else {
			int size = m_cols*channels();
			std::vector<_Tp> v;
			v.resize(size*m_rows);
			for (int i = 0; i < m_rows; i++)
				memcpy(&v[i*size], data.ptr[i], size * sizeof(_Tp));
			return v;
		}
	}
	template<typename _Tp> inline Mat::operator Point2_<_Tp>() const {
		return Point2_<_Tp>(((_Tp*)data.ptr[0])[0], ((_Tp*)data.ptr[0])[1]);
	}
	template<typename _Tp> inline Mat::operator Point3_<_Tp>() const {
		return Point3_<_Tp>(((_Tp*)data.ptr[0])[0], ((_Tp*)data.ptr[0])[1], ((_Tp*)data.ptr[0])[2]);
	}
	template<typename _Tp> inline Mat::operator Point4_<_Tp>() const {
		return Point4_<_Tp>(((_Tp*)data.ptr[0])[0], ((_Tp*)data.ptr[0])[1], ((_Tp*)data.ptr[0])[2], ((_Tp*)data.ptr[0])[3]);
	}
	template<typename _Tp> inline _Tp& Mat::at(int i0, int i1) {
		return ((_Tp*)data.ptr[i0])[i1];
	}
	template<typename _Tp> inline const _Tp& Mat::at(int i0, int i1) const {
		return ((const _Tp*)data.ptr[i0])[i1];
	}

	template<typename _Tp>
	Mat_<_Tp>::Mat_(const Mat& m)
	{
		if (m.isValid())
		{
			if (DataDepth<_Tp>::value == m.type())
			{
				Mat::operator =(m);
			}
			Mat::operator =(m);
			convert((TYPE)DataDepth<_Tp>::value, CT_Cast);
		}
	}

	template<typename _Tp>
	Mat_<_Tp>::Mat_(const _MyMat& mat)
	{
		if (this == &mat) return;
		Mat::create(mat, true);
	}
	template<typename _Tp>
	Mat_<_Tp>::Mat_(int _rows, int _cols)
	{
		create(_rows, _cols);
	}
	template<typename _Tp>
	Mat_<_Tp>::Mat_(_Tp** ptr, int _rows, int _cols)
	{
		create(ptr, _rows, _cols);
	}
	template<typename _Tp>
	Mat_<_Tp>::Mat_(_Tp* ptr, int _rows, int _cols, bool fAlloc)
	{
		create(ptr, _rows, _cols, fAlloc);
	}
	template<typename _Tp>
	int	Mat_<_Tp>::create(int r, int c)
	{
		if (this->m_rows == r && this->m_cols == c &&
			this->m_type == (TYPE)DataDepth<_Tp>::value && data.ptr)
			return 0;
		release();
		this->m_rows = r;
		this->m_cols = c;
		this->m_type = (TYPE)DataDepth<_Tp>::value;
		m_step = sizeof(_Tp);
		int linestep = m_step*m_cols*channels();
		data.ptr = (uchar**)new uchar*[m_rows];
		data.ptr[0] = (uchar*)new uchar[m_rows * linestep];
		int i;
		for (i = 1; i < m_rows; i++)
			data.ptr[i] = data.ptr[i - 1] + linestep;
		m_depend = 0;
		return 1;
	}
	template<typename _Tp>
	int	Mat_<_Tp>::create(_Tp** ptr, int r, int c)
	{
		release();
		this->m_rows = r;
		this->m_cols = c;
		this->m_type = (TYPE)DataDepth<_Tp>::value;
		this->m_step = sizeof(_Tp);
		this->data.ptr = (uchar**)ptr;
		m_depend = 1;
		return 1;
	}
	template<typename _Tp>
	int	Mat_<_Tp>::create(_Tp* ptr, int r, int c, bool fAlloc)
	{
		release();
		int i;
		this->m_rows = r;
		this->m_cols = c;
		this->m_type = (TYPE)DataDepth<_Tp>::value;
		m_step = sizeof(_Tp);
		if (fAlloc)
		{
			data.ptr = (uchar**)new _Tp*[m_rows];
			data.ptr[0] = (uchar*)ptr;
			for (i = 1; i < m_rows; i++)
				data.ptr[i] = (uchar*)(ptr + i * m_cols);
			m_depend = 2;
		}
		else
		{
			data.ptr = (uchar**)(ptr + m_rows * m_cols);
			data.ptr[0] = (uchar*)ptr;
			for (i = 1; i < m_rows; i++)
				data.ptr[i] = (uchar*)(ptr + i * m_cols);
			m_depend = 4;
		}
		return 1;
	}
	template<typename _Tp>
	int	Mat_<_Tp>::create(const _MyMat& mat, bool fCopy)
	{
		int nRet;
		if (size() == mat.size() && m_type == mat.type1() && data.ptr)
			nRet = 1;
		else
		{
			release();
			nRet = create(mat.rows(), mat.cols());
		}
		if (nRet)
		{
			if (fCopy)
			{
				for (int i = 0; i < m_rows; i++)
					memcpy(data.ptr[i], mat.data.ptr[i], step() * m_cols);
			}
			return 1;
		}
		return 0;
	}
	template<typename _Tp>
	Mat_<_Tp>&	Mat_<_Tp>::operator=(const _MyMat& other)
	{
		if (this == &other)
			return *this;
		if (this->isValid())
			memcpy(data.ptr[0], other.data.ptr[0], step()*m_rows*m_cols*channels());
		else
			Mat::create(other, true);
		return *this;
	}
	template<typename _Tp>
	Mat_<_Tp>& Mat_<_Tp>::operator=(double value)
	{
		int i;
		if ((value == 0.0) && (m_rows != 0))
		{
			assert(m_rows > 0 && m_cols > 0);
			if (isContinuous())
				memset(data.ptr[0], 0, m_rows*m_cols * sizeof(_Tp)*channels());
			else
			{
				int nlinestep = m_cols * sizeof(_Tp)*channels();
				for (i = 0; i < m_rows; i++)
					memset(data.ptr[i], 0, nlinestep);
			}
		}
		else
		{
			int n = m_rows * m_cols;
			_Tp *p = (_Tp*)data.ptr[0];
			for (i = 0; i < n; i++)
				p[i] = (_Tp)value;
		}
		return *this;
	}
	template<typename _Tp>
	bool Mat_<_Tp>::operator==(const _MyMat& refmatrix) const
	{
		if ((m_rows == refmatrix.m_rows) && (m_cols == refmatrix.cols()))
		{
			if ((m_rows == 0) || (m_cols == 0))
				return true;
			int cbRow = m_cols() * sizeof(_Tp);
			for (int iRow = 0; iRow < m_rows; ++iRow)
			{
				if (memcmp(data.ptr[iRow], refmatrix.data.ptr[iRow], cbRow) != 0)
					return false;
			}
			return true;
		}
		return false;
	}
	template<typename _Tp>
	bool Mat_<_Tp>::operator!=(const _MyMat& refmatrix) const
	{
		return !(*this == refmatrix);
	}
	template<typename _Tp>
	bool Mat_<_Tp>::operator<(const _MyMat& refmatrix) const
	{
		if (m_rows == refmatrix.m_rows)
		{
			if (m_cols == refmatrix.cols())
			{
				if ((m_rows == 0) || (m_cols == 0))
					return false;
				int cbRow = m_cols() * sizeof(_Tp);
				for (int iRow = 0; iRow < m_rows; ++iRow)
				{
					int wCmp = memcmp(data.ptr[iRow], refmatrix.data.ptr[iRow], cbRow);
					if (wCmp != 0)
						return (wCmp < 0);
				}
				return false;
			}
			return (m_cols < refmatrix.cols());
		}
		return (m_rows < refmatrix.rows());
	}
	template<typename _Tp>
	Mat_<_Tp> Mat_<_Tp>::operator+(const _MyMat& refmatrix) const
	{
		Mat_<_Tp> matrixRet(*this);
		return matrixRet.eqSum(*this, refmatrix);
	}
	template<typename _Tp>
	Mat_<_Tp> Mat_<_Tp>::operator-(void) const
	{
		Mat_<_Tp> matrixRet(m_rows, m_cols);
		for (int i = 0; i < m_rows; i++)
			for (int j = 0; j < m_cols; j++)
				matrixRet.data.ptr[i][j] = -data.ptr[i][j];
		return matrixRet;
	}
	template<typename _Tp>
	Mat_<_Tp> Mat_<_Tp>::operator-(const _MyMat& refmatrix) const
	{
		Mat_<_Tp> matrixRet(*this);
		return matrixRet.eqDiff(*this, refmatrix);
	}
	template<typename _Tp>
	Mat_<_Tp> Mat_<_Tp>::operator*(double dbl) const
	{
		Mat_<_Tp> matrixRet(*this);
		return (matrixRet *= dbl);
	}
	template<typename _Tp>
	Mat_<_Tp> Mat_<_Tp>::operator*(const _MyMat& refmatrix) const
	{
		Mat_<_Tp> matrixRet(m_rows, refmatrix.cols());
		return matrixRet.eqProd(*this, refmatrix);
	}
	template<typename _Tp>
	Vec_<_Tp> Mat_<_Tp>::operator*(const _MyVec& refvector) const
	{
		Vec_<_Tp> vectorRet(m_rows);
		return vectorRet.eqProd(*this, refvector);
	}
	template<typename _Tp>
	Mat_<_Tp> Mat_<_Tp>::operator/(double dbl) const
	{
		Mat_<_Tp> matrixRet(*this);
		return (matrixRet /= dbl);
	}
	template<typename _Tp>
	Mat_<_Tp>& Mat_<_Tp>::operator+=(const _MyMat& refmatrix)
	{
		assert((m_rows == refmatrix.rows()) && (m_cols == refmatrix.cols()));
		for (int i = 0; i < m_rows; i++)for (int j = 0; j < m_cols; j++)
			(*this)[i][j] += refmatrix[i][j];
		return *this;
	}
	template<typename _Tp>
	Mat_<_Tp>& Mat_<_Tp>::operator-=(const _MyMat& refmatrix)
	{
		assert((m_rows == refmatrix.rows()) && (m_cols == refmatrix.cols()));
		for (int i = 0; i < m_rows; i++)
			for (int j = 0; j < m_cols; j++)
				(*this)[i][j] -= refmatrix[i][j];
		return *this;
	}
	template<typename _Tp>
	Mat_<_Tp>& Mat_<_Tp>::operator*=(double dbl)
	{
		for (int i = 0; i < m_rows; i++)
			for (int j = 0; j < m_cols; j++)
				(*this)[i][j] = (_Tp)((*this)[i][j] * dbl);
		return *this;
	}
	template<typename _Tp>
	Mat_<_Tp>& Mat_<_Tp>::operator*=(const _MyMat& refmatrix)
	{
		Mat_<_Tp> matrixT(*this);
		assert(matrixT.cols() == refmatrix.rows());
		if (refmatrix.cols() != refmatrix.rows())
		{
			resizeRows(m_rows);
			resizeCols(refmatrix.cols());
		}
		for (int i = 0; i < matrixT.rows(); i++)
		{
			for (int j = 0; j < refmatrix.cols(); j++)
			{
				double sum = 0.0;
				for (int k = 0; k < m_cols; k++)
					sum += matrixT[i][k] * refmatrix[k][j];
				(*this)[i][j] = (_Tp)sum;
			}
		}
		return *this;
	}
	template<typename _Tp>
	Mat_<_Tp>& Mat_<_Tp>::operator/=(double dbl)
	{
		assert(dbl != 0);
		for (int i = 0; i < m_rows; i++)
			for (int j = 0; j < m_cols; j++)
				(*this)[i][j] = (_Tp)((*this)[i][j] / dbl);
		return *this;
	}
	template<typename _Tp>
	Mat_<_Tp>& Mat_<_Tp>::eqSum(const _MyMat &refmatrixA, const _MyMat &refmatrixB)
	{
		assert(m_rows == refmatrixA.rows());
		assert(m_cols == refmatrixA.cols());
		assert(m_rows == refmatrixB.rows());
		assert(m_cols == refmatrixB.cols());
		for (int i = 0; i < m_rows; i++)
			for (int j = 0; j < m_cols; j++)
				(*this)[i][j] = refmatrixA[i][j] + refmatrixB[i][j];
		return (*this);
	}
	template<typename _Tp>
	Mat_<_Tp>& Mat_<_Tp>::eqDiff(const _MyMat &refmatrixA, const _MyMat &refmatrixB)
	{
		assert(m_rows == refmatrixA.rows());
		assert(m_cols == refmatrixA.cols());
		assert(m_rows == refmatrixB.rows());
		assert(m_cols == refmatrixB.cols());
		for (int i = 0; i < m_rows; i++)
			for (int j = 0; j < m_cols; j++)
				(*this)[i][j] = refmatrixA[i][j] - refmatrixB[i][j];
		return (*this);
	}
	template<typename _Tp>
	Mat_<_Tp>& Mat_<_Tp>::eqProd(const _MyMat &refmatrixA, const _MyMat &refmatrixB)
	{
		int nTerms = refmatrixA.cols();
		assert(m_rows == refmatrixA.rows());
		assert(nTerms == refmatrixB.rows());
		assert(refmatrixB.cols() == m_cols);
		for (int i = 0; i < m_rows; i++) {
			for (int j = 0; j < m_cols; j++) {
				double sum = 0.0;
				for (int k = 0; k < nTerms; k++)
					sum += refmatrixA[i][k] * refmatrixB[k][j];
				(*this)[i][j] = (_Tp)sum;
			}
		}
		return (*this);
	}
	template<typename _Tp>
	Vec_<_Tp> Mat_<_Tp>::row(int r) const
	{
		assert(r < m_rows);
		assert(r >= 0);
		Vec_<_Tp> _row(m_cols);
		memcpy(_row.data.ptr, this->data.ptr[r], m_cols*step());
		return _row;
	}
	template<typename _Tp>
	Vec_<_Tp> Mat_<_Tp>::column(int c) const
	{
		assert(c < m_cols);
		assert(c >= 0);
		Vec_<_Tp> col(m_rows);
		for (int i = 0; i < m_rows; i++)
			col[i] = (*this)[i][c];
		return col;
	}
	template<typename _Tp>
	void Mat_<_Tp>::setRow(int r, const _MyVec &v)
	{
		int N = m_cols;
		if (N > v.length())
			N = v.length();
		memcpy(&(*this)[r][0], &v[0], N * sizeof(_Tp));
	}
	template<typename _Tp>
	void Mat_<_Tp>::setColumn(int c, const _MyVec &v)
	{
		int N = m_rows;
		if (N > v.length())
			N = v.length();
		for (int i = 0; i < N; i++)
			(*this)[i][c] = v[i];
	}
	template<typename _Tp>
	double Mat_<_Tp>::det() const
	{
		double d;
		assert(m_rows == m_cols);
		if (m_rows == 3)
		{
			d = (double)(data.db[0][0] * data.db[1][1] * data.db[2][2] + data.db[0][1] * data.db[1][2] * data.db[2][0] + data.db[0][2] * data.db[1][0] * data.db[2][1]
				- data.db[0][0] * data.db[1][2] * data.db[2][1] - data.db[0][1] * data.db[1][0] * data.db[2][2] - data.db[0][2] * data.db[1][1] * data.db[2][0]);
		}
		if (m_rows == 2)
		{
			d = (double)(data.db[0][0] * data.db[1][1] - data.db[1][0] * data.db[0][1]);
		}
		if (m_rows == 1)
			d = (double)data.db[0][0];
		if (m_rows > 3)
		{
			d = determinant();
		}
		return d;
	}
	template<typename _Tp>
	void Mat_<_Tp>::col(int i, _MyVec& vCol) const
	{
		int nr = m_rows;
		if (nr != vCol.length())
			vCol.resize(nr);
		for (int r = 0; r < nr; r++)
			vCol[r] = (*this)[r][i];
	}
	template<typename _Tp>
	void Mat_<_Tp>::row(int i, _MyVec& vRow) const
	{
		int nc = m_cols;
		if (nc != vRow.length())
			vRow.resize(nc);
		memcpy(vRow.data.ptr, data.ptr[i], nc*step());
	}
	template<typename _Tp>
	Mat_<_Tp>& Mat_<_Tp>::submatrix(const int newRows, const int newCols, _MyMat& submat, const int i0, const int j0) const
	{
		assert(newRows > 0);
		assert(newCols > 0);
		assert(i0 >= 0);
		assert(j0 >= 0);
		assert(i0 + newRows <= m_rows);
		assert(j0 + newCols <= m_cols);
		if (submat.m_rows != newRows || submat.cols() != newCols)
		{
			submat.release();
			submat.create(newRows, newCols);
		}
		for (int i = 0; i < newRows; i++)
			memcpy(submat.data.ptr[i], &data.ptr[i0 + i][j0*step()], newCols*step());
		return submat;
	}
	template<typename _Tp>
	Mat_<_Tp> Mat_<_Tp>::submatrix(const int newRows, const int newCols, const int i0, const int j0) const
	{
		Mat_<_Tp> submat(newRows, newCols);
		submatrix(newRows, newCols, submat, i0, j0);
		return submat;
	}
	template<typename _Tp>
	Mat_<_Tp> Mat_<_Tp>::transposed(void) const
	{
		int i, j, n = m_rows, m = m_cols;
		Mat_<_Tp> matrixTranspose(m, n);
		assert(m > 0 && n > 0);
		for (i = 0; i < m; i++)
			for (j = 0; j < n; j++)
				matrixTranspose[i][j] = (*this)[j][i];
		return matrixTranspose;
	}
	template<typename _Tp>
	Mat_<_Tp>& Mat_<_Tp>::transpose(void)
	{
		*this = transposed();
		return *this;
	}

}