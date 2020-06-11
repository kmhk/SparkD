
/*!
 * \file    Vec.h
 * \ingroup base
 * \brief   
 * \author  
 */

#pragma once

namespace cvlib
{

#define ExternalStorage 0
#define NonExternalStorage 1

	template<typename _Tp>
	Vec_<_Tp>::Vec_(const _MyVec& vec)
	{
		if (this == &vec) return;
		Vec::create(vec, true);
	}
	template<typename _Tp>
	Vec_<_Tp>::Vec_(int _len)
	{
		data.ptr = 0;
		create(_len);
	}
	template<typename _Tp>
	Vec_<_Tp>::Vec_(_Tp* ptr, int _len)
	{
		data.ptr = 0;
		create(ptr, _len);
	}
	template<typename _Tp>
	int Vec_<_Tp>::create(int _len)
	{
		release();
		if (_len <= 0) return 0;
		this->m_len = _len;
		this->m_type = (TYPE)DataDepth<_Tp>::value;
		m_nMaxLen = _len;
		m_step = sizeof(_Tp);
		data.ptr = new uchar[_len * m_step];
		m_depend = ExternalStorage;
		return 1;
	}
	template<typename _Tp>
	int Vec_<_Tp>::create(_Tp* ptr, int _len)
	{
		release();
		this->m_len = _len;
		m_nMaxLen = _len;
		this->m_type = (TYPE)DataDepth<_Tp>::value;
		m_step = sizeof(_Tp);
		data.ptr = (uchar*)ptr;
		m_depend = NonExternalStorage;
		return 1;
	}
	template<typename _Tp>
	Vec_<_Tp>& Vec_<_Tp>::operator=(const _MyVec& vecSrc)
	{
		if (this == &vecSrc)
			return *this;
		if (m_len == vecSrc.m_len)
			memcpy(data.ptr, vecSrc.data.ptr, vecSrc.step()*vecSrc.length());
		else
			Vec::create(vecSrc, true);
		return *this;
	}
	template<typename _Tp>
	bool Vec_<_Tp>::operator==(const _MyVec& refvector) const
	{
		if (length() == refvector.length())
		{
			if (length() == 0) return true;
			if (memcmp(data.ptr, refvector.data.ptr, length() * sizeof(_Tp)) == 0)
				return true;
		}
		return false;
	}
	template<typename _Tp>
	bool Vec_<_Tp>::operator!=(const _MyVec& refvector) const
	{
		return !(*this == refvector);
	}
	template<typename _Tp>
	bool Vec_<_Tp>::operator<(const _MyVec& refvector) const
	{
		if (length() == refvector.length())
		{
			if (length() == 0) return false;
			return (memcmp(data.ptr, refvector.data.ptr, length() * sizeof(_Tp)) < 0);
		}
		return (length() < refvector.length());
	}
	template<typename _Tp>
	Vec_<_Tp> Vec_<_Tp>::operator+(const _MyVec &refvector) const
	{
		assert(length() == refvector.length());
		Vec_<_Tp> vectorRet(*this);
		return (vectorRet += refvector);
	}
	template<typename _Tp>
	Vec_<_Tp> Vec_<_Tp>::operator-(void) const
	{
		Vec_<_Tp> vectorRet(length());
		for (int i = 0; i < length(); i++)
			vectorRet[i] = -((_Tp*)data.ptr)[i];
		return vectorRet;
	}
	template<typename _Tp>
	Vec_<_Tp> Vec_<_Tp>::operator-(const _MyVec& refvector) const
	{
		assert(length() == refvector.length());
		Vec_<_Tp> vectorRet(*this);
		return (vectorRet -= refvector);
	}
	template<typename _Tp>
	Vec_<_Tp> Vec_<_Tp>::operator*(double dbl) const
	{
		Vec_<_Tp> vectorRet(*this);
		return (vectorRet *= dbl);
	}
	template<typename _Tp>
	double Vec_<_Tp>::operator*(const _MyVec &refvector) const
	{
		double sum = 0.0;
		assert(length() == refvector.length());
		for (int i = 0; i < length(); i++)
			sum += ((_Tp*)data.ptr)[i] * ((_Tp*)refvector.data.ptr)[i];
		return sum;
	}
	template<typename _Tp>
	Vec_<_Tp> Vec_<_Tp>::operator/(double dbl) const
	{
		assert(dbl != 0);
		Vec_<_Tp> vectorRet(*this);
		return (vectorRet /= dbl);
	}
	template<typename _Tp>
	Vec_<_Tp>& Vec_<_Tp>::operator+=(const _MyVec &refvector)
	{
		return eqSum(*this, refvector);
	}
	template<typename _Tp>
	Vec_<_Tp>& Vec_<_Tp>::operator-=(const _MyVec &refvector)
	{
		return eqDiff(*this, refvector);
	}
	template<typename _Tp>
	Vec_<_Tp>& Vec_<_Tp>::operator+=(double dbl)
	{
		for (int i = 0; i < length(); i++)
			((_Tp*)data.ptr)[i] = (_Tp)(((_Tp*)data.ptr)[i] + dbl);
		return *this;
	}
	template<typename _Tp>
	Vec_<_Tp>& Vec_<_Tp>::operator-=(double dbl)
	{
		for (int i = 0; i < length(); i++)
			((_Tp*)data.ptr)[i] = (_Tp)(((_Tp*)data.ptr)[i] - dbl);
		return *this;
	}
	template<typename _Tp>
	Vec_<_Tp>& Vec_<_Tp>::operator*=(double dbl)
	{
		for (int i = 0; i < length(); i++)
			((_Tp*)data.ptr)[i] = (_Tp)(((_Tp*)data.ptr)[i] * dbl);
		return *this;
	}
	template<typename _Tp>
	Vec_<_Tp>& Vec_<_Tp>::operator/=(double dbl)
	{
		assert(dbl != 0);
		for (int i = 0; i < length(); i++)
			((_Tp*)data.ptr)[i] = (_Tp)(((_Tp*)data.ptr)[i] / dbl);
		return *this;
	}
	template<typename _Tp>
	Vec_<_Tp>& Vec_<_Tp>::eqSum(const _MyVec &refvectorA, const _MyVec &refvectorB)
	{
		assert(refvectorA.length() == refvectorB.length());

		int n = length();
		if (n == 0) {
			create(refvectorA.length());
			n = length();
		}
		else {
			assert(n == refvectorA.length());
		}
		for (int i = 0; i < n; i++)
			((_Tp*)data.ptr)[i] = ((_Tp*)refvectorA.data.ptr)[i] + ((_Tp*)refvectorB.data.ptr)[i];
		return *this;
	}
	template<typename _Tp>
	Vec_<_Tp>& Vec_<_Tp>::eqDiff(const _MyVec &refvectorA, const _MyVec &refvectorB)
	{
		int n = length();
		assert(n == refvectorA.length());
		assert(n == refvectorB.length());
		for (int i = 0; i < n; i++)
			((_Tp*)data.ptr)[i] = ((_Tp*)refvectorA.data.ptr)[i] - ((_Tp*)refvectorB.data.ptr)[i];
		return *this;
	}
	template<typename _Tp>
	Vec_<_Tp>& Vec_<_Tp>::eqProd(const _MyMat &refmatrixA, const _MyVec &refvectorB)
	{
		int nRows = refmatrixA.rows();
		int nCols = refmatrixA.cols();
		assert(nRows == length());
		assert(refmatrixA.cols() == refvectorB.length());
		for (int i = 0; i < nRows; i++)
		{
			double dblSum = 0.0;
			for (int j = 0; j < nCols; j++)
				dblSum += refmatrixA[i][j] * refvectorB[j];
			((_Tp*)data.ptr)[i] = (_Tp)dblSum;
		}
		return (*this);
	}
	template<typename _Tp>
	Vec_<_Tp> Vec_<_Tp>::vecCat(const _MyVec &v) const
	{
		int tlen = this->length();
		Vec_<_Tp> ret(tlen + v.length());
		for (int i = 0; i < ret.length(); i++)
			ret[i] = i < tlen ? (*this)[i] : v[i - tlen];
		return ret;
	}
	template<typename _Tp>
	bool Vec_<_Tp>::crossProduct(const _MyVec& v1, const _MyVec& v2)
	{
		int N = v1.length();
		if (N < 2 || N != v2.length())
			return false;
		int size = N * (N - 1) / 2 - 1;
		create(size + 1);
		int inc = 0;
		_Tp cross;
		for (int i = N - 1; i > 0; i--) for (int j = (i - 1); j >= 0; j--)
		{
			cross = v1[i] * v2[j] - v2[i] * v1[j];
			((_Tp*)data.ptr)[inc] = (((i - j) % 2) == 0) ? cross : -cross;
			inc++;
		}
		return true;
	}
	template<typename _Tp>
	void Vec_<_Tp>::alignTo(const _MyVec& v, double *pA, double *pB)
	{
		assert(this->length() == v.length());
		int n = this->length();
		assert(n == v.length());
		double x, y, a, b, Sx = .0, Sy = .0, Sxx = .0, Sxy = .0;
		int i;
		for (i = 0; i < n; i++)
		{
			x = ((_Tp*)data.ptr)[i];
			y = v[i];
			Sx += x;
			Sy += y;
			Sxx += x*x;
			Sxy += x*y;
		}
		double d = n*Sxx - Sx*Sx;
		a = (n*Sxy - Sx*Sy) / d;
		b = (Sy*Sxx - Sx*Sxy) / d;
		for (i = 0; i < n; i++)
			((_Tp*)data.ptr)[i] = (_Tp)(a * (*this)[i] + b);
		if (pA) { *pA = a; }
		if (pB) { *pB = b; }
	}
	template<typename _Tp>
	Vec_<_Tp>& Vec_<_Tp>::operator=(_Tp value)
	{
		if ((value == 0) && (length() != 0))
		{
			assert(length() > 0);
			memset(data.ptr, 0, length() * sizeof(_Tp));
		}
		else
		{
			for (int i = 0; i < length(); i++)
				((_Tp*)data.ptr)[i] = value;
		}
		return *this;
	}
	template<typename _Tp>
	void Vec_<_Tp>::resize(int num)
	{
		if (m_nMaxLen >= num) m_len = num;
		else if (m_depend == ExternalStorage)
		{
			Vec_<_Tp> v(num);
			memcpy(v.data.ptr, data.ptr, m_len * m_step);
			release();
			Vec::create(v, true);
		}
	}

}