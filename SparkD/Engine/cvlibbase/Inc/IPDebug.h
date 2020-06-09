#pragma once

#include "Mat.h"
#include "StringArray.h"

namespace cvlib
{

	class CVLIB_DECLSPEC IPDebug
	{
	public:
		IPDebug();
		~IPDebug();

		//! logging
		static void	resetLog();
		static void logging(const char* fmt, ...);
		static void	loggingTime(const char* fmt, ...);
		static void	getLog(StringArray& log);

		//! mat array
		static void	resetDebug();
		static void	addDebug(const Mat&, const char* comment = 0);
		static void	getDebugInfoX(Mat& image, int nMaxCol = -1);
		static Mat*	getDebugInfoX(int nMaxCol = -1);
		static void	getDebugInfoY(Mat& image, int nMaxRow = -1);
		static Mat*	getDebugInfoY(int nMaxRow = -1);
	};

}