
/*!
 * \file    IILog.h
 * \ingroup	base
 * \brief   
 * \author  
 */

#pragma once

#include "cvlibmacros.h"
#include <stdio.h>

namespace cvlib
{

	class CVLIB_DECLSPEC IILog
	{
	public:
		IILog(const char* szfilename = 0);
		virtual ~IILog();

		void setLogFile(const char* szFileName);
		const char* getLogFile() const;
		void transferStatus(const char* szStatus);
		void transferStatus(char* fmt, ...);
		void info(char* fmt, ...);
		void infoflush();
		bool isValid() const;
	private:
		FILE*	m_pFile;
		char	m_szfilename[260];
	};

}