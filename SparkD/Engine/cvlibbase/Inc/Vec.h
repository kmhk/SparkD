/*!
 * \file    Vec.h
 * \ingroup base
 * \brief   
 * \author  
 */

#pragma once

#include "Object.h"
#include "String.hpp"
#include "XFile.h"
#include "cvlibstructs.h"
#include "Template.h"
#include <vector>

namespace cvlib
{
#undef max
#undef min
#undef sqr

	class Mat;
	template<typename _Tp> class Mat_;

	class CVLIB_DECLSPEC Vec : public Object
	{
		friend class Mat;
	public:
		union
		{
			unsigned char* ptr;
			short* s;
			int* i;
			float* fl;
			double* db;
		} data;

	public:

		// Construction and Destruction
		Vec();
		Vec(const Vec& vec);
		Vec(int m_len, TYPE type);
		Vec(void* ptr, int m_len, TYPE type);
		Vec(const Mat& image, bool fcopydata = true);
		template<typename _Tp> explicit Vec(Vector<_Tp>& points) { create((void*)&points[0], points.getSize(), (TYPE)DataDepth<_Tp>::value); }
		template<typename _Tp> explicit Vec(std::vector<_Tp>& points) { create((void*)&points[0], (int)points.size(), (TYPE)DataDepth<_Tp>::value); }
		virtual ~Vec();

		//! Initialize
		int		create(const Vec& vec, bool fCopy = false);
		int		create(int m_len, TYPE type);
		int		create(void* ptr, int m_len, TYPE type);
		void	release();

		//! import and export, conversion operations
		virtual int fromFile(const char* szFilename);
		virtual int fromFile(XFile* pFile);
		virtual int toFile(const char* szFilename) const;
		virtual int toFile(XFile* pFile) const;
		void toMatrix(const int nRows, const int nCols, Mat &m, bool rowWise = true);
		void toMatlab(const String& sFilename, const String& sName, const String& sComment = "", bool fAppend = true) const;
		void fromMatlab(const String& sFilename, const String& sName);
		void toC(const String& sFilename, const String& sVarName, const int nElement = 16, bool fAppend = false) const;
		String toString(const bool fNewline = true) const;

		// Attribute Operations
		inline int	length() const { return m_len; };
		inline int	maxLength() const { return m_nMaxLen; };
		inline TYPE	type() const { return m_type; };
		inline int step() const { return m_step; }

		//! valid check operations
		inline bool isValid() const { return data.ptr != 0; }
		inline bool isInside(int nX) const { return (nX < m_len && nX >= 0); }

		//! Element operations
		double	value(int i) const;
		Vec* subVec(int nFirst, int nLast, bool fShare = false) const;
		void	zero();
		void	resize(int num);
		Vec& operator=(const Vec& vecSrc);
		Vec& operator=(double value);

		void convertTo(Vec& dst, TYPE type) const;

		// norms
		double norm1() const;
		double norm2() const;
		double normInf() const;
		double sq_norm() const;
		double unit();

		// Statistical functions
		double autoCorrelation(const int lag = 1) const;
		double max() const;
		double max(int& iPos) const;
		double mean() const;
		double median() const;
		Vec	trim(const double percentage = .10) const;
		double trimmedMean(const double percentage = .10) const;
		double trimmedVar(const double percentage = .10) const;
		double trimmedStd(const double percentage = .10) const;
		double min() const;
		double min(int& iPos) const;
		double skewness() const;
		double std() const;
		double sum() const;
		double var() const;
		double var(double *pMean) const;

		void rand();
		void rand(const int st, const int end);
		void ceil();
		void floor();
		void round();
		void linspace(const double x1, const double x2, const int n);

		// Sorting
		void sort(bool ascending = true);
		// radomization
		void shuffle();

		void elementMultiply(const Vec& vector);
		void elementDivide(const Vec& vector);

		// math functions
		void abs();
		void clamp(const double min, const double max);
		void pow(double dP);
		void sqrt();
		void sqr();
		void log();

		// Saving and retrieving from binary file (Much faster then the MatLab Comunication routines).
		void reverse();

		// Conversions and manipulations
		Vec range(const int st, const int end) const;

		void swap(Vec& other);

		bool operator==(const Vec& refvector) const;
		bool operator!=(const Vec& refvector) const;
		bool operator<(const Vec& refvector) const;
		Vec operator+(const Vec &refvectorB) const;
		Vec operator-(void) const;
		Vec operator-(const Vec &refvectorB) const;
		Vec operator*(double dbl) const;
		double operator*(const Vec &refvectorB) const;
		Vec operator/(double dbl) const;
		Vec& operator+=(const Vec &refvectorB);
		Vec& operator-=(const Vec &refvectorB);
		Vec& operator+=(double dbl);
		Vec& operator-=(double dbl);
		Vec& operator*=(double dbl);
		Vec& operator/=(double dbl);
	protected:
		TYPE m_type;
		int m_step;
		int m_depend;
		int m_len;
		int m_nMaxLen;
	};

	template <typename _Tp> class CVLIB_DECLSPEC Vec_ : public Vec
	{
	public:
		typedef Mat_<_Tp> _MyMat;
		typedef Vec_<_Tp> _MyVec;

		Vec_() {};
		Vec_(const _MyVec& vec);
		Vec_(int m_len);
		Vec_(_Tp* ptr, int m_len);
		Vec_(Vector<_Tp>& points) : Vec(points) {}
		Vec_(std::vector<_Tp>& points) : Vec(points) {}

		virtual int		create(int len);
		int		create(_Tp* ptr, int m_len);
		_MyVec& operator=(const _MyVec& vecSrc);
		void resize(int num);
		operator _Tp*() { return (_Tp*)data.ptr; }
		operator _Tp* const() const { return (_Tp* const)data.ptr; }
		_MyVec& operator=(_Tp value);
		bool operator==(const _MyVec& refvector) const;
		bool operator!=(const _MyVec& refvector) const;
		virtual bool operator<(const _MyVec& refvector) const;
		virtual _MyVec operator+(const _MyVec &refvectorB) const;
		virtual _MyVec operator-(void) const;
		virtual _MyVec operator-(const _MyVec &refvectorB) const;
		virtual _MyVec operator*(double dbl) const;
		virtual double operator*(const _MyVec &refvectorB) const;
		_MyVec operator/(double dbl) const;
		virtual _MyVec& operator+=(const _MyVec &refvectorB);
		virtual _MyVec& operator-=(const _MyVec &refvectorB);
		virtual _MyVec& operator+=(double dbl);
		virtual _MyVec& operator-=(double dbl);
		virtual _MyVec& operator*=(double dbl);
		virtual _MyVec& operator/=(double dbl);

		_MyVec& eqSum(const _MyVec &refvectorA, const _MyVec &refvectorB);
		_MyVec& eqDiff(const _MyVec &refvectorA, const _MyVec &refvectorB);
		_MyVec& eqProd(const _MyMat &refmatrixA, const _MyVec &refvectorB);
		_MyVec vecCat(const _MyVec &v) const;
		bool crossProduct(const _MyVec& v1, const _MyVec& v2);
		void alignTo(const _MyVec& v, double *pA = NULL, double *pB = NULL);
	};

}

#include "xVec.h"

namespace cvlib
{
	typedef Vec_<uchar>		Vecb;
	typedef Vec_<short>		Vecs;
	typedef Vec_<int>		Veci;
	typedef Vec_<float>		Vecf;
	typedef Vec_<double>	Vecd;
}