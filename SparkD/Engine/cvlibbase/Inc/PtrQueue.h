
/*!
 * \file	PtrQueue.h
 * \ingroup base
 * \brief	.
 * \author	
 */

#pragma once

#include "cvlibmacros.h"
#include <assert.h>

namespace cvlib 
{
	
/**
 * @brief  
 */
class CVLIB_DECLSPEC PtrQueue
{
public:
	/**
	 *  @brief  
	 */
	enum
	{
		POS_HEAD   = 0,
		POS_TAIL   = 1 
	};
	
public:
	// Constructor and Destructor
	PtrQueue ();
	PtrQueue (int nCount);
	virtual ~PtrQueue ();

	// Check if the instance was created
	bool flagCreate ();

	// Initialize the queue.
	bool create(int	nCount);

	// Destroy the queue.
	virtual void release();

	// Get the size of data.
	int	count() const;

	// Get the data at the current queue position. 
	void* data() const;

	// Pushes data to the head of queue.
	/**
	 * @param   pData [in] : 
	 * @return  void
	 *
	 * @see	 pushTail(), popHead(), popTail()
	 *
	 * @par	Example:
	 * @code
	 *
	 *	PtrQueue intQueue;
	 *	intQueue.create(5);
	 *	int* pn1 = new int;
	 *	*pn1 = 1;
	 *	int* pn2 = new int;
	 *	*pn1 = 2;
	 *	int* pn5 = new int;
	 *	*pn5 = 5;
	 *
	 *	intQueue.pushHead(pn1);
	 *	intQueue.pushHead(pn2);
	 *	intQueue.pushHead(pn5);
	 *
	 *	int* pnData = (int*)intQueue.popHead();
	 *	assert (*pnData == 5);
	 *	pnData = (int*)intQueue.popTail();
	 *	assert (*pnData == 1);
	 *	pnData = (int*)intQueue.popTail();
	 *	assert (*pnData == 2);
	 *	...
	 * @endcode
	 *
	 */
	void pushHead(void* pData);

	// Pushes data to the tail of queue.
	/**
	 * @param   pData [in] : 
	 * @return  void
	 *
	 * @see	 pushHead(), popHead(), popTail()
	 *
	 * @par	Example:
	 * @code
	 *
	 *	PtrQueue intQueue;
	 *	intQueue.create(5);
	 *	int* pn1 = new int;
	 *	*pn1 = 1;
	 *	int* pn2 = new int;
	 *	*pn1 = 2;
	 *	int* pn5 = new int;
	 *	*pn5 = 5;
	 *
	 *	intQueue.pushTail(pn1);
	 *	intQueue.pushTail(pn2);
	 *	intQueue.pushTail(pn5);
	 *
	 *	int* pnData = (int*)intQueue.popHead();
	 *	assert (*pnData == 1);
	 *	pnData = (int*)intQueue.popHead();
	 *	assert (*pnData == 5);
	 *	pnData = (int*)intQueue.popTail();
	 *	assert (*pnData == 2);
	 *	...
	 * @endcode
	 *
	 */
	void pushTail(void* pData);

	// Pops data from the head of queue.

	void* popHead ();

	// Pops data from the tail of queue.
	void* popTail ();

	// Set current position of the queue.
	/**
	 * @brief   
	 *
	 * @param   nWhence [in] : 
	 * @param   nIndex  [in] : 
	 * @return	void
	 *
	 * @see	 data()
	 *
	 * @par	Example:
	 * @code
	 *
	 *	PtrQueue intQueue;
	 *	intQueue.create(5);
	 *	int* pn1 = new int;
	 *	*pn1 = 1;
	 *	int* pn5 = new int;
	 *	*pn5 = 5;
	 *
	 *	intQueue.pushTail(pn1);
	 *	intQueue.pushTail(pn5);
	 *
	 *	int* pnData;
	 *	intQueue.setPosition (PtrQueue::POS_HEAD, 1);
	 *	pnData = (int*)intQueue.data();
	 *	assert (*pnData == 5);
	 *	...
	 * @endcode
	 *
	 */
	void setPosition(int nWhence, int nIndex);

private:
	void*	m_pData;
};

}