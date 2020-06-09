/*!
 * \file	Tree.h
 * \ingroup base
 * \brief	
 * \author	
 */

#pragma once

#include "cvlibmacros.h"

namespace cvlib
{
class List;
class ListNode;
class Tree;

#define	DECLARE_STD_NODE_METHODS(nodeClass, listClass)	\
public:	const nodeClass* prev() const;\
const nodeClass* next() const;\
const listClass* parent() const;\
nodeClass* prev();\
nodeClass* next();\
listClass* parent();

#define	IMPLEMENT_STD_NODE_METHODS(nodeClass, listClass)	\
inline const nodeClass* nodeClass::prev() const { return static_cast<const nodeClass*>(ListNode::prev()); }	\
inline const nodeClass* nodeClass::next() const { return static_cast<const nodeClass*>(ListNode::next()); }	\
inline const listClass* nodeClass::parent() const { return static_cast<const listClass*>(ListNode::parent()); } \
inline nodeClass* nodeClass::prev() { return static_cast<nodeClass*>(ListNode::prev()); }	\
inline nodeClass* nodeClass::next() { return static_cast<nodeClass*>(ListNode::next()); }	\
inline listClass* nodeClass::parent() { return static_cast<listClass*>(ListNode::parent()); }

class CVLIB_DECLSPEC ListNode {
public:
	ListNode() { m_parent = 0; m_prev = m_next = 0; }
	virtual ~ListNode() { detach(); }

	const ListNode* prev() const { return m_prev; }
	const ListNode* next() const { return m_next; }
	const List* parent() const { return m_parent; }
	ListNode* prev() { return m_prev; }
	ListNode* next() { return m_next; }
	List* parent() { return m_parent; }

	void insertPrev( ListNode* toIns );
	void insertNext( ListNode* toIns );

	void detach();

private:
	ListNode( const ListNode& ); // Withoun body
	ListNode& operator = ( ListNode& ); // Withoun body

	List* m_parent;
	ListNode* m_prev;
	ListNode* m_next;

	friend class List;
};

//------------------------------------------------------------------------
#define	DECLARE_STD_LIST_METHODS(listClass, nodeClass)	\
public:	const nodeClass* first() const;\
const nodeClass* last() const;\
nodeClass* first();\
nodeClass* last();

#define	IMPLEMENT_STD_LIST_METHODS(listClass, nodeClass)	\
inline const nodeClass* listClass::first() const { return static_cast<const nodeClass*>(List::first()); }	\
inline const nodeClass* listClass::last() const { return static_cast<const nodeClass*>(List::last()); }	\
inline nodeClass* listClass::first() { return static_cast<nodeClass*>(List::first()); }	\
inline nodeClass* listClass::last() { return static_cast<nodeClass*>(List::last()); }

class CVLIB_DECLSPEC List {
public:
	List() { m_first = m_last = 0; }
	virtual ~List() { deleteAll(); }

	const ListNode* first() const { return m_first; }
	const ListNode* last() const { return m_last; }
	ListNode* first() { return m_first; }
	ListNode* last() { return m_last; }

	int numberOfChildren() const;
	bool isEmpty() const { return m_first == 0; }
	bool hasChild( const ListNode* child ) const { return this == child->parent(); }

	void addFirst( ListNode* child );
		// adds the element to the top of list
	void addLast( ListNode* child );
		// adds the element to the end of list
	void addListFirst( List& toAdd );
	void addListLast( List& toAdd );
	void deleteAll(); 
	void detachAll(); 

	void sort( int (*fcmp)( const ListNode*, const ListNode* ) );
	void qsort( int (*fcmp)(const ListNode*, const ListNode*) );

protected:
	ListNode* m_first;
	ListNode* m_last;

private:
	List( const List& ); // Withoun body
	const List& operator = ( const List& ); // Withoun body

	friend class ListNode;
};

//-------------------------------------------------------------------------
#define	DEFINE_STD_TREE_METHODS(nodeClass)	\
public:	const nodeClass* prev() const { return static_cast<const nodeClass*>(Tree::prev()); }	\
const nodeClass* next() const { return static_cast<const nodeClass*>(Tree::next()); }	\
const nodeClass* parent() const { return static_cast<const nodeClass*>(Tree::parent()); }	\
const nodeClass* first() const { return static_cast<const nodeClass*>(Tree::first()); }	\
const nodeClass* last() const { return static_cast<const nodeClass*>(Tree::last()); }	\
nodeClass* prev() { return static_cast<nodeClass*>(Tree::prev()); }	\
nodeClass* next() { return static_cast<nodeClass*>(Tree::next()); }	\
nodeClass* parent() { return static_cast<nodeClass*>(Tree::parent()); }	\
nodeClass* first() { return static_cast<nodeClass*>(Tree::first()); }	\
nodeClass* last() { return static_cast<nodeClass*>(Tree::last()); }

#define	DECLARE_STD_TREE_METHODS(nodeClass, parentClass, childClass)	\
public:	const nodeClass* prev() const;\
const nodeClass* next() const;\
const parentClass* parent() const;\
const childClass* first() const;\
const childClass* last() const;\
nodeClass* prev();\
nodeClass* next();\
parentClass* parent();\
childClass* first();\
childClass* last();

#define	IMPLEMENT_STD_TREE_METHODS(nodeClass, parentClass, childClass)	\
inline const nodeClass* nodeClass::prev() const { return static_cast<const nodeClass*>(Tree::prev()); }	\
inline const nodeClass* nodeClass::next() const { return static_cast<const nodeClass*>(Tree::next()); }	\
inline const parentClass* nodeClass::parent() const { return static_cast<const parentClass*>(Tree::parent()); }	\
inline const childClass* nodeClass::first() const { return static_cast<const childClass*>(Tree::first()); }	\
inline const childClass* nodeClass::last() const { return static_cast<const childClass*>(Tree::last()); }	\
inline nodeClass* nodeClass::prev() { return static_cast<nodeClass*>(Tree::prev()); }	\
inline nodeClass* nodeClass::next() { return static_cast<nodeClass*>(Tree::next()); }	\
inline parentClass* nodeClass::parent() { return static_cast<parentClass*>(Tree::parent()); }	\
inline childClass* nodeClass::first() { return static_cast<childClass*>(Tree::first()); }	\
inline childClass* nodeClass::last() { return static_cast<childClass*>(Tree::last()); }

class CVLIB_DECLSPEC Tree : private ListNode, private List {
public:
	const Tree* prev() const { return (const Tree*) ListNode::prev(); }
	const Tree* next() const { return (const Tree*) ListNode::next(); }
	const Tree* parent() const { return (const Tree*) ListNode::parent(); }
	const Tree* first() const { return (const Tree*) List::first(); }
	const Tree* last() const { return (const Tree*) List::last(); }
	Tree* prev() { return (Tree*) ListNode::prev(); }
	Tree* next() { return (Tree*) ListNode::next(); }
	Tree* parent() { return (Tree*) ListNode::parent(); }
	Tree* first() { return (Tree*) List::first(); }
	Tree* last() { return (Tree*) List::last(); }

	bool hasChild( const Tree* child ) const { return List::hasChild( child ); }
	
	void insertPrev( Tree* toIns ) { ListNode::insertPrev( toIns ); }
	void insertNext( Tree* toIns ) { ListNode::insertNext( toIns ); }

	void addFirst( Tree* child ) { List::addFirst( child ); }
		// add childs to the top of childs' list
	void addLast( Tree* child ) { List::addLast( child ); }
		// add childs to the end of childs' list

	void AddTreeFirst( Tree& tree ) { List::addListFirst( tree ); }
	void AddTreeLast( Tree& tree ) { List::addListLast( tree ); }

 	void sort( int (*fcmp)( const Tree*, const Tree* ) )
 		{ List::sort( (int (*)(const ListNode*, const ListNode*)) fcmp ); }
		
	using List::isEmpty;
	using List::deleteAll;
	using List::detachAll;
	using List::numberOfChildren;
	
	using ListNode::detach;
};

//----------------------------------------------------------------------
inline void ListNode::insertPrev( ListNode* toIns )
{
	assert( toIns->parent() == 0 && parent() != 0 );

	if( m_prev != 0 ) {
		toIns->m_prev = m_prev;
		m_prev->m_next = toIns;
	} else
		m_parent->m_first = toIns;

	toIns->m_next = this;
	m_prev = toIns;
	toIns->m_parent = m_parent;
}

inline void ListNode::insertNext( ListNode* toIns )
{
	assert( toIns->parent() == 0 && parent() != 0 );

	if(m_next != 0 ) {
		toIns->m_next = m_next;
		m_next->m_prev = toIns;
	} else
		m_parent->m_last = toIns;

	toIns->m_prev = this;
	m_next = toIns;
	toIns->m_parent = m_parent;
}

inline void List::addFirst( ListNode* child )
{
	assert( child->parent() == 0 );
 	if(m_first != 0 ) {
		m_first->insertPrev( child );
	} else {
		m_first = m_last = child;
		child->m_parent = this;
	}
}

inline void List::addLast( ListNode* child )
{
	assert( child->parent() == 0 );
 	if(m_last != 0 ) {
		m_last->insertNext( child );
	} else {
		m_first = m_last = child;
 		child->m_parent = this;
	}
}

}