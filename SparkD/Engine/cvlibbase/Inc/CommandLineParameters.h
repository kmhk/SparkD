
/*!
 * \file CommandLineParameters.h
 * \brief CommandLine
 * \author 
 */

#pragma once

#include "cvlibbaseDef.h"
#include "String.hpp"

namespace cvlib
{

	/**
	 * @brief : CommandLine
	 */
	class CVLIB_DECLSPEC CommandLineParameters
	{
	public:
		CommandLineParameters(char* szCommandLine = NULL);
		virtual ~CommandLineParameters();

		bool checkHelp(const bool fNoSwitches = false);

		int paramCount() { return m_nParamCount; }
		String paramLine();
		String commandLine();
		String paramStr(int index, const bool fGetAll = false);
		int paramInt(int index);

		int firstNonSwitchIndex();
		String firstNonSwitchStr();
		int switchCount();
		int switchParamIndex(const char *sz, const bool fCase = false);
		String getSwitchStr(const char *sz, const char *szDefault = "", const bool fCase = false);
		int getSwitchInt(const char *sz, const int iDefault = -1, const bool fCase = false);
		String getNonSwitchStr(const bool fBreakAtSwitch = true, const bool fFirstOnly = false);

	private:
		char* m_pszParams[100];
		int	m_nMaxParams;
		int	m_nParamCount;
		char* m_pszCmdLineDup;
		const char* m_szSwitchChars;
		bool isSwitch(const char *sz);
		int createParameterFromString(char *pszParams, char *argv[], int max);
	};

}