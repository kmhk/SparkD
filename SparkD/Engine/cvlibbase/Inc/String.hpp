
/*!
 * \file    String.h
 * \ingroup base
 * \brief   
 * \author  
 */

#pragma once

#include "cvlibbaseDef.h"
	
namespace cvlib
{

struct StringData
{
	int nRefs;
	int nDataLength;
	int nAllocLength;

	char* data()           // char* to managed data
	{ return (char *)(this+1); }
};

/**
 * @brief
 */
class CVLIB_DECLSPEC String
{
public:
// Constructors
	String();
	String(const String& stringSrc);


	String(char ch, int nLength=1);

	String(char * lpsz);
	String(const char * lpsz);
	String(char * lpch, int nLength);
	
// Attributes & Operations
	int length() const;
	ulong isEmpty() const;
	void empty();
	uchar getAt(int nIndex) const;
	uchar operator[](int nIndex) const;
	void setAt(int nIndex, char ch);

	// overloaded assignment
	const String& operator=(const String & stringSrc);
	const String& operator=(const char* lpsz);
	const String& operator=(char ch);
	const String& operator+=(const String& string);
	const String& operator+=(char ch);
	const String& operator+=(const char* lpsz);
	const char * str() const {return m_pchData;}
	operator const char*() const {return m_pchData;}

	// string comparison
	int compare(const String& pStr) const;

	// straight character comparison
	int compare(const char * lpsz) const;

	// compare ignoring case
	int compareNoCase(const char * lpsz) const;
	
	// NLS aware comparison, case sensitive
	int collate(char * lpsz) const;

	// NLS aware comparison, case insensitive
	int collateNoCase(char * lpsz) const;

	// simple sub-string extraction

  /******************************************************/
  //MD_906 by pch
	String mid(int nFirst, int nCount) const;
	String mid(int nFirst) const;
	String left(int nCount) const;
	String right(int nCount) const;
  /******************************************************/

	static String format(const char * lpszFormat, ...);
	void vsprintf(const char * lpszFormat, ...);

  /******************************************************/
  //MD_906 by pch
	String spanIncluding(const char * lpszCharSet) const;
	String spanExcluding(const char * lpszCharSet) const;
  /******************************************************/
  
	// upper/lower/reverse conversion
	void makeUpper();
	void makeLower();
	void makeReverse();

	// trimming whitespace (either side)
	void trimRight();
	void trimLeft();

	// trimming anything (either side)
	void trimRight(char chTarget);

	void trimRight(char * lpszTargets);
	
	void trimLeft(char chTarget);
	
	void trimLeft(char * lpszTargets);

	// advanced manipulation

	int replace(char chOld, char chNew);

	int replace(char * lpszOld, char * lpszNew);

	int replace(String Old, String New);
	
	int remove(char chRemove);

	int insert(int nIndex, char ch);

	int insert(int nIndex, char * pstr);
	
	int Delete(int nIndex, int nCount = 1);

	// searching
	int reverseFind(char ch) const;
	
	int reverseFind(char* lpszSub, int nBackStart = 0) const;

	int find(String *pStr, int nStart) const;
	
	int find(String Str, int nStart) const;
	
	int find(String Str) const;
	
	int find(String *pStr) const;
	
	int find(char ch) const;
	
	int find(char ch, int nStart) const;

	int find(char*lpszSub) const;
	
	int find(char*lpszSub, int nStart) const;
	
	int findOneOf(const char * lpszCharSet, int nStart = 0) const;
	
	int getCharCount(char *lpszSub,int nStart = 0) const;

	// get pointer to modifiable buffer at least as int as nMinBufLength
	char * getBuffer(int nMinBufLength);
	
	void releaseBuffer(int nNewLength = -1);

	// get pointer to modifiable buffer exactly as int as nNewLength
	char * getBufferSetLength(int nNewLength);
	// release memory allocated to but unused by string
	void freeExtra();

	// Use lockBuffer/unlockBuffer to turn refcounting off

	// turn refcounting back on
	char * lockBuffer();
	// turn refcounting off
	void unlockBuffer();
	int fromFile(XFile* pfile);
	int toFile(XFile* pfile) const;

// Implementation
public:
	~String();
	int         getAllocLength() ;
	StringData* getData() const;

	void        concatCopy(int nSrc1Len, const char * lpszSrc1Data, int nSrc2Len, const char * lpszSrc2Data);

	int         safeStrlen(const char * lpsz) const;

	char*       m_pchData;   

protected:

	// implementation helpers

	void Init();
	void AllocCopy(String& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
	void AllocBuffer(int nLen);
	void AssignCopy(int nSrcLen, const char * lpszSrcData);

	void ConcatInPlace(int nSrcLen, const char * lpszSrcData);
	void CopyBeforeWrite();
	void AllocBeforeWrite(int nLen);
	void release();
	void release(StringData* pData);

	void ivsprintf(const char* format, va_list list);

	void  FreeData(StringData* pData);

};


inline bool CVLIB_DECLSPEC operator==( const String& s1, const  String& s2)
	{ if (s1.compare(s2) == 0) return true; return false; }
inline bool CVLIB_DECLSPEC operator==(const String& s1, const char* s2)
	{ if (s1.compare(s2) == 0) return true; return false; }
inline bool CVLIB_DECLSPEC operator==(const char* s1, const String& s2)
	{ if (s2.compare(s1) == 0) return true; return false; }
inline bool CVLIB_DECLSPEC operator!=(const String& s1, const String& s2)
	{ if (s1.compare(s2) != 0) return true; return false; }
inline bool CVLIB_DECLSPEC operator!=(const String& s1, const char * s2)
	{ if (s1.compare(s2) != 0) return true; return false; }
inline bool CVLIB_DECLSPEC operator!=(const char* s1, const String& s2)
	{ if (s2.compare(s1) != 0) return true; return false; }
inline bool CVLIB_DECLSPEC operator<(const String& s1, const String& s2)
	{ if (s1.compare(s2) < 0) return true; return false; }
inline bool CVLIB_DECLSPEC operator<(const String& s1, const char* s2)
	{ if (s1.compare(s2) < 0) return true; return false; }
inline bool CVLIB_DECLSPEC operator<(const char* s1, const String& s2)
	{ if (s2.compare(s1) > 0) return true; return false; }
inline bool CVLIB_DECLSPEC operator>(const String& s1, const String& s2)
	{ if (s1.compare(s2) > 0) return true; return false; }
inline bool CVLIB_DECLSPEC operator>(const String& s1, const char* s2)
	{ if (s1.compare(s2) > 0) return true; return false; }
inline bool CVLIB_DECLSPEC operator>(const char* s1, const String& s2)
	{ if (s2.compare(s1) < 0) return true; return false; }
inline bool CVLIB_DECLSPEC operator<=(const String& s1, const String& s2)
	{ if (s1.compare(s2) <= 0) return true; return false; }
inline bool CVLIB_DECLSPEC operator<=(const String& s1, const char* s2)
	{ if (s1.compare(s2) <= 0) return true; return false; }
inline bool CVLIB_DECLSPEC operator<=(const char* s1, const String& s2)
	{ if (s2.compare(s1) >= 0) return true; return false; }
inline bool CVLIB_DECLSPEC operator>=(const String& s1, const String& s2)
	{ if (s1.compare(s2) >= 0) return true; return false; }
inline bool CVLIB_DECLSPEC operator>=(const String& s1, const char* s2)
	{ if (s1.compare(s2) >= 0) return true; return false; }
inline bool CVLIB_DECLSPEC operator>=(const char* s1, const String& s2)
	{ if (s2.compare(s1) <= 0) return true; return false; }
inline String CVLIB_DECLSPEC operator+(const String& string1, const String& string2)
	{ String dest = string1; dest += string2; return dest; }
inline String CVLIB_DECLSPEC operator+(const String& string, char ch)
	{ String dest = string; dest += ch; return dest; }
inline String CVLIB_DECLSPEC operator+(char ch, const String& string)
	{ String dest = string; dest += ch; return dest; }
inline String CVLIB_DECLSPEC operator+(const String& string, const char* lpsz)
	{ String dest = string; dest += lpsz; return dest; }
inline String CVLIB_DECLSPEC operator+(const char* lpsz, const String& string)
	{ String dest = string; dest += lpsz; return dest; }


#define IsTCharCode(c) ((((uchar)(c)) > 0x7F) ? (true) : (false))
#define IsSpaceCode(c) (((c) == 0x20 || (c) == 0xA1A1) ? (true) : (false))
	
/*increase the pointer to current string, appositing the multibyte string*/
CVLIB_DECLSPEC inline uchar* cbsinc(const uchar * current)
{
  uchar * rslt;
  rslt = (uchar *)current;
  if(*rslt > 0x80)
    rslt += 2;
  else
    rslt++;
  return rslt;
}

CVLIB_DECLSPEC inline uchar* cbsstr(const uchar *string1, const uchar *string2)
{
	uchar * pszFind = NULL ;

	if(string1 == NULL || string2 == NULL)
		return pszFind ;
	
	int nLen1 = (int)strlen((const char*)string1) ;
	int nLen2 = (int)strlen((const char*)string2) ;
	if(nLen1 == 0 || nLen2 == 0)
		return pszFind ;

	int nIdx1 = 0 ;
	int nIdx2 = 0 ;
	while( nIdx1 < nLen1 - nLen2 + 1 )
	{
		nIdx2 = 0 ;
		while( nIdx2 < nLen2 && string2[nIdx2] == string1[nIdx1 + nIdx2] )
		{
			nIdx2++ ;
		}
		if( nIdx2 >= nLen2 )
		{
			pszFind = (uchar*)(string1 + nIdx1);
			break ;
		}
		if(IsTCharCode(string1[nIdx1]))
			nIdx1 += 2;
		else
			nIdx1++ ;
	}
	
	return pszFind ;
}

CVLIB_DECLSPEC inline uchar* cbschr(const uchar *strSrc, int nCode)
{
	uchar * pszFind = NULL ;
	int nLen = (int)strlen((const char*)strSrc) ;
	int nIdx = 0 ;
	while( nIdx < nLen )
	{
		int nSrcCode = strSrc[nIdx] ;
		if(IsTCharCode(strSrc[nIdx]))
			nSrcCode = (nSrcCode << 8) + (strSrc[nIdx+1] & 0xFF) ;

		if(nCode == nSrcCode)
		{
			pszFind = (uchar*)(strSrc + nIdx);
			break ;
		}

		if(IsTCharCode(strSrc[nIdx]))
			nIdx += 2;
		else
			nIdx++ ;
	}
	
	return pszFind ;
}

}