/*!
 * \file	Timer.cpp
 * \ingroup base
 * \brief
 * \author
 */

#include "Timer.h"

#ifdef _MSC_VER
#include <windows.h>
#endif

#if CVLIB_OS == CVLIB_OS_APPLE || CVLIB_OS == CVLIB_OS_LINUX
#include <sys/times.h>
#include <unistd.h>
#endif

#pragma warning (push)
#pragma warning (disable : 4127)

namespace cvlib
{

#ifdef _MSC_VER
	time_t Timer::base_time = 0;
#endif

	Timer::Timer()
	{
#ifdef _MSC_VER
		while (!base_time)
			time(&base_time);
#endif

		total_time = 0;
		is_running = false;
		start_time = getRunTime();
#ifdef _MSC_VER
		//#if CVLIB_OS == CVLIB_OS_WIN32
		m_StartTime = 0;
		m_StopTime = 0;
		QueryPerformanceFrequency((LARGE_INTEGER*)&m_Frequency);
		QueryPerformanceCounter((LARGE_INTEGER*)&m_StartTime);
#endif
	}

	Timer::~Timer()
	{
	}

	char* Timer::ascCurrentTime()
	{
		struct tm *newtime;
		time_t aclock;

		time(&aclock);                 /* Get time in seconds */

		newtime = localtime(&aclock);  /* convert time to struct */
		return asctime(newtime);
	}

	double Timer::getRunTime()
	{
#ifdef _MSC_VER
		time_t truc_foireux;
		time(&truc_foireux);
		return (double)(difftime(truc_foireux, base_time));
#endif
#if CVLIB_OS == CVLIB_OS_APPLE || CVLIB_OS == CVLIB_OS_LINUX
		struct tms current;
		times(&current);
		double norm = (double)sysconf(_SC_CLK_TCK);
		return(((double)current.tms_utime) / norm);
#endif
	}

	void Timer::reset()
	{
#ifdef _MSC_VER
		//#if CVLIB_OS == CVLIB_OS_WIN32
		m_StartTime = 0;
		m_StopTime = 0;
		QueryPerformanceCounter((LARGE_INTEGER*)&m_StartTime);
#endif
		total_time = 0;
		start_time = getRunTime();
	}

	void Timer::stop()
	{
		if (!is_running)
			return;

#ifdef _MSC_VER
		//#if CVLIB_OS == CVLIB_OS_WIN32
		QueryPerformanceCounter((LARGE_INTEGER*)&m_StopTime);
		total_time += static_cast<double>(m_StopTime - m_StartTime) / m_Frequency;
#else
		double current_time = getRunTime() - start_time;
		total_time += current_time;
#endif
		is_running = false;
	}

	void Timer::resume()
	{
		if (is_running)
			return;

#ifdef _MSC_VER
		//#if CVLIB_OS == CVLIB_OS_WIN32
		QueryPerformanceCounter((LARGE_INTEGER*)&m_StartTime);
#else
		start_time = getRunTime();
#endif
		is_running = true;
	}

	double Timer::getElapsedTime()
	{
		if (is_running)
		{
#ifdef _MSC_VER
			//#if CVLIB_OS == CVLIB_OS_WIN32
			__int64 curTime;
			QueryPerformanceCounter((LARGE_INTEGER*)&curTime);
			return total_time + static_cast<double>(curTime - m_StartTime) / (double)m_Frequency;
#else
			double current_time = getRunTime() - start_time;
			return(total_time + current_time);
#endif
		}
		else
			return total_time;
	}

	void Timer::sleep(int nMilliseconds)
	{
		reset();
		double rSecond = (double)nMilliseconds / 1000.0;
		while (true)
		{
			double rNow = getRunTime();
			double rDelay = rNow - start_time;
			if (rDelay > rSecond)
				break;
		}
	}

}

#pragma warning (pop)