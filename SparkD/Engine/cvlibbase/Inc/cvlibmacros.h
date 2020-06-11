/*! 
 * \file    cvlibmacros.h
 * \ingroup base
 * \brief   cvlib.
 * \author  
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include <time.h>
#include <float.h>
#include <assert.h>

// Old systems need that to define FLT_MAX and DBL_MAX
#ifndef DBL_MAX
#include <values.h>
#endif

#if !defined _MSC_VER && !defined __BORLANDC__
#  include <stdint.h>
#endif


#if defined(_MSC_VER)
	#if defined(CVLIB_EXPORTS)
	#define CVLIB_DECLSPEC __declspec(dllexport)
	#define EXPIMP_TEMPLATE
	#elif defined(CVLIB_IMPORTS)
	#define CVLIB_DECLSPEC __declspec(dllimport)
	#define EXPIMP_TEMPLATE extern
	#else
	#define CVLIB_DECLSPEC
	#define EXPIMP_TEMPLATE
	#endif
#elif defined(__ANDROID__)
	#define CVLIB_DECLSPEC
	#define EXPIMP_TEMPLATE
#elif __GNUC__ >= 4
	#if defined(CVLIB_EXPORTS)
	#define CVLIB_DECLSPEC __attribute__ ((visibility("default")))
	#define EXPIMP_TEMPLATE
	#elif defined(CVLIB_IMPORTS)
	#define CVLIB_DECLSPEC
	#define EXPIMP_TEMPLATE
    #else
    #define CVLIB_DECLSPEC
    #define EXPIMP_TEMPLATE
	#endif
#else
	#define CVLIB_DECLSPEC
	#pragma warning Unknown dyanamic library
#endif



#ifdef __GNUC__
    #define DEPRICATED(func) func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
	#define DEPRICATED(func)  __declspec (deprecated) func
#else
	#define DEPRICATED(func) func
#endif


#define CVLIB_OS_UNKNOWN -1
#define CVLIB_OS_SUN 0
#define CVLIB_OS_LINUX 1
#define CVLIB_OS_WIN32 2
#define CVLIB_OS_APPLE 3
#define CVLIB_OS_FreeBSD 4

#ifndef CVLIB_OS
#if defined(sun) || defined(__sun)
// Sun/Solaris configuration
#define CVLIB_OS            CVLIB_OS_SUN
#ifndef CVLIB_DISPLAY_TYPE
#define CVLIB_DISPLAY_TYPE  CVLIB_OS_LINUX
#endif
#ifndef CVLIB_COLOR_TERMINAL
#define CVLIB_COLOR_TERMINAL
#endif
#elif defined(linux) || defined(__linux) || defined(__CYGWIN__)
// PC Linux configuration
#define CVLIB_OS            CVLIB_OS_LINUX
#ifndef CVLIB_DISPLAY_TYPE
#define CVLIB_DISPLAY_TYPE  CVLIB_OS_LINUX
#endif
#ifndef CVLIB_COLOR_TERMINAL
#define CVLIB_COLOR_TERMINAL
#endif
#elif defined(_WIN32) || defined(__WIN32__)
// PC Windows configuration
#define CVLIB_OS            CVLIB_OS_WIN32
#ifndef CVLIB_DISPLAY_TYPE
#define CVLIB_DISPLAY_TYPE  CVLIB_OS_WIN32
#endif
#elif defined(__MACOSX__) || defined(__APPLE__)
// Mac OS X configuration
#define CVLIB_OS            CVLIB_OS_APPLE
#ifndef CVLIB_DISPLAY_TYPE
#define CVLIB_DISPLAY_TYPE  CVLIB_OS_LINUX
#endif
#elif defined(__FreeBSD__)
// FreeBSD configuration
#define CVLIB_OS            CVLIB_OS_FreeBSD
#ifndef CVLIB_DISPLAY_TYPE
#define CVLIB_DISPLAY_TYPE  CVLIB_OS_LINUX
#endif
#ifndef CVLIB_COLOR_TERMINAL
#define CVLIB_COLOR_TERMINAL
#endif
#else
// Unknown configuration : minimal dependencies.
#define CVLIB_OS           CVLIB_OS_UNKNOWN
#ifndef CVLIB_DISPLAY_TYPE
#define CVLIB_DISPLAY_TYPE  0
#endif
#endif
#endif


#define getRValue(rgb)      ((unsigned char)(rgb))
#define getGValue(rgb)      ((unsigned char)(((unsigned short)(rgb)) >> 8))
#define getBValue(rgb)      ((unsigned char)((rgb)>>16))
#define rgb_pixels(r,g,b)          ((unsigned int)(((unsigned char)(r)|((unsigned short)((unsigned char)(g))<<8))|(((unsigned int)(unsigned char)(b))<<16)))

namespace cvlib
{

#define CVLIB_CN_MAX		64
#define CVLIB_CN_SHIFT		3
#define CVLIB_DEPTH_MAX		(1 << CVLIB_CN_SHIFT)

typedef enum {
	MAT_Tchar=0, 
	MAT_Tbyte=1, 
	MAT_Tshort=2, 
	MAT_Tint=3, 
	MAT_Tfloat=4, 
	MAT_Tdouble=5,
	MAT_USRTYPE=7,
	
	MAT_Tchar2=MAT_Tchar+(1<<CVLIB_CN_SHIFT),
	MAT_Tbyte2=MAT_Tbyte+(1<<CVLIB_CN_SHIFT),
	MAT_Tshort2=MAT_Tshort+(1<<CVLIB_CN_SHIFT),
	MAT_Tint2=MAT_Tint+(1<<CVLIB_CN_SHIFT),
	MAT_Tfloat2=MAT_Tfloat+(1<<CVLIB_CN_SHIFT),
	MAT_Tdouble2=MAT_Tdouble+(1<<CVLIB_CN_SHIFT),

	MAT_Tchar3=MAT_Tchar+(2<<CVLIB_CN_SHIFT),
	MAT_Tbyte3=MAT_Tbyte+(2<<CVLIB_CN_SHIFT),
	MAT_Tshort3=MAT_Tshort+(2<<CVLIB_CN_SHIFT),
	MAT_Tint3=MAT_Tint+(2<<CVLIB_CN_SHIFT),
	MAT_Tfloat3=MAT_Tfloat+(2<<CVLIB_CN_SHIFT),
	MAT_Tdouble3=MAT_Tdouble+(2<<CVLIB_CN_SHIFT),

	MAT_Tchar4=MAT_Tchar+(3<<CVLIB_CN_SHIFT),
	MAT_Tbyte4=MAT_Tbyte+(3<<CVLIB_CN_SHIFT),
	MAT_Tshort4=MAT_Tshort+(3<<CVLIB_CN_SHIFT),
	MAT_Tint4=MAT_Tint+(3<<CVLIB_CN_SHIFT),
	MAT_Tfloat4=MAT_Tfloat+(3<<CVLIB_CN_SHIFT),
	MAT_Tdouble4=MAT_Tdouble+(3<<CVLIB_CN_SHIFT),
} TYPE;

#define MAT_Tuchar MAT_Tbyte
#define MAT_Tuchar2 MAT_Tbyte2
#define MAT_Tuchar3 MAT_Tbyte3
#define MAT_Tuchar4 MAT_Tbyte4

#define CVLIB_MAT_DEPTH_MASK	(CVLIB_DEPTH_MAX - 1)
#define CVLIB_MAT_DEPTH(n)		((n)&CVLIB_MAT_DEPTH_MASK)
#define CVLIB_MAT_CN_MASK          ((CVLIB_CN_MAX - 1) << CVLIB_CN_SHIFT)
#define CVLIB_MAT_CN(n)				((((n) & CVLIB_MAT_CN_MASK) >> CVLIB_CN_SHIFT) + 1)
#define CVLIB_MAT_TYPE_MASK        (CVLIB_DEPTH_MAX*CVLIB_CN_MAX - 1)
#define CVLIB_MAT_TYPE(n)      ((n) & CVLIB_MAT_TYPE_MASK)
#define CVLIB_MAKETYPE(depth, cn) (CVLIB_MAT_DEPTH(depth)+(((cn)-1)<<CVLIB_CN_SHIFT))

//! stream buffer
enum
{
	enARGB = 33,
	enBGRA = 32,
	enBGR888 = 24,
	enBGR555 = 16,
	enBGR565 = 17,
};
typedef enum {CT_Default, CT_Cast} CASTTYPE;
static const int ganElemsize[]={1,1,2,4,4,8};
#define CVLIB_ELEM_SIZE(t) ganElemsize[CVLIB_MAT_DEPTH((int)t)]
#define CVLIB_ELEM_SIZE2(t) (CVLIB_ELEM_SIZE(t)*CVLIB_MAT_CN(t))

typedef unsigned char       uchar;
typedef signed char			schar;
typedef unsigned short      ushort;
typedef unsigned int        uint;
typedef unsigned long       ulong;

#if CVLIB_OS == CVLIB_OS_APPLE
#include <_types.h>
#define __int64 int64_t
#define __uint64 uint64_t
#elif CVLIB_OS == CVLIB_OS_LINUX
#include <sys/types.h>
#define __int64 int64_t
#define __uint64 u_int64_t
#elif __MINGW32__ == 1
#define __uint64 unsigned __int64
#else
#define __int64 __int64
#define __uint64 unsigned __int64
#endif


#if defined _MSC_VER || defined __BORLANDC__
typedef __int64 int64;
typedef unsigned __int64 uint64;
#define CVLIB_BIG_INT(n)   n##I64
#define CVLIB_BIG_UINT(n)  n##UI64
#else
typedef int64_t int64;
typedef int64_t uint64;
#define CVLIB_BIG_INT(n)   n##LL
#define CVLIV_BIG_UINT(n)  n##ULL
#endif

#if !defined _M_X64
typedef unsigned int	Handle;
#else
typedef uint64			Handle;
#endif

#ifndef SIZE_T_MAX
	#define SIZE_T_MAX  0xffffffff
#endif

#define  CVLIB_MALLOC_ALIGN    32

#ifndef SWAP
#define SWAP(a,b,t) ((t) = (a), (a) = (b), (b) = (t))
#endif

#ifndef SQR
#define	SQR(x)	((x)*(x))
#endif

#ifndef MIN
#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif
    
#ifndef MIN3
#define MIN3(a,b,c) MIN( MIN(a,b), c )
#endif
    
#ifndef MAX
#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif
    
#ifndef MAX3
#define MAX3(a,b,c) MAX( MAX(a,b), c )
#endif
    
#define MINMAX(a,b,c)  (MIN(MAX(a,b),c))

#ifndef ABS
#define ABS(a)  ((a) > 0 ? (a) : (-(a)))
#endif

#define  CVLIB_CMP(a,b)    (((a) > (b)) - ((a) < (b)))
#define  CVLIB_SIGN(a)     CVLIB_CMP((a),0)


#ifndef GRAY_LEVEL_NUMBER
#define	GRAY_LEVEL_NUMBER	256
#endif

#ifndef MAX_8BIT_VALUE
#define	MAX_8BIT_VALUE		255
#endif

#ifndef MAX_FLOAT
#define	MAX_FLOAT	        1E17f
#endif

#ifndef CVLIB_INF
#define CVLIB_INF			FLT_MAX
#endif

#ifndef CVLIB_PI
#define CVLIB_PI   3.1415926535897932384626433832795
#endif

#ifndef CVLIB_LOG2
#define CVLIB_LOG2 0.69314718055994530941723212145818
#endif

#ifndef CVLIB_EXP
#define	CVLIB_EXP  2.7182818284590452353602874713527					//	e
#endif

#define CVLIB_PATH_MAX				260
#define CVLIB_MAX_IDLEN				32

}

/*!
 * \defgroup math Mathematics Classes
 * \brief Mathematics Library Classes Related Definition
 */

/*!
 * \defgroup base Object Base Classes
 * \brief Object Base Library Classes Related Definition
 */

