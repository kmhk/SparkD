#include "FileFinder.h"

namespace cvlib
{

#ifdef _MSC_VER

//#if CVLIB_OS == CVLIB_OS_WIN32
FileFinder::FileFinder()
{
	m_pFoundInfo = NULL;
	m_pNextInfo = NULL;
	m_hContext = NULL;
	m_chDirSeparator = '\\';
}

FileFinder::~FileFinder()
{
	close();
}

void FileFinder::close()
{
	if (m_pFoundInfo != NULL)
	{
		delete m_pFoundInfo;
		m_pFoundInfo = NULL;
	}

	if (m_pNextInfo != NULL)
	{
		delete m_pNextInfo;
		m_pNextInfo = NULL;
	}

	if (m_hContext != NULL && m_hContext != INVALID_HANDLE_VALUE)
	{
		closeContext();
		m_hContext = NULL;
	}
}

void FileFinder::closeContext()
{
	::FindClose(m_hContext);
	return;
}

bool FileFinder::findFile(const char* pstrName /* = NULL */,
	ulong /*dwUnused  = 0 */)
{
	close();
	m_pNextInfo = new WIN32_FIND_DATA;
	m_bGotLast = false;

#ifndef __QT__
    if (pstrName == NULL)
		pstrName = "*.*";
	lstrcpy(((WIN32_FIND_DATA*) m_pNextInfo)->cFileName, pstrName);

	m_hContext = ::FindFirstFile(pstrName, (WIN32_FIND_DATA*) m_pNextInfo);

	if (m_hContext == INVALID_HANDLE_VALUE)
	{
		ulong dwTemp = ::GetLastError();
		close();
		::SetLastError(dwTemp);
		return false;
	}

	LPTSTR pstrRoot = m_strRoot.getBufferSetLength(CVLIB_PATH_MAX);
	LPCTSTR pstr = _fullpath(pstrRoot, pstrName, CVLIB_PATH_MAX);

	// passed name isn't a valid path but was found by the API
	assert(pstr != NULL);
	if (pstr == NULL)
	{
		m_strRoot.releaseBuffer(-1);
		close();
		::SetLastError(ERROR_INVALID_NAME);
		return false;
	}
	else
	{
		// find the last forward or backward whack
		LPTSTR pstrBack  = strrchr(pstrRoot, '\\');
		LPTSTR pstrFront = strrchr(pstrRoot, '/');

		if (pstrFront != NULL || pstrBack != NULL)
		{
			if (pstrFront == NULL)
				pstrFront = pstrRoot;
			if (pstrBack == NULL)
				pstrBack = pstrRoot;

			// from the start to the last whack is the root

			if (pstrFront >= pstrBack)
				*pstrFront = '\0';
			else
				*pstrBack = '\0';
		}
		m_strRoot.releaseBuffer(-1);
	}
#endif
	return true;
}

bool FileFinder::matchesMask(ulong dwMask) const
{
	assert(m_hContext != NULL);

	if (m_pFoundInfo != NULL)
		return (!!(((LPWIN32_FIND_DATA) m_pFoundInfo)->dwFileAttributes & dwMask));
	else
		return false;
}

bool FileFinder::getLastAccessTime(FILETIME* pTimeStamp) const
{
	assert(m_hContext != NULL);
	assert(pTimeStamp != NULL);

	if (m_pFoundInfo != NULL && pTimeStamp != NULL)
	{
		*pTimeStamp = ((LPWIN32_FIND_DATA) m_pFoundInfo)->ftLastAccessTime;
		return true;
	}
	else
		return false;
}

bool FileFinder::getLastWriteTime(FILETIME* pTimeStamp) const
{
	assert(m_hContext != NULL);
	assert(pTimeStamp != NULL);

	if (m_pFoundInfo != NULL && pTimeStamp != NULL)
	{
		*pTimeStamp = ((LPWIN32_FIND_DATA) m_pFoundInfo)->ftLastWriteTime;
		return true;
	}
	else
		return false;
}

bool FileFinder::getCreationTime(FILETIME* pTimeStamp) const
{
	assert(m_hContext != NULL);

	if (m_pFoundInfo != NULL && pTimeStamp != NULL)
	{
		*pTimeStamp = ((LPWIN32_FIND_DATA) m_pFoundInfo)->ftCreationTime;
		return true;
	}
	else
		return false;
}

bool FileFinder::isDots() const
{
	assert(m_hContext != NULL);

	// return true if the file name is "." or ".." and
	// the file is a directory

	bool bResult = false;
	if (m_pFoundInfo != NULL && isDirectory())
	{
		LPWIN32_FIND_DATA pFindData = (LPWIN32_FIND_DATA) m_pFoundInfo;
		if (pFindData->cFileName[0] == '.')
		{
			if (pFindData->cFileName[1] == '\0' ||
				(pFindData->cFileName[1] == '.' &&
				 pFindData->cFileName[2] == '\0'))
			{
				bResult = true;
			}
		}
	}

	return bResult;
}

bool FileFinder::findNextFile()
{
	assert(m_hContext != NULL);

	if (m_hContext == NULL)
		return false;
	if (m_pFoundInfo == NULL)
		m_pFoundInfo = new WIN32_FIND_DATA;


	void* pTemp = m_pFoundInfo;
	m_pFoundInfo = m_pNextInfo;
	m_pNextInfo = pTemp;

	return ::FindNextFileA(m_hContext, (LPWIN32_FIND_DATA) m_pNextInfo) != 0;
}

String FileFinder::getFileURL() const
{
	assert(m_hContext != NULL);

	String strResult("file://");
	strResult += getFilePath();
	return strResult;
}

String FileFinder::getRoot() const
{
	assert(m_hContext != NULL);

	return m_strRoot;
}

String FileFinder::getFilePath() const
{
	assert(m_hContext != NULL);

	String strResult = m_strRoot;
	if (strResult[strResult.length()-1] != '\\' &&
		strResult[strResult.length()-1] != '/')
		strResult += m_chDirSeparator;
	strResult += getFileName();
	return strResult;
}

String FileFinder::getFileTitle() const
{
	assert(m_hContext != NULL);

	String strFullName = getFileName();
	String strResult;

	_splitpath(strFullName, NULL, NULL, strResult.getBuffer(CVLIB_PATH_MAX), NULL);
	strResult.releaseBuffer();
	return strResult;
}

String FileFinder::getFileName() const
{
	assert(m_hContext != NULL);

	String ret;

	if (m_pFoundInfo != NULL)
        ret = (const char*)((LPWIN32_FIND_DATA) m_pFoundInfo)->cFileName;
	return ret;
}

ulong FileFinder::length() const
{
	assert(m_hContext != NULL);

	if (m_pFoundInfo != NULL)
		return ((LPWIN32_FIND_DATA) m_pFoundInfo)->nFileSizeLow;
	else
		return 0;
}

void FileFinder::AssertValid() const
{
	// if you trip the assert in the else side, you've called
	// a Get() function without having done at least one
	// FindNext() call

	if (m_hContext == NULL)
		assert(m_pFoundInfo == NULL && m_pNextInfo == NULL);
	else
		assert(m_pFoundInfo != NULL && m_pNextInfo != NULL);

}

#endif

}
