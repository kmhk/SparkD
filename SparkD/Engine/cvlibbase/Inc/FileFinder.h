/*!
 * \file    XFile.h
 * \ingroup base
 * \brief   .
 * \author  
 */

#pragma once

#include "cvlibmacros.h"

#ifdef _MSC_VER
#include <windows.h>
#endif

#include "cvlibbaseDef.h"
#include "Object.h"
#include "String.hpp"

namespace cvlib
{

#ifdef _MSC_VER
//#if CVLIB_OS == CVLIB_OS_WIN32
class CVLIB_DECLSPEC FileFinder : public Object
{
public:
	FileFinder();
	virtual ~FileFinder();
	
	// Attributes
public:
	ulong length() const;
	virtual String getFileName() const;
	virtual String getFilePath() const;
	virtual String getFileTitle() const;
	virtual String getFileURL() const;
	virtual String getRoot() const;
	
	virtual bool getLastWriteTime(FILETIME* pTimeStamp) const;
	virtual bool getLastAccessTime(FILETIME* pTimeStamp) const;
	virtual bool getCreationTime(FILETIME* pTimeStamp) const;

	virtual bool matchesMask(ulong dwMask) const;
	
	virtual bool isDots() const;
	// these aren't virtual because they all use matchesMask(), which is
	bool isReadOnly() const;
	bool isDirectory() const;
	bool isCompressed() const;
	bool isSystem() const;
	bool isHidden() const;
	bool isTemporary() const;
	bool isNormal() const;
	bool isArchived() const;
	
	// Operations
	void close();
	virtual bool findFile(const char* pstrName = NULL, ulong dwUnused = 0);
	virtual bool findNextFile();
	
protected:
	virtual void closeContext();
	
	// Implementation
protected:
	void* m_pFoundInfo;
	void* m_pNextInfo;
	HANDLE m_hContext;
	bool m_bGotLast;
	String m_strRoot;
	char m_chDirSeparator;     // not '\\' for Internet classes
	
	void AssertValid() const;
};

inline bool FileFinder::isReadOnly() const
{ return matchesMask(FILE_ATTRIBUTE_READONLY); }
inline bool FileFinder::isDirectory() const
{ return matchesMask(FILE_ATTRIBUTE_DIRECTORY); }
inline bool FileFinder::isCompressed() const
{ return matchesMask(FILE_ATTRIBUTE_COMPRESSED); }
inline bool FileFinder::isSystem() const
{ return matchesMask(FILE_ATTRIBUTE_SYSTEM); }
inline bool FileFinder::isHidden() const
{ return matchesMask(FILE_ATTRIBUTE_HIDDEN); }
inline bool FileFinder::isTemporary() const
{ return matchesMask(FILE_ATTRIBUTE_TEMPORARY); }
inline bool FileFinder::isNormal() const
{ return matchesMask(FILE_ATTRIBUTE_NORMAL); }
inline bool FileFinder::isArchived() const
{ return matchesMask(FILE_ATTRIBUTE_ARCHIVE); }

#endif

}