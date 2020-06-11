
/*!
 * \file	Object.h
 * \ingroup base
 * \brief	.
 * \author	
 */

#pragma once

#include "cvlibbaseDef.h"
#include "XFile.h"

namespace cvlib
{

	class CVLIB_DECLSPEC Object
	{
	public:
		// Construction and Destruction
		Object();
		virtual ~Object();

		virtual int	fromFile(const char* szFilename);
		virtual int	toFile(const char* szFilename) const;
		virtual int	fromFile(XFile* pFile);
		virtual int	toFile(XFile* pFile) const;
	};

}