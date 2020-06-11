
#pragma once

#include "Template.h"

#if defined WIN32 || defined _WIN32
#  ifndef WIN32
#    define WIN32
#  endif
#  ifndef _WIN32
#    define _WIN32
#  endif
#endif

#if !defined WIN32 && !defined WINCE
#  include <pthread.h>
#endif

#ifdef __BORLANDC__
#  ifndef WIN32
#    define WIN32
#  endif
#  ifndef _WIN32
#    define _WIN32
#  endif
//#  define CV_DLL
//#  undef _CV_ALWAYS_PROFILE_
//#  define _CV_ALWAYS_NO_PROFILE_
#endif

#ifdef HAVE_IPP
#  include "ipp.h"

inline IppiSize ippiSize(int width, int height)
{
    IppiSize size = { width, height };
    return size;
}
#endif

#define CVLIB_CPU_NONE    0
#define CVLIB_CPU_MMX     1
#define CVLIB_CPU_SSE     2
#define CVLIB_CPU_SSE2    3
#define CVLIB_CPU_SSE3    4
#define CVLIB_CPU_SSSE3   5
#define CVLIB_CPU_SSE4_1  6
#define CVLIB_CPU_SSE4_2  7
#define CVLIB_CPU_POPCNT  8
#define CVLIB_CPU_AVX    10
#define CVLIB_HARDWARE_MAX_FEATURE 255

#if defined __SSE2__ || defined _M_X64  || (defined _M_IX86_FP && _M_IX86_FP >= 2)
#  include "emmintrin.h"
#  define CVLIB_SSE 1
#  define CVLIB_SSE2 1
#  if defined __SSE3__ || (defined _MSC_VER && _MSC_VER >= 1500)
#    include "pmmintrin.h"
#    define CVLIB_SSE3 1
#  endif
#  if defined __SSSE3__  || (defined _MSC_VER && _MSC_VER >= 1500)
#    include "tmmintrin.h"
#    define CVLIB_SSSE3 1
#  endif
#  if defined __SSE4_1__ || (defined _MSC_VER && _MSC_VER >= 1500)
#    include <smmintrin.h>
#    define CVLIB_SSE4_1 1
#  endif
#  if defined __SSE4_2__ || (defined _MSC_VER && _MSC_VER >= 1500)
#    include <nmmintrin.h>
#    define CVLIB_SSE4_2 1
#  endif
#  if defined __AVX__ || (defined _MSC_FULL_VER && _MSC_FULL_VER >= 160040219)
// MS Visual Studio 2010 (2012?) has no macro pre-defined to identify the use of /arch:AVX
// See: http://connect.microsoft.com/VisualStudio/feedback/details/605858/arch-avx-should-define-a-predefined-macro-in-x64-and-set-a-unique-value-for-m-ix86-fp-in-win32
#    include <immintrin.h>
#    define CVLIB_AVX 1
#    if defined(_XCR_XFEATURE_ENABLED_MASK)
#      define __xgetbv() _xgetbv(_XCR_XFEATURE_ENABLED_MASK)
#    else
#      define __xgetbv() 0
#    endif
#  endif
#endif

#ifdef __ARM_NEON__
#  include <arm_neon.h>
#  define CVLIB_NEON 1
#  define CPU_HAS_NEON_FEATURE (true)
#endif

#ifndef CVLIB_SSE
#  define CVLIB_SSE 0
#endif
#ifndef CVLIB_SSE2
#  define CVLIB_SSE2 0
#endif
#ifndef CVLIB_SSE3
#  define CVLIB_SSE3 0
#endif
#ifndef CVLIB_SSSE3
#  define CVLIB_SSSE3 0
#endif
#ifndef CVLIB_SSE4_1
#  define CVLIB_SSE4_1 0
#endif
#ifndef CVLIB_SSE4_2
#  define CVLIB_SSE4_2 0
#endif
#ifndef CVLIB_AVX
#  define CVLIB_AVX 0
#endif
#ifndef CVLIB_AVX2
#  define CVLIB_AVX2 0
#endif
#ifndef CVLIB_NEON
#  define CVLIB_NEON 0
#endif

#ifdef HAVE_TBB
#  include "tbb/tbb_stddef.h"
#  if TBB_VERSION_MAJOR*100 + TBB_VERSION_MINOR >= 202
#    include "tbb/tbb.h"
#    include "tbb/task.h"
#    undef min
#    undef max
#  else
#    undef HAVE_TBB
#  endif
#endif

#ifdef HAVE_EIGEN
#  if defined __GNUC__ && defined __APPLE__
#    pragma GCC diagnostic ignored "-Wshadow"
#  endif
#  include <Eigen/Core>
#endif

namespace cvlib
{
#ifdef HAVE_TBB

    typedef tbb::blocked_range<int> BlockedRange;

    template<typename Body> static inline
    void parallel_for( const BlockedRange& range, const Body& body )
    {
        tbb::parallel_for(range, body);
    }

    template<typename Iterator, typename Body> static inline
    void parallel_do( Iterator first, Iterator last, const Body& body )
    {
        tbb::parallel_do(first, last, body);
    }

    typedef tbb::split Split;

    template<typename Body> static inline
    void parallel_reduce( const BlockedRange& range, Body& body )
    {
        tbb::parallel_reduce(range, body);
    }

    typedef tbb::concurrent_vector<Rect> ConcurrentRectVector;
    typedef tbb::concurrent_vector<double> ConcurrentDoubleVector;
#else
    class BlockedRange
    {
    public:
        BlockedRange() : _begin(0), _end(0), _grainsize(0) {}
        BlockedRange(int b, int e, int g=1) : _begin(b), _end(e), _grainsize(g) {}
        int begin() const { return _begin; }
        int end() const { return _end; }
        int grainsize() const { return _grainsize; }
        
    protected:
        int _begin, _end, _grainsize;
    };

    template<typename Body> static inline
    void parallel_for( const BlockedRange& range, const Body& body )
    {
        body(range);
    }
	typedef Vector<Rect> ConcurrentRectVector;
    typedef Vector<double> ConcurrentDoubleVector;

    template<typename Iterator, typename Body> static inline
    void parallel_do( Iterator first, Iterator last, const Body& body )
    {
        for( ; first != last; ++first )
            body(*first);
    }
    
    class Split {};
    
    template<typename Body> static inline
    void parallel_reduce( const BlockedRange& range, Body& body )
    {
        body(range);
    }
#endif
}

#ifdef __GNUC__
#  define CVLIB_DECL_ALIGNED(x) __attribute__ ((aligned (x)))
#elif defined _MSC_VER
#  define CVLIB_DECL_ALIGNED(x) __declspec(align(x))
#else
#  define CVLIB_DECL_ALIGNED(x)
#endif

