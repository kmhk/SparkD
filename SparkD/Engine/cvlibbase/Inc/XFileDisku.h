
/*!
 * \file    XFileDisk.h
 * \ingroup base
 * \brief   .
 * \author  
 */

#pragma once

#include "XFile.h"

namespace cvlib
{

#if defined(_MSC_VER)

class CVLIB_DECLSPEC XFileDisku : public XFile
{
public:
	XFileDisku(FILE* fp = NULL);
	~XFileDisku();

	bool open(const wchar_t *filename, const wchar_t *mode);

	virtual bool close();
  	
	virtual size_t	read(void *buffer, size_t size, size_t count);
	
	virtual size_t	write(const void *buffer, size_t size, size_t count);
	
	virtual bool    seek(int offset, int origin);
	
	virtual int     tell();
	
	virtual int     size();
	
	virtual bool	flush();
	
	virtual bool	eof();
	
	virtual int     error();
	
	virtual bool putC(unsigned char c);

	virtual int     getC();
protected:
	FILE *m_fp;
	bool m_bCloseFile;
};

#else

#endif

}