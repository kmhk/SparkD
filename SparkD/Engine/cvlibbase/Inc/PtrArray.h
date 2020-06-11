/*!
 * \file	PtrArray.h
 * \ingroup base
 * \brief	.
 * \author	
 */

#pragma once

#include "cvlibmacros.h"

namespace cvlib 
{

/**
 * @brief.
 */
class CVLIB_DECLSPEC PtrArray
{
public:
	// Construction and Destruction
	PtrArray();
	virtual ~PtrArray();

	int getSize() const;
	int getUpperBound() const;
	void setSize(int nNewSize, int nGrowBy = -1);

	// Operations for Cleaning up
	void freeExtra();
	void removeAll();

	// Operations for Accessing elements
	void* getAt(int nIndex) const;
	void setAt(int nIndex, void* newElement);
	void*& elementAt(int nIndex);
	const void** getData() const;
	void** getData();
	void setAtGrow(int nIndex, void* newElement);
	int add(void* newElement);
	int append(const PtrArray& src);
	void copy(const PtrArray& src);

	// overloaded operator helpers
	void* operator[](int nIndex) const;
	void*& operator[](int nIndex);

	// Operations that move elements around
	void insertAt(int nIndex, void* newElement, int nCount = 1);

	void insertAt(int nStartIndex, PtrArray* pNewArray);

	void removeAt(int nIndex, int nCount = 1);

// Implementation
protected:
	void** m_pData;
	int m_nSize;
	int m_nMaxSize;
	int m_nGrowBy;

protected:
	// local typedefs for class templates
	typedef void* BASE_TYPE;
	typedef void* BASE_ARG_TYPE;
};

}