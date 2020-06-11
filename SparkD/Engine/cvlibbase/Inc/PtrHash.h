/*!
 * \file	PtrHash.h
 * \ingroup base
 * \brief	List.
 * \author	
 */

#pragma once

#include "cvlibmacros.h"

namespace cvlib
{
/**
 * @brief	Hash.
 */
class CVLIB_DECLSPEC PtrHash
{
public:

	PtrHash( int nHashSize );
	virtual ~PtrHash();

	int getHashSize();
	int getHashValue(void* ptr);
	void *getKey(void *ptr);
	int addKey(void *ptr, void *pValue);
	void *removeKey(void *ptr);

private:
	int m_nHashSize;
	void** m_ppHashTable;
};

}