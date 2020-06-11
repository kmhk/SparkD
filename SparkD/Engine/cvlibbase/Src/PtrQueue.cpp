/*!
 * \file	PtrQueue.cpp
 * \ingroup base
 * \brief	
 * \author	
 */

#include "PtrQueue.h"

namespace cvlib 
{

/**
 * class PtrQueueData : PtrQueue
 */
class PtrQueueData
{
public:
	int		m_nFirst;
	int		m_nLast;
	int		m_nCur;	
	int		m_nCount;
	void**	m_ppData;

public:
	// Constructor and Destructor
	PtrQueueData()
	{
		m_nCount = 0; m_ppData = NULL; m_nFirst = 0; m_nLast = -1; m_nCur = 0;
	}

	virtual ~PtrQueueData()	{};
	void Init ()
	{
		m_nFirst = m_nCur = 0;
		m_nLast = m_nCount - 1;
	}
	void pushHead(void* pData)
	{
		m_nFirst = (m_nFirst - 1 + m_nCount) % m_nCount;
		m_ppData[m_nFirst] = pData;
		m_nCur = m_nFirst;
	}
	void pushTail(void* pData)
	{
		m_nLast = (m_nLast + 1) % m_nCount;
		m_ppData[m_nLast] = pData;
	}
	void* popHead ()
	{
		void* pRet = m_ppData[m_nFirst];
		m_ppData[m_nFirst] = NULL;
		m_nFirst = (m_nFirst + 1) % m_nCount;
		m_nCur = m_nFirst;
		return pRet;
	}
	void* popTail ()
	{
		void* pRet = m_ppData[m_nLast];
		m_ppData[m_nLast] = NULL;
		m_nLast = (m_nLast - 1 + m_nCount) % m_nCount;
		return pRet;
	}
	void setPosition(int nWhence, int nIndex)
	{
		switch (nWhence)
		{
		case PtrQueue::POS_HEAD:
			m_nCur = (m_nFirst + nIndex + m_nCount) % m_nCount;
			break;
		case PtrQueue::POS_TAIL:
			m_nCur = (m_nLast - nIndex + m_nCount) % m_nCount;
			break;
		}
	}
	bool find(const void* pData)
	{
		int i;
		for (i = 0; i < m_nCount; i ++)
		{
			if (m_ppData[i] == pData)
				break;
		}
		if (i == m_nCount)
			return false;
		else 
			return true;
	}
};

PtrQueue::PtrQueue ()
{
	m_pData = NULL;
}

PtrQueue::PtrQueue (int	nCount)
{
	create (nCount);
}

PtrQueue::~PtrQueue ()
{
	if (flagCreate())
		release();
	assert(m_pData == NULL);
}

bool PtrQueue::flagCreate ()
{
	return m_pData ? true : false;
}

bool PtrQueue::create(int nCount)
{
	assert (m_pData == NULL);
	assert (nCount > 0);
	m_pData = new PtrQueueData;
//	Init();
	((PtrQueueData*)m_pData)->m_nCount = nCount;
	((PtrQueueData*)m_pData)->m_ppData = new void*[nCount];
	memset (((PtrQueueData*)m_pData)->m_ppData, 0, sizeof(void*) * nCount);
	((PtrQueueData*)m_pData)->m_nFirst = 0;
	((PtrQueueData*)m_pData)->m_nLast = nCount - 1;
	return true;
}

void PtrQueue::release()
{
	delete []((PtrQueueData*)m_pData)->m_ppData;
	delete (PtrQueueData*)m_pData;
	m_pData = NULL;
}

int	PtrQueue::count() const
{
	return	((PtrQueueData*)m_pData)->m_nCount;
}

void* PtrQueue::data() const
{
	PtrQueueData* pData = ((PtrQueueData*)m_pData);
	return pData->m_ppData[pData->m_nCur];
}

void PtrQueue::pushHead(void* pData)
{
	((PtrQueueData*)m_pData)->pushHead(pData);
}

void PtrQueue::pushTail(void* pData)
{
	((PtrQueueData*)m_pData)->pushTail(pData);
}

void* PtrQueue::popHead ()
{
	return 	((PtrQueueData*)m_pData)->popHead();
}

void* PtrQueue::popTail ()
{
	return 	((PtrQueueData*)m_pData)->popTail();
}

void PtrQueue::setPosition(int	nWhence, int nIndex)
{
	((PtrQueueData*)m_pData)->setPosition(nWhence, nIndex);
}

}
