/*!
 * \file	Timer.h
 * \ingroup base
 * \brief	
 * \author	
 */

#pragma once

#include <time.h>
#include "Object.h"

namespace cvlib
{

	/**
	 * @brief
	 */
	class CVLIB_DECLSPEC Timer : public Object
	{
	private:
#ifdef _MSC_VER
#if CVLIB_OS == CVLIB_OS_WIN32
		__int64 m_Frequency;
		__int64 m_StartTime;
		__int64 m_StopTime;
#endif

		static time_t base_time;
#endif

		bool is_running;
		double start_time;
		double total_time;
	public:

		// Construction and Destruction
		Timer();
		virtual ~Timer();

		void reset();

		void stop();

		void resume();

		double getElapsedTime();

		char* ascCurrentTime();

		static double getRunTime();

		void sleep(int nMilliseconds);

	};

}