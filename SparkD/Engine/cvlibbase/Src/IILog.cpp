#include "IILog.h"
#include <assert.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include "cvlibutil.h"

namespace cvlib {
	

IILog::IILog(const char* szfilename)
{ 
/*#ifdef WIN32
	m_hwnd = NULL;
#endif*/
	m_pFile = NULL;
	m_szfilename[0]=0;
	if (szfilename)
	{
		setLogFile (szfilename);
	}
}

IILog::~IILog()
{
	if (m_pFile)		fclose(m_pFile);
}
const char* IILog::getLogFile() const
{
	return m_szfilename;
}

/*#ifdef WIN32
void IILog::setLogWindow (HWND hwnd, char* szFileName)
{
	if (hwnd)
		m_hwnd = hwnd;
	if (szFileName != NULL)
		m_pFile = fopen (szFileName, "a+");
}
#endif*/
void IILog::setLogFile (const char* szFileName)
{
	if (szFileName != NULL)
	{
		strcpy (m_szfilename, szFileName);
		cvutil::mkDir(szFileName);
		if (m_pFile)
			fclose (m_pFile);
		m_pFile = fopen (szFileName, "a+");
	}
}

void IILog::transferStatus (const char* szStatus)
{
	static char szLF[] = {0x0d, 0x0a, 0x0};
	assert (szStatus != NULL);

/*#ifdef WIN32
	if (m_hwnd)
	{
		char* szTemp = NULL;
		static int nLen = 1000;
		MALLOC (szTemp, nLen + strlen(szStatus) + 3, char);
		GetWindowText(m_hwnd, szTemp, nLen);
		strcat (szTemp, szStatus);
		strcat (szTemp, szLF);
		::SetWindowText(m_hwnd, szTemp);

		nLen += strlen (szStatus) + 2;

		FREE (szTemp);
	}
#endif*/
	if (m_pFile)
	{
		fprintf (m_pFile, "%s%s", szStatus, szLF);
		fflush (m_pFile);
	}
	{
		printf ("%s\n", szStatus);
	}
}
void IILog::transferStatus (char* fmt, ...)
{
	static char szLF[] = {0x0d, 0x0a, 0x0};

	va_list ap;
	va_start(ap,fmt);
	vprintf(fmt,ap);
	printf("\n");
	if (m_pFile)
	{
		vfprintf (m_pFile, fmt,ap);
		fprintf (m_pFile, "%s", szLF);
		fflush (m_pFile);
	}
	va_end(ap);
}

void IILog::info(char* fmt, ...)
{
	va_list ap;
	va_start(ap,fmt);
	vprintf(fmt,ap);
	va_end(ap);
}

void IILog::infoflush()
{
//	fflush(stdout);
}
bool IILog::isValid() const
{
	if (m_pFile)
		return true;
	return false;
}

}