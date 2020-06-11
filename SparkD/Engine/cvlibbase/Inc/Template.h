
/*!
 * \file	Template.h
 * \ingroup base
 * \brief	
 * \author	
 */

#pragma once

#include "cvlibbaseDef.h"
#include "Object.h"
#include "String.hpp"
#include "Plex.h"
#include <new>

namespace cvlib
{

	struct _CVLIB_POSITION { };
	typedef _CVLIB_POSITION* CVLIB_POSITION;
#define CVLIB_BEFORE_START_POSITION ((CVLIB_POSITION)-1L)

	template<class TYPE>
	void CVLIB_DECLSPEC constructElements1(TYPE* pElements, int nCount)
	{
		// first do bit-wise zero initialization
		memset((void*)pElements, 0, nCount * sizeof(TYPE));

		// then call the constructor(s)
		for (; nCount--; pElements++)
			::new((void*)pElements) TYPE;
	}

	template<class TYPE>
	void CVLIB_DECLSPEC constructElements1(TYPE* pElements, int nCount, const TYPE& newelem)
	{
		// then call the constructor(s)
		for (; nCount--; pElements++)
		{
			::new((void*)pElements) TYPE;
			*pElements = newelem;
		}
	}

	template<class TYPE>
	void CVLIB_DECLSPEC destructElements1(TYPE* pElements, int nCount)
	{
		// call the destructor(s)
		for (; nCount--; pElements++)
			pElements->~TYPE();
	}

	template<class TYPE>
	void CVLIB_DECLSPEC copyElements1(TYPE* pDest, const TYPE* pSrc, int nCount)
	{
		// default is element-copy using assignment
		while (nCount--)
			*pDest++ = *pSrc++;
	}

	template<class TYPE, class ARG_TYPE>
	bool CVLIB_DECLSPEC compareElements1(const TYPE* pElement1, const ARG_TYPE* pElement2)
	{
		return *pElement1 == *pElement2;
	}

	template<class ARG_KEY>
	inline uint CVLIB_DECLSPEC cvlHashKey(ARG_KEY key)
	{
		// default identity hash - works for most primitive values
		return ((uintptr_t)(void*)(ulong)key) >> 4;
	}

#ifdef _MSC_VER
	//#if CVLIB_OS==CVLIB_OS_WIN32

#if _MSC_VER >= 1100
	template<> void CVLIB_DECLSPEC constructElements1<String>(String* pElements, int nCount);
	template<> void CVLIB_DECLSPEC destructElements1<String>(String* pElements, int nCount);
	template<> void CVLIB_DECLSPEC copyElements1<String>(String* pDest, const String* pSrc, int nCount);
	template<> uint CVLIB_DECLSPEC cvlHashKey<const String&>(const String& key);
#else // _MSC_VER >= 1100
	void CVLIB_DECLSPEC constructElements1(String* pElements, int nCount);
	void CVLIB_DECLSPEC destructElements1(String* pElements, int nCount);
	void CVLIB_DECLSPEC copyElements1(String* pDest, const String* pSrc, int nCount);
	uint CVLIB_DECLSPEC cvlHashKey(const String& key);
#endif

#elif CVLIB_OS==CVLIB_OS_APPLE
	template<> void CVLIB_DECLSPEC constructElements1<String>(String* pElements, int nCount);
	template<> void CVLIB_DECLSPEC destructElements1<String>(String* pElements, int nCount);
	template<> void CVLIB_DECLSPEC copyElements1<String>(String* pDest, const String* pSrc, int nCount);
	template<> uint CVLIB_DECLSPEC cvlHashKey<const String&>(const String& key);
#else
	template<> void CVLIB_DECLSPEC constructElements1<String>(String* pElements, int nCount);
	template<> void CVLIB_DECLSPEC destructElements1<String>(String* pElements, int nCount);
	template<> void CVLIB_DECLSPEC copyElements1<String>(String* pDest, const String* pSrc, int nCount);
	template<> uint CVLIB_DECLSPEC cvlHashKey<const String&>(const String& key);
#endif
	/////////////////////////////////////////////////////////////////////////////
	// #define new new(__FILE__, __LINE__)
	template<class TYPE, class ARG_TYPE = const TYPE&>
	class CVLIB_DECLSPEC ConstArray : public Object
	{
	public:
		typedef ConstArray<TYPE, ARG_TYPE> _MyArray;
		ConstArray() {
			m_fcopydata = false;
			m_pData = 0; m_nSize = m_nMaxSize = m_nGrowBy = 0;
		}
		ConstArray(const _MyArray& t) {
			m_fcopydata = false;
			m_pData = t.m_pData;
			m_nSize = m_nMaxSize = t.m_nSize;
			m_nGrowBy = 0;
		}
		ConstArray(const TYPE* parray, int len) {
			m_fcopydata = false;
			m_pData = parray;
			m_nSize = m_nMaxSize = len;
			m_nGrowBy = 0;
		}
		_MyArray& operator=(const _MyArray& other)
		{
			if (this == &other)
				return *this;
			removeAll();
			append(other);
			return *this;
		}
		virtual void removeAll() { m_pData = 0; m_nSize = m_nMaxSize = m_nGrowBy = 0; }

		int getSize() const { return m_nSize; }
		int length() const { return m_nSize; }
		int getUpperBound() const { return m_nSize - 1; }
		const TYPE& getAt(int nIndex) const { assert(nIndex >= 0 && nIndex < m_nSize); return m_pData[nIndex]; }
		const TYPE* getData() const { return (const TYPE*)m_pData; }
		const TYPE& operator[](int nIndex) const { return getAt(nIndex); }

		const TYPE& front() const { return getAt(0); }
		const TYPE& back() const { return getAt(getSize() - 1); }

		bool isEmpty() const { return m_nSize == 0; }
	protected:
		TYPE* m_pData;
		int m_nSize;     // # of elements (upperBound - 1)
		int m_nMaxSize;
		int m_nGrowBy;
		bool m_fcopydata;

	public:
		~ConstArray() {
			removeAll();
		}
	};

	template<class TYPE, class ARG_TYPE = const TYPE&>
	class CVLIB_DECLSPEC Array : public Object
	{
	public:
		typedef Array<TYPE, ARG_TYPE> _MyArray;
	public:
		Array() { m_fcopydata = true; m_pData = 0; m_nSize = m_nMaxSize = m_nGrowBy = 0; }
		Array(int nSize, const TYPE& _V = TYPE())
		{
			m_fcopydata = true;
			m_pData = NULL;
			m_nSize = m_nMaxSize = m_nGrowBy = 0;
			setSize(nSize);
			for (int i = 0; i < getSize(); i++)
				m_pData[i] = _V;
		}
		Array(const _MyArray& other)
		{
			m_fcopydata = true;
			m_pData = NULL;
			m_nSize = m_nMaxSize = m_nGrowBy = 0;
			append(other);
		}
		Array(const TYPE* parray, int len, bool fcopydata = true)
		{
			m_fcopydata = fcopydata;
			if (fcopydata) {
				m_pData = NULL;
				m_nSize = m_nMaxSize = m_nGrowBy = 0;
				append(parray, len);
			}
			else {
				m_pData = (TYPE*)parray;
				m_nSize = m_nMaxSize = len;
				m_nGrowBy = 0;
			}
		}
		_MyArray& operator=(const _MyArray& other)
		{
			if (this == &other)
				return *this;
			removeAll();
			m_fcopydata = true;
			append(other);
			return *this;
		}

		// Attributes
		void setSize(int nNewSize, int nGrowBy = -1);
		void resize(int nNewSize, ARG_TYPE newElement = TYPE());

		// Operations
		void freeExtra();
		void removeAll();

		inline void setAt(int nIndex, ARG_TYPE newElement) { assert(nIndex >= 0 && nIndex < m_nSize); m_pData[nIndex] = newElement; }
		inline int add(ARG_TYPE newElement) { int nIndex = m_nSize; setAtGrow(nIndex, newElement); return nIndex; }


		void setAtGrow(int nIndex, ARG_TYPE newElement);
		int append(const Array& src);
		int append(const TYPE* parray, int size);
		void copyFrom(const Array& src);
		void removeAt(int nIndex, int nCount = 1);
		void insertAt(int nIndex, ARG_TYPE newElement = TYPE(), int nCount = 1);
		void insertAt(int nStartIndex, Array* pNewArray);
		void insertAt(int nStartIndex, const _MyArray& newArray);
		void swap(_MyArray& aray);

		// readonly apis
		inline const TYPE& operator[](int nIndex) const { return getAt(nIndex); }
		inline operator const TYPE*() const { return (const TYPE*)m_pData; }
		inline const TYPE& getAt(int nIndex) const { assert(nIndex >= 0 && nIndex < m_nSize); return m_pData[nIndex]; }
		inline const TYPE* getData() const { return (const TYPE*)m_pData; }
		inline const TYPE& front() const { return getAt(0); }
		inline const TYPE& back() const { return getAt(getSize() - 1); }

		// write apis
		inline TYPE& operator[](int nIndex) { return getAt(nIndex); }
		inline operator TYPE*() { return (TYPE*)m_pData; }
		inline TYPE& getAt(int nIndex) { assert(nIndex >= 0 && nIndex < m_nSize); return m_pData[nIndex]; }
		inline TYPE* getData() { return (TYPE*)m_pData; }
		inline TYPE& front() { return getAt(0); }
		inline TYPE& back() { return getAt(getSize() - 1); }
		inline void pushBack(ARG_TYPE t) { add(t); }
		inline void popBack() { removeAt(getSize() - 1); }

		int getSize() const { return m_nSize; }
		int length() const { return m_nSize; }
		int getUpperBound() const { return m_nSize - 1; }
		bool isEmpty() const { return m_nSize == 0; }

	public:
		~Array();
	protected:
		TYPE* m_pData;
		int m_nSize;     // # of elements (upperBound - 1)
		int m_nMaxSize;
		int m_nGrowBy;
		bool m_fcopydata;
	};

	/////////////////////////////////////////////////////////////////////////////
	// Array<TYPE, ARG_TYPE> inline functions

	template<class TYPE, class ARG_TYPE>
	void Array<TYPE, ARG_TYPE>::removeAll() {
		if (m_fcopydata)
			setSize(0, -1);
		else {
			m_fcopydata = true;
			m_pData = 0; m_nSize = m_nMaxSize = m_nGrowBy = 0;
		}
	}

	/////////////////////////////////////////////////////////////////////////////
	// Array<TYPE, ARG_TYPE> out-of-line functions

	template<class TYPE, class ARG_TYPE>
	Array<TYPE, ARG_TYPE>::~Array()
	{
		assert(this);
		if (m_fcopydata) {
			if (m_pData != NULL)
			{
				destructElements1<TYPE>(m_pData, m_nSize);
				delete[](uchar*)m_pData;
			}
		}
		else {
			m_fcopydata = true;
			m_pData = 0; m_nSize = m_nMaxSize = m_nGrowBy = 0;
		}
	}

	template<class TYPE, class ARG_TYPE>
	void Array<TYPE, ARG_TYPE>::setSize(int nNewSize, int _nGrowBy)
	{
		assert(this);
		assert(nNewSize >= 0);

		if (_nGrowBy != -1)
			m_nGrowBy = _nGrowBy;  // set new size

		if (nNewSize == 0)
		{
			// shrink to nothing
			if (m_pData != NULL)
			{
				destructElements1<TYPE>(m_pData, m_nSize);
				delete[](uchar*)m_pData;
				m_pData = NULL;
			}
			m_nSize = m_nMaxSize = 0;
		}
		else if (m_pData == NULL)
		{
			// create one with exact size
#ifdef SIZE_T_MAX
			assert(nNewSize <= SIZE_T_MAX / sizeof(TYPE));    // no overflow
#endif
			m_pData = (TYPE*) new uchar[nNewSize * sizeof(TYPE)];
			constructElements1<TYPE>(m_pData, nNewSize);
			m_nSize = m_nMaxSize = nNewSize;
		}
		else if (nNewSize <= m_nMaxSize)
		{
			// it fits
			if (nNewSize > m_nSize)
			{
				// initialize the new elements
				constructElements1<TYPE>(&m_pData[m_nSize], nNewSize - m_nSize);
			}
			else if (m_nSize > nNewSize)
			{
				// destroy the old elements
				destructElements1<TYPE>(&m_pData[nNewSize], m_nSize - nNewSize);
			}
			m_nSize = nNewSize;
		}
		else
		{
			// otherwise, grow array
			int nGrowBy = m_nGrowBy;
			if (nGrowBy == 0)
			{
				// heuristically determine growth when nGrowBy == 0
				//  (this avoids heap fragmentation in many situations)
				nGrowBy = m_nSize / 8;
				nGrowBy = (nGrowBy < 4) ? 4 : ((nGrowBy > 1024) ? 1024 : nGrowBy);
			}
			int nNewMax;
			if (nNewSize < m_nMaxSize + nGrowBy)
				nNewMax = m_nMaxSize + nGrowBy;  // granularity
			else
				nNewMax = nNewSize;  // no slush

			assert(nNewMax >= m_nMaxSize);  // no wrap around
#ifdef SIZE_T_MAX
			assert(nNewMax <= SIZE_T_MAX / sizeof(TYPE)); // no overflow
#endif
			TYPE* pNewData = (TYPE*) new uchar[nNewMax * sizeof(TYPE)];

			// copy new data from old
			memcpy((void*)pNewData, (const void*)m_pData, m_nSize * sizeof(TYPE));

			// construct remaining elements
			assert(nNewSize > m_nSize);
			constructElements1<TYPE>(&pNewData[m_nSize], nNewSize - m_nSize);

			// get rid of old stuff (note: no destructors called)
			delete[](uchar*)m_pData;
			m_pData = pNewData;
			m_nSize = nNewSize;
			m_nMaxSize = nNewMax;
		}
	}

	template<class TYPE, class ARG_TYPE>
	void Array<TYPE, ARG_TYPE>::resize(int nNewSize, ARG_TYPE elem)
	{
		assert(this);
		assert(nNewSize >= 0);

		if (nNewSize == 0)
		{
			// shrink to nothing
			if (m_pData != NULL)
			{
				destructElements1<TYPE>(m_pData, m_nSize);
				delete[](uchar*)m_pData;
				m_pData = NULL;
			}
			m_nSize = m_nMaxSize = 0;
		}
		else if (m_pData == NULL)
		{
			// create one with exact size
#ifdef SIZE_T_MAX
			assert(nNewSize <= SIZE_T_MAX / sizeof(TYPE));    // no overflow
#endif
			m_pData = (TYPE*) new uchar[nNewSize * sizeof(TYPE)];
			constructElements1<TYPE>(m_pData, nNewSize, elem);
			m_nSize = m_nMaxSize = nNewSize;
		}
		else if (nNewSize <= m_nMaxSize)
		{
			// it fits
			if (nNewSize > m_nSize)
			{
				// initialize the new elements
				constructElements1<TYPE>(&m_pData[m_nSize], nNewSize - m_nSize, elem);
			}
			else if (m_nSize > nNewSize)
			{
				// destroy the old elements
				destructElements1<TYPE>(&m_pData[nNewSize], m_nSize - nNewSize);
			}
			m_nSize = nNewSize;
		}
		else
		{
			// otherwise, grow array
			int nGrowBy = m_nGrowBy;
			if (nGrowBy == 0)
			{
				// heuristically determine growth when nGrowBy == 0
				//  (this avoids heap fragmentation in many situations)
				nGrowBy = m_nSize / 8;
				nGrowBy = (nGrowBy < 4) ? 4 : ((nGrowBy > 1024) ? 1024 : nGrowBy);
			}
			int nNewMax;
			if (nNewSize < m_nMaxSize + nGrowBy)
				nNewMax = m_nMaxSize + nGrowBy;  // granularity
			else
				nNewMax = nNewSize;  // no slush

			assert(nNewMax >= m_nMaxSize);  // no wrap around
#ifdef SIZE_T_MAX
			assert(nNewMax <= SIZE_T_MAX / sizeof(TYPE)); // no overflow
#endif
			TYPE* pNewData = (TYPE*) new uchar[nNewMax * sizeof(TYPE)];

			// copy new data from old
			memcpy((void*)pNewData, (const void*)m_pData, m_nSize * sizeof(TYPE));

			// construct remaining elements
			assert(nNewSize > m_nSize);
			constructElements1<TYPE>(&pNewData[m_nSize], nNewSize - m_nSize, elem);

			// get rid of old stuff (note: no destructors called)
			delete[](uchar*)m_pData;
			m_pData = pNewData;
			m_nSize = nNewSize;
			m_nMaxSize = nNewMax;
		}
	}

	template<class TYPE, class ARG_TYPE>
	int Array<TYPE, ARG_TYPE>::append(const Array& src)
	{
		assert(this);
		assert(this != &src);   // cannot append to itself

		int nOldSize = m_nSize;
		setSize(m_nSize + src.m_nSize);
		copyElements1<TYPE>(m_pData + nOldSize, src.m_pData, src.m_nSize);
		return nOldSize;
	}
	template<class TYPE, class ARG_TYPE>
	int Array<TYPE, ARG_TYPE>::append(const TYPE* parray, int size)
	{
		assert(this);
		int nOldSize = m_nSize;
		setSize(m_nSize + size);
		copyElements1<TYPE>(m_pData + nOldSize, parray, size);
		return nOldSize;
	}
	template<class TYPE, class ARG_TYPE>
	void Array<TYPE, ARG_TYPE>::copyFrom(const Array& src)
	{
		assert(this);
		assert(this != &src);   // cannot append to itself

		setSize(src.m_nSize);
		copyElements1<TYPE>(m_pData, src.m_pData, src.m_nSize);
	}

	template<class TYPE, class ARG_TYPE>
	void Array<TYPE, ARG_TYPE>::freeExtra()
	{
		assert(this);

		if (m_nSize != m_nMaxSize && m_fcopydata)
		{
			// shrink to desired size
#ifdef SIZE_T_MAX
			assert(m_nSize <= SIZE_T_MAX / sizeof(TYPE)); // no overflow
#endif
			TYPE* pNewData = NULL;
			if (m_nSize != 0)
			{
				pNewData = (TYPE*) new uchar[m_nSize * sizeof(TYPE)];
				// copy new data from old
				memcpy((void*)pNewData, (const void*)m_pData, m_nSize * sizeof(TYPE));
			}

			// get rid of old stuff (note: no destructors called)
			delete[](uchar*)m_pData;
			m_pData = pNewData;
			m_nMaxSize = m_nSize;
		}
	}

	template<class TYPE, class ARG_TYPE>
	void Array<TYPE, ARG_TYPE>::setAtGrow(int nIndex, ARG_TYPE newElement)
	{
		assert(this);
		assert(nIndex >= 0);

		if (nIndex >= m_nSize)
			setSize(nIndex + 1, -1);
		m_pData[nIndex] = newElement;
	}

	template<class TYPE, class ARG_TYPE>
	void Array<TYPE, ARG_TYPE>::insertAt(int nIndex, ARG_TYPE newElement, int nCount /*=1*/)
	{
		assert(this);
		assert(nIndex >= 0);    // will expand to meet need
		assert(nCount > 0);     // zero or negative size not allowed

		if (nIndex >= m_nSize)
		{
			// adding after the end of the array
			setSize(nIndex + nCount, -1);   // grow so nIndex is valid
		}
		else
		{
			// inserting in the middle of the array
			int nOldSize = m_nSize;
			setSize(m_nSize + nCount, -1);  // grow it to new size
			// destroy intial data before copying over it
			destructElements1<TYPE>(&m_pData[nOldSize], nCount);
			// shift old data up to fill gap
			memmove((void*)&m_pData[nIndex + nCount], (const void*)&m_pData[nIndex], (nOldSize - nIndex) * sizeof(TYPE));

			// re-init slots we copied from
			constructElements1<TYPE>(&m_pData[nIndex], nCount);
		}

		// insert new value in the gap
		assert(nIndex + nCount <= m_nSize);
		while (nCount--)
			m_pData[nIndex++] = newElement;
	}

	template<class TYPE, class ARG_TYPE>
	void Array<TYPE, ARG_TYPE>::removeAt(int nIndex, int nCount)
	{
		assert(this);
		assert(nIndex >= 0);
		assert(nCount >= 0);
		assert(nIndex + nCount <= m_nSize);

		// just remove a range
		int nMoveCount = m_nSize - (nIndex + nCount);
		destructElements1<TYPE>(&m_pData[nIndex], nCount);
		if (nMoveCount)
			memmove((void*)&m_pData[nIndex], (const void*)&m_pData[nIndex + nCount], nMoveCount * sizeof(TYPE));
		m_nSize -= nCount;
	}

	template<class TYPE, class ARG_TYPE>
	void Array<TYPE, ARG_TYPE>::insertAt(int nStartIndex, Array* pNewArray)
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

	template<class TYPE, class ARG_TYPE>
	void Array<TYPE, ARG_TYPE>::insertAt(int nStartIndex, const _MyArray& newArray)
	{
		assert(this);
		assert(nStartIndex >= 0);

		if (newArray.getSize() > 0)
		{
			int nCount = newArray.getSize();
			if (nStartIndex >= m_nSize)
			{
				// adding after the end of the array
				setSize(nStartIndex + nCount, -1);   // grow so nIndex is valid
			}
			else
			{
				// inserting in the middle of the array
				int nOldSize = m_nSize;
				setSize(m_nSize + nCount, -1);  // grow it to new size
				// destroy intial data before copying over it
				destructElements1<TYPE>(&m_pData[nOldSize], nCount);
				// shift old data up to fill gap
				memmove((void*)&m_pData[nStartIndex + nCount], (const void*)&m_pData[nStartIndex], (nOldSize - nStartIndex) * sizeof(TYPE));

				// re-init slots we copied from
				constructElements1<TYPE>(&m_pData[nStartIndex], nCount);
			}

			// insert new value in the gap
			assert(nStartIndex + nCount <= m_nSize);
			int i = 0;
			while (nCount--)
				m_pData[nStartIndex++] = newArray[i++];
		}
	}

	template<class TYPE, class ARG_TYPE>
	void Array<TYPE, ARG_TYPE>::swap(_MyArray& aray)
	{
		int nTemp;
		TYPE* pType;
		SWAP(m_nGrowBy, aray.m_nGrowBy, nTemp);
		SWAP(m_nMaxSize, aray.m_nMaxSize, nTemp);
		SWAP(m_nSize, aray.m_nSize, nTemp);
		SWAP(m_pData, aray.m_pData, pType);
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	template<class KEY, class VALUE, class ARG_KEY = const KEY&, class ARG_VALUE = const VALUE&>
	class CVLIB_DECLSPEC Map : public Object
	{
	protected:
		// Association
		struct mapAssoc
		{
			mapAssoc* pNext;
			uint nHashValue;  // needed for efficient iteration
			KEY key;
			VALUE value;
		};
	public:
		// Construction
		Map(int nBlockSize = 10);

		// Attributes
			// number of elements
		int count() const;
		bool isEmpty() const;

		// lookup
		bool lookup(ARG_KEY key, VALUE& rValue) const;

		// Operations
			// lookup and add if not there
		VALUE& operator[](ARG_KEY key);

		// add a new (key, value) pair
		void setAt(ARG_KEY key, ARG_VALUE newValue);

		// removing existing (key, ?) pair
		bool removeKey(ARG_KEY key);
		void removeAll();

		// iterating all (key, value) pairs
		CVLIB_POSITION getStartPosition() const;
		void getNextAssoc(CVLIB_POSITION& rNextPosition, KEY& rKey, VALUE& rValue) const;

		// advanced features for derived classes
		uint getHashTableSize() const;
		void initHashTable(uint hashSize, bool bAllocNow = true);

		// Implementation
	protected:
		mapAssoc** m_pHashTable;
		uint m_nHashTableSize;
		int m_nCount;
		mapAssoc* m_pFreeList;
		struct Plex* m_pBlocks;
		int m_nBlockSize;

		mapAssoc* newAssoc();
		void freeAssoc(mapAssoc*);
		mapAssoc* getAssocAt(ARG_KEY, uint&) const;

	public:
		~Map();
	};

	/////////////////////////////////////////////////////////////////////////////
	// Map<KEY,  VALUE, ARG_KEY,ARG_VALUE> inline functions

	template<class KEY, class VALUE, class ARG_KEY, class ARG_VALUE>
	inline int Map<KEY, VALUE, ARG_KEY, ARG_VALUE>::count() const
	{
		return m_nCount;
	}
	template<class KEY, class VALUE, class ARG_KEY, class ARG_VALUE>
	inline bool Map<KEY, VALUE, ARG_KEY, ARG_VALUE>::isEmpty() const
	{
		return m_nCount == 0;
	}
	template<class KEY, class VALUE, class ARG_KEY, class ARG_VALUE>
	inline void Map<KEY, VALUE, ARG_KEY, ARG_VALUE>::setAt(ARG_KEY key, ARG_VALUE newValue)
	{
		(*this)[key] = newValue;
	}
	template<class KEY, class VALUE, class ARG_KEY, class ARG_VALUE>
	inline CVLIB_POSITION Map<KEY, VALUE, ARG_KEY, ARG_VALUE>::getStartPosition() const
	{
		return (m_nCount == 0) ? NULL : CVLIB_BEFORE_START_POSITION;
	}
	template<class KEY, class VALUE, class ARG_KEY, class ARG_VALUE>
	inline uint Map<KEY, VALUE, ARG_KEY, ARG_VALUE>::getHashTableSize() const
	{
		return m_nHashTableSize;
	}

	/////////////////////////////////////////////////////////////////////////////
	// Map<KEY,  VALUE, ARG_KEY,ARG_VALUE> out-of-line functions

	template<class KEY, class VALUE, class ARG_KEY, class ARG_VALUE>
	Map<KEY, VALUE, ARG_KEY, ARG_VALUE>::Map(int nBlockSize)
	{
		assert(nBlockSize > 0);

		m_pHashTable = NULL;
		m_nHashTableSize = 17;  // default size
		m_nCount = 0;
		m_pFreeList = NULL;
		m_pBlocks = NULL;
		m_nBlockSize = nBlockSize;
	}

	template<class KEY, class VALUE, class ARG_KEY, class ARG_VALUE>
	void Map<KEY, VALUE, ARG_KEY, ARG_VALUE>::initHashTable(uint nHashSize, bool bAllocNow)
		//
		// Used to force allocation of a hash table or to override the default
		//   hash table size of (which is fairly small)
	{
		assert(this);
		assert(m_nCount == 0);
		assert(nHashSize > 0);

		if (m_pHashTable != NULL)
		{
			// free hash table
			delete[] m_pHashTable;
			m_pHashTable = NULL;
		}

		if (bAllocNow)
		{
			m_pHashTable = new mapAssoc*[nHashSize];
			memset(m_pHashTable, 0, sizeof(mapAssoc*) * nHashSize);
		}
		m_nHashTableSize = nHashSize;
	}

	template<class KEY, class VALUE, class ARG_KEY, class ARG_VALUE>
	void Map<KEY, VALUE, ARG_KEY, ARG_VALUE>::removeAll()
	{
		assert(this);

		if (m_pHashTable != NULL)
		{
			// destroy elements (values and keys)
			for (uint nHash = 0; nHash < m_nHashTableSize; nHash++)
			{
				mapAssoc* pAssoc;
				for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL;
					pAssoc = pAssoc->pNext)
				{
					destructElements1<VALUE>(&pAssoc->value, 1);
					destructElements1<KEY>(&pAssoc->key, 1);
				}
			}
		}

		// free hash table
		delete[] m_pHashTable;
		m_pHashTable = NULL;

		m_nCount = 0;
		m_pFreeList = NULL;
		m_pBlocks->freeDataChain();
		m_pBlocks = NULL;
	}

	template<class KEY, class VALUE, class ARG_KEY, class ARG_VALUE>
	Map<KEY, VALUE, ARG_KEY, ARG_VALUE>::~Map()
	{
		removeAll();
		assert(m_nCount == 0);
	}

	template<class KEY, class VALUE, class ARG_KEY, class ARG_VALUE>
	typename Map<KEY, VALUE, ARG_KEY, ARG_VALUE>::mapAssoc* Map<KEY, VALUE, ARG_KEY, ARG_VALUE>::newAssoc()
	{
		if (m_pFreeList == NULL)
		{
			// add another block
			Plex* newBlock = Plex::create(m_pBlocks, m_nBlockSize, sizeof(Map::mapAssoc));
			// chain them into free list
			typename Map::mapAssoc* pAssoc = (typename Map::mapAssoc*) newBlock->data();
			// free in reverse order to make it easier to debug
			pAssoc += m_nBlockSize - 1;
			for (int i = m_nBlockSize - 1; i >= 0; i--, pAssoc--)
			{
				pAssoc->pNext = m_pFreeList;
				m_pFreeList = pAssoc;
			}
		}
		assert(m_pFreeList != NULL);  // we must have something

		typename Map::mapAssoc* pAssoc = m_pFreeList;
		m_pFreeList = m_pFreeList->pNext;
		m_nCount++;
		assert(m_nCount > 0);  // make sure we don't overflow
		constructElements1<KEY>(&pAssoc->key, 1);
		constructElements1<VALUE>(&pAssoc->value, 1);   // special construct values
		return pAssoc;
	}

	template<class KEY, class VALUE, class ARG_KEY, class ARG_VALUE>
	void Map<KEY, VALUE, ARG_KEY, ARG_VALUE>::freeAssoc(typename Map::mapAssoc* pAssoc)
	{
		destructElements1<VALUE>(&pAssoc->value, 1);
		destructElements1<KEY>(&pAssoc->key, 1);
		pAssoc->pNext = m_pFreeList;
		m_pFreeList = pAssoc;
		m_nCount--;
		assert(m_nCount >= 0);  // make sure we don't underflow

		// if no more elements, cleanup completely
		if (m_nCount == 0)
			removeAll();
	}

	template<class KEY, class VALUE, class ARG_KEY, class ARG_VALUE>
	typename Map<KEY, VALUE, ARG_KEY, ARG_VALUE>::mapAssoc*
		Map<KEY, VALUE, ARG_KEY, ARG_VALUE>::getAssocAt(ARG_KEY key, uint& nHash) const
		// find association (or return NULL)
	{
		nHash = cvlHashKey<ARG_KEY>(key) % m_nHashTableSize;

		if (m_pHashTable == NULL)
			return NULL;

		// see if it exists
		mapAssoc* pAssoc;
		for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext)
		{
			if (compareElements1(&pAssoc->key, &key))
				return pAssoc;
		}
		return NULL;
	}

	template<class KEY, class VALUE, class ARG_KEY, class ARG_VALUE>
	bool Map<KEY, VALUE, ARG_KEY, ARG_VALUE>::lookup(ARG_KEY key, VALUE& rValue) const
	{
		assert(this);

		uint nHash;
		mapAssoc* pAssoc = getAssocAt(key, nHash);
		if (pAssoc == NULL)
			return false;  // not in map

		rValue = pAssoc->value;
		return true;
	}

	template<class KEY, class VALUE, class ARG_KEY, class ARG_VALUE>
	VALUE& Map<KEY, VALUE, ARG_KEY, ARG_VALUE>::operator[](ARG_KEY key)
	{
		assert(this);

		uint nHash;
		mapAssoc* pAssoc;
		if ((pAssoc = GetAssocAt(key, nHash)) == NULL)
		{
			if (m_pHashTable == NULL)
				initHashTable(m_nHashTableSize);

			// it doesn't exist, add a new Association
			pAssoc = newAssoc();
			pAssoc->nHashValue = nHash;
			pAssoc->key = key;
			// 'pAssoc->value' is a constructed object, nothing more

			// put into hash table
			pAssoc->pNext = m_pHashTable[nHash];
			m_pHashTable[nHash] = pAssoc;
		}
		return pAssoc->value;  // return new reference
	}

	template<class KEY, class VALUE, class ARG_KEY, class ARG_VALUE>
	bool Map<KEY, VALUE, ARG_KEY, ARG_VALUE>::removeKey(ARG_KEY key)
		// remove key - return true if removed
	{
		assert(this);

		if (m_pHashTable == NULL)
			return false;  // nothing in the table

		mapAssoc** ppAssocPrev;
		ppAssocPrev = &m_pHashTable[cvlHashKey<ARG_KEY>(key) % m_nHashTableSize];

		mapAssoc* pAssoc;
		for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext)
		{
			if (CompareElements1(&pAssoc->key, &key))
			{
				// remove it
				*ppAssocPrev = pAssoc->pNext;  // remove from list
				FreeAssoc(pAssoc);
				return true;
			}
			ppAssocPrev = &pAssoc->pNext;
		}
		return false;  // not found
	}

	template<class KEY, class VALUE, class ARG_KEY, class ARG_VALUE>
	void Map<KEY, VALUE, ARG_KEY, ARG_VALUE>::getNextAssoc(CVLIB_POSITION& rNextPosition, KEY& rKey, VALUE& rValue) const
	{
		assert(this);
		assert(m_pHashTable != NULL);  // never call on empty map

		mapAssoc* pAssocRet = (mapAssoc*)rNextPosition;
		assert(pAssocRet != NULL);

		if (pAssocRet == (mapAssoc*)CVLIB_BEFORE_START_POSITION)
		{
			// find the first association
			for (uint nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
				if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
					break;
			assert(pAssocRet != NULL);  // must find something
		}

		// find next association
		mapAssoc* pAssocNext;
		if ((pAssocNext = pAssocRet->pNext) == NULL)
		{
			// go to next bucket
			for (uint nBucket = pAssocRet->nHashValue + 1;
				nBucket < m_nHashTableSize; nBucket++)
				if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
					break;
		}

		rNextPosition = (CVLIB_POSITION)pAssocNext;

		// fill in return data
		rKey = pAssocRet->key;
		rValue = pAssocRet->value;
	}

#define Vector Array
#define ConstVector ConstArray

	template<class TYPE, class ARG_TYPE = const TYPE&>
	class CVLIB_DECLSPEC Matrix : public Vector<Vector<TYPE, ARG_TYPE>, const Vector<TYPE, ARG_TYPE>&>
	{
	public:
		typedef Matrix<TYPE, ARG_TYPE> _MyMatrix;
		Matrix() {}
		Matrix(int nRows, int nCols, const TYPE& _V = TYPE())
		{
			create(nRows, nCols, _V);
		}
		int create(int nRows, int nCols, const TYPE& _V = TYPE())
		{
			Vector<Vector<TYPE, ARG_TYPE>, const Vector<TYPE, ARG_TYPE>&>::removeAll();
			Vector<Vector<TYPE, ARG_TYPE>, const Vector<TYPE, ARG_TYPE>&>::setSize(nRows);
			for (int i = 0; i < Vector<Vector<TYPE, ARG_TYPE>, const Vector<TYPE, ARG_TYPE>&>::length(); i++)
			{
				Vector<Vector<TYPE, ARG_TYPE>, const Vector<TYPE, ARG_TYPE>&>::getAt(i).insertAt(0, _V, nCols);
			}
			return 1;
		}
	};

#pragma warning(disable: 4786)

#undef new
#ifdef _REDEF_NEW
	//#define new new(__FILE__, __LINE__)
#undef _REDEF_NEW
#endif

}