/*!
 * \file    XFile.h
 * \ingroup base
 * \brief   
 * \author  
 */

#pragma once

#include "cvlibmacros.h"

namespace cvlib
{
/**
 * @brief  
 */
class CVLIB_DECLSPEC XFile
{
public:
	XFile(void) { };
	virtual ~XFile() { };

	virtual bool	close() = 0;
	
	virtual size_t		read(void *buffer, size_t size, size_t count) = 0;

	virtual size_t		write(const void *buffer, size_t size, size_t count) = 0;
	
	virtual bool	seek(int offset, int origin) = 0;
		
	virtual int     tell() = 0;
	
	virtual int     size() = 0;

	virtual bool	flush() = 0;
	
	virtual bool	eof() = 0;

	virtual int     error() = 0;
	
	virtual bool	putC(unsigned char c)
		{
		// Default implementation
		size_t nWrote = write(&c, 1, 1);
		return (bool)(nWrote == 1);
		}

	virtual int     getC() = 0;
};

}