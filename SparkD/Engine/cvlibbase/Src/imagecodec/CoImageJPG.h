/*! 
 * \file    CoImageJPG.h
 * \ingroup base
 * \brief   JPG
 * \author  
 */

#pragma once

#ifndef CVLIB_IMG_NOCODEC

#include "CoImage.h"

namespace cvlib 
{

/**
 * @brief jpg
 */

#if CVLIB_IMG_SUPPORT_JPG

class CoImageJPG : public CoImage
{
public:
	// Construction and Destruction
	CoImageJPG ();

	bool Decode (Mat& image, XFile* pxfile);

#if CVLIB_IMG_SUPPORT_ENCODE
	bool Encode(const Mat& image, XFile* pFile);
#endif // CVLIB_IMG_SUPPORT_ENCODE
	
};

#endif

}

#endif //CVLIB_IMG_NOCODEC
