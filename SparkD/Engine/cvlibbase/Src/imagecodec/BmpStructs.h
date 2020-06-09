#pragma once

#include "cvlibmacros.h"
#include "cvlibstructs.h"


#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L

namespace cvlib 
{

	/**
	 * @brief  BMP
	 */
	typedef struct tagBITMAPFILEHEADER1 
	{
		ushort    bfType;
		uint   bfSize;
		ushort    bfReserved1;
		ushort    bfReserved2;
		uint   bfOffBits;
	} BITMAPFILEHEADER1;
	
	typedef struct tagBITMAPINFO1 { // bmi 
		BITMAPINFOHEADER1 bmiHeader; 
		COLOR          bmiColors[1]; 
	} BITMAPINFO1; 

	/**
	 * @brief  BMP
	 */
	typedef struct tagBITMAPCOREHEADER1 
	{
		uint   bcSize;
		ushort    bcWidth;
		ushort    bcHeight;
		ushort    bcPlanes;
		ushort    bcBitCount;
	} BITMAPCOREHEADER1;
	
	typedef struct tagRGBTRIPLE1 {
		uchar    rgbtBlue;
		uchar    rgbtGreen;
		uchar    rgbtRed;
	} RGBTRIPLE1;

}