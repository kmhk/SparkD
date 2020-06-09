/*!
 * \file    Mat.h
 * \ingroup base
 * \brief   
 * \author  
 * \last update date	2017/05/26
 */

#pragma once

#include "Vec.h"
#include "cvlibformats.h"
#include "Template.h"
#include <algorithm>
#include <string>
#include <cstring>

namespace cvlib
{

#undef abs
#undef min
#undef max

	enum ENUM_CVLIB_MATIMG_CONVERT_FORMATS
	{
		CVLIB_MATIMG_DEFAULT,
		CVLIB_MATIMG_EXPANSION
	};

	enum {
		NORM_INF = 1, NORM_L1 = 2, NORM_L2 = 4, NORM_L2SQR = 5, NORM_HAMMING = 6,
		NORM_HAMMING2 = 7, NORM_TYPE_MASK = 7, NORM_RELATIVE = 8, NORM_MINMAX = 32
	};

	enum
	{
		FONT_HERSHEY_SIMPLEX = 0,
		FONT_HERSHEY_PLAIN = 1,
		FONT_HERSHEY_DUPLEX = 2,
		FONT_HERSHEY_COMPLEX = 3,
		FONT_HERSHEY_TRIPLEX = 4,
		FONT_HERSHEY_COMPLEX_SMALL = 5,
		FONT_HERSHEY_SCRIPT_SIMPLEX = 6,
		FONT_HERSHEY_SCRIPT_COMPLEX = 7,
		FONT_ITALIC = 16
	};

	class CVLIB_DECLSPEC BitmapData
	{
		friend class Mat;
	public:
		BitmapData();
		~BitmapData();
		BITMAPINFOHEADER1*	getBitmapInfoHeader();
		uchar*				getBuffer();
		int					getEffWidth();
	private:
		uchar*	pdata;
		int		effWidth;
	};

	/**
	 * @brief matrix class
	 */
	class CVLIB_DECLSPEC Mat : public Object
	{
	public:
		union
		{
			unsigned char**	ptr;
			short** s;
			int** i;
			float** fl;
			double** db;
		} data;

	public:
		// Construction and Destruction
		Mat() { data.ptr = 0; m_rows = 0; m_cols = 0; m_depend = 0; m_step = 0; m_type = MAT_Tuchar; }
		Mat(int rows, int cols, TYPE type);
		Mat(int rows, int cols, TYPE type, const Scalar& _s);
		Mat(const Size& size, TYPE type);
		Mat(void** ptr, int rows, int cols, TYPE type);
		Mat(void* ptr, int rows, int cols, TYPE type, bool fAlloc = true);
		Mat(const Mat& mat);
		Mat(const Mat& mat, const Rect& roi);
		Mat(const Mat& mat, const Range& range, bool frows = true);
		template<typename _Tp> explicit Mat(Vector<Point2_<_Tp> >& points);
		template<typename _Tp> explicit Mat(Vector<Point3_<_Tp> >& points);
		template<typename _Tp> explicit Mat(Vector<Point4_<_Tp> >& points);
		template<typename _Tp> explicit Mat(std::vector<Point2_<_Tp> >& points);
		template<typename _Tp> explicit Mat(std::vector<Point3_<_Tp> >& points);
		template<typename _Tp> explicit Mat(std::vector<Point4_<_Tp> >& points);
		template<typename _Tp> explicit Mat(std::vector<_Tp>& values);
		template<typename _Tp> explicit Mat(const Point2_<_Tp>& point, bool copyData = true);
		template<typename _Tp> explicit Mat(const Point3_<_Tp>& point, bool copyData = true);
		template<typename _Tp> explicit Mat(const Point4_<_Tp>& point, bool copyData = true);

		virtual ~Mat();

		// creation Operations
		int		create(int rows, int cols, TYPE type);
		int		create(const Size& size, TYPE type);
		int		create(void** ptr, int rows, int cols, TYPE type);
		int		create(void* ptr, int rows, int cols, TYPE type, bool fAlloc = true);
		int		create(Vec& vec);
		int		create(const Mat& mA, bool fCopy = false);
		int		create(const BITMAPINFOHEADER1* pBI);
		int		createFromArray(uchar* pbData, int nRows, int nCols, int nBitsperpixel, int nBytesperline, bool bFlipImage = 0);
		void	release();

		int		disableRelease();
		void	enableRelease(int depend);
		void	sharedTo(Mat& a) const;

		//! Attributes
		inline TYPE	type() const { return (TYPE)CVLIB_MAT_DEPTH((int)m_type); };
		inline TYPE type1() const { return (TYPE)CVLIB_MAT_TYPE((int)m_type); }
		inline int rows() const { return m_rows; };
		inline int cols() const { return m_cols; };
		inline int total() const { return m_rows*m_cols; }
		inline int step() const { return m_step; }
		inline int channels() const { return CVLIB_MAT_CN((int)m_type); }
		inline int rowByteSize() const { return m_cols*m_step*CVLIB_MAT_CN((int)m_type); }
		inline Size size() const { return Size(m_cols, m_rows); }
		inline int elemSize() const { return CVLIB_ELEM_SIZE((int)type())*channels(); }

		template<typename _Tp> operator std::vector<_Tp>() const;
		template<typename _Tp> operator Point2_<_Tp>() const;
		template<typename _Tp> operator Point3_<_Tp>() const;
		template<typename _Tp> operator Point4_<_Tp>() const;

		Mat& operator=(const Mat& matSrc);
		Mat& operator=(const Scalar& value);
		Mat& operator=(double value);
		Mat& operator-=(const Mat& A);
		Mat& operator+=(const Mat& A);
		Mat& operator*=(double value);
		Mat& operator|=(const Mat& A);
		Mat& operator&=(const Mat& A);
		template<typename _Tp> _Tp& at(int i0, int i1);
		template<typename _Tp> const _Tp& at(int i0, int i1) const;

		//! valid check operations
		inline bool	isInside(int nX, int nY) const { return (nX < m_cols && nX >= 0 && nY < m_rows && nY >= 0); }
		inline bool isValid() const { return data.ptr != NULL; }
		inline bool isEqualSize(const Size& sz) const { return m_rows == sz.height && m_cols == sz.width; }
		inline bool isEqualSize(const Mat& mat) const { return m_rows == mat.rows() && m_cols == mat.cols(); }
		inline bool isEqualMat(const Mat& mat) const { return isEqualSize(mat) && m_type == mat.m_type; }
		inline bool isContinuous() const { return isValid() && data.ptr[1] == (data.ptr[0] + m_cols*elemSize()); }
		bool isSymmetric(void) const;

		//! import and export, conversion operations
		virtual int fromFile(const char* szFilename);
		virtual int fromFile(XFile* fp);
		virtual int toFile(const char* szFilename) const;
		virtual int toFile(XFile* fp) const;
		void	fromMatlab(const String& sFilename, const String& sName);
		void	toMatlab(const String& sFilename, const String& sName, const String& sComment = "", bool fAppend = true) const;
		bool	fromImage(const char* szFilename, ulong nImageType = 0);
#if defined(_MSC_VER)
		bool	fromImage(const wchar_t* szFilename, ulong nImageType = 0);
#endif
		bool	fromImage(XFile* fp, ulong nImageType = 0);
		bool	fromImage(uchar* buffer, int size, ulong nImageType = 0);
		bool	toImage(const char* szFilename, ulong nImageType = 1/*bmp*/) const;
		bool	toImage(const char* szFilename, ulong nImageType, bool fExpand, float rMin[]/*={0.0f}*/, float rMax[]/*={1.0f}*/) const;
		bool	toImage(const char* szFilename, ulong nImageType, bool fExpand, float rMin = 0.0f, float rMax = 1.0f) const;
#if defined(_MSC_VER)
		//bool	toImage(const wchar_t* szFilename, ulong nImageType = 1/*bmp*/) const;
		//bool	toImage(const wchar_t* szFilename, ulong nImageType, bool fExpand, float rMin[]/*={0.0f}*/, float rMax[]/*={1.0f}*/) const;
		//bool	toImage(const wchar_t* szFilename, ulong nImageType, bool fExpand, float rMin = 0.0f, float rMax = 1.0f) const;
#endif
		bool	toImage(XFile* fp, ulong nImageType = 0) const;
		void	toC(const String& szFilename, const String& szVarName, bool fAppend = true) const;
		String	toString() const;
		void	toRefVec(Vec &v) const;
		void	toVec(Vec& v) const;

		void	convert(TYPE type, CASTTYPE casttype = CT_Default);
		void	convert(TYPE type, double alpha, double beta);
		void	convertTo(Mat& dst, TYPE type, CASTTYPE casttype = CT_Default) const;
		void	convertTo(Mat& dst, TYPE type, double alpha, double beta) const;
		void	copyTo(Mat& dst, const Mat& mask = Mat()) const;
		void	reverseChannel();
		int		checkVector(int _elemChannels, int _depth = -1, bool _requireContinuous = true) const;

		//! Element Access
		double	value(int i, int j, int cn = 0) const;
		void	setValue(double rVal);
		void	setElement(int i0, int i1, const void* p);
		void	subRefMat(const Rect& region, Mat& mRef) const;
		void	subMat(const Rect& region, Mat& mRef) const;

		void	resizeRows(int r) { this->m_rows = r; }
		void	resizeCols(int c) { this->m_cols = c; }

		void	zero();
		void	identity(double val);
		void	identity();
		double	min(int cn = 0) const;
		double	max(int cn = 0) const;
		double	min(int& row, int& col) const;
		double	max(int& row, int& col) const;
		void	minMaxLoc(double* minVal, double* maxVal, Point2i* minLoc = 0, Point2i* maxLoc = 0, const Mat& mask = Mat()) const;
		double	mean() const;
		double	norm(int norm_type = NORM_L2, const Mat& mask = Mat()) const;
		double	normDiff(int norm_type, const Mat& other, const Mat& mask = Mat()) const;
		double	determinant() const;

		// Statistical functions
		Mat meanRow() const;
		Mat meanCol() const;
		double std() const;
		void stdCol(Vec& vStd) const;
		Scalar sum() const;
		Mat sumRows() const;
		void sumRows(Vec& vB) const;
		void sumCols(Vec& vB) const;
		double var() const;

		void diag(const Vec& vec);
		void diag(double rval);

		//! Matlab-style matrix initialization
		//! constructs a square diagonal matrix which main diagonal is vector "d"
		static Mat diag(const Mat& d);
		static Mat zeros(int rows, int cols, TYPE type);
		static Mat zeros(Size size, TYPE type);
		static Mat ones(int rows, int cols, TYPE type);
		static Mat ones(Size size, TYPE type);
		static Mat eye(int rows, int cols, TYPE type);
		static Mat eye(Size size, TYPE type);

		// math functions
		void abs();
		void sqr();
		void sqrt();
		void log();

		// Flips
		void flipLR();
		void flipUD();

		// Elementwise equal to
		void eq(const double B, Mat& C) const;
		void eq(const Mat& B, Mat& C) const;
		// Elementwise not equal to
		void ne(const double B, Mat& C) const;
		void ne(const Mat& B, Mat& C) const;
		// Elementwise less than
		void lt(const double B, Mat& C) const;
		void lt(const Mat& B, Mat& C) const;
		// Elementwise less than or equal
		void le(const double B, Mat& C) const;
		void le(const Mat& B, Mat& C) const;
		// Elementwise greater than
		void gt(const double B, Mat& C) const;
		void gt(const Mat& B, Mat& C) const;
		// Elementwise greater than or equal
		void ge(const double B, Mat& C) const;
		void ge(const Mat& B, Mat& C) const;

		// determinant, trace
		double trace() const;
		Mat inverted(void) const;
		Mat& invert(void);
		Mat transposed(void) const;
		Mat& transpose(void);
		Mat sqrtm() const;
		int countNonZero() const;

		void swap(Mat& other);

		// Simple Drawing Functions
		COLOR	getPixelColor(int x, int y) const;
		void	setPixelColor(int x, int y, const COLOR& cr, bool fSetAlpha = false);
		void	setPixelColor(int x, int y, float* color);
		void	drawMat(const Mat& img, const Point2i& pt = Point2i(), const float rOpacity = 1);
		void	drawRect(const Rect& rect, COLOR color, int thick = 1, const float rOpacity = 1);
		void	drawRect(const RotatedRect& rect, COLOR color, int thickness = 1, const float rOpacity = 1);
		void	drawCircle(const Point2i& center, int radius, COLOR color, int thickness = 1, const float rOpacity = 1);
		void	drawEllipse(const Rect& rect, COLOR color, int thickness = 1, const float rOpacity = 1);
		void	drawEllipse(const RotatedRect& rect, COLOR color, int thickness = 1, const float rOpacity = 1);
		void	drawPixel(const Point2i& point, COLOR color, const float rOpacity = 1);
		void	drawPixel(int x, int y, COLOR color, const float rOpacity = 1);
		void	drawLine(const Point2i& pt1, const Point2i& pt2, COLOR color, int thickness = 1, const float rOpacity = 1);
		void	drawCross(const Point2i& pt, int nSide, COLOR color, int thickness = 1, const float rOpacity = 1);
		void	drawPolygon(const Point2i* pts, int num, COLOR color, int thickness = 1, bool open = true, const float rOpacity = 1);
		void	drawString(const char* szText, const Point2i& pt, int fontFace, double fontScale,
			COLOR color, int thickness = 1, int lineType = 8, bool bottomLeftOrigin = false);
		void	fillConvexPoly(const Point2i* pts, int npts, COLOR color, float rOpacity = 1.0f);

		void	getBMPInfoHeader(BitmapData& data) const;
#if _MSC_VER
		//#if CVLIB_OS==CVLIB_OS_WIN32
		void	drawTo(Handle hDC, int nDstX, int nDstY, int nDstCX, int nDstCY) const;
		void	drawTo(Handle hDC, int nDstX, int nDstY, int nDstCX, int nDstCY,
			int nSrcX, int nSrcY, int nSrcCX, int nSrcCY) const;
		void	drawTo(Handle hDC) const;
		int		drawString(Handle hdc, long x, long y, const char* szText, COLOR color, const char* szFont,
			long lSize, long lWeight, uchar bItalic, uchar bUnderline, const float rOpacity = 1);
#if defined(_MSC_VER)
		int		drawString(Handle hdc, long x, long y, const wchar_t* szText, COLOR color, const wchar_t* szFont,
			long lSize, long lWeight, uchar bItalic, uchar bUnderline, const float rOpacity = 1);
#endif
		int		createFromHBITMAP(Handle hBitmap, Handle hPalette = 0);
		Handle	makeBitmap(Handle hdc = 0, bool bTransparency = false) const;
		bool	loadResource(Handle hRes, ulong imagetype, Handle hModule = NULL);
		bool	tile(Handle hdc, const Rect& rc);
		bool	createFromHANDLE(Handle hMem);
		Handle	copyToHandle() const;
#endif

	protected:
		TYPE	m_type;
		int		m_rows;
		int		m_cols;
		int		m_step;
		int		m_depend;
	};

	template <typename _Tp>class CVLIB_DECLSPEC Mat_ : public Mat
	{
	public:
		typedef Mat_<_Tp> _MyMat;
		typedef Vec_<_Tp> _MyVec;
		Mat_() {};
		Mat_(int rows, int cols);
		Mat_(_Tp** ptr, int rows, int cols);
		Mat_(_Tp* ptr, int rows, int cols, bool fAlloc = true);
		Mat_(const Mat& mat);
		Mat_(const _MyMat& mat);
		int		create(int rows, int cols);
		int		create(_Tp** ptr, int rows, int cols);
		int		create(_Tp* ptr, int rows, int cols, bool fAlloc = true);
		int		create(const _MyMat& mat, bool fCopy = false);

		_MyMat&	operator=(const _MyMat& other);
		_Tp*	operator[](int index) { return (_Tp*)data.ptr[index]; }
		const _Tp*	operator[](int index) const { return (const _Tp*)data.ptr[index]; }
		_MyMat& operator=(double value);
		bool operator==(const _MyMat& refmatrix) const;
		bool operator!=(const _MyMat& refmatrix) const;
		bool operator<(const _MyMat& refmatrix) const;

		_MyMat operator+(const _MyMat& A) const;

		_MyMat operator-(void) const;
		_MyMat operator-(const _MyMat& A) const;

		_MyMat operator*(double dbl) const;
		_MyMat operator*(const _MyMat& refmatrix) const;
		_MyVec operator*(const _MyVec& refvector) const;

		_MyMat operator/(double dbl) const;

		_MyMat& operator+=(const _MyMat& A);
		_MyMat& operator-=(const _MyMat& A);
		_MyMat& operator*=(double dbl);
		_MyMat& operator*=(const _MyMat& A);
		_MyMat& operator/=(double dbl);

		_MyMat& eqSum(const _MyMat &refmatrixA, const _MyMat &refmatrixB);
		_MyMat& eqDiff(const _MyMat &refmatrixA, const _MyMat &refmatrixB);
		_MyMat& eqProd(const _MyMat &refmatrixA, const _MyMat &refmatrixB);

		_MyMat transposed(void) const;
		_MyMat& transpose(void);

		_MyVec row(int r) const;
		_MyVec column(int c) const;
		void col(int i, _MyVec& vCol) const;
		void row(int i, _MyVec& vRow) const;
		void setRow(int r, const _MyVec &v);
		void setColumn(int c, const _MyVec &v);

		double det() const;
		_MyMat& submatrix(const int newRows, const int newCols, _MyMat& submat, const int i0, const int j0) const;
		_MyMat submatrix(const int newRows, const int newCols, const int i0 = 0, const int j0 = 0) const;
	};

}

#include "xMat.h"

namespace cvlib
{
	typedef Mat_<uchar>		Matb;
	typedef Mat_<short>		Mats;
	typedef Mat_<int>		Mati;
	typedef Mat_<float>		Matf;
	typedef Mat_<double>	Matd;


	CVLIB_DECLSPEC Mat operator + (const Mat& a, const Mat& b);
	CVLIB_DECLSPEC Mat operator + (const Mat& a, const Scalar& s);
	CVLIB_DECLSPEC Mat operator + (const Scalar& s, const Mat& a);

	CVLIB_DECLSPEC Mat operator - (const Mat& a, const Mat& b);
	CVLIB_DECLSPEC Mat operator - (const Mat& a, const Scalar& s);
	CVLIB_DECLSPEC Mat operator - (const Scalar& s, const Mat& a);

	CVLIB_DECLSPEC Mat operator - (const Mat& m);

	CVLIB_DECLSPEC Mat operator * (const Mat& a, const Mat& b);
	CVLIB_DECLSPEC Mat operator * (const Mat& a, double s);
	CVLIB_DECLSPEC Mat operator * (double s, const Mat& a);

	CVLIB_DECLSPEC Mat operator / (const Mat& a, double s);
	CVLIB_DECLSPEC Mat operator / (double s, const Mat& a);

	CVLIB_DECLSPEC Mat operator < (const Mat& a, const Mat& b);
	CVLIB_DECLSPEC Mat operator < (const Mat& a, double s);
	CVLIB_DECLSPEC Mat operator < (double s, const Mat& a);

	CVLIB_DECLSPEC Mat operator <= (const Mat& a, const Mat& b);
	CVLIB_DECLSPEC Mat operator <= (const Mat& a, double s);
	CVLIB_DECLSPEC Mat operator <= (double s, const Mat& a);

	CVLIB_DECLSPEC Mat operator == (const Mat& a, const Mat& b);
	CVLIB_DECLSPEC Mat operator == (const Mat& a, double s);
	CVLIB_DECLSPEC Mat operator == (double s, const Mat& a);

	CVLIB_DECLSPEC Mat operator != (const Mat& a, const Mat& b);
	CVLIB_DECLSPEC Mat operator != (const Mat& a, double s);
	CVLIB_DECLSPEC Mat operator != (double s, const Mat& a);

	CVLIB_DECLSPEC Mat operator >= (const Mat& a, const Mat& b);
	CVLIB_DECLSPEC Mat operator >= (const Mat& a, double s);
	CVLIB_DECLSPEC Mat operator >= (double s, const Mat& a);

	CVLIB_DECLSPEC Mat operator > (const Mat& a, const Mat& b);
	CVLIB_DECLSPEC Mat operator > (const Mat& a, double s);
	CVLIB_DECLSPEC Mat operator > (double s, const Mat& a);

	CVLIB_DECLSPEC Mat operator & (const Mat& a, const Mat& b);
	CVLIB_DECLSPEC Mat operator & (const Mat& a, const Scalar& s);
	CVLIB_DECLSPEC Mat operator & (const Scalar& s, const Mat& a);

	CVLIB_DECLSPEC Mat operator | (const Mat& a, const Mat& b);
	CVLIB_DECLSPEC Mat operator | (const Mat& a, const Scalar& s);
	CVLIB_DECLSPEC Mat operator | (const Scalar& s, const Mat& a);

	CVLIB_DECLSPEC Mat operator ^ (const Mat& a, const Mat& b);
	CVLIB_DECLSPEC Mat operator ^ (const Mat& a, const Scalar& s);
	CVLIB_DECLSPEC Mat operator ^ (const Scalar& s, const Mat& a);

	CVLIB_DECLSPEC Mat operator ~(const Mat& m);

	static inline size_t getElemSize(int type) { return CVLIB_ELEM_SIZE(type); }

	inline Size getContinuousSize(const Mat& m1, int widthScale = 1)
	{
		int rows = m1.rows();
		int cols = m1.cols()*widthScale;
		int64 sz = (int64)rows*cols;
		return m1.isContinuous() ? Size((int)sz, 1) : Size(cols, rows);
	}

	CVLIB_DECLSPEC void mixChannels(const Mat* src, size_t nsrcs, Mat* dst, size_t ndsts, const int* fromTo, size_t npairs);

}
