
/*!
 * \file	Plex.h
 * \ingroup base
 * \brief	
 * \author	
 */

#pragma once

#include "cvlibbaseDef.h"

namespace cvlib
{

struct CVLIB_DECLSPEC Plex     // warning variable length structure
{
	Plex* pNext;
	// uchar data[maxNum*elementSize];
	
	void* data() { return this+1; }
	
	static Plex* create(Plex*& head, uint nMax, uint cbElement);
	// like 'calloc' but no zero fill
	// may throw memory exceptions
	
	void freeDataChain();       // free this one and links
};

}