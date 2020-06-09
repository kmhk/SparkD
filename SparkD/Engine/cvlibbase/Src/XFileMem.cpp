#include "XFileMem.h"

namespace cvlib
{

	XFileMem::XFileMem(uchar* pBuffer, uint size)
	{
		m_pBuffer = pBuffer;
		m_Position = 0;
		m_Size = m_Edge = size;
		m_bFreeOnClose = (bool)(pBuffer == 0);
	}

	XFileMem::~XFileMem()
	{
		close();
	}

	bool XFileMem::close()
	{
		if ((m_pBuffer) && (m_bFreeOnClose)) {
			free(m_pBuffer);
			m_pBuffer = NULL;
			m_Size = 0;
		}
		return true;
	}

	bool XFileMem::open()
	{
		if (m_pBuffer) return false;	// Can't re-open without closing first

		m_Position = m_Size = m_Edge = 0;
		m_pBuffer = (uchar*)malloc(1);
		m_bFreeOnClose = true;

		return (m_pBuffer != 0);
	}

	uchar* XFileMem::getBuffer(bool bDetachBuffer)
	{
		m_bFreeOnClose = !bDetachBuffer;
		return m_pBuffer;
	}

	size_t XFileMem::read(void *buffer, size_t size, size_t count)
	{
		if (buffer == NULL) return 0;

		if (m_pBuffer == NULL) return 0;
		if (m_Position >= (long)m_Size) return 0;

		long nCount = (long)(count*size);
		if (nCount == 0) return 0;

		long nRead;
		if (m_Position + nCount > (long)m_Size)
			nRead = (m_Size - m_Position);
		else
			nRead = nCount;

		memcpy(buffer, m_pBuffer + m_Position, nRead);
		m_Position += nRead;

		return (size_t)(nRead / size);
	}

	size_t XFileMem::write(const void *buffer, size_t size, size_t count)
	{
		if (m_pBuffer == NULL) return 0;
		if (buffer == NULL) return 0;

		long nCount = (long)(count*size);
		if (nCount == 0) return 0;

		if (m_Position + nCount > m_Edge) alloc((uint)(m_Position + nCount));

		memcpy(m_pBuffer + m_Position, buffer, nCount);

		m_Position += nCount;

		if (m_Position > (long)m_Size) m_Size = m_Position;

		return (size_t)count;
	}

	bool XFileMem::seek(int offset, int origin)
	{
		if (m_pBuffer == NULL) return false;
		int lNewPos = m_Position;

		if (origin == SEEK_SET)		 lNewPos = offset;
		else if (origin == SEEK_CUR) lNewPos += offset;
		else if (origin == SEEK_END) lNewPos = m_Size + offset;
		else return false;

		if (lNewPos < 0) lNewPos = 0;

		m_Position = lNewPos;
		return true;
	}

	int XFileMem::tell()
	{
		if (m_pBuffer == NULL) return -1;
		return m_Position;
	}

	int XFileMem::size()
	{
		if (m_pBuffer == NULL) return -1;
		return m_Size;
	}

	bool XFileMem::flush()
	{
		if (m_pBuffer == NULL) return false;
		return true;
	}

	bool XFileMem::eof()
	{
		if (m_pBuffer == NULL) return true;
		return (m_Position >= (long)m_Size);
	}

	int XFileMem::error()
	{
		if (m_pBuffer == NULL) return -1;
		return (m_Position > (long)m_Size);
	}

	bool XFileMem::putC(unsigned char c)
	{
		if (m_pBuffer == NULL) return false;
		if (m_Position + 1 > m_Edge) alloc(m_Position + 1);

		memcpy(m_pBuffer + m_Position, &c, 1);

		m_Position += 1;

		if (m_Position > (long)m_Size) m_Size = m_Position;

		return true;
	}

	int XFileMem::getC()
	{
		if (eof()) return EOF;
		return *(uchar*)((uchar*)m_pBuffer + m_Position++);
	}

	void XFileMem::alloc(uint dwNewLen)
	{
		if (dwNewLen > (ulong)m_Edge)
		{
			// find new buffer size
			uint dwNewBufferSize = (uint)(((dwNewLen >> 12) + 1) << 12);

			// allocate new buffer
			if (m_pBuffer == NULL) m_pBuffer = (uchar*)malloc(dwNewBufferSize);
			else	m_pBuffer = (uchar*)realloc(m_pBuffer, dwNewBufferSize);
			// I own this buffer now (caller knows nothing about it)
			m_bFreeOnClose = true;

			m_Edge = (int)dwNewBufferSize;
		}
		return;
	}

	void XFileMem::Free()
	{
		close();
	}

}