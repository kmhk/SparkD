
/*!
 * \file	BitOperation.h
 * \ingroup base
 * \brief	
 * \author	
 */

#pragma once

#include "cvlibbaseDef.h"

namespace cvlib
{

/**
 * @brief 
 */
class CVLIB_DECLSPEC BitOperation
{
public:

	/**
	 * @brief
	 *
	 * @param   c    [in] :	
	 * @param   n    [in] :	
	 * @param   bits [in] :	
	 * @return
	 *
	 * @par Example:
	 * @code
	 *  ...
	 *	unsigned int a, b;
	 *
	 *	a = BitOperation::rol(0xfd93, 3, 8);
	 *	assert (a == 0x7ec9c);
	 *	a = BitOperation::rol(0xfd93, 3, 32);
	 *	assert (a == 0x7ec98);
	 *  ...
	 * @endcode
	 * 
	 */
	static unsigned int rol(unsigned int c, int n, int bits = 8);
	
	/**
	 * @brief 
	 *
	 * @param   c    [in] :	
	 * @param   n    [in] :
	 * @param   bits [in] :	
	 * @return	
	 *
	 * @par Example:
	 * @code
	 *  ...
	 *	unsigned int a, b;
	 *
	 *	b = BitOperation::ror(0xfd93, 4, 8);
	 *	assert (b == 0xff9);
	 *	b = BitOperation::ror(0xfd93, 4, 32); 
	 *	assert (b == 0x30000fd9);
	 *  ...
	 * @endcode
	 * 
	 */	
	static unsigned int ror(unsigned int c, int n, int bits = 8);
	
	/**
	 * @brief  
	 *
	 * @param   c    [in] :
	 * @param   bits [in] :
	 * @return	
	 *
	 * @par Example:
	 * @code
	 *  ...
	 *	unsigned int a, b;
	 *
	 *	b = BitOperation::oneCount(0xfd93, 8);
	 *	assert (b == 4);
	 *	b = BitOperation::oneCount(0xfd93, 16);
	 *	assert (b == 11);
	 *  ...
	 * @endcode
	 * 
	 */	
	static int oneCount(unsigned int c, int bits = 8);

	/**
	 * @brief   
	 *
	 * @param   c    [in] : 
	 * @param   bits [in] : 
	 * @return	
	 *
	 * @par Example:
	 * @code
	 *  ...
	 *	unsigned int b;
	 *
	 *	b = BitOperation::transitions(0xfd93, 8);
	 *	assert (b == 4);
	 *	b = BitOperation::transitions(0xfd93, 16);
	 *	assert (b == 6);
	 *  ...
	 * @endcode
	 * 
	 */	
	static int transitions(unsigned int c, int bits = 8);

	/**
	 * @brief   
	 *
	 * @param   n    [in] : 
	 * @param   bits [in] : 
	 * @return	 
	 *
	 */	
	static unsigned int rotMin(unsigned int n, int bits = 8);

	/**
	 * @brief   
	 *
	 * @param   a    [in] :
	 * @param   b    [in] :
	 * @param   bits [in] :
	 * @return	
	 *
	 * @par Example:
	 * @code
	 *
	 *  int i, j;
	 *  
	 *  Mat mHDist;
	 *  mHDist.create(32, 32, MAT_Tint);
	 *  for(i = 0; i < 32; i ++)
	 *  for (j = 0; j < 32; j ++)
	 *  mHDist.data.i[i][j] = BitOperation::hammingDistance(i, j, 8);
	 *  
	 *	for(i = 0; i < 32; i ++)
	 *	   for(j = 0; j < 32; j ++)
	 *	   {
	 *	       if(i == j)
	 *	          assert( mHDist.data.i[i][j] == 0 );
	 *	       else
	 *	          assert( mHDist.data.i[i][j] == mHDist.data.i[j][i] );
	 *	   }
	 *	 
	 *  assert( mHDist.data.i[1][6] == 3 );
	 *
	 *  mHDist.release(); 
	 *  ...
	 * @endcode
	 * 
	 */	
	static int hammingDistance(unsigned int a, unsigned int b, int bits = 8);
	
};

}