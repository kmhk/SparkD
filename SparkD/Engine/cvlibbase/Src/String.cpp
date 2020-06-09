#include "cvlibmacros.h"
#include "String.hpp"
#include "Template.h"

#include <string.h>

#pragma warning (push)
#pragma warning (disable : 4127)

#if CVLIB_OS != CVLIB_OS_APPLE && CVLIB_OS != CVLIB_OS_LINUX
  #include <tchar.h>
  #define CUCHAR const uchar
#else
   #include <ctype.h>
   #define CUCHAR const uchar
#endif

#if CVLIB_OS == CVLIB_OS_APPLE
#include <_types.h>
//#define __int64 __int64_t
#elif CVLIB_OS == CVLIB_OS_LINUX
#include <sys/types.h>
#define __int64 int64_t
#endif

#ifndef _MSC_VER
#if __GNUC__ < 4
#include <mbstring.h>
#endif
#endif

#define lstrlenA lstrlen

namespace cvlib
{

#ifndef _MSC_VER
//#if CVLIB_OS != CVLIB_OS_WIN32

#define _T 
#if __GNUC__ >= 4
	unsigned char* _mbsupr (unsigned char* str)
	{
		return str;
	}
	unsigned char* _mbslwr (unsigned char* str)
	{
		return str;
	}
	unsigned char* _mbsrev (unsigned char* str)
	{
		return str;
	}
	int _ismbblead (unsigned int c)
	{
		return (int)c;
	}
	int _ismbcspace(unsigned int c)
	{
		return (int)c;
	}
#endif
    /*size_t _tcsspn(const char* str1, const char* str2)
	{
		return 0;
	}
	
	int _ttoi (const char* str)
	{
		return atoi(str);
	}
	size_t _tcscspn(const char* str1, const char* str2)
	{
		return 0;
	}
	size_t _tcslen(const char* str)
	{
        return 0;//		return strlen(str);
	}

	int _tcsncmp(const char* str1, const char* str2, size_t n)
	{
		return strncmp(str1, str2, n);
	}
	int wcslen (const wchar_t* str)
	{
		return 0;
    }*/
    int _istdigit(int c)
    {
        return 0;//isdigit(c);
    }
#endif

/*get the length in bytes(ANSI version)
or characters(Unicode version) of the specified string*/
static int lstrlen(const char* lpString)
{
  int nLen = 0;
  int c;
  char * pc;
  pc = (char *)lpString;
  ulong bpreValue = 0;
  ulong i;
  for( i = 0; i < strlen(pc); i++)
  {
    c = *pc++;
    if(c < 0x80)
      nLen++;
    else if(bpreValue)
    {
      nLen++;
      bpreValue = 0; continue;
    }
    else
    {
      bpreValue = 1; continue;
    }
  }
  return nLen;
}

static int InterlockedIncrement(int* pData)
{
  int i;
  i = (int) ++(*pData);
  if(i < 0)
	  return -1;
  else if(i > 0)
	  return 1;
  else
	  return 0;
}
static int InterlockedDecrement(int* pData)
{
  int i;
  i = (int) --(*pData);
  if(i < 0)
	  return -1;
  else if(i > 0)
	  return 1;
  else
	  return 0;
  
}

/////////////////////////////////////////////////////////////////////////////
// static class data, special inlines
// SafxChNil is left for backward compatibility
static char SafxChNil = '\0';


// For an empty string, m_pchData will point here
// (note: avoids special case of checking for NULL m_pchData)
// empty string data (and locked)
int _SafxInitData[] = { -1, 0, 0, 0  };
StringData* _SafxDataNil = (StringData*)&_SafxInitData;
char *_SafxPchNil = (char*)(((uchar*)&_SafxInitData)+sizeof(StringData));
// special function to make SafxEmptyString work even during initialization
#define SafxEmptyString ((String&)*(String*)&_SafxPchNil)

String g_StrBuf;//Global String buffer:MD_906 by pch


template<> void constructElements1<String> (String* pElements, int nCount)
{
	for (; nCount--; ++pElements)
		memcpy(pElements, &SafxEmptyString, sizeof(*pElements));
}

template<> void destructElements1<String> (String* pElements, int nCount)
{
	for (; nCount--; ++pElements)
		pElements->~String();
}

template<> void copyElements1<String> (String* pDest, const String* pSrc, int nCount)
{
	for (; nCount--; ++pDest, ++pSrc)
		*pDest = *pSrc;
}

template<> uint cvlHashKey<const String&>(const String& key)
{
	const char* data=key.str();
	uint nHash = 0;
	while (*data)
		nHash = (nHash<<5) + nHash + *data++;
	return nHash;
}

String& SGetEmptyString()
	{ return *(String*)&_SafxPchNil; }

StringData* String::getData() const
{
	return ((StringData*)m_pchData)-1; 
}
void String::Init()
{ 
	m_pchData = SafxEmptyString.m_pchData; 
}

String::String()
{
	m_pchData = SafxEmptyString.m_pchData; 
}

String::String(char* lpsz)
{ 
	Init();
	*this = (char *)lpsz; 
}

String::String(const char* lpsz)
{ 
	Init();
	*this = (char *)lpsz; 
}

int String::length() const
{ 
	return getData()->nDataLength; 
}

int String::getAllocLength() 
{ 
	return getData()->nAllocLength; 
}

ulong String::isEmpty() const
{ 
	return getData()->nDataLength == 0; 
}


int  String::safeStrlen(const char * lpsz) const
{ 
	return (int)((lpsz == NULL) ? 0 : strlen(lpsz));
}

// String support (windows specific)
int String::compare(const char * lpsz) const
{ 
//	return _mbscmp((CUCHAR *)m_pchData, (CUCHAR *)lpsz); 
	return strcmp(m_pchData, lpsz); 
}
// MBCS/Unicode aware
int String::compare(const String& str) const
{
	const char *ch;
	ch = str.str();
//	return _mbscmp((CUCHAR *)m_pchData, (CUCHAR *)ch);
	return strcmp(m_pchData, ch);
}

int String::compareNoCase(const char * lpsz) const
{ 
//	return _mbsicmp((CUCHAR *)m_pchData, (CUCHAR *)lpsz); 
	return strcmp(m_pchData, lpsz); 
}
// MBCS/Unicode aware
// String::collate is often slower than compare but is MBSC/Unicode
//  aware as well as locale-sensitive with respect to sort order.
int String::collate(char * lpsz) const
{ 
//	return _mbscoll((CUCHAR *)m_pchData, (CUCHAR *)lpsz); 
	return strcoll(m_pchData, lpsz); 
}   // locale sensitive

int String::collateNoCase(char * lpsz) const
{ 
//	return _mbsicoll((CUCHAR *)m_pchData, (CUCHAR *)lpsz); 
	return strcoll(m_pchData, lpsz); 
}   // locale sensitive

uchar String::getAt(int nIndex) const
{
	return m_pchData[nIndex];
}

uchar String::operator[](int nIndex) const
{
	return m_pchData[nIndex];
}

String::String(const String & stringSrc)
{
	if (stringSrc.getData()->nRefs >= 0)
	{
		m_pchData = stringSrc.m_pchData;
		InterlockedIncrement(&getData()->nRefs);
	}
	else
	{
		Init();
		*this = stringSrc.m_pchData;
	}
}
/*
#ifndef _DEBUG

#pragma warning(disable: 4074)
#pragma init_seg(compiler)

#define ROUND(x,y) (((x)+(y-1))&~(y-1))
#define ROUND4(x) ROUND(x, 4)
AFX_STATIC CFixedAlloc _afxAlloc64(ROUND4(65*sizeof(char)+sizeof(StringData)));
AFX_STATIC CFixedAlloc _afxAlloc128(ROUND4(129*sizeof(char)+sizeof(StringData)));
AFX_STATIC CFixedAlloc _afxAlloc256(ROUND4(257*sizeof(char)+sizeof(StringData)));
AFX_STATIC CFixedAlloc _afxAlloc512(ROUND4(513*sizeof(char)+sizeof(StringData)));

#endif //!_DEBUG
*/
void String::AllocBuffer(int nLen)
// always allocate one extra character for '\0' termination
// assumes [optimistically] that data length will equal allocation length
{
	if (nLen == 0)
		Init();
	else
	{
		StringData* pData;
/*
#ifndef _DEBUG
		if (nLen <= 64)
		{
			pData = (StringData*)_afxAlloc64.alloc();

			pData->nAllocLength = 64;
		}
		else if (nLen <= 128)
		{
			pData = (StringData*)_afxAlloc128.alloc();

			pData->nAllocLength = 128;
		}
		else if (nLen <= 256)
		{
			pData = (StringData*)_afxAlloc256.alloc();
			pData->nAllocLength = 256;
		}
		else if (nLen <= 512)
		{
			pData = (StringData*)_afxAlloc512.alloc();
			pData->nAllocLength = 512;
		}
		else
#endif
*/
		{
			pData = (StringData*)
//				new uchar[sizeof(StringData) + (nLen+1)*sizeof(char)];
				malloc(sizeof(StringData) + (nLen+1)*sizeof(char));
			pData->nAllocLength = nLen;
		}
		pData->nRefs = 1;
		pData->data()[nLen] = '\0';
		pData->nDataLength = nLen;
		m_pchData = pData->data();
	}
}

void String::FreeData(StringData* pData)
{
/*
#ifndef _DEBUG
	int nLen = pData->nAllocLength;
	if (nLen == 64)
		_afxAlloc64.Free(pData);
	else if (nLen == 128)
		_afxAlloc128.Free(pData);
	else if (nLen == 256)
		_afxAlloc256.Free(pData);
	else  if (nLen == 512)
		_afxAlloc512.Free(pData);

	else
	{
		delete[] (uchar*)pData;
	}
*/
//#else
//	delete[] (uchar*)pData;
	free(pData);
//#endif
}

void String::release()
{
	if (getData() != _SafxDataNil)
	{
		if (InterlockedDecrement(&getData()->nRefs) <= 0)
			FreeData(getData());
		Init();
	}
}

void String::release(StringData* pData)
{
	if (pData != _SafxDataNil)
	{
		if (InterlockedDecrement(&pData->nRefs) <= 0)
			FreeData(pData);
	}
}

void String::empty()
{
	if (getData()->nDataLength == 0)
		return;
	if (getData()->nRefs >= 0)
		release();
	else
		*this = &SafxChNil;
}

void String::CopyBeforeWrite()
{
	if (getData()->nRefs > 1)
	{
		StringData* pData = getData();
		release();
		AllocBuffer(pData->nDataLength);
		memcpy(m_pchData, pData->data(), (pData->nDataLength+1)*sizeof(char));
	}
}

void String::AllocBeforeWrite(int nLen)
{
	if (getData()->nRefs > 1 || nLen > getData()->nAllocLength)
	{
		release();
		AllocBuffer(nLen);
	}
}

String::~String()
//  free any attached data
{
	if (getData() != _SafxDataNil)
	{
		if (InterlockedDecrement(&getData()->nRefs) <= 0)
			FreeData(getData());
	}

}

//////////////////////////////////////////////////////////////////////////////
// Helpers for the rest of the implementation

void String::AllocCopy(String& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const
{
	// will clone the data attached to this string
	// allocating 'nExtraLen' characters
	// Places results in uninitialized string 'dest'
	// Will copy the part or all of original data to start of new string

	int nNewLen = nCopyLen + nExtraLen;
	if (nNewLen == 0)
	{
		dest.Init();
	}
	else
	{
		dest.AllocBuffer(nNewLen);
		memcpy(dest.m_pchData, m_pchData+nCopyIndex, nCopyLen*sizeof(char));
	}
}

//////////////////////////////////////////////////////////////////////////////
// More sophisticated ruction


//////////////////////////////////////////////////////////////////////////////
// Assignment operators
//  All assign a new value to the string
//      (a) first see if the buffer is big enough
//      (b) if enough room, copy on top of old buffer, set size and type
//      (c) otherwise free old string data, and create a new one
//
//  All routines return the new string (but as a 'String&' so that
//      assigning it again will cause a copy, eg: s1 = s2 = "hi there".
//

void String::AssignCopy(int nSrcLen, const char* lpszSrcData)
{
	AllocBeforeWrite(nSrcLen);
	memcpy(m_pchData, lpszSrcData, nSrcLen*sizeof(char));
	getData()->nDataLength = nSrcLen;
	m_pchData[nSrcLen] = '\0';
}

const String& String::operator=(const String & stringSrc)
{
	if (m_pchData != stringSrc.m_pchData)
	{
		if ((getData()->nRefs < 0 && getData() != _SafxDataNil) ||
			stringSrc.getData()->nRefs < 0)
		{
			// actual copy necessary since one of the strings is locked
			AssignCopy(stringSrc.getData()->nDataLength, stringSrc.m_pchData);
		}
		else
		{
			// can just copy references around
			release();
			m_pchData = stringSrc.m_pchData;
			InterlockedIncrement(&getData()->nRefs);
		}
	}
	return *this;
}

const String& String::operator=(char ch)
{
	if(ch == 0)
		return *this;
	AssignCopy(1, &ch);
	return *this;
}

const String& String::operator=(const char* lpsz)
{
	AssignCopy(safeStrlen(lpsz), lpsz);
	return *this;
}

void String::concatCopy(int nSrc1Len, const char* lpszSrc1Data, int nSrc2Len, const char* lpszSrc2Data)
{
	// -- master concatenation routine
	// Concatenate two sources
	// -- assume that 'this' is a new String object

	int nNewLen = nSrc1Len + nSrc2Len;
	if (nNewLen != 0)
	{
		AllocBuffer(nNewLen);
		memcpy(m_pchData, lpszSrc1Data, nSrc1Len*sizeof(char));
		memcpy(m_pchData+nSrc1Len, lpszSrc2Data, nSrc2Len*sizeof(char));
	}
}

void String::ConcatInPlace(int nSrcLen, const char* lpszSrcData)
{
	//  -- the main routine for += operators

	// concatenating an empty string is a no-op!
	if (nSrcLen == 0)

		return;

	// if the buffer is too small, or we have a width mis-match, just
	//   allocate a new buffer (slow but sure)
	if (getData()->nRefs > 1 || getData()->nDataLength + nSrcLen > getData()->nAllocLength)
	{
		// we have to grow the buffer, use the concatCopy routine
		StringData* pOldData = getData();
		concatCopy(getData()->nDataLength, m_pchData, nSrcLen, lpszSrcData);
		String::release(pOldData);
	}
	else
	{
		// fast concatenation when buffer big enough
		memcpy(m_pchData+getData()->nDataLength, lpszSrcData, nSrcLen*sizeof(char));
		getData()->nDataLength += nSrcLen;
		m_pchData[getData()->nDataLength] = '\0';
	}
}

const String& String::operator+=(const char* lpsz)
{
	ConcatInPlace(safeStrlen(lpsz), lpsz);
	return *this;
}

const String& String::operator+=(char ch)
{
	if(ch == 0)
		return *this;
	ConcatInPlace(1, &ch);
	return *this;
}

const String& String::operator+=(const String& string)
{
	ConcatInPlace(string.getData()->nDataLength, string.m_pchData);
	return *this;
}


///////////////////////////////////////////////////////////////////////////////
// Advanced direct buffer access

char* String::getBuffer(int nMinBufLength)
{
	if (getData()->nRefs > 1 || nMinBufLength > getData()->nAllocLength)
	{
		// we have to grow the buffer
		StringData* pOldData = getData();
		int nOldLen = getData()->nDataLength;   // AllocBuffer will tromp it
		if (nMinBufLength < nOldLen)
			nMinBufLength = nOldLen;

		AllocBuffer(nMinBufLength);
		memcpy(m_pchData, pOldData->data(), (nOldLen+1)*sizeof(char));
		getData()->nDataLength = nOldLen;
		String::release(pOldData);
	}
	return m_pchData;
}

void String::releaseBuffer(int nNewLength)
{
	CopyBeforeWrite();  // just in case getBuffer was not called

	if (nNewLength == -1)
		nNewLength = (int)strlen(m_pchData); // zero terminated

	getData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
}

char* String::getBufferSetLength(int nNewLength)
{
	getBuffer(nNewLength);
	getData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
	return m_pchData;
}

void String::freeExtra()
{
	if (getData()->nDataLength != getData()->nAllocLength)
	{
		StringData* pOldData = getData();
		AllocBuffer(getData()->nDataLength);
		memcpy(m_pchData, pOldData->data(), pOldData->nDataLength*sizeof(char));
		String::release(pOldData);
	}
}

char* String::lockBuffer()
{
	char* lpsz = getBuffer(0);
	getData()->nRefs = -1;
	return lpsz;
}

void String::unlockBuffer()
{
	if (getData() != _SafxDataNil)
		getData()->nRefs = 1;
}

int String::find(char ch) const
{
	return find(ch, 0);
}

int String::find(char ch, int nStart) const
{
	int nLength = getData()->nDataLength;
	if (nStart >= nLength)
		return -1;

	// find first single character
	char* lpsz = (char*)cbschr((CUCHAR*)m_pchData + nStart, ch);

	// return -1 if not found and index otherwise
	return (int)((lpsz == NULL) ? -1 : (lpsz - m_pchData));//MD_906 by pch
}

int String::find(char *lpszSub) const
{
 	return find(lpszSub, 0);
}

int String::find(String *pStr) const
{
	return find(pStr,0);
}

int String::find(String *pStr, int nStart) const
{
	int nLength = getData()->nDataLength;
	if (nStart > nLength)
		return -1;

	// find first matching substring
	char* lpsz = (char*)cbsstr((CUCHAR *)m_pchData + nStart, (CUCHAR *)pStr->m_pchData);

	// return -1 for not found, distance from beginning otherwise
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

int String::find(String Str, int nStart) const
{
	return find(&Str,nStart);
}

int String::find(String Str) const
{
	return find(&Str);
}

int String::getCharCount(char *lpszSub, int nStart) const
{
	int count = 0;
	char *p;
	while( (nStart = find(lpszSub, nStart)) != -1)
	{
		count++;
		p = (char*)cbsinc((CUCHAR *)m_pchData + nStart);
		nStart = (int)(p - m_pchData);
	}
	return count;

}

int String::find(char*lpszSub, int nStart) const
{

 	int nLength = getData()->nDataLength;
 	if (nStart > nLength)
 		return -1;
	if(!strlen(lpszSub)) return -1;
 
 	// find first matching substring
 	char *lpsz = (char*)cbsstr((CUCHAR *)m_pchData + nStart, (CUCHAR *)lpszSub);
 
 	// return -1 for not found, distance from beginning otherwise
 	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);

}

int String::reverseFind(char ch) const
{
	// find last single character
//	char * lpsz = (char*)_mbsrchr((CUCHAR *)m_pchData,ch);
	char * lpsz = (char*)strrchr(m_pchData,ch);

	// return -1 if not found, distance from beginning otherwise
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);

}

int String::reverseFind(char* lpszSub, int nBackStart) const
{
	// find last single character
	int nRightStart = 0;
	int nBoundLen = (int)(strlen(m_pchData) - nBackStart - strlen(lpszSub));

	int SearchPos = -1;
	while(true){
		nRightStart = find(lpszSub, nRightStart);

		if(nRightStart == -1) break;
		if(nRightStart > nBoundLen) break;
		SearchPos = nRightStart;
		if(nRightStart == nBoundLen) break;
		( *((uchar*)(m_pchData + nRightStart))) >= 0x80 ? nRightStart += 2 : nRightStart++;
		// ATTENTION 2BYTE process
	}

	// return -1 if not found, distance from beginning otherwise
	return SearchPos;
}

int String::findOneOf(const char* lpszCharSet, int nStart) const
{
//	char* lpsz = (char*)_mbspbrk((CUCHAR *)m_pchData + nStart,(CUCHAR *)lpszCharSet);
	char* lpsz = (char*)strpbrk(m_pchData + nStart,lpszCharSet);
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

void String::makeUpper()
{
	CopyBeforeWrite();
#ifndef __QT__
	_mbsupr((uchar *)m_pchData);
#endif
}

void String::makeLower()
{
    CopyBeforeWrite();
#ifndef __QT__
    _mbslwr((uchar *)m_pchData);
#endif
}

void String::makeReverse()
{
	CopyBeforeWrite();
#ifndef __QT__
    _mbsrev((uchar *)m_pchData);
#endif
}

void String::setAt(int nIndex, char ch)
{
	CopyBeforeWrite();
	m_pchData[nIndex] = ch;
}

int String::Delete(int nIndex, int nCount /* = 1 */)
{
	if (nIndex < 0)
		nIndex = 0;
	int nNewLength = getData()->nDataLength;
	if (nCount > 0 && nIndex < nNewLength)
	{
		CopyBeforeWrite();
		int nBytesToCopy = nNewLength - (nIndex + nCount) + 1;

		memcpy(m_pchData + nIndex,
			m_pchData + nIndex + nCount, nBytesToCopy * sizeof(char));
		getData()->nDataLength = nNewLength - nCount;
	}

	return nNewLength;
}

int String::insert(int nIndex, char ch)
{
	CopyBeforeWrite();

	if (nIndex < 0)
		nIndex = 0;

	int nNewLength = getData()->nDataLength;
	if (nIndex > nNewLength)
		nIndex = nNewLength;
	nNewLength += 2;


	if (getData()->nAllocLength < nNewLength)
	{
		StringData* pOldData = getData();
		char *pstr = m_pchData;
		AllocBuffer(nNewLength);
		memcpy(m_pchData, pstr, (pOldData->nDataLength+1)*sizeof(char));

		String::release(pOldData);
	}

	// move existing bytes down
	memcpy(m_pchData + nIndex + 1,
		m_pchData + nIndex, (nNewLength-nIndex)*sizeof(char));
	m_pchData[nIndex] = ch;
	getData()->nDataLength = nNewLength;

	return nNewLength;
}
int String::insert(int nIndex, char * pstr)
{
	if (nIndex < 0)
		nIndex = 0;

	int nInsertLength = safeStrlen(pstr);
	int nNewLength = getData()->nDataLength;
	if (nInsertLength > 0)
	{
		CopyBeforeWrite();
		if (nIndex > nNewLength)
			nIndex = nNewLength;
		nNewLength += nInsertLength + 2;

		if (getData()->nAllocLength < nNewLength)
		{
			StringData* pOldData = getData();
			char *pstr_ = m_pchData;
			AllocBuffer(nNewLength);
			memcpy(m_pchData, pstr_, (pOldData->nDataLength+1)*sizeof(char));
			String::release(pOldData);
		}


		// move existing bytes down
		memcpy(m_pchData + nIndex + nInsertLength,
			m_pchData + nIndex,
			(nNewLength-nIndex-nInsertLength+1)*sizeof(char));
		*(m_pchData + nNewLength) = 0; //2004.6.11 By AJN
		memcpy(m_pchData + nIndex,
			pstr, nInsertLength*sizeof(char));
		getData()->nDataLength = nNewLength;
	}

	return nNewLength;
}

/****************************************************************/
//MD_906 by pch
String String::mid(int nFirst) const
{
	int nCount;
	nCount = getData()->nDataLength - nFirst;
	return mid(nFirst, nCount);
}

String String::mid(int nFirst, int nCount) const
{
	// out-of-bounds requests return sensible things
	if (nFirst < 0)
		nFirst = 0;
	if (nCount < 0)
		nCount = 0;

	if (nFirst + nCount > getData()->nDataLength)
		nCount = getData()->nDataLength - nFirst;
	if (nFirst > getData()->nDataLength)
		nCount = 0;

	// optimize case of returning entire string
	if (nFirst == 0 && nFirst + nCount == getData()->nDataLength)
		return *this;
	String dest;
	AllocCopy(dest, nCount, nFirst, 0);
	g_StrBuf = dest;
	return g_StrBuf;
}

String String::right(int nCount) const
{
	if (nCount < 0)
		nCount = 0;
	if (nCount >= getData()->nDataLength)
		return *this;
	
	String dest;
	AllocCopy(dest, nCount, getData()->nDataLength-nCount, 0);
	g_StrBuf = dest;
	return g_StrBuf;
}

String String::left(int nCount) const
{
	if (nCount < 0)
		nCount = 0;
	if (nCount >= getData()->nDataLength)
		return *this;

	String dest;
	AllocCopy(dest, nCount, 0, 0);
	g_StrBuf = dest;
	return g_StrBuf;
}
/*****************************************************************/

void String::trimRight(char * lpszTargetList)
{
	// find beginning of trailing matches
	// by starting at beginning (DBCS aware)
#ifndef __QT__
	CopyBeforeWrite();
	uchar* lpsz = (uchar*)m_pchData;
	uchar* lpszLast = NULL;

	while (*lpsz != '\0')
	{
		ushort code = *lpsz;
        if(_ismbblead(code)) {
			code = (code << 8) + *(lpsz + 1);
		}
		if (cbschr((CUCHAR *)lpszTargetList, code) != NULL)
		{
			if (lpszLast == NULL)
				lpszLast = lpsz;
		}
		else
			lpszLast = NULL;
		lpsz = (uchar *)cbsinc((CUCHAR *)lpsz);
	}

	if (lpszLast != NULL)
	{
		// truncate at left-most matching character
		*lpszLast = '\0';
		getData()->nDataLength = (int)((size_t)lpszLast - (size_t)m_pchData);
	}
#endif
}


void String::trimRight(char chTarget)
{
#ifndef __QT__
    // find beginning of trailing matches
	// by starting at beginning (DBCS aware)

	CopyBeforeWrite();
	uchar * lpsz = (uchar*)m_pchData;
	uchar * lpszLast = NULL;

	ushort code = *lpsz;
	if(_ismbblead(code)) {
		code = (code << 8) + *(lpsz + 1);
	}


	while (code != '\0'){
		if (code == chTarget)
		{
			if (lpszLast == NULL)
				lpszLast = lpsz;
		}
		else
			lpszLast = NULL;
		lpsz = (uchar*)cbsinc((CUCHAR *)lpsz);
		code = *lpsz;
		if(_ismbblead(code)) {
			code = (code << 8) + *(lpsz + 1);
		}
	}

	if (lpszLast != NULL)
	{
		// truncate at left-most matching character
		*lpszLast = '\0';
		getData()->nDataLength = (int)((size_t)lpszLast - (size_t)m_pchData);
	}
#endif
}

void String::trimRight()
{
#ifndef __QT__
    // find beginning of trailing spaces by starting at beginning (DBCS aware)

	CopyBeforeWrite();
	uchar * lpsz = (uchar*)m_pchData;

	uchar * lpszLast = NULL;

	ushort code = *lpsz;
	if(_ismbblead(code)) {
		code = (code << 8) + *(lpsz + 1);
	}
	while (code != '\0'){
		if (_ismbcspace(code)){
			if (lpszLast == NULL)
				lpszLast = lpsz;
		}
		else
			lpszLast = NULL;
		lpsz = (uchar*)cbsinc((CUCHAR *)lpsz);

		code = *lpsz;
		if(_ismbblead(code)) {
			code = (code << 8) + *(lpsz + 1);
		}
	}

	if (lpszLast != NULL)
	{
		// truncate at trailing space start
		*lpszLast = '\0';
		getData()->nDataLength = (int)((size_t)lpszLast - (size_t)m_pchData);
	}
#endif
}

void String::trimLeft(char * lpszTargets)
{
#ifndef __QT__
    // if we're not trimming anything, we're not doing any work
	if (safeStrlen(lpszTargets) == 0)
		return;

	CopyBeforeWrite();
	uchar * lpsz = (uchar*)m_pchData;
	ushort code = *lpsz;
	if(_ismbblead(code)) {
		code = (code << 8) + *(lpsz + 1);
	}
	while (*lpsz != '\0'){
		
		if (cbschr((CUCHAR *)lpszTargets, code) == NULL)
			break;
		lpsz = (uchar*)cbsinc((CUCHAR *)lpsz);
		code = *lpsz;
		if(_ismbblead(code)) {
			code = (code << 8) + *(lpsz + 1);
		}
	}

	if (lpsz != (uchar*)m_pchData)
	{
		// fix up data and length
		int nDataLength = (int)(getData()->nDataLength - ((size_t)lpsz - (size_t)m_pchData));
		memmove(m_pchData, lpsz, (nDataLength+1)*sizeof(char));
		getData()->nDataLength = nDataLength;
	}
#endif
}

void String::trimLeft(char chTarget)
{
	// find first non-matching character

	CopyBeforeWrite();
	char * lpsz = m_pchData;

	while (chTarget == *lpsz)
	{
		lpsz = (char*)cbsinc((CUCHAR *)lpsz);
	}

	if (lpsz != m_pchData)
	{
		// fix up data and length
		size_t nDataLength = getData()->nDataLength - (lpsz - m_pchData);
		memmove(m_pchData, lpsz, (nDataLength+1)*sizeof(char));
		getData()->nDataLength = (int)nDataLength;
	}
}

void String::trimLeft()
{
#ifndef __QT__
    // find first non-space character

	CopyBeforeWrite();
	uchar * lpsz = (uchar*)m_pchData;
	ushort code = *lpsz;
	if(_ismbblead(code))
		code = (code << 8) + *(lpsz + 1);

	while (_ismbcspace(code))
	{
		lpsz = (uchar*)cbsinc((CUCHAR *)lpsz);
		code = *lpsz;
		if(_ismbblead(code))
			code = (code << 8) + *(lpsz + 1);
	}

	if (lpsz != (uchar*)m_pchData)
	{
		// fix up data and length
		int nDataLength = (int)(getData()->nDataLength - ((size_t)lpsz - (size_t)m_pchData));
		memmove(m_pchData, lpsz, (nDataLength+1)*sizeof(char));
		getData()->nDataLength = nDataLength;
	}
#endif
}

int String::remove(char chRemove)
{
	CopyBeforeWrite();

	char * pstrSource = m_pchData;
	char * pstrDest = m_pchData;
	char * pstrEnd = m_pchData + getData()->nDataLength;

	while (pstrSource < pstrEnd)
	{
		if (*pstrSource != chRemove)
		{
			*pstrDest = *pstrSource;
			pstrDest = (char*)cbsinc((CUCHAR *)pstrDest);
		}
		pstrSource = (char*)cbsinc((CUCHAR *)pstrSource);
	}
	*pstrDest = '\0';
	int nCount = (int)(pstrSource - pstrDest);

	getData()->nDataLength -= nCount;

	return nCount;
}
int String::replace(char chOld, char chNew)

{
	int nCount = 0;

	// short-circuit the nop case
	if (chOld != chNew)
	{
		// otherwise modify each character that matches in the string
		CopyBeforeWrite();
		char * psz = m_pchData;
		char * pszEnd = psz + getData()->nDataLength;
		while (psz < pszEnd)
		{
			// replace instances of the specified character only
			if (*psz == chOld)
			{
				*psz = chNew;
				nCount++;
			}

			psz = (char*)cbsinc((CUCHAR *)psz);
		}
	}
	return nCount;
}

int String::replace(String Old, String New)
{
	return replace(Old.m_pchData, New.m_pchData);
}


int String::replace(char * lpszOld, char * lpszNew)
{
	// can't have empty or NULL lpszOld

	int nSourceLen = safeStrlen(lpszOld);
	if (nSourceLen == 0)
		return 0;
	int nReplacementLen = safeStrlen(lpszNew);

	// loop once to figure out the size of the result string
	int nCount = 0;
	char * lpszStart = m_pchData;
	char * lpszEnd = m_pchData + getData()->nDataLength;
	char * lpszTarget;
	while (lpszStart < lpszEnd)
	{
		while ((lpszTarget = (char*)cbsstr((CUCHAR *)lpszStart, (CUCHAR *)lpszOld)) != NULL)
		{
			nCount++;
			lpszStart = lpszTarget + nSourceLen;
		}

		lpszStart += strlen(lpszStart) + 1;
	}

	// if any changes were made, make them
	if (nCount > 0)
	{
		CopyBeforeWrite();

		// if the buffer is too small, just
		//   allocate a new buffer (slow but sure)
		int nOldLength = getData()->nDataLength;
		int nNewLength =  nOldLength + (nReplacementLen-nSourceLen)*nCount;
		if (getData()->nAllocLength < nNewLength || getData()->nRefs > 1)
		{
			StringData* pOldData = getData();
			char * pstr = m_pchData;
			AllocBuffer(nNewLength);
			memcpy(m_pchData, pstr, pOldData->nDataLength*sizeof(char));
			String::release(pOldData);
		}

		// else, we just do it in-place
		lpszStart = m_pchData;
		lpszEnd = m_pchData + getData()->nDataLength;

		// loop again to actually do the work
		while (lpszStart < lpszEnd)
		{
			while ( (lpszTarget = (char*)cbsstr((CUCHAR *)lpszStart, (CUCHAR *)lpszOld)) != NULL)
			{
				int nBalance = (int)(nOldLength - (lpszTarget - m_pchData + nSourceLen));
				memmove(lpszTarget + nReplacementLen, lpszTarget + nSourceLen,
					nBalance * sizeof(char));
				memcpy(lpszTarget, lpszNew, nReplacementLen*sizeof(char));
				lpszStart = lpszTarget + nReplacementLen;
				lpszStart[nBalance] = '\0';
				nOldLength += (nReplacementLen - nSourceLen);
			}
			lpszStart += strlen(lpszStart) + 1;
		}
		getData()->nDataLength = nNewLength;
	}

	return nCount;
}

String::String(char ch, int nLength)
{
	Init();
	if (nLength >= 1)
	{
		AllocBuffer(nLength);
#ifdef _UNICODE
      int i;
		for ( i = 0; i < nLength; i++)
			m_pchData[i] = ch;
#else
		memset(m_pchData, ch, nLength);
#endif
	}

}

String::String(char * lpch, int nLength)
{
	Init();
	if (nLength != 0)
	{
		AllocBuffer(nLength);
		memcpy(m_pchData, lpch, nLength*sizeof(char));
	}
}

String String::spanIncluding(const char * lpszCharSet) const
{//MD_906 by pch
	return left((int)strspn(m_pchData, lpszCharSet));
}


String String::spanExcluding(const char * lpszCharSet) const
{//MD_906 by pch
    return left((int)strcspn(m_pchData, lpszCharSet));
}

//#define TCHAR_ARG   TCHAR//MD_821 by pch
#define WCHAR_ARG   unsigned short
#define CHAR_ARG    char

#define DOUBLE_ARG  double

#define FORCE_ANSI      0x10000

#define FORCE_UNICODE   0x20000
#define FORCE_INT64     0x40000


//MD_817 by pch
#ifndef va_copy
#define va_copy(dst, src) ((void)((dst)=(src)))
#endif
/*void String::formatV(char * lpszFormat, va_list argList)
{
	va_list argListSave;
    va_copy(argListSave, argList);

	// make a guess at the maximum length of the resulting string
	int nMaxLen = 0;
	//MD_820 by pch
	String str(lpszFormat);
	const char * lpsz;
	for (lpsz = str.str(); *lpsz != '\0'; lpsz = (char*)cbsinc((CUCHAR *)lpsz))
	{
		// handle '%' character, but watch out for '%%'
		if (*lpsz != '%' || *(lpsz = (char*)cbsinc((CUCHAR *)lpsz)) == '%')
		{
			nMaxLen += (int)strlen(lpsz);
			continue;
		}

		int nItemLen = 0;

		// handle '%' character with format
		int nWidth = 0;

		for (; *lpsz != '\0'; lpsz = (char*)cbsinc((CUCHAR *)lpsz))
		{
			// check for valid flags
			if (*lpsz == '#')
				nMaxLen += 2;   // for '0x'
			else if (*lpsz == '*')
				nWidth = va_arg(argList, int);
			else if (*lpsz == '-' || *lpsz == '+' || *lpsz == '0' ||
				*lpsz == ' ')
				;
			else // hit non-flag character
				break;
		}
		// get width and skip it
		if (nWidth == 0)
		{
			// width indicated by
			nWidth = atoi(lpsz);
			for (; *lpsz != '\0' && _istdigit(*lpsz);
			lpsz = (char*)cbsinc((CUCHAR *)lpsz));
		}

		int nPrecision = 0;
		if (*lpsz == '.')
		{
			// skip past '.' separator (width.precision)
			lpsz = (char*)cbsinc((CUCHAR *)lpsz);

			// get precision and skip it
			if (*lpsz == '*')
			{
				nPrecision = va_arg(argList, int);
				lpsz = (char*)cbsinc((CUCHAR *)lpsz);
			}

			else
			{
				nPrecision = atoi(lpsz);
				for (; *lpsz != '\0' && _istdigit(*lpsz); lpsz = (char*)cbsinc((CUCHAR *)lpsz))
					;
			}

		}

		// should be on type modifier or specifier
		int nModifier = 0;
		if (strncmp(lpsz, _T("I64"), 3) == 0)
		{
			lpsz += 3;
			nModifier = FORCE_INT64;
		}
		else
		{
			switch (*lpsz)
			{
			// modifiers that affect size
			case 'h':
				nModifier = FORCE_ANSI;
				lpsz = (char*)cbsinc((CUCHAR *)lpsz);
				break;
			case 'l':
				nModifier = FORCE_UNICODE;
				lpsz = (char*)cbsinc((CUCHAR *)lpsz);
				break;

			// modifiers that do not affect size
			case 'F':
			case 'N':
			case 'L':
				lpsz = (char*)cbsinc((CUCHAR *)lpsz);
				break;
			}
		}

		// now should be on specifier
		switch (*lpsz | nModifier)
		{
		// single characters
		case 'c'|FORCE_ANSI:
		case 'C'|FORCE_ANSI:
			nItemLen = 2;
			va_arg(argList, CHAR_ARG );
			break;
		case 'c'|FORCE_UNICODE:
		case 'C'|FORCE_UNICODE:

			nItemLen = 2;
			va_arg(argList, WCHAR_ARG );
			break;

		// strings
		case 's':
			{
				char* pstrNextArg = va_arg(argList, char*);

				if (pstrNextArg == NULL)
				   nItemLen = 6;  // "(null)"
				else
				{
				   nItemLen = lstrlen(pstrNextArg);
				   nItemLen = MAX(1, nItemLen);
				}
			}
			break;

		case 'S':
			{
#ifndef _UNICODE
				ushort* pstrNextArg = va_arg(argList, ushort* );
				if (pstrNextArg == NULL)
				   nItemLen = 6;  // "(null)"
				else
				{
				   nItemLen = (int)strlen((const char*)pstrNextArg);
				   nItemLen = MAX(1, nItemLen);
				}
#else
				const char* pstrNextArg = va_arg(argList, const char* );
				if (pstrNextArg == NULL)
				   nItemLen = 6; // "(null)"

				else
				{
				   nItemLen = lstrlenA(pstrNextArg);
				   nItemLen = MAX(1, nItemLen);
				}
#endif
			}
			break;

		case 's'|FORCE_ANSI:
		case 'S'|FORCE_ANSI:
			{
				const char*  pstrNextArg = va_arg(argList, const char* );
				if (pstrNextArg == NULL)
				   nItemLen = 6; // "(null)"
				else
				{
				   nItemLen = lstrlenA(pstrNextArg);
				   nItemLen = MAX(1, nItemLen);
				}
			}
			break;

		case 's'|FORCE_UNICODE:
		case 'S'|FORCE_UNICODE:
			{

				ushort* pstrNextArg = va_arg(argList, ushort* );
				if (pstrNextArg == NULL)
				   nItemLen = 6; // "(null)"
				else
				{
				   nItemLen = (int)strlen((const char*)pstrNextArg);
				   nItemLen = MAX(1, nItemLen);
				}
			}
			break;
		}

		// adjust nItemLen for strings
		if (nItemLen != 0)
		{
			if (nPrecision != 0)
				nItemLen = MIN(nItemLen, nPrecision);
			nItemLen = MAX(nItemLen, nWidth);
		}
		else
		{
			switch (*lpsz)
			{
			// integers
			case 'd':
			case 'i':
			case 'u':

			case 'x':
			case 'X':
			case 'o':
				if (nModifier & FORCE_INT64)
					va_arg(argList, int64);
				else
					va_arg(argList, int);
				nItemLen = 32;
				nItemLen = MAX(nItemLen, nWidth+nPrecision);
				break;


			case 'e':
			case 'g':
			case 'G':
				va_arg(argList, DOUBLE_ARG);
				nItemLen = 128;
				nItemLen = MAX(nItemLen, nWidth+nPrecision);
				break;

			case 'f':
				va_arg(argList, DOUBLE_ARG);
				nItemLen = 128; // width isn't truncated
				// 312 == strlen("-1+(309 zeroes).")
				// 309 zeroes == MAX precision of a double
				nItemLen = MAX(nItemLen, 312+nPrecision);
				break;

			case 'p':
				va_arg(argList, void*);
				nItemLen = 32;
				nItemLen = MAX(nItemLen, nWidth+nPrecision);
				break;

			// no output
			case 'n':
				va_arg(argList, int*);
				break;

			default:
				;
			}
		}

		// adjust nMaxLen for output nItemLen
		nMaxLen += nItemLen;
	}

	getBuffer(nMaxLen);
//	VERIFY(_vstprintf(m_pchData, lpszFormat, argListSave) <= getAllocLength());
	sprintf(m_pchData, lpszFormat, argListSave);
	releaseBuffer();

	va_end(argListSave);
}*/

// formatting (using wsprintf style formatting)

void String::ivsprintf(const char* format, va_list list)
{
	for (int count = 256; ; count *= 2) {
#ifdef WIN32
		int ret = _vsnprintf(getBuffer(count), (size_t)count, format, list);
#else
		int ret = vsnprintf(getBuffer(count), (size_t)count, format, list);
#endif
		if (ret >= 0) {
			releaseBuffer(ret);
			break;
		}
		assert(ret == -1);
		releaseBuffer(count);
	}
}

String String::format(const char * lpszFormat, ...)
{
	va_list argList;
	va_start(argList, lpszFormat);
	String str;
	str.ivsprintf(lpszFormat, argList);
	va_end(argList);
	return str;
}

void String::vsprintf(const char * lpszFormat, ...)
{
	va_list argList;
	va_start(argList, lpszFormat);
	this->ivsprintf(lpszFormat, argList);
	va_end(argList);
}


int String::fromFile(XFile* pfile)
{
	int n;
	pfile->read(&n, sizeof(n), 1);
	char* buffer = getBufferSetLength(n);
	pfile->read(buffer, n, 1);
	buffer[n] = 0;
	return 1;
}
int String::toFile(XFile* pfile) const
{
	int nlength = length();
	pfile->write(&nlength, sizeof(nlength), 1);
	pfile->write(this->str(), nlength, 1);
	return 1;
}

}

//////////////////////////////////////////////////////////////////////////////
// less common string expressions




#pragma warning (pop)
