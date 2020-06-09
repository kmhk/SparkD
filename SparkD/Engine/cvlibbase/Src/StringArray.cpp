/*!
 * \file	StringArray.cpp
 * \ingroup base
 * \brief 
 * \author
 */

#include "StringArray.h"
#include "cvlibbaseDef.h"
#include <assert.h>

namespace cvlib
{

	int StringArray::getSize() const
	{
		return m_nSize;
	}
	
	 int StringArray::getUpperBound() const
	{
		return m_nSize-1;
	}
	
	 void StringArray::removeAll()
	{
		setSize(0);
	}
	
	 String StringArray::getAt(int nIndex) const
	{
		assert(nIndex >= 0 && nIndex < m_nSize);
		return m_pData[nIndex];
	}
	
	 void StringArray::setAt(int nIndex, char* newElement)
	{
		assert(nIndex >= 0 && nIndex < m_nSize);
		m_pData[nIndex] = newElement;
	}
	
	 void StringArray::setAt(int nIndex, String& newElement)
	{
		assert(nIndex >= 0 && nIndex < m_nSize);
		m_pData[nIndex] = newElement;
	}
	
	 String& StringArray::elementAt(int nIndex)
	{
		assert(nIndex >= 0 && nIndex < m_nSize);
		return m_pData[nIndex];
	}
	
	 const String* StringArray::getData() const
	{	
		 return (const String*)m_pData;
	}
	
	 String* StringArray::getData()
	{
		return (String*)m_pData;
	}
	
	 int StringArray::add(const char* newElement)
	{
		int nIndex = m_nSize;
		setAtGrow(nIndex, newElement);
		return nIndex;
	}
	
	 int StringArray::add(String& newElement)
	{
		int nIndex = m_nSize;
		setAtGrow(nIndex, newElement);
		return nIndex;
	}
	
	 String StringArray::operator[](int nIndex) const
	{
		return getAt(nIndex);
	}
	
	 String& StringArray::operator[](int nIndex)
	{
		return elementAt(nIndex);
	}

static String EmptyString;

static inline void ConstructElement(String* pNewData)
{
	memcpy(pNewData, &EmptyString, sizeof(String));
}

static inline void DestructElement(String* pOldData)
{
	pOldData->~String();
}

static void ConstructElements(String* pNewData, int nCount)
{
	assert(nCount >= 0);

	while (nCount--)
	{
		ConstructElement(pNewData);
		pNewData++;
	}
}

static void DestructElements(String* pOldData, int nCount)
{
	assert(nCount >= 0);

	while (nCount--)
	{
		DestructElement(pOldData);
		pOldData++;
	}
}

static void CopyElements(String* pDest, String* pSrc, int nCount)
{
	assert(nCount >= 0);

	while (nCount--)
	{
		*pDest = *pSrc;
		++pDest;
		++pSrc;
	}
}

/////////////////////////////////////////////////////////////////////////////

StringArray::StringArray()
{
	m_pData = NULL;
	m_nSize = m_nMaxSize = m_nGrowBy = 0;
}
StringArray::StringArray(const StringArray& t)
{
	m_pData = NULL;
	m_nSize = m_nMaxSize = m_nGrowBy = 0;

	for (int i=0; i<t.getSize(); i++)
		add (t.getAt(i));
}

StringArray::~StringArray()
{
	assert(this);

	DestructElements(m_pData, m_nSize);
	delete[] (uchar*)m_pData;
}
StringArray& StringArray::operator=(const StringArray& t)
{
	removeAll();
	for (int i=0; i<t.getSize(); i++)
		add (t.getAt(i));
	return *this;
}

void StringArray::setSize(int nNewSize, int nGrowBy)
{
	assert(this);
	assert(nNewSize >= 0);

	if (nGrowBy != -1)
		m_nGrowBy = nGrowBy;  // set new size

	if (nNewSize == 0)
	{
		// shrink to nothing

		DestructElements(m_pData, m_nSize);
		delete[] (uchar*)m_pData;
		m_pData = NULL;
		m_nSize = m_nMaxSize = 0;
	}
	else if (m_pData == NULL)
	{
		// create one with exact size
#ifdef SIZE_T_MAX
		assert(nNewSize <= SIZE_T_MAX/sizeof(String));    // no overflow
#endif
		m_pData = (String*) new uchar[nNewSize * sizeof(String)];

		ConstructElements(m_pData, nNewSize);

		m_nSize = m_nMaxSize = nNewSize;
	}
	else if (nNewSize <= m_nMaxSize)
	{
		// it fits
		if (nNewSize > m_nSize)
		{
			// initialize the new elements

			ConstructElements(&m_pData[m_nSize], nNewSize-m_nSize);

		}

		else if (m_nSize > nNewSize)  // destroy the old elements
			DestructElements(&m_pData[nNewSize], m_nSize-nNewSize);

		m_nSize = nNewSize;
	}
	else
	{
		// otherwise, grow array
		int nGrowBy2 = m_nGrowBy;
		if (nGrowBy2 == 0)
		{
			// heuristically determine growth when nGrowBy == 0
			//  (this avoids heap fragmentation in many situations)
			nGrowBy2 = MIN(1024, MAX(4, m_nSize / 8));
		}
		int nNewMax;
		if (nNewSize < m_nMaxSize + nGrowBy2)
			nNewMax = m_nMaxSize + nGrowBy2;  // granularity
		else
			nNewMax = nNewSize;  // no slush

		assert(nNewMax >= m_nMaxSize);  // no wrap around
#ifdef SIZE_T_MAX
		assert(nNewMax <= SIZE_T_MAX/sizeof(String)); // no overflow
#endif
		String* pNewData = (String*) new uchar[nNewMax * sizeof(String)];

		// copy new data from old
		memcpy(pNewData, m_pData, m_nSize * sizeof(String));

		// construct remaining elements
		assert(nNewSize > m_nSize);

		ConstructElements(&pNewData[m_nSize], nNewSize-m_nSize);


		// get rid of old stuff (note: no destructors called)
		delete[] (uchar*)m_pData;
		m_pData = pNewData;
		m_nSize = nNewSize;
		m_nMaxSize = nNewMax;
	}
}

int StringArray::append(const StringArray& src)
{
	assert(this);
	assert(this != &src);   // cannot append to itself

	int nOldSize = m_nSize;
	setSize(m_nSize + src.m_nSize);

	CopyElements(m_pData + nOldSize, src.m_pData, src.m_nSize);

	return nOldSize;
}

void StringArray::copy(const StringArray& src)
{
	assert(this);
	assert(this != &src);   // cannot append to itself

	setSize(src.m_nSize);

	CopyElements(m_pData, src.m_pData, src.m_nSize);

}

void StringArray::freeExtra()
{
	assert(this);

	if (m_nSize != m_nMaxSize)
	{
		// shrink to desired size
#ifdef SIZE_T_MAX
		assert(m_nSize <= SIZE_T_MAX/sizeof(String)); // no overflow
#endif
		String* pNewData = NULL;
		if (m_nSize != 0)
		{
			pNewData = (String*) new uchar[m_nSize * sizeof(String)];
			// copy new data from old
			memcpy(pNewData, m_pData, m_nSize * sizeof(String));
		}

		// get rid of old stuff (note: no destructors called)
		delete[] (uchar*)m_pData;
		m_pData = pNewData;
		m_nMaxSize = m_nSize;
	}
}

/////////////////////////////////////////////////////////////////////////////

void StringArray::setAtGrow(int nIndex, const char* newElement)
{
	assert(this);
	assert(nIndex >= 0);

	if (nIndex >= m_nSize)
		setSize(nIndex+1);
	m_pData[nIndex] = newElement;
}


void StringArray::setAtGrow(int nIndex, String& newElement)
{
	assert(this);
	assert(nIndex >= 0);

	if (nIndex >= m_nSize)
		setSize(nIndex+1);
	m_pData[nIndex] = newElement;
}



void StringArray::InsertEmpty(int nIndex, int nCount)
{
	assert(this);
	assert(nIndex >= 0);    // will expand to meet need
	assert(nCount > 0);     // zero or negative size not allowed

	if (nIndex >= m_nSize)
	{
		// adding after the end of the array
		setSize(nIndex + nCount);  // grow so nIndex is valid
	}
	else
	{
		// inserting in the middle of the array
		int nOldSize = m_nSize;
		setSize(m_nSize + nCount);  // grow it to new size
		// shift old data up to fill gap
		memmove(&m_pData[nIndex+nCount], &m_pData[nIndex],
			(nOldSize-nIndex) * sizeof(String));

		// re-init slots we copied from

		ConstructElements(&m_pData[nIndex], nCount);

	}

	// insert new value in the gap
	assert(nIndex + nCount <= m_nSize);
}


void StringArray::insertAt(int nIndex, char* newElement, int nCount)
{

	// make room for new elements
	InsertEmpty(nIndex, nCount);

	// copy elements into the empty space
	String temp = String(newElement);
	while (nCount--)
		m_pData[nIndex++] = temp;

}


void StringArray::insertAt(int nIndex, String& newElement, int nCount)
{
	// make room for new elements
	InsertEmpty(nIndex, nCount);

	// copy elements into the empty space
	while (nCount--)
		m_pData[nIndex++] = newElement;
}


void StringArray::removeAt(int nIndex, int nCount)
{
	assert(this);
	assert(nIndex >= 0);
	assert(nCount >= 0);
	assert(nIndex + nCount <= m_nSize);

	// just remove a range
	int nMoveCount = m_nSize - (nIndex + nCount);

	DestructElements(&m_pData[nIndex], nCount);

	if (nMoveCount)
		memmove(&m_pData[nIndex], &m_pData[nIndex + nCount],
			nMoveCount * sizeof(String));
	m_nSize -= nCount;
}

void StringArray::insertAt(int nStartIndex, StringArray* pNewArray)
{
	assert(this);
	assert(pNewArray != NULL);
	assert(pNewArray);
	assert(nStartIndex >= 0);

	if (pNewArray->getSize() > 0)
	{
		insertAt(nStartIndex, (char*)pNewArray->getAt(0).getBuffer(0), pNewArray->getSize());
		for (int i = 0; i < pNewArray->getSize(); i++)
			setAt(nStartIndex + i, pNewArray->getAt(i).getBuffer(0));
	}
}
int	StringArray::read(const char* szFilename)
{
	int nFlag = 0;
	FILE* pFile = fopen (szFilename, "rb");
	if (pFile != NULL)
	{
		nFlag = iFromFile (pFile);
		fclose (pFile);
	}
	return nFlag;
}
int	StringArray::write (const char* szFilename) const
{
	int nFlag = 0;
	FILE* pFile = fopen (szFilename, "wb");
	if (pFile != NULL)
	{
		nFlag = iToFile(pFile);
		fclose (pFile);
	}
	return nFlag;
}

int	StringArray::iFromFile (FILE* pFile)
{
	char szline[1024];
	char* szpos=fgets (szline, 1024, pFile); 
	if (szpos==0)
		return 0;
	int nnum=atoi(szpos);
	for (int i=0; i<nnum; i++)
	{
		szpos=fgets(szline, 1024, pFile);
		if (szpos[strlen(szpos)-1]==10)
			szpos[strlen(szpos)-1]=0;
		add (szpos);
	}
	return 1;
}
int	StringArray::iToFile (FILE* pFile) const
{
	int nnum=getSize();
	fprintf (pFile, "%d\n", nnum);
	for (int i=0; i<nnum; i++)
	{
		fprintf (pFile, "%s\n", getAt(i).str());
	}
	return 1;
}

}
