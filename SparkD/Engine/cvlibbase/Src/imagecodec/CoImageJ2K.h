/*! 
 * \file    CoImageJAS.h
 * \ingroup base
 * \brief   Jasper
 * \author  
 */

#pragma once


#ifndef CVLIB_IMG_NOCODEC


#include "CoImage.h"

namespace cvlib
{

	/**
	 * @brief jasper
	 */

#if CVLIB_IMG_SUPPORT_J2K

	class  CoImageJ2K : public CoImage
	{
	public:
		// Construction and Destruction
		//! 
		CoImageJ2K();

		bool Decode(Mat& image, XFile* pFile);

#if CVLIB_IMG_SUPPORT_ENCODE
		bool Encode(const Mat& image, XFile* pFile);
#endif // CVLIB_IMG_SUPPORT_ENCODE

	};

#endif //CVLIB_IMG_SUPPORT_J2K

}


#endif //CVLIB_IMG_NOCODEC
