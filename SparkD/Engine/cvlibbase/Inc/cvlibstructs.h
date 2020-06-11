/*! 
 * \file    cvlibstructs.h
 * \ingroup base
 * \brief   cvlibbase.
 * \author  
 */
#pragma once

#include "cvlibmacros.h"
#include "XFile.h"
#if defined _MSC_VER || (defined __GNUC__ && defined __SSE2__&& !defined __APPLE__)
#include <complex>
#endif

#pragma warning (push)
#pragma warning (disable : 4201)

namespace cvlib
{

#undef abs
#undef Complex

	class CVLIB_DECLSPEC TermCriteria
	{
	public:
		enum
		{
			COUNT = 1, //!< the maximum number of iterations or elements to compute
			MAX_ITER = COUNT, //!< ditto
			EPS = 2 //!< the desired accuracy or change in parameters at which the iterative algorithm stops
		};
		int		nType;		/**< */
		int		nMaxIter;	/**< */
		double	rEpsilon;	/**< */
		TermCriteria() :nType(TermCriteria::MAX_ITER), nMaxIter(0), rEpsilon(0) {}
		TermCriteria(int type, int iter, double epsilon) :nType(type), nMaxIter(iter), rEpsilon(epsilon) {}
		TermCriteria(const TermCriteria& other) :nType(other.nType), nMaxIter(other.nMaxIter), rEpsilon(other.rEpsilon) {}
		TermCriteria& operator=(const TermCriteria& other) { nType = other.nType; nMaxIter = other.nMaxIter; rEpsilon = other.rEpsilon; return *this; }

		TermCriteria check(double default_eps, int default_max_iters) const;
	};

#define CVLIB_TITLE_LEN		64
#define CVLIB_COPYRIGHT_LEN		64
	typedef struct tagLibraryInfo
	{
		char szTitle[CVLIB_TITLE_LEN];         /**< */
		char szCopyright[CVLIB_COPYRIGHT_LEN]; /**< */
		int nMajor;                            /**< */
		int nMinor;                            /**< */
		int nBuild;                            /**< */
	} SLibraryInfo;

	/**
	 * @brief  BMP
	 */
	typedef struct tagBITMAPINFOHEADER1
	{
		uint      biSize;
		int       biWidth;
		int       biHeight;
		ushort    biPlanes;
		ushort		biBitCount;
		uint      biCompression;
		uint      biSizeImage;
		int       biXPelsPerMeter;
		int       biYPelsPerMeter;
		uint      biClrUsed;
		uint      biClrImportant;
	} BITMAPINFOHEADER1;

	template <class TypeName> class Point3_;
	template <class TypeName> class Point4_;

	template <class TypeName>
	class CVLIB_DECLSPEC Point2_
	{
	public:
		typedef Point2_<TypeName> _MyPoint;

		union {
			struct {
				TypeName x, y;        // standard names for components
			};
			TypeName vec_array[2];     // array access
		};
	public:
		Point2_() :x(0), y(0) {}
		Point2_(const TypeName* xy) :x(xy[0]), y(xy[1]) {}
		Point2_(TypeName const x_, TypeName const y_) :x(x_), y(y_) {}
		Point2_(_MyPoint const &p) :x(p.x), y(p.y) {}
		Point2_(Point3_<TypeName> const &p) { TypeName k = TypeName(1 / p.z); x = k*p.x; y = k*p.y; }

		template<typename T2> inline operator Point2_<T2>() const { return Point2_<T2>((T2)x, (T2)y); }

		inline _MyPoint& operator= (const _MyPoint& a) { x = a.x; y = a.y; return *this; }
		inline _MyPoint operator- ()        const { return _MyPoint(-x, -y); }
		inline _MyPoint operator+ (const _MyPoint& a) const { return _MyPoint(x + a.x, y + a.y); }
		inline _MyPoint operator- (const _MyPoint& a) const { return _MyPoint(x - a.x, y - a.y); }
		inline _MyPoint operator* (const double& lambda) const { return _MyPoint((TypeName)(x*lambda), (TypeName)(y*lambda)); }
		inline _MyPoint operator* (const _MyPoint &a) const { return _MyPoint((TypeName)(x*a.x), (TypeName)(y*a.y)); }
		inline bool  operator< (const _MyPoint& a) const { return (x < a.x) && (y < a.y); }
		inline bool  operator<=(const _MyPoint& a) const { return (x <= a.x) && (y <= a.y); }
		inline bool  operator> (const _MyPoint& a) const { return (x > a.x) && (y > a.y); }
		inline bool  operator>=(const _MyPoint& a) const { return (x >= a.x) && (y >= a.y); }
		inline bool  operator==(const _MyPoint& a) const { return (x == a.x) && (y == a.y); }
		inline bool  operator!=(const _MyPoint& a) const { return (x != a.x) || (y != a.y); }
		inline void operator+=(const _MyPoint a) { x += a.x; y += a.y; }
		inline void operator-=(const _MyPoint a) { x -= a.x; y -= a.y; }
		inline _MyPoint& operator*=(const double& lambda) { x = (TypeName)(x*lambda); y = (TypeName)(y*lambda); return *this; }

		inline TypeName& operator[](int i) { return vec_array[i]; }
		inline const TypeName& operator[](int i) const { return vec_array[i]; }
		inline double distTo(const _MyPoint& o) const { return sqrt(double((x - o.x)*(x - o.x) + (y - o.y)*(y - o.y))); }
		inline double distTo(TypeName const x_, TypeName const y_) const { return sqrt(double((x - x_)*(x - x_) + (y - y_)*(y - y_))); }
		inline TypeName dot(const _MyPoint& pt) const { return (TypeName)(x*pt.x + y*pt.y); }
		inline double cross(const _MyPoint& pt) const { return (double)x*pt.y - (double)y*pt.x; }
		inline float norm() const { return sqrtf(sq_norm()); }
		inline float sq_norm() const { return (float)(x*x + y*y); }
		inline bool fromFile(XFile* pFile) { return pFile->read(vec_array, sizeof(vec_array), 1) == 1; }
		inline bool toFile(XFile* pFile) const { return pFile->write(vec_array, sizeof(vec_array), 1) == 1; }
	};

	template <class TypeName>
	inline Point2_<TypeName> operator+(const Point2_<TypeName>& u, const Point2_<TypeName>& v)
	{
		return Point2_<TypeName>(u.x + v.x, u.y + v.y);
	}

	template <class TypeName>
	inline Point2_<TypeName> operator-(const Point2_<TypeName>& u, const Point2_<TypeName>& v)
	{
		return Point2_<TypeName>(u.x - v.x, u.y - v.y);
	}

	template <class TypeName>
	inline Point2_<TypeName> operator*(const float s, const Point2_<TypeName>& u)
	{
		return Point2_<TypeName>(s * u.x, s * u.y);
	}

	template <class TypeName>
	inline Point2_<TypeName> operator/(const Point2_<TypeName>& u, const float s)
	{
		return Point2_<TypeName>(TypeName(u.x / s), TypeName(u.y / s));
	}

	template <class TypeName>
	inline Point2_<TypeName> operator*(const Point2_<TypeName>&u, const Point2_<TypeName>&v)
	{
		return Point2_<TypeName>(u.x * v.x, u.y * v.y);
	}

	/**
	* @brief
	*/
	template <class TypeName>
	class CVLIB_DECLSPEC Size_
	{
	public:
		typedef Size_<TypeName> _MySize;
	public:
		TypeName width;
		TypeName height;


		Size_() { width = height = 0; }
		Size_(TypeName w, TypeName h) :width(w), height(h) {}
		Size_(const _MySize& other) :width(other.width), height(other.height) {}
		Size_(const Point2_<TypeName>& pt) { width = pt.x, height = pt.y; }

		inline _MySize& operator=(const _MySize& other) { width = other.width; height = other.height; return *this; }
		inline bool operator==(const Size_& o) const { return width == o.width && height == o.height; }
		inline bool operator!=(const Size_& o) const { return width != o.width || height != o.height; }
		inline _MySize operator*(float s) const { return _MySize((TypeName)(width*s), (TypeName)(height*s)); }
		inline _MySize& operator*=(float s) { width = (TypeName)(width*s), height = (TypeName)(height*s); return *this; }
		inline TypeName area() const { return width*height; }
	};

	//***bd*** simple floating point point
	template <class TypeName>
	class CVLIB_DECLSPEC Point3_
	{
	public:
		typedef Point3_<TypeName> _MyPoint3;

		union {
			struct {
				TypeName x, y, z;          // standard names for components
			};
			struct {
				TypeName r, g, b;          // standard names for components
			};
			TypeName vec_array[3];     // array access
		};
	public:
		Point3_() :x(0), y(0), z(0) {}
		Point3_(const TypeName *xyz) :x(xyz[0]), y(xyz[1]), z(xyz[2]) {}
		Point3_(TypeName const x_, TypeName const y_, TypeName const z_) :x(x_), y(y_), z(z_) {}
		Point3_(Point2_<TypeName> const &p) :x(p.x), y(p.y), z((TypeName)1.0) {}
		Point3_(_MyPoint3 const &p) :x(p.x), y(p.y), z(p.z) {}
		Point3_(Point4_<TypeName> const &p) :x(p.x), y(p.y), z(p.z) {}
		Point3_(ulong color) { r = (TypeName)getRValue(color), g = (TypeName)getGValue(color), b = (TypeName)getBValue(color); }

		template<typename T2> inline operator Point3_<T2>() const { return Point3_<T2>((T2)x, (T2)y, (T2)z); }

		inline _MyPoint3& operator= (const _MyPoint3& a) { x = a.x; y = a.y; z = a.z; return *this; }
		inline _MyPoint3 operator- ()       const { return _MyPoint3(-x, -y, -z); }
		inline _MyPoint3 operator+ (const _MyPoint3& a) const { return _MyPoint3(x + a.x, y + a.y, z + a.z); }
		inline _MyPoint3 operator- (const _MyPoint3& a) const { return _MyPoint3(x - a.x, y - a.y, z - a.z); }
		inline _MyPoint3 operator* (const double a)const { return _MyPoint3((TypeName)(a*x), (TypeName)(a*y), (TypeName)(a*z)); }
		inline _MyPoint3 operator* (const _MyPoint3& a) const { return _MyPoint3((TypeName)(a.x*x), (TypeName)(a.y*y), (TypeName)(a.z*z)); }
		inline _MyPoint3 operator^ (const _MyPoint3& a)const { return _MyPoint3(y*a.z - z*a.y, z*a.x - x*a.z, x*a.y - y*a.x); }
		inline bool  operator< (const _MyPoint3& a) const { return (x < a.x) && (y < a.y) && (z < a.z); }
		inline bool  operator<=(const _MyPoint3& a) const { return (x <= a.x) && (y <= a.y) && (z <= a.z); }
		inline bool  operator> (const _MyPoint3& a) const { return (x > a.x) && (y > a.y) && (z > a.z); }
		inline bool  operator>=(const _MyPoint3& a) const { return (x >= a.x) && (y >= a.y) && (z >= a.z); }
		inline bool  operator==(const _MyPoint3& a) const { return (x == a.x) && (y == a.y) && (z == a.z); }
		inline bool  operator!=(const _MyPoint3& a) const { return (x != a.x) || (y != a.y) || (z != a.z); }
		inline _MyPoint3& operator+=(const _MyPoint3& a) { x += a.x; y += a.y; z += a.z; return *this; }
		inline _MyPoint3& operator-=(const _MyPoint3& a) { x -= a.x; y -= a.y; z -= a.z; return *this; }
		inline _MyPoint3& operator*=(const double& lambda) { x = (TypeName)(x*lambda); y = (TypeName)(y*lambda); z = (TypeName)(z*lambda); return *this; }
		inline _MyPoint3& operator/=(const double& lambda) { return operator*=((TypeName)(1.0 / lambda)); }

		inline TypeName& operator[](int i) { return vec_array[i]; }
		inline const TypeName& operator[](int i) const { return vec_array[i]; }
		inline double distTo(const _MyPoint3& o) const { return sqrt(double((x - o.x)*(x - o.x) + (y - o.y)*(y - o.y) + (z - o.z)*(z - o.z))); }
		inline double distTo(TypeName const x_, TypeName const y_, TypeName const z_) const { return sqrt(double((x - x_)*(x - x_) + (y - y_)*(y - y_) + (z - z_)*(z - z_))); }
		inline TypeName dot(const _MyPoint3& a) const { return x*a.x + y*a.y + z*a.z; }
		inline _MyPoint3 cross(const _MyPoint3& pt) const { return _MyPoint3(y*pt.z - z*pt.y, z*pt.x - x*pt.z, x*pt.y - y*pt.x); }
		inline TypeName normalize() {
			double norm = sqrt((double)(x * x + y * y + z * z));
			if (norm > 10e-6)
				norm = (1.0 / norm);
			else
				norm = 0.0;
			x = (TypeName)(x*norm);
			y = (TypeName)(y*norm);
			z = (TypeName)(z*norm);
			return (TypeName)norm;
		}
		inline TypeName sq_norm() const { return (TypeName)(x * x + y * y + z * z); }
		inline TypeName norm() const { return (TypeName)sqrt(double(x * x + y * y + z * z)); }
		inline bool fromFile(XFile* pFile) { return pFile->read(vec_array, sizeof(vec_array), 1) == 1; }
		inline bool toFile(XFile* pFile) const { return pFile->write(vec_array, sizeof(vec_array), 1) == 1; }
	};

	template <class TypeName>
	inline Point3_<TypeName> operator+(const Point3_<TypeName>& u, const Point3_<TypeName>& v)
	{
		return Point3_<TypeName>(u.x + v.x, u.y + v.y, u.z + v.z);
	}

	template <class TypeName>
	inline Point3_<TypeName> operator-(const Point3_<TypeName>& u, const Point3_<TypeName>& v)
	{
		return Point3_<TypeName>(u.x - v.x, u.y - v.y, u.z - v.z);
	}

	template <class TypeName>
	inline Point3_<TypeName> operator^(const Point3_<TypeName>& u, const Point3_<TypeName>& v)
	{
		return Point3_<TypeName>(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
	}

	template <class TypeName>
	inline Point3_<TypeName> operator*(const float s, const Point3_<TypeName>& u)
	{
		return Point3_<TypeName>(s * u.x, s * u.y, s * u.z);
	}

	template <class TypeName>
	inline Point3_<TypeName> operator/(const Point3_<TypeName>& u, const float s)
	{
		return Point3_<TypeName>(u.x / s, u.y / s, u.z / s);
	}

	template <class TypeName>
	inline Point3_<TypeName> operator*(const Point3_<TypeName>& u, const Point3_<TypeName>& v)
	{
		return Point3_<TypeName>(u.x * v.x, u.y * v.y, u.z * v.z);
	}

	template <class TypeName>
	class CVLIB_DECLSPEC Point4_
	{
	public:
		typedef Point4_<TypeName> _MyPoint4;

		union {
			struct {
				TypeName x, y, z, w;          // standard names for components
			};
			struct {
				TypeName r, g, b, a;          // standard names for components
			};
			TypeName vec_array[4];     // array access
		};
	public:
		inline Point4_() { x = y = z = w = 0; }
		inline Point4_(const TypeName* xyzw) :x(xyzw[0]), y(xyzw[1]), z(xyzw[2]), w(xyzw[3]) {}
		inline Point4_(TypeName const x_) : x(x_), y(x_), z(x_), w(x_) {}
		inline Point4_(TypeName const x_, TypeName const y_, TypeName const z_, TypeName const w_ = 0) : x(x_), y(y_), z(z_), w(w_) {}
		inline Point4_(Point3_<TypeName> const &p) : x(p.x), y(p.y), z(p.z), w(TypeName(1.0)) {}
		inline Point4_(_MyPoint4 const &p) : x(p.x), y(p.y), z(p.z), w(p.w) {}

		inline static _MyPoint4 All(TypeName v0) { return _MyPoint4(v0, v0, v0, v0); }
		template<typename T2> inline operator Point4_<T2>() const { return Point4_<T2>((T2)x, (T2)y, (T2)z, (T2)w); }
		inline double distTo(const _MyPoint4& o) const { return sqrt(double((x - o.x)*(x - o.x) + (y - o.y)*(y - o.y) + (z - o.z)*(z - o.z)) + (w - o.w)*(w - o.w)); }

		inline _MyPoint4& operator= (const _MyPoint4& t) { x = t.x; y = t.y; z = t.z; w = t.w; return *this; }
		inline _MyPoint4 operator- ()       const { return _MyPoint4(-x, -y, -z, -w); }
		inline _MyPoint4 operator+ (const _MyPoint4& t) const { return _MyPoint4(x + t.x, y + t.y, z + t.z, w + t.w); }
		inline _MyPoint4 operator+ (const TypeName& t) const { return _MyPoint4(x + t, y + t, z + t, w + t); }
		inline _MyPoint4 operator- (const _MyPoint4& t) const { return _MyPoint4(x - t.x, y - t.y, z - t.z, w - t.w); }
		inline _MyPoint4 operator* (const _MyPoint4& t) const { return _MyPoint4(x*t.x, y*t.y, z*t.z, w*t.w); }
		inline bool  operator==(const _MyPoint4& t) const { return (x == t.x) && (y == t.y) && (z == t.z) && (w == t.w); }
		inline bool  operator!=(const _MyPoint4& t) const { return (x != t.x) || (y != t.y) || (z != t.z) || (w != t.w); }
		inline _MyPoint4& operator+=(const _MyPoint4& t) { x += t.x; y += t.y; z += t.z; w += t.w; return *this; }
		inline _MyPoint4& operator-=(const _MyPoint4& t) { x -= t.x; y -= t.y; z -= t.z; w -= t.w; return *this; }
		inline _MyPoint4& operator*=(const double& lambda) { x = (TypeName)(x*lambda); y = (TypeName)(y*lambda); z = (TypeName)(z*lambda); w = (TypeName)(w*lambda); return *this; }
		inline _MyPoint4& operator/=(const double& lambda) { return operator*=((TypeName)(1.0 / lambda)); }

		inline TypeName& operator[](int i) { return vec_array[i]; }
		inline const TypeName& operator[](int i) const { return vec_array[i]; }
		inline TypeName dot(const _MyPoint4& pt) const { return (TypeName)(x*pt.x + y*pt.y + z*pt.z + w*pt.w); }
		template<typename T2> inline void convertTo(T2* buf, int cn, int unroll_to) const {
			int i;
			assert(cn <= 4);
			for (i = 0; i < cn; i++)
				buf[i] = (T2)(this->vec_array[i]);
			for (; i < unroll_to; i++)
				buf[i] = buf[i - cn];
		}
		_MyPoint4 mul(const _MyPoint4& t, double scale = 1) const {
			return _MyPoint4((TypeName)(this->vec_array[0] * t.vec_array[0] * scale),
				(TypeName)(this->vec_array[1] * t.vec_array[1] * scale),
				(TypeName)(this->vec_array[2] * t.vec_array[2] * scale),
				(TypeName)(this->vec_array[3] * t.vec_array[3] * scale));
		}
		inline bool fromFile(XFile* pFile) { return pFile->read(vec_array, sizeof(vec_array), 1) == 1; }
		inline bool toFile(XFile* pFile) const { return pFile->write(vec_array, sizeof(vec_array), 1) == 1; }
	};

	template <class TypeName>
	inline Point4_<TypeName> operator+(const Point4_<TypeName>& u, const Point4_<TypeName>& v)
	{
		return Point4_<TypeName>(u.x + v.x, u.y + v.y, u.z + v.z, u.w + v.w);
	}

	template <class TypeName>
	inline Point4_<TypeName> operator-(const Point4_<TypeName>& u, const Point4_<TypeName>& v)
	{
		return Point4_<TypeName>(u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w);
	}

	template <class TypeName>
	inline Point4_<TypeName> operator*(const float s, const Point4_<TypeName>& u)
	{
		return Point4_<TypeName>(s * u.x, s * u.y, s * u.z, s * u.w);
	}
	template <class TypeName>
	inline Point4_<TypeName> operator*(const Point4_<TypeName>& u, const float s)
	{
		return Point4_<TypeName>(s * u.x, s * u.y, s * u.z, s * u.w);
	}
	template <class TypeName>
	inline Point4_<TypeName> operator/(const Point4_<TypeName>& u, const float s)
	{
		return Point4_<TypeName>(u.x / s, u.y / s, u.z / s, u.w / s);
	}

	template <class TypeName>
	inline Point4_<TypeName> operator*(const Point4_<TypeName>& u, const Point4_<TypeName>& v)
	{
		return Point4_<TypeName>(u.x * v.x, u.y * v.y, u.z * v.z, u.w * v.w);
	}

	template <class TypeName> class CVLIB_DECLSPEC Rect_
	{
	public:
		typedef Rect_<TypeName> _MyRect;
	public:
		TypeName x, y, width, height;

		Rect_() { x = y = width = height = 0; }
		Rect_(TypeName x_, TypeName y_, TypeName w, TypeName h) :x(x_), y(y_), width(w), height(h) {}
		Rect_(const Point2_<TypeName>& center, const Size_<TypeName >& sz) {
			x = center.x - sz.width / 2, y = center.y - sz.height / 2;
			width = sz.width, height = sz.height;
		}
		Rect_(const _MyRect& other) :x(other.x), y(other.y), width(other.width), height(other.height) {}
		Rect_(const Point2_<TypeName>& pt1, const Point2_<TypeName>& pt2) {
			x = MIN(pt1.x, pt2.x);
			y = MIN(pt1.y, pt2.y);
			width = MAX(pt1.x, pt2.x) - x;
			height = MAX(pt1.y, pt2.y) - y;
		}

		template<typename T2> inline operator Rect_<T2>() const { return Rect_<T2>((T2)x, (T2)y, (T2)width, (T2)height); }

		inline _MyRect& operator=(const _MyRect& other) { x = other.x; y = other.y; width = other.width; height = other.height; return *this; }
		inline bool operator==(const _MyRect& t) { return x == t.x&& y == t.y&&width == t.width&&height == t.height; }
		inline Point2_<TypeName> tl() const { return Point2_<TypeName>(x, y); }
		inline Point2_<TypeName> br() const { return Point2_<TypeName>(x + width, y + height); }
		inline Point2_<int> centeri() const { return Point2_<int>((int)(x + width / 2.0f), (int)(y + height / 2.0f)); }
		inline Point2_<float> centerf() const { return Point2_<float>(x + width / 2.0f, y + height / 2.0f); }
		inline Point2_<TypeName> center() const { return Point2_<TypeName>((TypeName)(x + width / 2.0f), (TypeName)(y + height / 2.0f)); }
		inline TypeName limx() const { return x + width; }
		inline TypeName limy() const { return y + height; }
		inline Size_<TypeName> size() const { return Size_<TypeName>(width, height); }
		inline void setSize(const Size_<TypeName>& sz) { width = sz.width; height = sz.height; }
		inline bool PtInRect(const Point2_<TypeName>& pt) const { return pt.x >= x&&pt.y >= y&&pt.x < x + width&&pt.y < y + height; }
		inline _MyRect scale(float sx, float sy) const { return _MyRect((TypeName)(x*sx), (TypeName)(y*sy), (TypeName)(width*sx), (TypeName)(height*sy)); }
		inline TypeName area() const { return width*height; }
		inline void inflateRect(TypeName a, TypeName b) { x -= a, y -= b, width += (TypeName)(a * 2), height += (TypeName)(b * 2); }
		inline _MyRect intersectRect(const _MyRect& r2) const {
			TypeName w = width + x, h = height + y;
			TypeName w2 = r2.width + r2.x, h2 = r2.height + r2.y;
			_MyRect r;
			r.x = MAX(x, r2.x); r.y = MAX(y, r2.y);
			r.width = MIN(w, w2); r.height = MIN(h, h2);
			if (r.x <= r.width && r.y <= r.height)
			{
				r.width -= r.x;
				r.height -= r.y;
				return r;
			}
			else
			{
				return _MyRect();
			}
		}
		inline _MyRect unionRect(const _MyRect& r) const {
			TypeName brx = limx(), bry = limy();
			TypeName brx_ = r.limx(), bry_ = r.limy();
			TypeName ux1 = MIN(x, r.x), uy1 = MIN(y, r.y);
			TypeName ux2 = MAX(brx, brx_), uy2 = MAX(bry, bry_);
			return _MyRect(ux1, uy1, ux2 - ux1, uy2 - uy1);
		}
		bool fromFile(XFile* pFile) {
			if (pFile->read(&x, sizeof(x), 1) != 1) return false;
			if (pFile->read(&y, sizeof(y), 1) != 1) return false;
			if (pFile->read(&width, sizeof(width), 1) != 1) return false;
			if (pFile->read(&height, sizeof(height), 1) != 1) return false;
			return true;
		}
		bool toFile(XFile* pFile)const {
			if (pFile->write(&x, sizeof(x), 1) != 1) return false;
			if (pFile->write(&y, sizeof(y), 1) != 1) return false;
			if (pFile->write(&width, sizeof(width), 1) != 1) return false;
			if (pFile->write(&height, sizeof(height), 1) != 1) return false;
			return true;
		}
	};

	typedef Rect_<int>		Rect;
	typedef Rect_<float>	Rectf;

	typedef Size_<int>		Size;
	typedef Size_<float>	Sizef;

	typedef Point2_<int> Point2i;
	typedef Point2i Point;
	typedef Point2_<float> Point2f;
	typedef Point2_<double> Point2d;

	typedef Point3_<int> Point3i;
	typedef Point3_<float> Point3f;
	typedef Point3_<double> Point3d;

	typedef Point2i vec2i;
	typedef Point2f vec2f;
	typedef Point2d vec2d;
	typedef Point3_<uchar> vec3b;
	typedef Point3i vec3i;
	typedef Point3f vec3f;
	typedef Point3d vec3d;

	typedef Point4_<uchar> vec4b;
	typedef Point4_<int> vec4i;
	typedef Point4_<float> vec4f;
	typedef Point4_<double> vec4d;

	typedef Point2f vec2;
	typedef Point3f vec3;
	typedef Point4_<float> vec4;
	typedef Point4_<double>	Scalar;
	typedef vec4b	COLOR;
#define Scalar_ Point4_

	class CVLIB_DECLSPEC RotatedRect
	{
	public:
		//! various constructors
		RotatedRect();
		RotatedRect(const Point2f& _center, const Sizef& _size, float _angle);
		RotatedRect(const RotatedRect& box);
		void points(Point2f pts[]) const;//! returns 4 vertices of the rectangle
		Rect boundingRect() const;//! returns the minimal up-right rectangle containing the rotated rectangle

		Point2f center; //< the rectangle mass center
		Sizef size;    //< width and height of the rectangle
		float angle;    //< the rotation angle. When the angle is 0, 90, 180, 270 etc., the rectangle becomes an up-right rectangle. 
	};

	template<typename _Tp> class CVLIB_DECLSPEC DataDepth { public: enum { value = -1 }; };

	template<> class DataDepth<bool> { public: enum { value = MAT_Tbyte }; };
	template<> class DataDepth<char> { public: enum { value = MAT_Tchar }; };
	template<> class DataDepth<uchar> { public: enum { value = MAT_Tbyte }; };
	template<> class DataDepth<short> { public: enum { value = MAT_Tshort }; };
	template<> class DataDepth<int> { public: enum { value = MAT_Tint }; };
	template<> class DataDepth<float> { public: enum { value = MAT_Tfloat }; };
	template<> class DataDepth<double> { public: enum { value = MAT_Tdouble }; };
	template<typename _Tp> class DataDepth<_Tp*> { public: enum { value = MAT_USRTYPE }; };

	typedef union CVLib32suf
	{
		int i;
		unsigned u;
		float f;
	} CVLib32suf;

	struct ValuePair
	{
		int i;
		float val;
	};

	class CVLIB_DECLSPEC Slice
	{
	public:
		int  start_index, end_index;
		Slice() { start_index = 0, end_index = 0; }
		Slice(int start, int end) { start_index = start, end_index = end; }
		Slice(const Slice& t) { start_index = t.start_index, end_index = t.end_index; }
	};
	class CVLIB_DECLSPEC Range
	{
	public:
		Range();
		Range(int _start, int _end);
		Range(const Slice& slice);
		int size() const;
		bool empty() const;
		static Range all();
		operator Slice() const;

		int start, end;
	};

	/*!
	  A complex number class.

	  The template class is similar and compatible with std::complex, however it provides slightly
	  more convenient access to the real and imaginary parts using through the simple field access, as opposite
	  to std::complex::real() and std::complex::imag().
	*/
	template<typename _Tp> class CVLIB_DECLSPEC Complex
	{
	public:

		//! constructors
		Complex() : re(0), im(0) {}
		Complex(_Tp _re, _Tp _im = 0) : re(_re), im(_im) {}
		//    Complex( const std::complex<_Tp>& c );

			//! conversion to another data type
		template<typename T2> operator Complex<T2>() const { return Complex<T2>((T2)(re), (T2)(im)); }
		//! conjugation
		Complex conj() const { return Complex<_Tp>(re, -im); }
		//! conversion to std::complex
	//    operator std::complex<_Tp>() const;

		_Tp re, im; //< the real and the imaginary parts
	};

	template<typename _Tp> static inline
		bool operator == (const Complex<_Tp>& a, const Complex<_Tp>& b)
	{
		return a.re == b.re && a.im == b.im;
	}

	template<typename _Tp> static inline
		bool operator != (const Complex<_Tp>& a, const Complex<_Tp>& b)
	{
		return a.re != b.re || a.im != b.im;
	}

	template<typename _Tp> static inline
		Complex<_Tp> operator + (const Complex<_Tp>& a, const Complex<_Tp>& b)
	{
		return Complex<_Tp>(a.re + b.re, a.im + b.im);
	}

	template<typename _Tp> static inline
		Complex<_Tp>& operator += (Complex<_Tp>& a, const Complex<_Tp>& b)
	{
		a.re += b.re; a.im += b.im; return a;
	}

	template<typename _Tp> static inline
		Complex<_Tp> operator - (const Complex<_Tp>& a, const Complex<_Tp>& b)
	{
		return Complex<_Tp>(a.re - b.re, a.im - b.im);
	}

	template<typename _Tp> static inline
		Complex<_Tp>& operator -= (Complex<_Tp>& a, const Complex<_Tp>& b)
	{
		a.re -= b.re; a.im -= b.im; return a;
	}

	template<typename _Tp> static inline
		Complex<_Tp> operator - (const Complex<_Tp>& a)
	{
		return Complex<_Tp>(-a.re, -a.im);
	}

	template<typename _Tp> static inline
		Complex<_Tp> operator * (const Complex<_Tp>& a, const Complex<_Tp>& b)
	{
		return Complex<_Tp>(a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re);
	}

	template<typename _Tp> static inline
		Complex<_Tp> operator * (const Complex<_Tp>& a, _Tp b)
	{
		return Complex<_Tp>(a.re*b, a.im*b);
	}

	template<typename _Tp> static inline
		Complex<_Tp> operator * (_Tp b, const Complex<_Tp>& a)
	{
		return Complex<_Tp>(a.re*b, a.im*b);
	}

	template<typename _Tp> static inline
		Complex<_Tp> operator + (const Complex<_Tp>& a, _Tp b)
	{
		return Complex<_Tp>(a.re + b, a.im);
	}

	template<typename _Tp> static inline
		Complex<_Tp> operator - (const Complex<_Tp>& a, _Tp b)
	{
		return Complex<_Tp>(a.re - b, a.im);
	}

	template<typename _Tp> static inline
		Complex<_Tp> operator + (_Tp b, const Complex<_Tp>& a)
	{
		return Complex<_Tp>(a.re + b, a.im);
	}

	template<typename _Tp> static inline
		Complex<_Tp> operator - (_Tp b, const Complex<_Tp>& a)
	{
		return Complex<_Tp>(b - a.re, -a.im);
	}

	template<typename _Tp> static inline
		Complex<_Tp>& operator += (Complex<_Tp>& a, _Tp b)
	{
		a.re += b; return a;
	}

	template<typename _Tp> static inline
		Complex<_Tp>& operator -= (Complex<_Tp>& a, _Tp b)
	{
		a.re -= b; return a;
	}

	template<typename _Tp> static inline
		Complex<_Tp>& operator *= (Complex<_Tp>& a, _Tp b)
	{
		a.re *= b; a.im *= b; return a;
	}

	template<typename _Tp> static inline
		double abs(const Complex<_Tp>& a)
	{
		return sqrt((double)a.re*a.re + (double)a.im*a.im);
	}

	template<typename _Tp> static inline
		Complex<_Tp> operator / (const Complex<_Tp>& a, const Complex<_Tp>& b)
	{
		double t = 1. / ((double)b.re*b.re + (double)b.im*b.im);
		return Complex<_Tp>((_Tp)((a.re*b.re + a.im*b.im)*t),
			(_Tp)((-a.re*b.im + a.im*b.re)*t));
	}

	template<typename _Tp> static inline Complex<_Tp>& operator /= (Complex<_Tp>& a, const Complex<_Tp>& b)
	{
		return (a = a / b);
	}

	template<typename _Tp> static inline Complex<_Tp> operator / (const Complex<_Tp>& a, _Tp b)
	{
		_Tp t = (_Tp)1 / b;
		return Complex<_Tp>(a.re*t, a.im*t);
	}

	template<typename _Tp> static inline Complex<_Tp> operator / (_Tp b, const Complex<_Tp>& a)
	{
		return Complex<_Tp>(b) / a;
	}

	template<typename _Tp> static inline Complex<_Tp>& operator /= (Complex<_Tp>& a, _Tp b)
	{
		_Tp t = (_Tp)1 / b;
		a.re *= t; a.im *= t; return a;
	}

	/*!
	  \typedef
	*/
	typedef Complex<float> Complexf;
	typedef Complex<double> Complexd;

	/*!
	  Aligns pointer by the certain number of bytes

	  This small inline function aligns the pointer by the certian number of bytes by shifting
	  it forward by 0 or a positive offset.
	*/

	template<typename _Tp> static inline _Tp* alignPtr(_Tp* ptr, int n = (int)sizeof(_Tp))
	{
		assert((n & (n - 1)) == 0); // n is a power of 2
		return (_Tp*)(((size_t)ptr + n - 1) & -n);
	}

	/*!
	  Aligns buffer size by the certain number of bytes

	  This small inline function aligns a buffer size by the certian number of bytes by enlarging it.
	*/
	static inline size_t alignSize(size_t sz, int n)
	{
		assert((n & (n - 1)) == 0); // n is a power of 2
		return (sz + n - 1) & -n;
	}

	static inline int alignLeft(int size, int align)
	{
		return size & -align;
	}
	CVLIB_DECLSPEC void scalarToRawData(const Scalar& s, void* buf, int type, int unroll_to = 0);

	template<typename _Tp> class DataType
	{
	public:
		typedef _Tp value_type;
		typedef value_type work_type;
		typedef value_type channel_type;
		typedef value_type vec_type;
		enum {
			generic_type = 1, depth = -1, channels = 1,
			type = CVLIB_MAKETYPE(depth, channels)
		};
	};

	template<> class DataType<uchar>
	{
	public:
		typedef uchar value_type;
		typedef int work_type;
		typedef value_type channel_type;
		typedef value_type vec_type;
		enum {
			generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
			type = CVLIB_MAKETYPE(depth, channels)
		};
	};

	template<> class DataType<char>
	{
	public:
		typedef char value_type;
		typedef int work_type;
		typedef value_type channel_type;
		typedef value_type vec_type;
		enum {
			generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
			type = CVLIB_MAKETYPE(depth, channels)
		};
	};

	template<> class DataType<short>
	{
	public:
		typedef short value_type;
		typedef int work_type;
		typedef value_type channel_type;
		typedef value_type vec_type;
		enum {
			generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
			type = CVLIB_MAKETYPE(depth, channels)
		};
	};

	template<> class DataType<int>
	{
	public:
		typedef int value_type;
		typedef value_type work_type;
		typedef value_type channel_type;
		typedef value_type vec_type;
		enum {
			generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
			type = CVLIB_MAKETYPE(depth, channels)
		};
	};

	template<> class DataType<float>
	{
	public:
		typedef float value_type;
		typedef value_type work_type;
		typedef value_type channel_type;
		typedef value_type vec_type;
		enum {
			generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
			type = CVLIB_MAKETYPE(depth, channels)
		};
	};

	template<> class DataType<double>
	{
	public:
		typedef double value_type;
		typedef value_type work_type;
		typedef value_type channel_type;
		typedef value_type vec_type;
		enum {
			generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
			type = CVLIB_MAKETYPE(depth, channels)
		};
	};

}

#pragma warning (pop)

/*!
 * \defgroup math Mathematics Classes
 * \brief Mathematics Library Classes Related Definition
 */

/*!
 * \defgroup base Object Base Classes
 * \brief Object Base Library Classes Related Definition
 */

#include "cvliboperation.h"
