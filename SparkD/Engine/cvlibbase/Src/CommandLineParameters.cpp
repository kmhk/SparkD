/*!
 * \file CommandLineParameters.cpp
 * \brief CommandLine
 * \author 
 */

#include "cvlibmacros.h"
#ifdef _MSC_VER
#include <windows.h>
#endif

#include "CommandLineParameters.h"
#include <string.h>

namespace cvlib
{

static char mszSwitchChars[] = "-/";

CommandLineParameters::CommandLineParameters(char* szCommandLine /*= 0*/)
{
#ifdef _MSC_VER
//#if CVLIB_OS == CVLIB_OS_WIN32
#ifndef __QT__
    if (szCommandLine)
		m_pszCmdLineDup = _strdup(szCommandLine);
	else
		m_pszCmdLineDup = _strdup(GetCommandLine());
#endif
#else
	if (szCommandLine)
	{
		m_pszCmdLineDup = (char*)malloc (strlen(szCommandLine) + 1);
		strcpy (m_pszCmdLineDup, szCommandLine);
	}
	else
		assert (false);
#endif
	m_nMaxParams = 100;
	m_nParamCount = createParameterFromString(m_pszCmdLineDup, m_pszParams, m_nMaxParams);

	m_szSwitchChars = mszSwitchChars;
}

CommandLineParameters::~CommandLineParameters()
{
	if (m_pszCmdLineDup)
	{
		free (m_pszCmdLineDup);
		m_pszCmdLineDup = NULL;
	}
}

int CommandLineParameters::createParameterFromString(char *pszParams, char *argv[], int max)
{
    int argc = 0;
    if (pszParams) 
	{
        char *p = pszParams;
        while (*p && (argc < max)) 
		{
            while (*p == ' ') 
			{
                p++;
            }
            if (!*p) 
			{
                break;
            }
            if (*p == '"') 
			{
                p++;
                argv[argc++] = p;
                while (*p && *p != '"') 
				{
                    p++;
                }
            } else 
			{
                argv[argc++] = p;
                while (*p && *p != ' ') 
				{
                    p++;
                }
            }
            if (*p) 
			{
                *p++ = 0;
            }
        }
    }
    return argc;
}

bool CommandLineParameters::checkHelp(const bool fNoSwitches /*= false */)
{
     if (fNoSwitches && (m_nParamCount < 2)) return true;
     if (m_nParamCount < 2) return false;
     if (strcmp(paramStr(1).str(),"-?") == 0) return true;
     if (strcmp(paramStr(1).str(),"/?") == 0) return true;
     if (strcmp(paramStr(1).str(),"?") == 0) return true;
     return false;
}

String CommandLineParameters::paramStr(const int index, const bool fGetAll /* = false */)
{
    if ((index < 0) || (index >= m_nParamCount)) {
        return String("");
    }
    String s = String(m_pszParams[index]);
    if (fGetAll) {
        for (int i = index+1;i < m_nParamCount; i++) {
              s += " ";
              s += m_pszParams[i];
        }
    }
    return s;
}

int CommandLineParameters::paramInt(const int index)
{
    return atoi(paramStr(index));
}

String CommandLineParameters::paramLine()
{
    String s;
#ifdef _MSC_VER
//#if CVLIB_OS == CVLIB_OS_WIN32
#ifndef __QT__
    char *p = strchr(GetCommandLine(),' ');
    if (p) {
        s.format("%s",p+1);
    }
#endif
#endif
    return s;
}

String CommandLineParameters::commandLine()
{
    String s;
#ifdef _MSC_VER
//#if CVLIB_OS == CVLIB_OS_WIN32
    s.format("%s",GetCommandLine());
#endif
    return s;
}

bool CommandLineParameters::isSwitch(const char *sz)
{
    return (strchr(m_szSwitchChars,sz[0]) != NULL);
}

int CommandLineParameters::switchCount()
{
    int count = 0;
    for (int i = 1;i < m_nParamCount; i++) {
        if (isSwitch(m_pszParams[i])) count++;
    }
    return count;
}

int CommandLineParameters::firstNonSwitchIndex()
{
    for (int i = 1;i < m_nParamCount; i++) {
        if (!isSwitch(m_pszParams[i])) {
            return i;
        }
    }
    return 0;
}

String CommandLineParameters::firstNonSwitchStr()     // 499.5 04/16/01 12:17 am
{
    // returns the first non-switch, handles lines such as:
    // [options] file [specs]
    return getNonSwitchStr(false,true);
}

//////////////////////////////////////////////////////////////////////////
// switchParameter() will return the parameter index if the switch exist.
// Return 0 if not found.  The logic will allow for two types of
// switches:
//
//          /switch value
//          /switch:value
//
// DO NOT PASS THE COLON. IT IS AUTOMATICALLY CHECKED.  In other
// words, the following statements are the same:
//
//         switchParameter("server");
//         switchParameter("-server");
//         switchParameter("/server");
//
// to handle the possible arguments:
//
//         /server:value
//         /server value
//         -server:value
//         -server value
//

int CommandLineParameters::switchParamIndex(const char *sz, const bool fCase /* = false */)
{
    if (!sz || !sz[0]) {
        return 0;
    }

    char sz2[255];
    strncpy(sz2,sz,sizeof(sz2)-1);
    sz2[sizeof(sz2)-1] = 0;

    char *p = sz2;
    if (strchr(m_szSwitchChars,*p) != NULL) p++;

    // check for abbrevation

    size_t amt = 0;
    char *abbr = strchr(p,'*');
    if (abbr) {
        *abbr = 0;
        amt = strlen(p);
        strcpy(abbr,abbr+1);
    }

    for (int i = 1;i < m_nParamCount; i++) {
      if (!isSwitch(m_pszParams[i])) continue;
      char *pColon = strchr(&m_pszParams[i][1],':');
      if (pColon && (amt == 0)) { amt = strlen(p); }

      if (fCase)
	  {
        if (amt > 0) {
          if (strncmp(p,&m_pszParams[i][1],strlen(p)) != 0) continue; // 450.6b20
          if (strncmp(p,&m_pszParams[i][1],amt) == 0) return i;
        } else {
          if (strcmp(p,&m_pszParams[i][1]) == 0) return i;
        }
      }
	  else
	  {
#if CVLIB_OS == CVLIB_OS_WIN32
	#if !defined __MINGW32__
        if (amt > 0) {
          if (_strnicmp(p,&m_pszParams[i][1],strlen(p)) != 0) continue; // 450.6b20
          if (_strnicmp(p,&m_pszParams[i][1],amt) == 0) return i;
        } else {
          if (_stricmp(p,&m_pszParams[i][1]) == 0) return i;
        }
	#else
		  if (amt > 0) {
			  if (strncmp(p,&m_pszParams[i][1],strlen(p)) != 0) continue; // 450.6b20
			  if (strncmp(p,&m_pszParams[i][1],amt) == 0) return i;
		  } else {
			  if (strcmp(p,&m_pszParams[i][1]) == 0) return i;
		  }
	#endif
#else
		  if (amt > 0) {
			  if (strncmp(p,&m_pszParams[i][1],strlen(p)) != 0) continue; // 450.6b20
			  if (strncmp(p,&m_pszParams[i][1],amt) == 0) return i;
		  } else {
			  if (strcmp(p,&m_pszParams[i][1]) == 0) return i;
		  }
#endif
      }
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
// GetSwitchStr() will return the string for the given switch. The logic
// will allow for two types of switches:
//
//   /switch value
//   /switch:value
//

String CommandLineParameters::getSwitchStr(const char *sz,
                                             const char *szDefault, /* = "" */
                                             const bool fCase /* = false */
                                            )
{
    int idx = switchParamIndex(sz,fCase);
    if (idx > 0) {
        String s = paramStr(idx);
        int n = s.find(':');
        if (n > -1) {
            return s.mid(n+1);
        } else {
          if ((idx+1) < m_nParamCount) {
              if (!isSwitch(m_pszParams[idx+1])) {
                  return String(m_pszParams[idx+1]);
              }
          }
        }
        //return szDefault;
    }
    return String(szDefault);
}

int CommandLineParameters::getSwitchInt(const char *sz,
                                          const int iDefault, /* = 0 */
                                          const bool fCase /* = false */
                                         )
{
    char szDefault[25];
    sprintf(szDefault,"%d",iDefault);
    return atoi(getSwitchStr(sz,szDefault,fCase));
}

String CommandLineParameters::getNonSwitchStr(
                                const bool bBreakAtSwitch, /* = true */
                                const bool bFirstOnly /* = false */)
{
    String sLine = String("");
    int i = 1;
    while (i < m_nParamCount) 
	{
        if (isSwitch(m_pszParams[i])) 
		{
            if (bBreakAtSwitch) 
				break;
        } 
		else 
		{
            sLine += m_pszParams[i];
            if (bFirstOnly) break;
            sLine += " ";
        }
        i++;
    }
    sLine.trimRight();
    return sLine;
}

}
