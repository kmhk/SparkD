/*!
 * \file	PumpABC.cpp
 * \ingroup base
 * \brief
 * \author
 */

#include <assert.h>
#include "Tree.h"

namespace cvlib
{

void ListNode::detach()
{
	if( parent() == 0 )
		return;
 	
 	if(m_parent->m_first == this )
		m_parent->m_first = m_next;
 	if(m_parent->m_last == this )
		m_parent->m_last = m_prev;
	m_parent = 0;
	
	if(m_prev != 0 )
		m_prev->m_next = m_next;
	if(m_next != 0 )
		m_next->m_prev = m_prev;
	m_prev = m_next = 0;
}

void List::detachAll()
{
	ListNode* node = m_first;
	while( node != 0 ) {
		ListNode* next = node->m_next;
		node->m_next = node->m_prev = 0;
		node->m_parent = 0;
		node = next;
	}
	m_first = m_last = 0;
}

void List::deleteAll()
{
	while( !isEmpty() )
		delete m_first;
	m_first = m_last = 0;
}

//-------------------------------------------------------------------------------------------

void List::sort( int (*fcmp)( const ListNode*, const ListNode* ) )
{
	if( isEmpty() )
		return;
	for( ListNode* listEnd = first(); listEnd->next() != 0; ) {
		if( fcmp( listEnd, listEnd->next() ) <= 0 )
			listEnd = listEnd->next();
		else {
			ListNode* node = listEnd->next();
			node->detach();
			ListNode* insBefore=NULL;
			for( insBefore = first(); fcmp(insBefore, node) <= 0; 
			insBefore = insBefore->next() )
				;
			insBefore->insertPrev( node );
		}
	}
}

void List::addListFirst( List& toAdd )
{
	if( toAdd.isEmpty() )
		return;

	for( ListNode* ptr = toAdd.first(); ptr != 0; ptr = ptr->next() )
		ptr->m_parent = this;
	
	if( isEmpty() ) {
		m_last = toAdd.last();
	} else {
		toAdd.last()->m_next = first();
		first()->m_prev = toAdd.last();
	}
	m_first = toAdd.first();
	toAdd.m_first = toAdd.m_last = 0;
}

void List::addListLast( List& toAdd )
{
	if( toAdd.isEmpty() )
		return;

	for( ListNode* ptr = toAdd.first(); ptr != 0; ptr = ptr->next() )
		ptr->m_parent = this;
	
	if( isEmpty() ) {
		m_first = toAdd.first();
	} else {
		toAdd.first()->m_prev = last();
		last()->m_next = toAdd.first();
	}
	m_last = toAdd.last();
	toAdd.m_first = toAdd.m_last = 0;
}

// class CListSortInfo
// {
// public:
// 	CListSortInfo() : CompareFunc(0) {}
// 
// 	int (*CompareFunc)(const ListNode*, const ListNode*);
// };
// 
// static CThreadLocal<CListSortInfo> sortInfo;
// 
// static int listCmp( const const ListNode* *n1, const const ListNode* *n2 )
// {
// 	return sortInfo->CompareFunc( *n1, *n2 );
// }
// 
// void CList::qsort( int (*fcmp)(const ListNode*, const ListNode*) )
// {
// 	CListSortArray buffer;
// 
// 	for( ListNode* node = first(); node != 0; node = node->next() )
// 		buffer.add( node );
// 
// 	sortInfo->CompareFunc = fcmp;
// 	buffer.qsort( listCmp );
// 
// 	for( int i = 0; i < buffer.Size(); i++ ) {
// 		ListNode*(buffer[i])->detach();
// 		addLast( ListNode*(buffer[i]) );
// 	}
// }

int List::numberOfChildren() const
{
	int count = 0;
	for( const ListNode* node = first(); node != 0; node = node->next() )
		count++;
	return count;
}

}
