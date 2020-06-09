
/*!
 * \file    StringArray.h
 * \ingroup base
 * \brief  
 * \author  
 */

#pragma once

#include "String.hpp"
#include "Object.h"

namespace cvlib
{

	class CVLIB_DECLSPEC StringArray : public Object
	{
	public:
		StringArray();
		StringArray(const StringArray& t);

		virtual ~StringArray();

		// Attributes
		int getSize() const;

		int getUpperBound() const;

		void setSize(int nNewSize, int nGrowBy = -1);

		// Operations
		void freeExtra();

		void removeAll();

		// Accessing elements
		String getAt(int nIndex) const;

		void setAt(int nIndex, char* newElement);

		void setAt(int nIndex, String& newElement);

		String& elementAt(int nIndex);

		// Direct Access to the element data (may return NULL)
		const String* getData() const;

		String* getData();

		// Potentially growing the array

		void setAtGrow(int nIndex, const char* newElement);
		void setAtGrow(int nIndex, String& newElement);

		int add(const char* newElement);

		int add(String& newElement);

		int append(const StringArray& src);

		void copy(const StringArray& src);

		// overloaded operator helpers
		String operator[](int nIndex) const;

		String& operator[](int nIndex);

		// Operations that move elements around
		void insertAt(int nIndex, char* newElement, int nCount = 1);

		void insertAt(int nIndex, String& newElement, int nCount = 1);

		void removeAt(int nIndex, int nCount = 1);

		void insertAt(int nStartIndex, StringArray* pNewArray);

		StringArray& operator=(const StringArray& t);
		// Implementation
		int	read(const char* szFilename);
		int	write(const char* szFilename) const;
	protected:
		int	iFromFile(FILE* pFile);
		int	iToFile(FILE* pFile) const;
	protected:
		String* m_pData;
		int m_nSize;
		int m_nMaxSize;
		int m_nGrowBy;

		void InsertEmpty(int nIndex, int nCount);

	protected:
		// local typedefs for class templates
		typedef String BASE_TYPE;
		typedef char* BASE_ARG_TYPE;
	};

}