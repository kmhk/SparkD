
#include "parallel.h"
#include "cvlibutil.h"


#if defined WIN32 || defined WINCE
    #include <windows.h>
	#if !defined __MINGW32__
		#include <intrin.h>
	#endif
    #undef small
    #undef min
    #undef max
    #undef abs
#endif

#if defined __linux__ || defined __APPLE__
    #include <unistd.h>
    #include <stdio.h>
    #include <sys/types.h>
    #include <sys/time.h>
    #include <errno.h>
    #if defined ANDROID
        #include <sys/sysconf.h>
	#elif !defined __MINGW32__
        #include <sys/sysctl.h>
    #endif
#endif

#ifdef _OPENMP
	#ifndef HAVE_OPENMP
    #define HAVE_OPENMP
	#endif
#endif

#ifdef __APPLE__
    #define HAVE_GCD
#endif

#if defined _MSC_VER && _MSC_VER >= 1600
    #define HAVE_CONCURRENCY
#endif

/* IMPORTANT: always use the same order of defines
   1. HAVE_TBB         - 3rdparty library, should be explicitly enabled
   2. HAVE_CSTRIPES    - 3rdparty library, should be explicitly enabled
   3. HAVE_OPENMP      - integrated to compiler, should be explicitly enabled
   4. HAVE_GCD         - system wide, used automatically        (APPLE only)
   5. HAVE_CONCURRENCY - part of runtime, used automatically    (Windows only - MSVS 10, MSVS 11)
*/

#if defined HAVE_TBB
    #include "tbb/tbb_stddef.h"
    #if TBB_VERSION_MAJOR*100 + TBB_VERSION_MINOR >= 202
        #include "tbb/tbb.h"
        #include "tbb/task.h"
        #if TBB_INTERFACE_VERSION >= 6100
            #include "tbb/task_arena.h"
        #endif
        #undef min
        #undef max
    #else
        #undef HAVE_TBB
    #endif // end TBB version
#endif

#ifndef HAVE_TBB
    #if defined HAVE_CSTRIPES
        #include "C=.h"
        #undef shared
    #elif defined HAVE_OPENMP
        #include <omp.h>
    #elif defined HAVE_GCD
        #include <dispatch/dispatch.h>
        #include <pthread.h>
    #elif defined HAVE_CONCURRENCY
        #include <ppl.h>
    #endif
#endif

#if defined HAVE_TBB && TBB_VERSION_MAJOR*100 + TBB_VERSION_MINOR >= 202
#  define CVLIB_PARALLEL_FRAMEWORK "tbb"
#elif defined HAVE_CSTRIPES
#  define CVLIB_PARALLEL_FRAMEWORK "cstripes"
#elif defined HAVE_OPENMP
#  define CVLIB_PARALLEL_FRAMEWORK "openmp"
#elif defined HAVE_GCD
#  define CVLIB_PARALLEL_FRAMEWORK "gcd"
#elif defined HAVE_CONCURRENCY
#  define CVLIB_PARALLEL_FRAMEWORK "ms-concurrency"
#endif

namespace cvlib
{
    ParallelLoopBody::~ParallelLoopBody() {}

	struct HWFeatures
	{
		enum { MAX_FEATURE = CVLIB_HARDWARE_MAX_FEATURE };

		HWFeatures(void)
		 {
			memset( have, 0, sizeof(have) );
			x86_family = 0;
		}

		static HWFeatures initialize(void)
		{
			HWFeatures f;
			int cpuid_data[4] = { 0, 0, 0, 0 };

		#if defined _MSC_VER && (defined _M_IX86 || defined _M_X64)
			__cpuid(cpuid_data, 1);
		#elif defined __GNUC__ && (defined __i386__ || defined __x86_64__)
			#ifdef __x86_64__
			asm __volatile__
			(
			 "movl $1, %%eax\n\t"
			 "cpuid\n\t"
			 :[eax]"=a"(cpuid_data[0]),[ebx]"=b"(cpuid_data[1]),[ecx]"=c"(cpuid_data[2]),[edx]"=d"(cpuid_data[3])
			 :
			 : "cc"
			);
			#else
			asm volatile
			(
			 "pushl %%ebx\n\t"
			 "movl $1,%%eax\n\t"
			 "cpuid\n\t"
			 "popl %%ebx\n\t"
			 : "=a"(cpuid_data[0]), "=c"(cpuid_data[2]), "=d"(cpuid_data[3])
			 :
			 : "cc"
			);
			#endif
		#endif

			f.x86_family = (cpuid_data[0] >> 8) & 15;
			if( f.x86_family >= 6 )
			{
				f.have[CVLIB_CPU_MMX]    = (cpuid_data[3] & (1 << 23)) != 0;
				f.have[CVLIB_CPU_SSE]    = (cpuid_data[3] & (1<<25)) != 0;
				f.have[CVLIB_CPU_SSE2]   = (cpuid_data[3] & (1<<26)) != 0;
				f.have[CVLIB_CPU_SSE3]   = (cpuid_data[2] & (1<<0)) != 0;
				f.have[CVLIB_CPU_SSSE3]  = (cpuid_data[2] & (1<<9)) != 0;
				f.have[CVLIB_CPU_SSE4_1] = (cpuid_data[2] & (1<<19)) != 0;
				f.have[CVLIB_CPU_SSE4_2] = (cpuid_data[2] & (1<<20)) != 0;
				f.have[CVLIB_CPU_POPCNT] = (cpuid_data[2] & (1<<23)) != 0;
				f.have[CVLIB_CPU_AVX]    = (((cpuid_data[2] & (1<<28)) != 0)&&((cpuid_data[2] & (1<<27)) != 0));//OS uses XSAVE_XRSTORE and CPU support AVX
			}

			return f;
		}

		int x86_family;
		bool have[MAX_FEATURE+1];
	};

	static HWFeatures  featuresEnabled = HWFeatures::initialize(), featuresDisabled = HWFeatures();
	static HWFeatures* currentFeatures = &featuresEnabled;

	bool checkHardwareSupport(int feature)
	{
		assert( 0 <= feature && feature <= CVLIB_HARDWARE_MAX_FEATURE );
		return currentFeatures->have[feature];
	}


	volatile bool useOptimizedFlag = true;
	#ifdef HAVE_IPP
	struct IPPInitializer
	{
		IPPInitializer(void) { ippStaticInit(); }
	};

	IPPInitializer ippInitializer;
	#endif

	volatile bool USE_SSE2 = featuresEnabled.have[CVLIB_CPU_SSE2];
	volatile bool USE_SSE4_2 = featuresEnabled.have[CVLIB_CPU_SSE4_2];
	volatile bool USE_AVX = featuresEnabled.have[CVLIB_CPU_AVX];

	void setUseOptimized( bool flag )
	{
		useOptimizedFlag = flag;
		currentFeatures = flag ? &featuresEnabled : &featuresDisabled;
		USE_SSE2 = currentFeatures->have[CVLIB_CPU_SSE2];
	}

	bool useOptimized(void)
	{
		return useOptimizedFlag;
	}

	bool useSSE2()
	{
		return USE_SSE2;
	}


}

namespace
{
#ifdef CVLIB_PARALLEL_FRAMEWORK
    class ParallelLoopBodyWrapper
    {
    public:
        ParallelLoopBodyWrapper(const cvlib::ParallelLoopBody& _body, const cvlib::Range& _r, double _nstripes)
        {
            body = &_body;
            wholeRange = _r;
            double len = wholeRange.end - wholeRange.start;
            nstripes = cvlib::cvutil::round(_nstripes <= 0 ? len : MIN(MAX(_nstripes, 1.), len));
        }
        void operator()(const cvlib::Range& sr) const
        {
            cvlib::Range r;
            r.start = (int)(wholeRange.start +
                            ((size_t)sr.start*(wholeRange.end - wholeRange.start) + nstripes/2)/nstripes);
            r.end = sr.end >= nstripes ? wholeRange.end : (int)(wholeRange.start +
                            ((size_t)sr.end*(wholeRange.end - wholeRange.start) + nstripes/2)/nstripes);
            (*body)(r);
        }
        cvlib::Range stripeRange() const { return cvlib::Range(0, nstripes); }

    protected:
        const cvlib::ParallelLoopBody* body;
        cvlib::Range wholeRange;
        int nstripes;
    };

#if defined HAVE_TBB
    class ProxyLoopBody : public ParallelLoopBodyWrapper
    {
    public:
        ProxyLoopBody(const cvlib::ParallelLoopBody& _body, const cvlib::Range& _r, double _nstripes)
        : ParallelLoopBodyWrapper(_body, _r, _nstripes)
        {}

        void operator ()(const tbb::blocked_range<int>& range) const
        {
            this->ParallelLoopBodyWrapper::operator()(cvlib::Range(range.begin(), range.end()));
        }
    };
#elif defined HAVE_CSTRIPES || defined HAVE_OPENMP
    typedef ParallelLoopBodyWrapper ProxyLoopBody;
#elif defined HAVE_GCD
    typedef ParallelLoopBodyWrapper ProxyLoopBody;
    static void block_function(void* context, size_t index)
    {
        ProxyLoopBody* ptr_body = static_cast<ProxyLoopBody*>(context);
        (*ptr_body)(cvlib::Range((int)index, (int)index + 1));
    }
#elif defined HAVE_CONCURRENCY
    class ProxyLoopBody : public ParallelLoopBodyWrapper
    {
    public:
        ProxyLoopBody(const cvlib::ParallelLoopBody& _body, const cvlib::Range& _r, double _nstripes)
        : ParallelLoopBodyWrapper(_body, _r, _nstripes)
        {}

        void operator ()(int i) const
        {
            this->ParallelLoopBodyWrapper::operator()(cvlib::Range(i, i + 1));
        }
    };
#else
    typedef ParallelLoopBodyWrapper ProxyLoopBody;
#endif

static int numThreads = -1;

#if defined HAVE_TBB
static tbb::task_scheduler_init tbbScheduler(tbb::task_scheduler_init::deferred);
#elif defined HAVE_CSTRIPES
// nothing for C=
#elif defined HAVE_OPENMP
static int numThreadsMax = omp_get_max_threads();
#elif defined HAVE_GCD
// nothing for GCD
#elif defined HAVE_CONCURRENCY
class SchedPtr
{
    Concurrency::Scheduler* sched_;
public:
    Concurrency::Scheduler* operator->() { return sched_; }
    operator Concurrency::Scheduler*() { return sched_; }

    void operator=(Concurrency::Scheduler* sched)
    {
        if (sched_) sched_->Release();
        sched_ = sched;
    }

    SchedPtr() : sched_(0) {}
    ~SchedPtr() { *this = 0; }
};
static SchedPtr pplScheduler;
#endif

#endif // CVLIB_PARALLEL_FRAMEWORK

} //namespace

/* ================================   parallel_for_  ================================ */

void cvlib::parallel_for_(const cvlib::Range& range, const cvlib::ParallelLoopBody& body, double nstripes)
{
#ifdef CVLIB_PARALLEL_FRAMEWORK

    if(numThreads != 0)
    {
        ProxyLoopBody pbody(body, range, nstripes);
        cvlib::Range stripeRange = pbody.stripeRange();

#if defined HAVE_TBB

        tbb::parallel_for(tbb::blocked_range<int>(stripeRange.start, stripeRange.end), pbody);

#elif defined HAVE_CSTRIPES

        parallel(MAX(0, numThreads))
        {
            int offset = stripeRange.start;
            int len = stripeRange.end - offset;
			Range r(offset + CPX_RANGE_START(len), offset + CPX_RANGE_END(m_len));
            pbody(r);
            barrier();
        }

#elif defined HAVE_OPENMP

        #pragma omp parallel for schedule(dynamic)
        for (int i = stripeRange.start; i < stripeRange.end; ++i)
            pbody(Range(i, i + 1));

#elif defined HAVE_GCD

        dispatch_queue_t concurrent_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
        dispatch_apply_f(stripeRange.end - stripeRange.start, concurrent_queue, &pbody, block_function);

#elif defined HAVE_CONCURRENCY

        if(!pplScheduler || pplScheduler->Id() == Concurrency::CurrentScheduler::Id())
        {
            Concurrency::parallel_for(stripeRange.start, stripeRange.end, pbody);
        }
        else
        {
            pplScheduler->Attach();
            Concurrency::parallel_for(stripeRange.start, stripeRange.end, pbody);
            Concurrency::CurrentScheduler::Detach();
        }

#else

#error You have hacked and compiling with unsupported parallel framework

#endif

    }
    else

#endif // CVLIB_PARALLEL_FRAMEWORK
    {
        (void)nstripes;
        body(range);
    }
}

int cvlib::getNumThreads(void)
{
#ifdef CVLIB_PARALLEL_FRAMEWORK

    if(numThreads == 0)
        return 1;

#endif

#if defined HAVE_TBB

    return tbbScheduler.is_active()
           ? numThreads
           : tbb::task_scheduler_init::default_num_threads();

#elif defined HAVE_CSTRIPES

    return numThreads > 0
            ? numThreads
            : cvlib::getNumberOfCPUs();

#elif defined HAVE_OPENMP

    return omp_get_max_threads();

#elif defined HAVE_GCD

    return 512; // the GCD thread pool limit

#elif defined HAVE_CONCURRENCY

    return 1 + (pplScheduler == 0
                ? Concurrency::CurrentScheduler::Get()->GetNumberOfVirtualProcessors()
                : pplScheduler->GetNumberOfVirtualProcessors());

#else

    return 1;

#endif
}

void cvlib::setNumThreads( int threads )
{
    (void)threads;
#ifdef CVLIB_PARALLEL_FRAMEWORK
    numThreads = threads;
#endif

#ifdef HAVE_TBB

    if(tbbScheduler.is_active()) tbbScheduler.terminate();
    if(threads > 0) tbbScheduler.initialize(threads);

#elif defined HAVE_CSTRIPES

    return; // nothing needed

#elif defined HAVE_OPENMP

    if(omp_in_parallel())
        return; // can't change number of openmp threads inside a parallel region

    omp_set_num_threads(threads > 0 ? threads : numThreadsMax);

#elif defined HAVE_GCD

    // unsupported
    // there is only private dispatch_queue_set_width() and only for desktop

#elif defined HAVE_CONCURRENCY

    if (threads <= 0)
    {
        pplScheduler = 0;
    }
    else if (threads == 1)
    {
        // Concurrency always uses >=2 threads, so we just disable it if 1 thread is requested
        numThreads = 0;
    }
    else if (pplScheduler == 0 || 1 + pplScheduler->GetNumberOfVirtualProcessors() != (unsigned int)threads)
    {
        pplScheduler = Concurrency::Scheduler::Create(Concurrency::SchedulerPolicy(2,
                       Concurrency::MinConcurrency, threads-1,
                       Concurrency::MaxConcurrency, threads-1));
    }

#endif
}


int cvlib::getThreadNum(void)
{
#if defined HAVE_TBB
    #if TBB_INTERFACE_VERSION >= 6100 && defined TBB_PREVIEW_TASK_ARENA && TBB_PREVIEW_TASK_ARENA
        return tbb::task_arena::current_slot();
    #else
        return 0;
    #endif
#elif defined HAVE_CSTRIPES
    return pix();
#elif defined HAVE_OPENMP
    return omp_get_thread_num();
#elif defined HAVE_GCD
    return (int)(size_t)(void*)pthread_self(); // no zero-based indexing
#elif defined HAVE_CONCURRENCY
    return std::max(0, (int)Concurrency::Context::VirtualProcessorId()); // zero for master thread, unique number for others but not necessary 1,2,3,...
#else
    return 0;
#endif
}

#ifdef ANDROID
static inline int getNumberOfCPUsImpl()
{
   FILE* cpuPossible = fopen("/sys/devices/system/cpu/possible", "r");
   if(!cpuPossible)
       return 1;

   char buf[2000]; //big enough for 1000 CPUs in worst possible configuration
   char* pbuf = fgets(buf, sizeof(buf), cpuPossible);
   fclose(cpuPossible);
   if(!pbuf)
      return 1;

   //parse string of form "0-1,3,5-7,10,13-15"
   int cpusAvailable = 0;

   while(*pbuf)
   {
      const char* pos = pbuf;
      bool range = false;
      while(*pbuf && *pbuf != ',')
      {
          if(*pbuf == '-') range = true;
          ++pbuf;
      }
      if(*pbuf) *pbuf++ = 0;
      if(!range)
        ++cpusAvailable;
      else
      {
          int rstart = 0, rend = 0;
          sscanf(pos, "%d-%d", &rstart, &rend);
          cpusAvailable += rend - rstart + 1;
      }

   }
   return cpusAvailable ? cpusAvailable : 1;
}
#endif

int cvlib::getNumberOfCPUs(void)
{
#if defined WIN32 || defined _WIN32
#if !defined __MINGW32__
    SYSTEM_INFO sysinfo;
    GetSystemInfo( &sysinfo );
    return (int)sysinfo.dwNumberOfProcessors;
#else
    return 1;
#endif
#elif defined ANDROID
    static int ncpus = getNumberOfCPUsImpl();
    return ncpus;
#elif defined __linux__
    return (int)sysconf( _SC_NPROCESSORS_ONLN );
#elif defined __APPLE__
    int numCPU=0;
    int mib[4];
    size_t len = sizeof(numCPU);

    /* set the mib for hw.ncpu */
    mib[0] = CTL_HW;
    mib[1] = HW_AVAILCPU;  // alternatively, try HW_NCPU;

    /* get the number of CPUs from the system */
    sysctl(mib, 2, &numCPU, &len, NULL, 0);

    if( numCPU < 1 )
    {
        mib[1] = HW_NCPU;
        sysctl( mib, 2, &numCPU, &len, NULL, 0 );

        if( numCPU < 1 )
            numCPU = 1;
    }

    return (int)numCPU;
#else
    return 1;
#endif
}

const char* cvlib::currentParallelFramework() 
{
#ifdef CVLIB_PARALLEL_FRAMEWORK
    return CVLIB_PARALLEL_FRAMEWORK;
#else
    return NULL;
#endif
}

namespace cvlib
{
	#if defined WIN32 || defined _WIN32 || defined WINCE

	#if !defined __MINGW32__
	struct Mutex::Impl
	{
		Impl() {
#if (_WIN32_WINNT >= 0x0700)
	        ::InitializeCriticalSectionEx(&cs, 1000, 0);
#else
		    ::InitializeCriticalSection(&cs);
#endif
			refcount = 1;
		}
		~Impl() { DeleteCriticalSection(&cs); }

		void lock() { EnterCriticalSection(&cs); }
		bool trylock() { return TryEnterCriticalSection(&cs) != 0; }
		void unlock() { LeaveCriticalSection(&cs); }

		CRITICAL_SECTION cs;
		int refcount;
	};
	#else
	struct Mutex::Impl
	{
		Impl() { refcount = 1; }
		~Impl() {  }

		void lock() {  }
		bool trylock() { return false; }
		void unlock() {  }
		int refcount;
	};
	#endif

	#ifndef __GNUC__
	int _interlockedExchangeAdd(int* addr, int delta)
	{
	#if defined _MSC_VER && _MSC_VER >= 1500
		return (int)_InterlockedExchangeAdd((long volatile*)addr, delta);
	#else
		return (int)InterlockedExchangeAdd((long volatile*)addr, delta);
	#endif
	}
	#endif // __GNUC__

	#elif defined __APPLE__

	#include <libkern/OSAtomic.h>

	struct Mutex::Impl
	{
		Impl() { sl = OS_SPINLOCK_INIT; refcount = 1; }
		~Impl() {}

		void lock() { OSSpinLockLock(&sl); }
		bool trylock() { return OSSpinLockTry(&sl); }
		void unlock() { OSSpinLockUnlock(&sl); }

		OSSpinLock sl;
		int refcount;
	};

	#elif defined __linux__ && !defined ANDROID

	struct Mutex::Impl
	{
		Impl() { pthread_spin_init(&sl, 0); refcount = 1; }
		~Impl() { pthread_spin_destroy(&sl); }

		void lock() { pthread_spin_lock(&sl); }
		bool trylock() { return pthread_spin_trylock(&sl) == 0; }
		void unlock() { pthread_spin_unlock(&sl); }

		pthread_spinlock_t sl;
		int refcount;
	};

	#else

	#include <pthread.h>
	struct Mutex::Impl
	{
		Impl() { pthread_mutex_init(&sl, 0); refcount = 1; }
		~Impl() { pthread_mutex_destroy(&sl); }

		void lock() { pthread_mutex_lock(&sl); }
		bool trylock() { return pthread_mutex_trylock(&sl) == 0; }
		void unlock() { pthread_mutex_unlock(&sl); }

		pthread_mutex_t sl;
		int refcount;
	};

	#endif

	Mutex::Mutex()
	{
		impl = new Mutex::Impl;
	}

	Mutex::~Mutex()
	{
		if( CVLIB_XADD(&impl->refcount, -1) == 1 )
			delete impl;
		impl = 0;
	}

	Mutex::Mutex(const Mutex& m)
	{
		impl = m.impl;
		CVLIB_XADD(&impl->refcount, 1);
	}

	Mutex& Mutex::operator = (const Mutex& m)
	{
		CVLIB_XADD(&m.impl->refcount, 1);
		if( CVLIB_XADD(&impl->refcount, -1) == 1 )
			delete impl;
		impl = m.impl;
		return *this;
	}

	void Mutex::lock() { impl->lock(); }
	void Mutex::unlock() { impl->unlock(); }
	bool Mutex::trylock() { return impl->trylock(); }
}

namespace cvlib
{
#if defined WIN32 || defined _WIN32 || defined WINCE

//#if !defined __MINGW32__
struct Event::Impl
{
    Impl(bool finit, bool fmanual)
    {
		m_fautoReset = !fmanual;
        m_eventHandle = ::CreateEventA (NULL, fmanual, finit, NULL);
		refcount = 0;
    }
    ~Impl()
    {
		if (m_eventHandle != 0)
		{
			::CloseHandle(m_eventHandle);
			m_eventHandle = 0;
		}
    }
    void setEvent()
    {
        if (!m_eventHandle)
            assert (false);
        SetEvent (m_eventHandle);
    }
    void pulseEvent()
    {
        if (!m_eventHandle)
            assert (false);
        PulseEvent(m_eventHandle);
    }
    void resetEvent()
    {
        if (!m_eventHandle)
            assert (false);
        ResetEvent(m_eventHandle);
    }
	bool lock(ulong dwTimeout)
	{
		DWORD dwRet = ::WaitForSingleObject(m_eventHandle, dwTimeout);
		if (dwRet == WAIT_OBJECT_0 || dwRet == WAIT_ABANDONED)
			return true;
		else
			return false;
	}
	bool m_fautoReset;
    HANDLE m_eventHandle;
	int refcount;
};
/*#else
struct Event::Impl
{
    Impl(bool finit, bool fmanual)  {refcount=1;}
    ~Impl() {}
    void setEvent() {}
    void pulseEvent() {}
    void resetEvent() {}
	bool lock(ulong dwTimeout) {return true;}
    int refcount;
};
#endif*/

#elif defined __APPLE__

#include <libkern/OSAtomic.h>
#include <pthread.h>
struct Event::Impl
{
    Impl(bool finit, bool fmanual) {sl = OS_SPINLOCK_INIT; triggered = false; refcount=0; }
    ~Impl() {}
    void setEvent()
    {
        sl = OS_SPINLOCK_INIT;
//        pthread_spin_init (&mutex, 0);
        pthread_cond_init (&cond, 0);
        triggered = false;
    }
    void pulseEvent()
    {
        OSSpinLockLock (&sl);
        triggered = true;
        pthread_cond_signal (&cond);
        OSSpinLockUnlock (&sl);
    }
    void resetEvent()
    {
        OSSpinLockLock (&sl);
        triggered = false;
        OSSpinLockUnlock (&sl);
    }
	bool lock(ulong dwTimeout)
	{
        assert (false);
        OSSpinLockLock (&sl);
        /*while (!triggered)
            pthread_cond_wait(&cond, &mutex);*/
       OSSpinLockUnlock (&sl);
	}
    OSSpinLock sl;
    int refcount;
    pthread_cond_t cond;
    bool triggered;
};
#elif defined __linux__ && !defined ANDROID
#include <pthread.h>
struct Event::Impl
{
#ifdef WFMO
	struct neosmart_wfmo_t_
	{
		pthread_mutex_t Mutex;
		pthread_cond_t CVariable;
		int RefCount;
		union
		{
			int FiredEvent; //WFSO
			int EventsLeft; //WFMO
		} Status;
		bool WaitAll;
		bool StillWaiting;

		void Destroy()
		{
			pthread_mutex_destroy(&Mutex);
			pthread_cond_destroy(&CVariable);
		}
	};
	typedef neosmart_wfmo_t_ *neosmart_wfmo_t;

	struct neosmart_wfmo_info_t_
	{
		neosmart_wfmo_t Waiter;
		int WaitIndex;
	};
	typedef neosmart_wfmo_info_t_ *neosmart_wfmo_info_t;
#endif

	struct neosmart_event_t_
	{
		pthread_cond_t CVariable;
		pthread_mutex_t Mutex;
		bool AutoReset;
		bool State;
#ifdef WFMO
		std::deque<neosmart_wfmo_info_t_> RegisteredWaits;
#endif
	};
	typedef neosmart_event_t_ *neosmart_event_t;
#ifdef WFMO
	bool RemoveExpiredWaitHelper(neosmart_wfmo_info_t_ wait)
	{
		int result = pthread_mutex_trylock(&wait.Waiter->Mutex);

		if (result == EBUSY)
		{
			return false;
		}

		assert(result == 0);

		if (wait.Waiter->StillWaiting == false)
		{
			--wait.Waiter->RefCount;
			assert(wait.Waiter->RefCount >= 0);
			if (wait.Waiter->RefCount == 0)
			{
				wait.Waiter->Destroy();
				delete wait.Waiter;
			}
			else
			{
				result = pthread_mutex_unlock(&wait.Waiter->Mutex);
				assert(result == 0);
			}

			return true;
		}

		result = pthread_mutex_unlock(&wait.Waiter->Mutex);
		assert(result == 0);

		return false;
	}
#endif
	int UnlockedWaitForEvent(neosmart_event_t event, uint64_t milliseconds)
	{
		int result = 0;
		if (!event->State)
		{
			//zero-timeout event state check optimization
			if (milliseconds == 0)
			{
				return WAIT_TIMEOUT;
			}

			timespec ts;
			if (milliseconds != (uint64_t) -1)
			{
				timeval tv;
				gettimeofday(&tv, NULL);

				uint64_t nanoseconds = ((uint64_t) tv.tv_sec) * 1000 * 1000 * 1000 + milliseconds * 1000 * 1000 + ((uint64_t) tv.tv_usec) * 1000;

				ts.tv_sec = nanoseconds / 1000 / 1000 / 1000;
				ts.tv_nsec = (nanoseconds - ((uint64_t) ts.tv_sec) * 1000 * 1000 * 1000);
			}

			do
			{
				//Regardless of whether it's an auto-reset or manual-reset event:
				//wait to obtain the event, then lock anyone else out
				if (milliseconds != (uint64_t) -1)
				{
					result = pthread_cond_timedwait(&event->CVariable, &event->Mutex, &ts);
				}
				else
				{
					result = pthread_cond_wait(&event->CVariable, &event->Mutex);
				}
			} while (result == 0 && !event->State);

			if (result == 0 && event->AutoReset)
			{
				//We've only accquired the event if the wait succeeded
				event->State = false;
			}
		}
		else if (event->AutoReset)
		{
			//It's an auto-reset event that's currently available;
			//we need to stop anyone else from using it
			result = 0;
			event->State = false;
		}
		//Else we're trying to obtain a manual reset event with a signaled state;
		//don't do anything

		return result;
	}

	int WaitForEvent(neosmart_event_t event, uint64_t milliseconds)
	{
		int tempResult;
		if (milliseconds == 0)
		{
			tempResult = pthread_mutex_trylock(&event->Mutex);
			if (tempResult == EBUSY)
			{
				return WAIT_TIMEOUT;
			}
		}
		else
		{
			tempResult = pthread_mutex_lock(&event->Mutex);
		}

		assert(tempResult == 0);

		int result = UnlockedWaitForEvent(event, milliseconds);

		tempResult = pthread_mutex_unlock(&event->Mutex);
		assert(tempResult == 0);

		return result;
	}

#ifdef WFMO
	int WaitForMultipleEvents(neosmart_event_t *events, int count, bool waitAll, uint64_t milliseconds)
	{
		int unused;
		return WaitForMultipleEvents(events, count, waitAll, milliseconds, unused);
	}

	int WaitForMultipleEvents(neosmart_event_t *events, int count, bool waitAll, uint64_t milliseconds, int &waitIndex)
	{
		neosmart_wfmo_t wfmo = new neosmart_wfmo_t_;

		int result = 0;
		int tempResult = pthread_mutex_init(&wfmo->Mutex, 0);
		assert(tempResult == 0);

		tempResult = pthread_cond_init(&wfmo->CVariable, 0);
		assert(tempResult == 0);

		neosmart_wfmo_info_t_ waitInfo;
		waitInfo.Waiter = wfmo;
		waitInfo.WaitIndex = -1;

		wfmo->WaitAll = waitAll;
		wfmo->StillWaiting = true;
		wfmo->RefCount = 1;

		if (waitAll)
		{
			wfmo->Status.EventsLeft = count;
		}
		else
		{
			wfmo->Status.FiredEvent = -1;
		}

		tempResult = pthread_mutex_lock(&wfmo->Mutex);
		assert(tempResult == 0);

		bool done = false;
		waitIndex = -1;

		for (int i = 0; i < count; ++i)
		{
			waitInfo.WaitIndex = i;

			//Must not release lock until RegisteredWait is potentially added
			tempResult = pthread_mutex_lock(&events[i]->Mutex);
			assert(tempResult == 0);

			//Before adding this wait to the list of registered waits, let's clean up old, expired waits while we have the event lock anyway
			events[i]->RegisteredWaits.erase(std::remove_if (events[i]->RegisteredWaits.begin(), events[i]->RegisteredWaits.end(), RemoveExpiredWaitHelper), events[i]->RegisteredWaits.end());

			if (UnlockedWaitForEvent(events[i], 0) == 0)
			{
				tempResult = pthread_mutex_unlock(&events[i]->Mutex);
				assert(tempResult == 0);

				if (waitAll)
				{
					--wfmo->Status.EventsLeft;
					assert(wfmo->Status.EventsLeft >= 0);
				}
				else
				{
					wfmo->Status.FiredEvent = i;
					waitIndex = i;
					done = true;
					break;
				}
			}
			else
			{
				events[i]->RegisteredWaits.push_back(waitInfo);
				++wfmo->RefCount;

				tempResult = pthread_mutex_unlock(&events[i]->Mutex);
				assert(tempResult == 0);
			}
		}

		timespec ts;
		if (!done)
		{
			if (milliseconds == 0)
			{
				result = WAIT_TIMEOUT;
				done = true;
			}
			else if (milliseconds != (uint64_t) -1)
			{
				timeval tv;
				gettimeofday(&tv, NULL);

				uint64_t nanoseconds = ((uint64_t) tv.tv_sec) * 1000 * 1000 * 1000 + milliseconds * 1000 * 1000 + ((uint64_t) tv.tv_usec) * 1000;

				ts.tv_sec = nanoseconds / 1000 / 1000 / 1000;
				ts.tv_nsec = (nanoseconds - ((uint64_t) ts.tv_sec) * 1000 * 1000 * 1000);
			}
		}

		while (!done)
		{
			//One (or more) of the events we're monitoring has been triggered?

			//If we're waiting for all events, assume we're done and check if there's an event that hasn't fired
			//But if we're waiting for just one event, assume we're not done until we find a fired event
			done = (waitAll && wfmo->Status.EventsLeft == 0) || (!waitAll && wfmo->Status.FiredEvent != -1);

			if (!done)
			{
				if (milliseconds != (uint64_t) -1)
				{
					result = pthread_cond_timedwait(&wfmo->CVariable, &wfmo->Mutex, &ts);
				}
				else
				{
					result = pthread_cond_wait(&wfmo->CVariable, &wfmo->Mutex);
				}

				if (result != 0)
				{
					break;
				}
			}
		}

		waitIndex = wfmo->Status.FiredEvent;
		wfmo->StillWaiting = false;

		--wfmo->RefCount;
		assert(wfmo->RefCount >= 0);
		if (wfmo->RefCount == 0)
		{
			wfmo->Destroy();
			delete wfmo;
		}
		else
		{
			tempResult = pthread_mutex_unlock(&wfmo->Mutex);
			assert(tempResult == 0);
		}

		return result;
	}
#endif

    Impl(bool finit, bool fmanual)
    {
        triggered = finit;
		refcount = 0;
		p = createEvent (finit, fmanual);
    }
    ~Impl()
    {
		destroyEvent(p);
//        pthread_spin_destroy(&mutex);
//        pthread_cond_destroy(&cond);
    }
    void setEvent()
    {
		neosmart_event_t event = p;
		int result = pthread_mutex_lock(&event->Mutex);
		assert(result == 0);

		event->State = true;

		//Depending on the event type, we either trigger everyone or only one
		if (event->AutoReset)
		{
#ifdef WFMO
			while (!event->RegisteredWaits.empty())
			{
				neosmart_wfmo_info_t i = &event->RegisteredWaits.front();

				result = pthread_mutex_lock(&i->Waiter->Mutex);
				assert(result == 0);

				--i->Waiter->RefCount;
				assert(i->Waiter->RefCount >= 0);
				if (!i->Waiter->StillWaiting)
				{
					if (i->Waiter->RefCount == 0)
					{
						i->Waiter->Destroy();
						delete i->Waiter;
					}
					else
					{
						result = pthread_mutex_unlock(&i->Waiter->Mutex);
						assert(result == 0);
					}
					event->RegisteredWaits.pop_front();
					continue;
				}

				event->State = false;

				if (i->Waiter->WaitAll)
				{
					--i->Waiter->Status.EventsLeft;
					assert(i->Waiter->Status.EventsLeft >= 0);
					//We technically should do i->Waiter->StillWaiting = Waiter->Status.EventsLeft != 0
					//but the only time it'll be equal to zero is if we're the last event, so no one
					//else will be checking the StillWaiting flag. We're good to go without it.
				}
				else
				{
					i->Waiter->Status.FiredEvent = i->WaitIndex;
					i->Waiter->StillWaiting = false;
				}

				result = pthread_mutex_unlock(&i->Waiter->Mutex);
				assert(result == 0);

				result = pthread_cond_signal(&i->Waiter->CVariable);
				assert(result == 0);

				event->RegisteredWaits.pop_front();

				result = pthread_mutex_unlock(&event->Mutex);
				assert(result == 0);

				return 0;
			}
#endif
			//event->State can be false if compiled with WFMO support
			if (event->State)
			{
				result = pthread_mutex_unlock(&event->Mutex);
				assert(result == 0);

				result = pthread_cond_signal(&event->CVariable);
				assert(result == 0);

				return;
			}
		}
		else
		{
#ifdef WFMO
			for (size_t i = 0; i < event->RegisteredWaits.size(); ++i)
			{
				neosmart_wfmo_info_t info = &event->RegisteredWaits[i];

				result = pthread_mutex_lock(&info->Waiter->Mutex);
				assert(result == 0);

				--info->Waiter->RefCount;
				assert(info->Waiter->RefCount >= 0);

				if (!info->Waiter->StillWaiting)
				{
					if (info->Waiter->RefCount == 0)
					{
						info->Waiter->Destroy();
						delete info->Waiter;
					}
					else
					{
						result = pthread_mutex_unlock(&info->Waiter->Mutex);
						assert(result == 0);
					}
					continue;
				}

				if (info->Waiter->WaitAll)
				{
					--info->Waiter->Status.EventsLeft;
					assert(info->Waiter->Status.EventsLeft >= 0);
					//We technically should do i->Waiter->StillWaiting = Waiter->Status.EventsLeft != 0
					//but the only time it'll be equal to zero is if we're the last event, so no one
					//else will be checking the StillWaiting flag. We're good to go without it.
				}
				else
				{
					info->Waiter->Status.FiredEvent = info->WaitIndex;
					info->Waiter->StillWaiting = false;
				}

				result = pthread_mutex_unlock(&info->Waiter->Mutex);
				assert(result == 0);

				result = pthread_cond_signal(&info->Waiter->CVariable);
				assert(result == 0);
			}
			event->RegisteredWaits.clear();
#endif
			result = pthread_mutex_unlock(&event->Mutex);
			assert(result == 0);

			result = pthread_cond_broadcast(&event->CVariable);
			assert(result == 0);
		}

		return;
		/*pthread_spin_init (&mutex, 0);
        pthread_cond_init (&cond, 0);
        triggered = false;
		*/
    }
    void pulseEvent()
    {
		neosmart_event_t event = p;
		//setEvent(event);
		//resetEvent(event);
    }
    void resetEvent()
    {
		neosmart_event_t event = p;
		int result = pthread_mutex_lock(&event->Mutex);
		assert(result == 0);

		event->State = false;

		result = pthread_mutex_unlock(&event->Mutex);
		assert(result == 0);

		return;
		/* pthread_spin_lock (&mutex);
        triggered = false;
        pthread_spin_unlock (&mutex);*/
    }
	bool lock(ulong dwTimeout)
	{
		neosmart_event_t event = p;
		ulong dwRet = WaitForEvent (event, dwTimeout);
//		DWORD dwRet = ::WaitForSingleObject(m_eventHandle, dwTimeout);
		if (dwRet == WAIT_OBJECT_0 || dwRet == WAIT_ABANDONED)
			return true;
		else
			return false;
/*		pthread_spin_lock (&mutex);
        while (!triggered)
            pthread_cond_wait(&cond, &mutex);
        pthread_spin_unlock (&mutex);*/
	}

	neosmart_event_t createEvent (bool finit, bool fmanual)
	{
		neosmart_event_t event = new neosmart_event_t_;

		int result = pthread_cond_init(&event->CVariable, 0);
		assert(result == 0);

		result = pthread_mutex_init(&event->Mutex, 0);
		assert(result == 0);

		event->State = false;
		event->AutoReset = !fmanual;

		if (finit)
		{
			//setEvent(event);
		}

		return event;
	}
	int destroyEvent(neosmart_event_t event)
	{
		int result = 0;

#ifdef WFMO
		result = pthread_mutex_lock(&event->Mutex);
		assert(result == 0);
		event->RegisteredWaits.erase(std::remove_if (event->RegisteredWaits.begin(), event->RegisteredWaits.end(), RemoveExpiredWaitHelper), event->RegisteredWaits.end());
		result = pthread_mutex_unlock(&event->Mutex);
		assert(result == 0);
#endif

		result = pthread_cond_destroy(&event->CVariable);
		assert(result == 0);

		result = pthread_mutex_destroy(&event->Mutex);
		assert(result == 0);

		delete event;

		return 0;
	}

	neosmart_event_t p;
//    pthread_spinlock_t mutex;
//    pthread_cond_t cond;
    bool triggered;
	int refcount;
};
#else
#include <pthread.h>
struct Event::Impl
{
    Impl(bool finit, bool fmanual)
    {
		if (finit)
			setEvent();
        triggered = finit;
		refcount = 0;
    }
    ~Impl()
    {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }
    void setEvent()
    {
        pthread_mutex_init (&mutex, 0);
        pthread_cond_init (&cond, 0);
        triggered = false;
    }
    void pulseEvent()
    {
        pthread_mutex_lock (&mutex);
        triggered = true;
        pthread_cond_signal (&cond);
        pthread_mutex_unlock (&mutex);
    }
    void resetEvent()
    {
        pthread_mutex_lock (&mutex);
        triggered = false;
        pthread_mutex_unlock (&mutex);
    }
	bool lock(ulong dwTimeout)
	{
        pthread_mutex_lock (&mutex);
        while (!triggered)
            pthread_cond_wait(&cond, &mutex);
        pthread_mutex_unlock (&mutex);
	}
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool triggered;
	int refcount;
};
#endif

Event::Event(bool finit, bool fmanual)
{
    impl = new Event::Impl(finit, fmanual);
}
Event::~Event()
{
    if( CVLIB_XADD(&impl->refcount, -1) == 1 )
        delete impl;
    impl = 0;
}
void Event::setEvent() { impl->setEvent();}
void Event::pulseEvent() { impl->pulseEvent();}
void Event::resetEvent() { impl->resetEvent();}
bool Event::lock(ulong dwTimeout) { return impl->lock(dwTimeout);}


}
