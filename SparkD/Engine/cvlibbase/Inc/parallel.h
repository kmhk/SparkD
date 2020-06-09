#pragma once

#include "cvlibstructs.h"

#ifndef SKIP_INCLUDES
  #include <string.h>
  #include <limits.h>
#endif // SKIP_INCLUDES


/////// exchange-add operation for atomic operations on reference counters ///////
#if defined __INTEL_COMPILER && !(defined WIN32 || defined _WIN32)   // atomic increment on the linux version of the Intel(tm) compiler
  #define CVLIB_XADD(addr,delta) _InterlockedExchangeAdd(const_cast<void*>(reinterpret_cast<volatile void*>(addr)), delta)
#elif defined __GNUC__

  #if defined __clang__ && __clang_major__ >= 3 && !defined __ANDROID__
    #ifdef __ATOMIC_SEQ_CST
        #define CVLIB_XADD(addr, delta) __c11_atomic_fetch_add((_Atomic(int)*)(addr), (delta), __ATOMIC_SEQ_CST)
    #else
        #define CVLIB_XADD(addr, delta) __atomic_fetch_add((_Atomic(int)*)(addr), (delta), 5)
    #endif
  #elif __GNUC__*10 + __GNUC_MINOR__ >= 42

    #if !(defined WIN32 || defined _WIN32) && (defined __i486__ || defined __i586__ || \
        defined __i686__ || defined __MMX__ || defined __SSE__  || defined __ppc__) || \
        (defined __GNUC__ && defined _STLPORT_MAJOR)
      #define CVLIB_XADD __sync_fetch_and_add
    #else
      #include <ext/atomicity.h>
      #define CVLIB_XADD __gnu_cxx::__exchange_and_add
    #endif

  #else
    #include <bits/atomicity.h>
    #if __GNUC__*10 + __GNUC_MINOR__ >= 34
      #define CVLIB_XADD __gnu_cxx::__exchange_and_add
    #else
      #define CVLIB_XADD __exchange_and_add
    #endif
  #endif

#elif defined WIN32 || defined _WIN32 || defined WINCE
  namespace cvlib { CVLIB_DECLSPEC int _interlockedExchangeAdd(int* addr, int delta); }
  #define CVLIB_XADD cvlib::_interlockedExchangeAdd

#else
  static inline int CVLIB_XADD(int* addr, int delta)
  { int tmp = *addr; *addr += delta; return tmp; }
#endif

namespace cvlib
{

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
#define CVLIB_CPU_AVX2    11
#define CVLIB_HARDWARE_MAX_FEATURE 255

extern CVLIB_DECLSPEC volatile bool USE_SSE2;

CVLIB_DECLSPEC bool checkHardwareSupport(int feature);
CVLIB_DECLSPEC bool useSSE2();

CVLIB_DECLSPEC void setNumThreads(int nthreads);
CVLIB_DECLSPEC int getNumThreads();
CVLIB_DECLSPEC int getThreadNum();
CVLIB_DECLSPEC int getNumberOfCPUs();

class CVLIB_DECLSPEC ParallelLoopBody
{
public:
    virtual ~ParallelLoopBody();
    virtual void operator() (const Range& range) const = 0;
};

CVLIB_DECLSPEC void parallel_for_(const Range& range, const ParallelLoopBody& body, double nstripes=-1.);
CVLIB_DECLSPEC const char* currentParallelFramework();


class CVLIB_DECLSPEC Mutex
{
public:
    Mutex();
    ~Mutex();
    Mutex(const Mutex& m);
    Mutex& operator = (const Mutex& m);

    void lock();
    bool trylock();
    void unlock();

    struct Impl;
protected:
    Impl* impl;
};

class CVLIB_DECLSPEC AutoLock
{
public:
    AutoLock(Mutex& m) : mutex(&m) { mutex->lock(); }
    ~AutoLock() { mutex->unlock(); }
protected:
    Mutex* mutex;
};

#ifndef WIN32
#define WAIT_TIMEOUT 0x102
#define WAIT_OBJECT_0 0
#define WAIT_ABANDONED 128
#define WAIT_FAILED 0xFFFFFFFF
#endif

class CVLIB_DECLSPEC Event
{
public:
    Event(bool finit = false, bool fmanual = false);
    ~Event();

    void setEvent();
    void pulseEvent();
    void resetEvent();
	bool lock(ulong dwTimeout = 0xFFFFFFFF);

    struct Impl;
protected:
    Impl* impl;
};

}
