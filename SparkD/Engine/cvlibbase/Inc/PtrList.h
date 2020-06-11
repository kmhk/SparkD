/*!
 * \file	PtrList.h
 * \ingroup base
 * \brief	.
 * \author	
 */

#pragma once

#include "cvlibmacros.h"

namespace cvlib
{

/**
 * @brief 
 */
class CVLIB_DECLSPEC PtrList
{
public:
	
	/**
	 * @brief 
	 */
	enum LISTAddMode
	{
		ADD_BEFORE = 0,     /**< .*/
		ADD_AFTER  = 1,     /**< .*/
	};

	/**
	 * @brief  
	 */
	enum LISTPositionType
	{
		POS_BEGIN   = 0,    /**<  */
		POS_CURRENT = 1,    /**<  */
		POS_END     = 2  	/**<  */ 
	};

public:
	
	// Constructor and Destructor
	PtrList();
	virtual ~PtrList();
	bool flagCreate(void) { return m_pData != NULL; }

	// Initialize the list.
	bool create(void);
	
	// Destroy the list and deletes all nodes.
	virtual void release(void);

	// Get the size of data.
	unsigned long count(void) const;
	
	// Get the data at the current list position. 
	/**
	 * @brief   
	 * @brief   
	 *
	 * @return  
	 *
	 * @see  setPosition()
	 *
	 * @par Example:
	 * @code
	 *
	 *	PtrList listDB;
	 *	if ( !listDB.FlagCreate() )
	 *		listDB.create();
	 *
	 *	Vec* pVec1 = new Vec;
	 *	Vec* pVec2 = new Vec;
	 *	listDB.add (pVec1);	//Element 0
	 *	listDB.add (pVec2);	//Element 1
	 *	assert( 2 == listDB.Count() );
	 *
	 *	listDB.setPosition(PtrList::POS_BEGIN, 0);	//Set element 0
	 *	assert( pVec1 == listDB.data() );
	 *	...
	 * @endcode
	 *
	 */
	void* data(void) const;

	// Adds data to the list.
	/**
	 * @brief   .
	 *
	 * @param   pData [in] : 
	 * @param   nMode [in] : 
	 * @return  
	 *
	 * @see  Count(), remove()
	 *
	 * @par Example:
	 * @code
	 *
	 *	PtrList listDB;
	 *	if ( !listDB.FlagpVec1 = new Vec;
	 *	Vec* pVec2 = new Vec;
	 *	listDB.add (pVec1);	//Element 0
	 *	listDB.add (pVec2);	//Element 1
	 *	assert( 2 == listDB.Count() );
	 *
	 *	listDB.remove ();	//remove element 1
	 *	assert( 1 == listDB.Count() );
	 *	...
	 * @endcode
	 *
	 */
	bool add(const void* pData, int nMode = ADD_AFTER);
	
	// Search the list.
	/**
	 * @brief   
	 <pre>
	 </pre>
	 *
	 * @param   pData [in] : 
	 * @return  .
	 *
	 * @par Example:
	 * @code
	 *
	 *	PtrList listDB;
	 *	if ( !listDB.FlagCreate() )
	 *		listDB.create();
	 *
	 *	Vec* pVec1 = new Vec;
	 *	Vec* pVec2 = new Vec;
	 *	listDB.add (pVec1);	//Element 0
	 *	listDB.add (pVec2);	//Element 1
	 *
	 *	assert( listDB.find(pVec1) == true );
	 *	...
	 * @endcode
	 *
	 */
	bool find(const void* pData) const;
	
	// Set current position of the list.
	/**
	 * @brief   .
	 *
	 * @param   nWhence	[in] : ?
	 * @param   nIndex	[in] : 
	 * @return  
	 *
	 * @see  data()
	 *
	 * @par Example:
	 * @code
	 *
	 *	PtrList listDB;
	 *	if ( !listDB.FlagCreate() )
	 *		listDB.create();
	 *
	 *	Vec* pVec1 = new Vec;
	 *	Vec* pVec2 = new Vec;
	 *	listDB.add (pVec1);	//Element 0
	 *	listDB.add (pVec2);	//Element 1
	 *	assert( 2 == listDB.Count() );
	 *
	 *	listDB.setPosition(PtrList::POS_BEGIN, 0);	//Set element 0
	 *	assert( pVec1 == listDB.data() );
	 *	...
	 * @endcode
	 *
	 */
	bool setPosition(int nWhence, int nIndex);
	
	// remove an element from the list.
	/**
	 * @brief	.
	 *
     * @return  void
	 *
	 * @see  add()
	 *
	 */
	void remove(void);

private:
	void*	m_pData;
};

}