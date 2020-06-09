
/*! 
 * \file    CoImagePNG.h
 * \ingroup base
 * \brief   PNG
 * \author  
 */

#pragma once

#ifndef CVLIB_IMG_NOCODEC

#include "CoImage.h"
#include "XFile.h"
#include "XFileDisk.h"

namespace cvlib 
{

/**
 * @brief png 
 */

#if CVLIB_IMG_SUPPORT_PNG

class CoImagePNG : public CoImage
{
public:

	CoImagePNG ();

	bool Decode (Mat& image, XFile* pFile);

#if CVLIB_IMG_SUPPORT_ENCODE
	bool Encode(const Mat& image, XFile* pFile);
#endif // CVLIB_IMG_SUPPORT_ENCODE

protected:
	void expand2to4bpp(uchar* prow);
};

#endif

}
#endif //CVLIB_IMG_NOCODEC
