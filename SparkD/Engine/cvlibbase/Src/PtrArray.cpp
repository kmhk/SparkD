/*!
 * \file	PtrArray.cpp
 * \ingroup base
 * \brief	
 * \author	
 */

#include "PtrArray.h"

namespace cvlib 
{

PtrArray::PtrArray()
{
	m_pData = NULL;
	m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

PtrArray::~PtrArray()
{
	assert(this);

	delete[] (uchar*)m_pData;
}

int PtrArray::getSize() const
{
	return m_nSize; 
}

int PtrArray::getUpperBound() const
{
	return m_nSize-1;
}

void PtrArray::setSize(int nNewSize, int nGrowBy)
{
	assert(this);
	assert(nNewSize >= 0);

	if (nGrowBy != -1)
		m_nGrowBy = nGrowBy;  // set new size

	if (nNewSize == 0)
	{
		// shrink to nothing
		if (m_pData)
		delete[] (uchar*)m_pData;
		m_pData = NULL;
		m_nSize = m_nMaxSize = 0;
	}
	else if (m_pData == NULL)
	{
		// create one with exact size
#ifdef SIZE_T_MAX
		assert(nNewSize <= SIZE_T_MAX/sizeof(void*));    // no overflow
#endif
		m_pData = (void**) new uchar[nNewSize * sizeof(void*)];

		memset(m_pData, 0, nNewSize * sizeof(void*));  // zero fill

		m_nSize = m_nMaxSize = nNewSize;
	}
	else if (nNewSize <= m_nMaxSize)
	{
		// it fits
		if (nNewSize > m_nSize)
		{
			// initialize the new elements
			memset(&m_pData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(void*));
		}
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
		assert(nNewMax <= SIZE_T_MAX/sizeof(void*)); // no overflow
#endif
		void** pNewData = (void**) new uchar[nNewMax * sizeof(void*)];

		// copy new data from old
		memcpy(pNewData, m_pData, m_nSize * sizeof(void*));

		// construct remaining elements
		assert(nNewSize > m_nSize);

		memset(&pNewData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(void*));


		// get rid of old stuff (note: no destructors called)
		delete[] (uchar*)m_pData;
		m_pData = pNewData;
		m_nSize = nNewSize;
		m_nMaxSize = nNewMax;
	}
}

void PtrArray::freeExtra()
{
	assert(this);

	if (m_nSize != m_nMaxSize)
	{
		// shrink to desired size
#ifdef SIZE_T_MAX
		assert(m_nSize <= SIZE_T_MAX/sizeof(void*)); // no overflow
#endif
		void** pNewData = NULL;
		if (m_nSize != 0)
		{
			pNewData = (void**) new uchar[m_nSize * sizeof(void*)];
			// copy new data from old
			memcpy(pNewData, m_pData, m_nSize * sizeof(void*));
		}

		// get rid of old stuff (note: no destructors called)
		delete[] (uchar*)m_pData;
		m_pData = pNewData;
		m_nMaxSize = m_nSize;
	}
}

void PtrArray::removeAll()
{
	setSize(0);
}

void* PtrArray::getAt(int nIndex) const
{
	assert(nIndex >= 0 && nIndex < m_nSize);
	return m_pData[nIndex];
}

void PtrArray::setAt(int nIndex, void* newElement)
{
	assert(nIndex >= 0 && nIndex < m_nSize);
	m_pData[nIndex] = newElement;
}

void*& PtrArray::elementAt(int nIndex)
{
	assert(nIndex >= 0 && nIndex < m_nSize);
	return m_pData[nIndex];
}

const void** PtrArray::getData() const
{
	return (const void**)m_pData;
}

void** PtrArray::getData()
{
	return (void**)m_pData;
}

void PtrArray::setAtGrow(int nIndex, void* newElement)
{
	assert(this);
	assert(nIndex >= 0);

	if (nIndex >= m_nSize)
		setSize(nIndex+1);
	m_pData[nIndex] = newElement;
}

int PtrArray::add(void* newElement)
{
	int nIndex = m_nSize;
	setAtGrow(nIndex, newElement);
	return nIndex;
}

int PtrArray::append(const PtrArray& src)
{
	assert(this);
	assert(this != &src);   // cannot append to itself

	int nOldSize = m_nSize;
	setSize(m_nSize + src.m_nSize);

	memcpy(m_pData + nOldSize, src.m_pData, src.m_nSize * sizeof(void*));

	return nOldSize;
}

void PtrArray::copy(const PtrArray& src)
{
	assert(this);
	assert(this != &src);   // cannot append to itself

	setSize(src.m_nSize);

	memcpy(m_pData, src.m_pData, src.m_nSize * sizeof(void*));

}

void* PtrArray::operator[](int nIndex) const
{
	return getAt(nIndex);
}

void*& PtrArray::operator[](int nIndex)
{
	return elementAt(nIndex);
};

void PtrArray::insertAt(int nIndex, void* newElement, int nCount)
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
			(nOldSize-nIndex) * sizeof(void*));

		// re-init slots we copied from
		memset(&m_pData[nIndex], 0, nCount * sizeof(void*));
	}

	// insert new value in the gap
	assert(nIndex + nCount <= m_nSize);



	// copy elements into the empty space
	while (nCount--)
		m_pData[nIndex++] = newElement;

}

void PtrArray::insertAt(int nStartIndex, PtrArray* pNewArray)
{
	assert(this);
	assert(pNewArray != NULL);
	assert(pNewArray);
	assert(nStartIndex >= 0);

	if (pNewArray->getSize() > 0)
	{
		insertAt(nStartIndex, pNewArray->getAt(0), pNewArray->getSize());
		for (int i = 0; i < pNewArray->getSize(); i++)
			setAt(nStartIndex + i, pNewArray->getAt(i));
	}
}

void PtrArray::removeAt(int nIndex, int nCount)
{
	assert(this);
	assert(nIndex >= 0);
	assert(nCount >= 0);
	assert(nIndex + nCount <= m_nSize);

	// just remove a range
	int nMoveCount = m_nSize - (nIndex + nCount);

	if (nMoveCount)
		memmove(&m_pData[nIndex], &m_pData[nIndex + nCount],
			nMoveCount * sizeof(void*));
	m_nSize -= nCount;
}

}
