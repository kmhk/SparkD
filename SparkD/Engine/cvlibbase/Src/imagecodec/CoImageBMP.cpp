/*! 
 * \file	CoImageBMP.cpp
 * \ingroup base
 * \brief	BMP
 * \author	
 */

#include "CoImageBMP.h"
#include "ImageIter.h"
#include "BmpStructs.h"

#if CVLIB_IMG_SUPPORT_BMP

namespace cvlib 
{

#define BFT_ICON   0x4349   /* 'IC' */
#define BFT_BITMAP 0x4d42   /* 'BM' */
#define BFT_CURSOR 0x5450   /* 'PT' */

#if CVLIB_IMG_SUPPORT_BMP

#ifndef WIDTHBYTES
#define WIDTHBYTES(i)           ((unsigned)((i+31)&(~31))/8)  /* ULONG aligned ! */
#endif

#endif
	
#define DibWidthBytesN(lpbi, n) (uint)WIDTHBYTES((uint)(lpbi)->biWidth * (uint)(n))
#define DibWidthBytes(lpbi)     DibWidthBytesN(lpbi, (lpbi)->biBitCount)
	
#define DibSizeImage(lpbi)      ((lpbi)->biSizeImage == 0 \
	? ((uint)DibWidthBytes(lpbi) * (uint)(lpbi)->biHeight) \
	: (lpbi)->biSizeImage)
	
#define DibNumColors(lpbi)      ((lpbi)->biClrUsed == 0 && (lpbi)->biBitCount <= 8 \
	? (int)(1 << (int)(lpbi)->biBitCount)          \
	: (int)(lpbi)->biClrUsed)

#define FixBitmapInfo(lpbi)     if ((lpbi)->biSizeImage == 0)                 \
	(lpbi)->biSizeImage = DibSizeImage(lpbi); \
	if ((lpbi)->biClrUsed == 0)                   \
	(lpbi)->biClrUsed = DibNumColors(lpbi);		\

const int RLE_COMMAND     = 0;
const int RLE_ENDOFLINE   = 0;
const int RLE_ENDOFBITMAP = 1;
const int RLE_DELTA       = 2;
	
#if !defined(BI_RLE8)
#define BI_RLE8  1L
#endif
#if !defined(BI_RLE4)
#define BI_RLE4  2L
#endif

CoImageBMP::CoImageBMP()
{
}

bool CoImageBMP::Decode (Mat& image, XFile* pFile)
{
	if (pFile == NULL) 
		return false;

	BITMAPFILEHEADER1   bf;
	int off = pFile->tell();
	try 
	{
		if (pFile->read(&bf.bfType,sizeof(bf.bfType),1) == 0)
			throw "Not a BMP";
		if (pFile->read(&bf.bfSize,sizeof(bf.bfSize),1) == 0)
			throw "Not a BMP";
		if (pFile->read(&bf.bfReserved1,sizeof(bf.bfReserved1),1) == 0)
			throw "Not a BMP";
		if (pFile->read(&bf.bfReserved2,sizeof(bf.bfReserved2),1) == 0)
			throw "Not a BMP";
		if (pFile->read(&bf.bfOffBits,sizeof(bf.bfOffBits),1) == 0)
			throw "Not a BMP";
// 		if (fread(&bf,MIN(14,sizeof(bf)),1, pFile)==0) 
// 			throw "Not a BMP";
		if (bf.bfType != BFT_BITMAP)
		{ //do we have a RC HEADER?
			bf.bfOffBits = 0L;
			pFile->seek(off, SEEK_SET);
		}

		BITMAPINFOHEADER1 tempHeader;
		if (!DibReadBitmapInfo(pFile,&tempHeader)) 
            return false;
//			throw "Error reading BMP info";

		tempHeader.biSizeImage = DibSizeImage(&tempHeader);
		size_t llen = tempHeader.biSizeImage + tempHeader.biClrUsed * sizeof(COLOR);

		uchar* pbData = new uchar[llen + sizeof(tempHeader)];
		memcpy(pbData, &tempHeader, sizeof(tempHeader));
		if (pFile->read (pbData + sizeof(tempHeader), llen, 1) != 1)
		{
			delete []pbData;
			return false;
		}

		BITMAPINFOHEADER1* pbmpHeader = (BITMAPINFOHEADER1*)pbData;

		bool bIsOldBmp = pbmpHeader->biSize == sizeof(BITMAPCOREHEADER1);
		
		bool bTopDownDib = pbmpHeader->biHeight<0; //<Flanders> check if it's a top-down bitmap
		if (bTopDownDib) pbmpHeader->biHeight=-pbmpHeader->biHeight;
		
		if (pbmpHeader->biBitCount > 8)
		{
			if (image.create((BITMAPINFOHEADER1*)pbData))
			{
				if (bTopDownDib)
					image.flipUD();
				delete []pbData;
				return true;
			}
			delete []pbData;
			return false;
		}
		Startup (0);
		uchar* pBMPData = pbData + sizeof(BITMAPINFOHEADER1);
		int nH = pbmpHeader->biHeight;
		int nW = pbmpHeader->biWidth;
		
		memcpy (&m_Head, pbmpHeader, sizeof(m_Head));
		CreateInfo (nH, nW, pbmpHeader->biBitCount, CVLIB_IMG_FORMAT_BMP);
		image.create (nH, nW, MAT_Tuchar3);

		m_Head.biXPelsPerMeter = pbmpHeader->biXPelsPerMeter;
		m_Head.biYPelsPerMeter = pbmpHeader->biYPelsPerMeter;
		m_Info.xDPI = (long) floor(pbmpHeader->biXPelsPerMeter * 254.0 / 10000.0 + 0.5);
		m_Info.yDPI = (long) floor(pbmpHeader->biYPelsPerMeter * 254.0 / 10000.0 + 0.5);
		
		if (m_Info.nEscape) 
			throw "Cancelled"; // <vho> - cancel decoding
		
		COLOR *pRgb = GetPalette();
		if (pRgb){
			if (bIsOldBmp){
				// convert a old color table (3 byte entries) to a new
				// color table (4 byte entries)
				memcpy (pRgb, pBMPData, DibNumColors(pbmpHeader) * sizeof(RGBTRIPLE1));
				for (int i=DibNumColors(&m_Head)-1; i>=0; i--){
					pRgb[i].x      = ((RGBTRIPLE1 *)pRgb)[i].rgbtRed;
					pRgb[i].z     = ((RGBTRIPLE1 *)pRgb)[i].rgbtBlue;
					pRgb[i].y    = ((RGBTRIPLE1 *)pRgb)[i].rgbtGreen;
					pRgb[i].w = (uchar)0;
				}
			} else {
				memcpy (pRgb, pBMPData, DibNumColors(pbmpHeader) * sizeof(COLOR));
				//force w=0, to avoid problems with some WinXp bitmaps
				for (unsigned int i=0; i<m_Head.biClrUsed; i++)
					pRgb[i].w=0;
			}
		}
		int cn = image.channels();
		{
			if (bf.bfOffBits != 0L) 
 				pFile->seek(off + bf.bfOffBits,SEEK_SET);
			switch (pbmpHeader->biCompression) 
			{
			case BI_RGB :
				{
					uchar* pbTemp=new uchar[m_Info.dwEffWidth];
					for (int i = nH - 1; i >= 0; i --)
					{
						uchar* pdst = image.data.ptr[i];
						pFile->read (pbTemp, m_Info.dwEffWidth, 1);
						int bt=pbTemp[0];
						switch (m_Head.biBitCount)
						{
						case 1:
							{
								for (int j=0; j<nW; j++)
								{
									if (j != 0 && (j%8) == 0)
										bt=pbTemp[j/8];
									bt = bt << 1;
									int nIdx=bt/256;
									pdst[j*cn] = pRgb[nIdx].x;
									pdst[j*cn+1] = pRgb[nIdx].y;
									pdst[j*cn+2] = pRgb[nIdx].z;
									bt = bt%256;
								}
							}
							break;
						case 4:
							{
								for (int j=0; j<nW; j++)
								{
									if (j != 0 && (j%2) == 0)
										bt=pbTemp[j/2];
									bt = bt << 4;
									int nIdx=bt/256;
									pdst[j*cn] = pRgb[nIdx].x;
									pdst[j*cn+1] = pRgb[nIdx].y;
									pdst[j*cn+2] = pRgb[nIdx].z;
									bt = bt%256;
								}
							}
							break;
						case 8:
							{
								for (int j=0; j<nW; j++)
								{
									int nIdx=pbTemp[j];
									pdst[j*cn] = pRgb[nIdx].x;
									pdst[j*cn+1] = pRgb[nIdx].y;
									pdst[j*cn+2] = pRgb[nIdx].z;
								}
							}
							break;
						}
					}
					delete []pbTemp;
				}
				break;
			case BI_RLE4 :
				{
					uchar status_byte = 0;
					uchar second_byte = 0;
					int scanline = 0;
					int bits = 0;
					bool low_nibble = false;
					ImageIterator iter(&image, this);

					for (bool bContinue = true; bContinue;)
					{
						pFile->read(&status_byte, sizeof(uchar), 1);
						switch (status_byte)
						{
						case RLE_COMMAND :
							pFile->read(&status_byte, sizeof(uchar), 1);
							switch (status_byte) 
							{
							case RLE_ENDOFLINE :
								bits = 0;
								scanline++;
								low_nibble = false;
								break;
							case RLE_ENDOFBITMAP :
								bContinue=false;
								break;
							case RLE_DELTA :
								{
									// read the delta values
									uchar delta_x;
									uchar delta_y;
									pFile->read(&delta_x, sizeof(uchar), 1);
									pFile->read(&delta_y, sizeof(uchar), 1);
									// apply them
									bits       += delta_x / 2;
									scanline   += delta_y;
									break;
								}
							default :
								pFile->read(&second_byte, sizeof(uchar), 1);
								uchar *sline = iter.GetRow(scanline);
								for (int i = 0; i < status_byte; i++)
								{
									if (low_nibble)
									{
										if ((size_t)(sline+bits) < (size_t)(pBMPData+m_Head.biSizeImage))
										{
											*(sline + bits) |=  (second_byte & 0x0F);
										}
										if (i != status_byte - 1)
											pFile->read(&second_byte, sizeof(uchar), 1);
										bits++;
									}
									else
									{
										if ((size_t)(sline+bits) < (size_t)(pBMPData+m_Head.biSizeImage)){
											*(sline + bits) = (uchar)(second_byte & 0xF0);
										}
									}
									low_nibble = !low_nibble;
								}
								if ((((status_byte+1) >> 1) & 1 )== 1)
									pFile->read(&second_byte, sizeof(uchar), 1);
								break;
							};
							break;
						default :
							{
								uchar *sline = iter.GetRow(scanline);
								pFile->read(&second_byte, sizeof(uchar), 1);
								for (int i = 0; i < status_byte; i++) {
									if (low_nibble) {
										if ((size_t)(sline+bits) < (size_t)(pBMPData+m_Head.biSizeImage)){
											*(sline + bits) |= (second_byte & 0x0F);
										}
										bits++;
									} else {
										if ((size_t)(sline+bits) < (size_t)(pBMPData+m_Head.biSizeImage)){
											*(sline + bits) = (uchar)(second_byte & 0xF0);
										}
									}				
									low_nibble = !low_nibble;
								}
							}
							break;
						};
					}
					break;
				}
			case BI_RLE8 :
				{
					uchar status_byte = 0;
					uchar second_byte = 0;
					int scanline = 0;
					int bits = 0;
					ImageIterator iter(&image, this);

					for (bool bContinue = true; bContinue; )
					{
						pFile->read(&status_byte, sizeof(uchar), 1);
						switch (status_byte)
						{
						case RLE_COMMAND :
							pFile->read(&status_byte, sizeof(uchar), 1);
							switch (status_byte) 
							{
							case RLE_ENDOFLINE :
								bits = 0;
								scanline++;
								break;
							case RLE_ENDOFBITMAP :
								bContinue=false;
								break;
							case RLE_DELTA :
								{
									// read the delta values
									uchar delta_x;
									uchar delta_y;
									pFile->read(&delta_x, sizeof(uchar), 1);
									pFile->read(&delta_y, sizeof(uchar), 1);
									// apply them
									bits     += delta_x;
									scanline += delta_y;
									break;
								}
							default :
								pFile->read((void *)(iter.GetRow(scanline) + bits), sizeof(uchar) * status_byte, 1);
								// align run length to even number of bytes 
								if ((status_byte & 1) == 1)
									pFile->read(&second_byte, sizeof(uchar), 1);
								bits += status_byte;													
								break;								
							};
							break;
						default :
							uchar *sline = iter.GetRow(scanline);
							pFile->read(&second_byte, sizeof(uchar), 1);
							for (int i = 0; i < status_byte; i++)
							{
								if ((ulong)bits<m_Info.dwEffWidth)
								{
									*(sline + bits) = second_byte;
									bits++;					
								}
								else
								{
									bContinue = false;
									break;
								}
							}
							break;
						};
					}
					break;
				}
			default :								
				throw "compression type not supported";
			}
		}
		m_Info.dwEffWidth = ((((24 * nW) + 31) / 32) * 4);
		m_Head.biSizeImage = m_Info.dwEffWidth * nH;
		
		delete []pbData;
 		if (bTopDownDib)
			image.flipUD(); //<Flanders>
	}
	catch (char* szMessage)
	{
		printf ("%s", szMessage);
		return false;
	}
    return true;
}

bool CoImageBMP::DibReadBitmapInfo(XFile* pFile, BITMAPINFOHEADER1 *pdib)
{
	if ((pFile==NULL)||(pdib==NULL))
		return false;
	
    if (pFile->read (pdib,sizeof(BITMAPINFOHEADER1),1)==0)
		return false;

    BITMAPCOREHEADER1   bc;

    switch (pdib->biSize) // what type of bitmap info is this?
    {
	case sizeof(BITMAPINFOHEADER1):
		break;

	case 64: //sizeof(OS2_BMP_HEADER):
		pFile->seek((long)(64 - sizeof(BITMAPINFOHEADER1)),SEEK_CUR);
		break;

	case sizeof(BITMAPCOREHEADER1):
		bc = *(BITMAPCOREHEADER1*)pdib;
		pdib->biSize               = bc.bcSize;
		pdib->biWidth              = (int)bc.bcWidth;
		pdib->biHeight             = (int)bc.bcHeight;
		pdib->biPlanes             =  bc.bcPlanes;
		pdib->biBitCount           =  bc.bcBitCount;
		pdib->biCompression        = BI_RGB;
		pdib->biSizeImage          = 0;
		pdib->biXPelsPerMeter      = 0;
		pdib->biYPelsPerMeter      = 0;
		pdib->biClrUsed            = 0;
		pdib->biClrImportant       = 0;
		pFile->seek((int)(sizeof(BITMAPCOREHEADER1)-sizeof(BITMAPINFOHEADER1)), SEEK_CUR);

		break;
	default:
		//give a last chance
		if (pdib->biSize>(sizeof(BITMAPINFOHEADER1))&&
			(pdib->biSizeImage==(unsigned long)(pdib->biHeight*((((pdib->biBitCount*pdib->biWidth)+31)/32)*4)))&&
			(pdib->biPlanes==1)&&(pdib->biCompression==BI_RGB)&&(pdib->biClrUsed==0))
		{
			pFile->seek((int)(pdib->biSize - sizeof(BITMAPINFOHEADER1)),SEEK_CUR);
			break;
		}
		return false;
    }
	
    FixBitmapInfo(pdib);
	
    return true;
}
		
#if CVLIB_IMG_SUPPORT_ENCODE
bool CoImageBMP::Encode(const Mat& image, XFile* pFile)
{
	if (pFile == 0)
		return false;
	cvlib::BitmapData bmpData;
	image.getBMPInfoHeader(bmpData);

	BITMAPINFOHEADER1* pBIF = bmpData.getBitmapInfoHeader();
	int nSize = pBIF->biSize + pBIF->biSizeImage + pBIF->biClrUsed * sizeof(COLOR);

	BITMAPFILEHEADER1	hdr;

	hdr.bfType = 0x4d42;   // 'BM' WINDOWS_BITMAP_SIGNATURE
	hdr.bfSize = nSize + 14/*sizeof(BITMAPFILEHEADER1)*/;
	hdr.bfReserved1 = hdr.bfReserved2 = 0;
	hdr.bfOffBits = 14/*sizeof(BITMAPFILEHEADER1)*/ + pBIF->biSize + pBIF->biClrUsed * sizeof(COLOR);
	
    // write the file header
//	fwrite (&hdr,MIN(14,sizeof(BITMAPFILEHEADER1)),1, pFile);
	pFile->write(&hdr.bfType,sizeof(hdr.bfType),1);
	pFile->write (&hdr.bfSize,sizeof(hdr.bfSize),1);
	pFile->write (&hdr.bfReserved1,sizeof(hdr.bfReserved1),1);
	pFile->write (&hdr.bfReserved2,sizeof(hdr.bfReserved2),1);
	pFile->write (&hdr.bfOffBits,sizeof(hdr.bfOffBits),1);

    // write the DIB header and the pixels
 	pFile->write ((uchar*)pBIF, nSize, 1);
	return true;
}
#endif // CVLIB_IMG_SUPPORT_ENCODE

}

#endif