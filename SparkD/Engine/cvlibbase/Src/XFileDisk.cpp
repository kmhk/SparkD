#include "XFileDisk.h"

namespace cvlib
{

XFileDisk::XFileDisk(FILE* fp /*= NULL*/)
{
	m_fp = fp;
	m_bCloseFile = (bool)(fp==0);
}

XFileDisk::~XFileDisk()
{
	close();
}

bool XFileDisk::open(const char *filename, const char *mode)
{
	if (m_fp) return false;	// Can't re-open without closing first

	m_fp = fopen(filename, mode);
	if (!m_fp) return false;

	m_bCloseFile = true;

	return true;
}

bool XFileDisk::close()
{
	int iErr = 0;
	if ( (m_fp) && (m_bCloseFile) ){ 
		iErr = fclose(m_fp);
		m_fp = NULL;
	}
	return (bool)(iErr==0);
}

size_t	XFileDisk::read(void *buffer, size_t size, size_t count)
{
	if (!m_fp) return 0;
	return fread(buffer, size, count, m_fp);
}

size_t	XFileDisk::write(const void *buffer, size_t size, size_t count)
{
	if (!m_fp) return 0;
	return fwrite(buffer, size, count, m_fp);
}

bool XFileDisk::seek(int offset, int origin)
{
	if (!m_fp) return false;
	return (bool)(fseek(m_fp, offset, origin) == 0);
}

int XFileDisk::tell()
{
	if (!m_fp) return 0;
	return (int)ftell(m_fp);
}

int XFileDisk::size()
{
	if (!m_fp) return -1;
	long pos,sz;
	pos = ftell(m_fp);
	fseek(m_fp, 0, SEEK_END);
	sz = ftell(m_fp);
	fseek(m_fp, pos,SEEK_SET);
	return (int)sz;
}

bool XFileDisk::flush()
{
	if (!m_fp) return false;
	return (bool)(fflush(m_fp) == 0);
}

bool XFileDisk::eof()
{
	if (!m_fp) return true;
	return (bool)(feof(m_fp) != 0);
}

int XFileDisk::error()
{
	if (!m_fp) return -1;
	return ferror(m_fp);
}

bool XFileDisk::putC(unsigned char c)
{
	if (!m_fp) return false;
	return (bool)(fputc(c, m_fp) == c);
}

int XFileDisk::getC()
{
	if (!m_fp) return EOF;
	return getc(m_fp);
}

}
