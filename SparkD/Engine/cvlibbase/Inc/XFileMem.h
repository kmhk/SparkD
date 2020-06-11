
/*!
 * \file    XFileMem.h
 * \ingroup base
 * \brief   
 * \author  
 */

#pragma once

#include "XFile.h"

namespace cvlib
{

class CVLIB_DECLSPEC XFileMem : public XFile
{
public:
	 XFileMem(uchar* pBuffer = NULL, uint size = 0);
	 
	~XFileMem();

	bool open();
	uchar* getBuffer(bool bDetachBuffer = true);

	virtual bool	close();

	virtual size_t	read(void *buffer, size_t size, size_t count);

	virtual size_t	write(const void *buffer, size_t size, size_t count);

	virtual bool	seek(int offset, int origin);

	virtual int     tell();

	virtual int     size();

	virtual bool	flush();

	virtual bool	eof();

	virtual int     error();

	virtual bool	putC(unsigned char c);

	virtual int     getC();

protected:
	void	alloc(uint nBytes);
	void	Free();

	uchar*	m_pBuffer;
	uint	m_Size;
	bool	m_bFreeOnClose;
	int     m_Position;
	int     m_Edge;
};

}