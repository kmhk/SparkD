#include "XFileDisku.h"

namespace cvlib
{

#if defined(_MSC_VER)

	XFileDisku::XFileDisku(FILE* fp /*= NULL*/)
	{
		m_fp = fp;
		m_bCloseFile = (bool)(fp == 0);
	}

	XFileDisku::~XFileDisku()
	{
		close();
	}

	bool XFileDisku::open(const wchar_t *filename, const wchar_t *mode)
	{
		if (m_fp) return false;	// Can't re-open without closing first

		m_fp = _wfopen(filename, mode);
		if (!m_fp) return false;

		m_bCloseFile = true;

		return true;
	}

	bool XFileDisku::close()
	{
		int iErr = 0;
		if ((m_fp) && (m_bCloseFile)) {
			iErr = fclose(m_fp);
			m_fp = NULL;
		}
		return (bool)(iErr == 0);
	}

	size_t	XFileDisku::read(void *buffer, size_t size, size_t count)
	{
		if (!m_fp) return 0;
		return fread(buffer, size, count, m_fp);
	}

	size_t	XFileDisku::write(const void *buffer, size_t size, size_t count)
	{
		if (!m_fp) return 0;
		return fwrite(buffer, size, count, m_fp);
	}

	bool XFileDisku::seek(int offset, int origin)
	{
		if (!m_fp) return false;
		return (bool)(fseek(m_fp, offset, origin) == 0);
	}

	int XFileDisku::tell()
	{
		if (!m_fp) return 0;
		return (int)ftell(m_fp);
	}

	int XFileDisku::size()
	{
		if (!m_fp) return -1;
		long pos, sz;
		pos = ftell(m_fp);
		fseek(m_fp, 0, SEEK_END);
		sz = ftell(m_fp);
		fseek(m_fp, pos, SEEK_SET);
		return (int)sz;
	}

	bool XFileDisku::flush()
	{
		if (!m_fp) return false;
		return (bool)(fflush(m_fp) == 0);
	}

	bool XFileDisku::eof()
	{
		if (!m_fp) return true;
		return (bool)(feof(m_fp) != 0);
	}

	int XFileDisku::error()
	{
		if (!m_fp) return -1;
		return ferror(m_fp);
	}

	bool XFileDisku::putC(unsigned char c)
	{
		if (!m_fp) return false;
		return (bool)(fputc(c, m_fp) == c);
	}

	int XFileDisku::getC()
	{
		if (!m_fp) return EOF;
		return getc(m_fp);
	}

#endif

}
