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

#if CVLIB_IMG_SUPPORT_JAS

class  CoImageJAS : public CoImage
{
public:
	// Construction and Destruction
	//! 
	CoImageJAS ();

	bool Decode (Mat& image, XFile* pFile, ulong nImagetype);

#if CVLIB_IMG_SUPPORT_ENCODE
	bool Encode(const Mat& image, XFile* pFile, ulong nImagetype);
#endif // CVLIB_IMG_SUPPORT_ENCODE
	
};

#endif //CVLIB_IMG_SUPPORT_JAS

}


#endif //CVLIB_IMG_NOCODEC
