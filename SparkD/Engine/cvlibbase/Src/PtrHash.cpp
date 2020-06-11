/*!
 * \file	PtrHash.cpp
 * \ingroup base
 * \brief	
 * \author	
 */

#include "PtrHash.h"

namespace cvlib
{

#if !defined LOWORD
#define LOWORD(l)			((ushort)(l))
#endif //LOWORD

#if !defined HIWORD
#define HIWORD(l)			((ushort)(((ulong)(l) >> 16) & 0xFFFF))
#endif //HIWORD

#if !defined LOBYTE
#define LOBYTE(w)			((uchar)(w))
#endif //LOBYTE

#if !defined HIBYTE
#define HIBYTE(w)			((uchar)(((ushort)(w) >> 8) & 0xFF))
#endif //HIBYTE

/**
 * @brief		
 */
struct SMemHash
{
	SMemHash *m_pPrev;
	SMemHash *m_pNext;
	void *m_ptr;
	void *m_pValue;

	SMemHash(){ m_pPrev = m_pNext = NULL; m_ptr = m_pValue = NULL; }
};

struct SMemHashList
{
	int m_nCount;
	SMemHash *m_pHead;
	SMemHash *m_pTail;

	SMemHashList();
	bool add( SMemHash *pMem );
	void *remove( void *ptr );
	void *Get( void *ptr );
	SMemHash *find( void *ptr );

};

SMemHashList::SMemHashList()
{
	m_nCount = 0;
	m_pHead = m_pTail = NULL;
}

bool SMemHashList::add( SMemHash *pMem )
{
	bool bResult = false;
	if ( pMem )
	{
		if ( m_pHead == NULL )
			m_pHead = m_pTail = pMem;
		else
		{
			m_pTail->m_pNext = pMem;
			pMem->m_pPrev = m_pTail;
			m_pTail = pMem;
		}
		m_nCount++;
		bResult = true;
	}
	return bResult;
}

SMemHash *SMemHashList::find( void *ptr )
{
	SMemHash *pMem = NULL;
	if ( ptr )
	{
		SMemHash *pNext = m_pHead;
		while ( pNext )
		{
			if ( ptr == pNext->m_ptr )
			{
				pMem = pNext;
				break;
			}
			pNext = pNext->m_pNext;
		}
	}
	return pMem;
}

void *SMemHashList::Get( void *ptr )
{
	void *ptrResult = NULL;
	if ( ptr )
	{
		SMemHash *pNext = find( ptr );
		if ( pNext )
			ptrResult = pNext->m_pValue;
	}
	return ptrResult;
}

void *SMemHashList::remove( void *ptr )
{
	void *ptrResult = NULL;
	if ( ptr )
	{
		SMemHash *pMem = find( ptr );
		if ( pMem )
		{
			ptrResult = pMem->m_pValue;

			if ( pMem == m_pHead )
			{
				if ( pMem->m_pNext == NULL )
				{
					m_pHead = m_pTail = NULL;
				}
				else
				{
					m_pHead = pMem->m_pNext;
					m_pHead->m_pPrev = NULL;
				}
			}
			else if ( pMem == m_pTail )
			{
				m_pTail = pMem->m_pPrev;
				m_pTail->m_pNext = NULL;
			}
			else
			{
				pMem->m_pNext->m_pPrev = pMem->m_pPrev;
				pMem->m_pPrev->m_pNext = pMem->m_pNext;
			}
		
			delete pMem;
			m_nCount--;
		}
	}
	return ptrResult;
}

PtrHash::PtrHash( int nHashSize )
{
	m_nHashSize = nHashSize;
	m_ppHashTable = (void**)new SMemHashList* [ m_nHashSize ];
	memset( ((SMemHashList**)m_ppHashTable), 0, sizeof( SMemHashList* ) * m_nHashSize );
}

PtrHash::~PtrHash()
{
	if ( ((SMemHashList**)m_ppHashTable) )
	{
		for ( int nIdx = 0; nIdx < m_nHashSize; nIdx++ )
		{
			SMemHashList *pList = ((SMemHashList**)m_ppHashTable)[ nIdx ];
			if ( pList )
			{
				if ( pList->m_pHead )
				{
					SMemHash *pPrev = pList->m_pTail;
					while ( pPrev )
					{
						SMemHash *pTemp = pPrev;
						pPrev = pPrev->m_pPrev;
						delete pTemp;
						pTemp = NULL;
					}
				}
				delete pList;
			}
		}
		delete [] ((SMemHashList**)m_ppHashTable);
	}
	m_ppHashTable = NULL;
}

int PtrHash::getHashValue(void *ptr)
{
	uintptr_t nHashValue = 0;
	uintptr_t nValue = ( uintptr_t )ptr;
	uintptr_t nCode = 0;

	// nHashValue = Sigma( byte(i) * byte(i) )
	nCode = (int)HIBYTE( HIWORD( nValue ) );
	nHashValue += nCode * nCode;

	nCode = LOBYTE( HIWORD( nValue ) );
	nHashValue += nCode * nCode;

	nCode = HIBYTE( LOWORD( nValue ) );
	nHashValue += nCode * nCode;

	nCode = LOBYTE( LOWORD( nValue ) );
	nHashValue += nCode * nCode;

	nHashValue %= m_nHashSize;
	return (int)nHashValue;
}

int PtrHash::addKey( void *ptr, void *pValue )
{
	if ( m_ppHashTable == NULL )
		return 0;

	int nHashValue = getHashValue( ptr );

	SMemHashList *pList = NULL;
	SMemHash *pMem = new SMemHash;
	pMem->m_ptr = ptr;
	pMem->m_pValue = pValue;

	if ( m_ppHashTable[ nHashValue ] == NULL )
	{
		pList = new SMemHashList;
		((SMemHashList**)m_ppHashTable)[ nHashValue ] = pList;
	}
	else
		pList = ((SMemHashList**)m_ppHashTable)[ nHashValue ];

	pList->add( pMem );

	return nHashValue;
}

void *PtrHash::removeKey( void *ptr )
{
	if ( m_ppHashTable == NULL )
		return 0;
	
	int nHashValue = getHashValue( ptr );
	void *ptrResult = NULL;

	SMemHashList *pList = ((SMemHashList**)m_ppHashTable)[ nHashValue ];
	if ( pList )
		ptrResult = pList->remove( ptr );

	return ptrResult;
}

void *PtrHash::getKey( void *ptr )
{
	if ( m_ppHashTable == NULL )
		return 0;
	
	void *pPoint = NULL;
	int nHashValue = getHashValue( ptr );
	SMemHashList *pList = ((SMemHashList**)m_ppHashTable)[ nHashValue ];

	if ( pList )
	{
		SMemHash *pNext = pList->m_pHead;
		while ( pNext )
		{
			if ( ptr == pNext->m_ptr )
			{
				pPoint = pNext->m_pValue;
				break;
			}
			pNext = pNext->m_pNext;
		}
	}

	return pPoint;
}

int PtrHash::getHashSize()
{
	return m_nHashSize;
}

}
