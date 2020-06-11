/*! 
 * \file    CoImageBMP.h
 * \ingroup base
 * \brief   BMP
 * \author
 */

#pragma once

#include "CoImage.h"

namespace cvlib 
{

/**
 * @brief bmp
 */

#if CVLIB_IMG_SUPPORT_BMP

class CoImageBMP : public CoImage
{
public:
	// Construction and Destruction
	//! 
	CoImageBMP ();

	/**
	 * @brief  BMP
	 *
	 * @param   pFile [in] :
	 * @return 
	 *
	 * @see  Encode()
	 *
	 * @par Example:
	 * @code
	 *
	 *  char szFileName[] = "c:/Test/TestImg1.bmp"; 
	 *  char szFileName1[] = "c:/Test/SaveImg1_4.bmp"; 
	 *
	 *  FILE* pFile = fopen(szFileName, "rb");
	 *
	 *  CoImageBMP testImg, Img; 
	 *  if ( testImg.Decode (pFile) ) 
	 *  {
	 *		Img.create (testImg, TRUE);
	 *		FILE* pFile1 = fopen(szFileName1, "wb");
	 *		Img.Encode(pFile1);
	 *		fclose(pFile1);
	 *  }
	 *
	 *  fclose(pFile);
	 *
	 * @endcode
	 * 
	 */
	bool Decode (Mat& image, XFile* pFile);

#if CVLIB_IMG_SUPPORT_ENCODE
	/**
	 * @brief  BMP
	 *
	 * @param   pFile [in] :
	 * @return  
	 *
	 * @see  Decode()
	 *
	 */
	bool Encode(const Mat& image, XFile* pFile);
#endif // CVLIB_IMG_SUPPORT_ENCODE
	
protected:
	bool DibReadBitmapInfo(XFile* pFile, BITMAPINFOHEADER1 *pdib);
};

#endif

}