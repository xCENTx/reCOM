#include "util_systemio.h"

CBufferIO::CBufferIO()
{
	m_buffer = NULL;
	m_file = NULL;
	m_bufsize = 0;
	m_filesize = 0;
	CFileIO();
}

CBufferIO::CBufferIO(const CBufferIO& other)
{
	m_filesize = other.m_filesize;
	m_bufsize = other.m_bufsize;
	m_file = other.m_file;
	m_buffer = other.m_buffer;
}

bool CBufferIO::Open(void* buf, size_t size)
{
	m_filesize = size;
	m_bufsize = size;
	m_buffer = buf;
	m_file = (s64)m_buffer;
	return true;
}

bool CBufferIO::Open(const char* buf)
{
	bool success = CFileIO::Open(buf);
	bool isOpen = CFileIO::IsOpen();

	if (isOpen)
	{
		LoadBuffer();
	}

	if (!success)
	{
		Close();
	}

	return success;
}

void CBufferIO::Close()
{
	if (m_buffer != NULL)
	{
		free(m_buffer);
	}

	m_file = NULL;
	m_bufsize = 0;
	m_buffer = NULL;
	m_filesize = 0;
	CFileIO::Close();
}

bool CBufferIO::LoadBuffer()
{
	bool success = false;

	if (CFileIO::IsOpen())
	{
		size_t size = GetSize();

		if (0 < size)
		{
			if (m_bufsize < size)
			{
				m_file = (s64)realloc((FILE*)m_file, size);
				m_bufsize = size;
			}

			m_filesize = size;

			if (m_file == NULL)
			{
				size = m_filesize;
				int position = CFileIO::fread(m_buffer, size);

				if (size == position)
				{
					m_file = (s64)m_buffer;
					success = m_buffer != NULL;
				}
				else
				{
					success = false;
					free((FILE*)m_file);
					m_file = NULL;
					m_bufsize = 0;
					m_filesize = 0;
				}
			}
		}
	}

	return success;
}

void CBufferIO::Release()
{
	m_bufsize = 0;
	m_filesize = 0;
	m_buffer = NULL;
	m_file = NULL;
	CFileIO::Release();
}

size_t CBufferIO::fread(int offset, void** buf)
{
	int position = ftell();
	size_t fileSize = m_filesize;

	if (position < fileSize)
	{
		if (fileSize <= position + offset)
		{
			offset = fileSize - position;
		}

		*buf = (void*)m_file;
		m_file = m_file + offset;
	}
	else
	{
		offset = 0;
	}

	return offset;
}

size_t CBufferIO::fread(void* buf, size_t size)
{
	int position = 0;
	size_t offset = 0;
	size_t fileSize = 0;

	offset = size;
	if (m_buffer == NULL)
	{
		position = CFileIO::fread(buf, offset);
		offset = position;
	}
	else
	{
		position = ftell();
		fileSize = m_filesize;

		if (position < fileSize)
		{
			if (fileSize <= position + size)
			{
				offset = (fileSize - position);
			}

			memcpy(buf, (void*)m_file, offset);
			m_file = m_file + offset;
		}
		else
		{
			offset = 0;
		}
	}

	return offset;
}

size_t CBufferIO::fseek(int offset, int mode)
{
	int pos = 0;

	if (m_file == NULL)
	{
		pos = CFileIO::fseek(offset, mode);
	}
	else
	{
		if (mode == 2)
		{
			pos = m_filesize + offset;
		}
		else if (mode == 1)
		{
			pos = ftell();
			pos += offset;
		}
		else
		{
			pos = 0;
			if (mode == 0)
			{
				pos = offset;
			}
		}

		if (m_filesize < pos || pos < 0)
		{
			pos = -1;
		}
		else
		{
			m_file = (s64)m_buffer + pos;
		}
	}

	return pos;
}

size_t CBufferIO::ftell()
{
	int position = 0;

	if (m_buffer == NULL)
	{
		position = CFileIO::ftell();
	}
	else
	{
		position = (s64)m_file - (s64)m_buffer;
	}

	return position;
}