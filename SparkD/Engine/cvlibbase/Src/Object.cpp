/*!
 * \file	Object.cpp
 * \ingroup base
 * \brief	
 * \author	 
 */

#include "Object.h"
#include "XFileDisk.h"

namespace cvlib 
{

Object::Object()
{
}

Object::~Object()
{
}

int	Object::fromFile(const char* szFilename)
{
	XFileDisk xfile;
	if (!xfile.open(szFilename, "rb"))
		return 0;
	return fromFile(&xfile);
}

int	Object::toFile(const char* szFilename) const
{
	XFileDisk xfile;
	if (!xfile.open(szFilename, "wb"))
		return 0;
	return toFile(&xfile);
}

int	Object::fromFile(XFile*) { return 0; }
int	Object::toFile(XFile*) const { return 0; }


/*
void* Object::operator new(size_t nSize)
{
	void* ptr;
#if CVLIB_MEMCHECK
	MallocOBJECT (ptr, nSize);
#else
	ptr = malloc(nSize);
#endif
	return ptr;
}

void* Object::operator new(size_t nSize, char* szFilename, int nLine)
{
	void* ptr;
#if CVLIB_MEMCHECK
	assert ((nSize) != 0);
	(ptr) = malloc (nSize);
	AddMemblock(ptr, nSize, szFilename, nLine, 1);
#else
	ptr = malloc(nSize);
#endif
	return ptr;
}

void* Object::operator new[](size_t nSize)
{
	void* ptr;
#if CVLIB_MEMCHECK
	MallocOBJECT (ptr, nSize);
#else
	ptr = malloc(nSize);
#endif
	return ptr;
}

void* Object::operator new[](size_t nSize, char* szFilename, int nLine)
{
	void* ptr;
#if CVLIB_MEMCHECK
	assert ((nSize) != 0);
	(ptr) = malloc (nSize);
	AddMemblock(ptr, nSize, szFilename, nLine, 1);
#else
	ptr = malloc(nSize);
#endif
	return ptr;
}

void Object::operator delete(void *ptr, char* szFilename, int nLine)
{
#if CVLIB_MEMCHECK
	FreeMembolck (ptr, 1);
#else
	free (ptr);
#endif
}

void Object::operator delete[](void *ptr, char* szFilename, int nLine)
{
#if CVLIB_MEMCHECK
	FreeMembolck (ptr, 1);
#else
	free (ptr);
#endif
}

void Object::operator delete(void *ptr)
{
#if CVLIB_MEMCHECK
	FreeMembolck (ptr, 1);
#else
	free (ptr);
#endif
}

void Object::operator delete[](void *ptr)
{
#if CVLIB_MEMCHECK
	FreeMembolck (ptr, 1);
#else
	free (ptr);
#endif
}*/

}
