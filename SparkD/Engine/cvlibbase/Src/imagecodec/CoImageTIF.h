
/*! 
 * \file    CoImageTIF.h
 * \ingroup base
 * \brief   TIFF
 * \author  
 */

#pragma once

#ifndef CVLIB_IMG_NOCODEC

#include "CoImage.h"

namespace cvlib 
{

/**
 * @brief png
 */

#if CVLIB_IMG_SUPPORT_TIF

class CoImageTIF : public CoImage
{
public:
	// Construction and Destruction
	CoImageTIF () {m_tif2=NULL; m_multipage=false; m_pages=0;}
	~CoImageTIF ();

	bool Decode (Mat& image, XFile* pFile);

#if CVLIB_IMG_SUPPORT_ENCODE
	bool Encode(const Mat& image, XFile* pFile, bool bAppend = false);
#endif // CVLIB_IMG_SUPPORT_ENCODE

protected:
	void TileToStrip(unsigned char* out, unsigned char* in,	unsigned int rows, unsigned int cols, int outskew, int inskew);
	bool EncodeBody(const Mat& image, void *m_tif, bool multipage=false, int page=0, int pagecount=0);
	void* m_tif2;
	bool  m_multipage;
	int   m_pages;
};

#endif

}

#endif //CVLIB_IMG_NOCODEC
