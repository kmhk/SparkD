/*! 
 * \file	CoImage.cpp
 * \ingroup base
 * \brief	
 * \author	
 */
#include "cvlibmacros.h"

#ifdef _MSC_VER
#include <windows.h>
#include "TCHAR.h"
#endif

#include "CoImage.h"
#include "MatOperation.h"
#include "CoImageBMP.h"
#include "CoImageJPG.h"
#include "CoImagePNG.h"
#include "CoImageTIF.h"
#include "CoImageJAS.h"
#include "CoImageJ2K.h"
#include "XFileDisk.h"
#include "BmpStructs.h"
#include <cmath>

namespace cvlib {

CoImage::CoImage ()
{
	m_pPalette = 0;
	Startup (0);
	InitBMPHeader (0, 0);
}

CoImage::~CoImage ()
{
	release();
}
void	CoImage::release()
{
	if (m_pPalette)
		free (m_pPalette);
	m_pPalette = 0;
}
ulong CoImage::GetPaletteSize() const
{
	return (m_Head.biClrUsed * sizeof(COLOR));
}

long CoImage::GetXDPI() const
{
	return m_Info.xDPI;
}

long CoImage::GetYDPI() const
{
	return m_Info.yDPI;
}

void CoImage::SetXDPI(long dpi)
{
	if (dpi<=0) dpi=96;
	m_Info.xDPI = dpi;
	m_Head.biXPelsPerMeter = (int) floor(dpi * 10000.0 / 254.0 + 0.5);
}

void CoImage::SetYDPI(long dpi)
{
	if (dpi<=0) dpi=96;
	m_Info.yDPI = dpi;
	m_Head.biYPelsPerMeter = (int) floor(dpi * 10000.0 / 254.0 + 0.5);
}
uchar CoImage::GetJpegScale() const
{
	return m_Info.nJpegScale;
}
uchar CoImage::GetJpegQuality() const
{
	return m_Info.nQuality;
}

void CoImage::SetJpegQuality(uchar q)
{
	m_Info.nQuality = q;
}

void CoImage::InitBMPHeader(int nW, int nH)
{
	memset (&m_Head, 0, sizeof(m_Head));

	m_Head.biSize = sizeof(BITMAPINFOHEADER1);
	m_Head.biCompression = BI_RGB;
	m_Head.biWidth = nW;
	m_Head.biHeight = nH;
	m_Head.biBitCount = 24;
	m_Head.biPlanes = 1;
	int wBpp = 24;
	m_Info.dwEffWidth = ((((wBpp * nW) + 31) / 32) * 4);
	m_Head.biSizeImage = m_Info.dwEffWidth * nH;
	SetXDPI (96);
	SetYDPI (96);

	int nSize = (int)GetPaletteSize();
	if (nSize)
		m_pPalette = (COLOR*)malloc(nSize);
}

void CoImage::Startup(ulong imagetype)
{
	memset(&m_Head,0,sizeof(BITMAPINFOHEADER1));
	memset(&m_Info,0,sizeof(COIMAGEINFO));
	//init default attributes
    m_Info.dwType = (uint)imagetype;
	m_Info.nQuality = 90;
	m_Info.nAlphaMax = 255;
	m_Info.nBkgndIndex = -1;
	m_Info.bEnabled = true;
	m_Info.nJpegScale = 1;
	SetXDPI(96);
	SetYDPI(96);
	m_pPalette = NULL;
}

/**
* \sa SetCodecOption
*/
ulong CoImage::GetCodecOption(ulong imagetype) const
{
	if (imagetype<CVLIB_IMG_MAX_FORMATS)
	{
		if (imagetype==0)
			imagetype = m_Info.dwType;
		return m_Info.dwCodecOpt[imagetype];
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////
/**
* Encode option for GIF, TIF and JPG.
* - GIF : 0 = LZW (default), 1 = none, 2 = RLE.
* - TIF : 0 = automatic (default), or a valid compression code as defined in "tiff.h" (COMPRESSION_NONE = 1, COMPRESSION_CCITTRLE = 2, ...)
* - JPG : valid values stored in enum CODEC_OPTION ( ENCODE_BASELINE = 0x01, ENCODE_PROGRESSIVE = 0x10, ...)
*
* \return true if everything is ok
*/
bool CoImage::SetCodecOption(ulong opt, ulong imagetype)
{
	if (imagetype<CVLIB_IMG_MAX_FORMATS)
	{
		if (imagetype==0)
			imagetype = m_Info.dwType;
		m_Info.dwCodecOpt[imagetype] = opt;
		return true;
	}
	return false;
}

/**
* Returns the color of the specified index.
* \param i = palette index
* \param r, g, b = output color channels
*/
bool CoImage::GetPaletteColor(uchar i, uchar* rgb) const
{
	COLOR* ppal = GetPalette();
	if (ppal)
	{
		*rgb = ppal[i].x;
		*(rgb + 1) = ppal[i].y;
		*(rgb + 2) = ppal[i].z;
		return true;
	}
	return false;
}
bool CoImage::GetPaletteColor(uchar i, uchar* r, uchar* g, uchar* b) const
{
	COLOR* ppal = GetPalette();
	if (ppal)
	{
		*r = ppal[i].x;
		*g = ppal[i].y;
		*b = ppal[i].z;
		return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////
void CoImage::SetPalette(ulong n, uchar *r, uchar *g, uchar *b)
{
	if ((!r) || (m_pPalette == NULL) || (m_Head.biClrUsed == 0)) return;
	if (!g) g = r;
	if (!b) b = g;
	COLOR* ppal = GetPalette();
	ulong m = MIN(n, m_Head.biClrUsed);
	for (ulong i = 0; i < m; i++) {
		ppal[i].x = r[i];
		ppal[i].y = g[i];
		ppal[i].z = b[i];
	}
	m_Info.last_c_isvalid = false;
}
////////////////////////////////////////////////////////////////////////////////
void CoImage::SetPalette(rgb_color *rgb, ulong nColors)
{
	if ((!rgb) || (GetPalette() == NULL) || (m_Head.biClrUsed == 0)) return;
	COLOR* ppal = GetPalette();
	ulong m = MIN(nColors, m_Head.biClrUsed);
	for (ulong i = 0; i < m; i++) {
		ppal[i].x = rgb[i].r;
		ppal[i].y = rgb[i].g;
		ppal[i].z = rgb[i].b;
	}
	m_Info.last_c_isvalid = false;
}
////////////////////////////////////////////////////////////////////////////////
void CoImage::SetPalette(COLOR* pPal,ulong nColors)
{
	if ((pPal==NULL)||(GetPalette()==NULL)||(m_Head.biClrUsed==0)) return;
	memcpy(GetPalette(),pPal,MIN(GetPaletteSize(),nColors*sizeof(COLOR)));
	m_Info.last_c_isvalid = false;
}
////////////////////////////////////////////////////////////////////////////////
/**
* Sets (or replaces) the palette to gray scale palette.
* The function doesn't change the pixels; for standard
* gray scale conversion use GrayScale().
*/
void CoImage::SetGrayPalette()
{
	if ((GetPalette()==NULL)||(m_Head.biClrUsed==0)) return;
	COLOR* pal=GetPalette();
	for (ulong ni=0;ni<m_Head.biClrUsed;ni++)
		pal[ni].z=pal[ni].y = pal[ni].x = (uchar)(ni*(255/(m_Head.biClrUsed-1)));
}
////////////////////////////////////////////////////////////////////////////////
/**
* Colorize the palette.
* \sa Colorize
*/
void CoImage::BlendPalette(COLOR cr,long perc) const
{
	if ((GetPalette()==NULL)||(m_Head.biClrUsed==0)) return;
	ulong i,r,g,b;
	COLOR* pPal=GetPalette();
	r = cr.x;
	g = cr.y;
	b = cr.z;
	if (perc>100) perc=100;
	for(i=0;i<m_Head.biClrUsed;i++){
		pPal[i].z=(uchar)((pPal[i].z*(100-perc)+b*perc)/100);
		pPal[i].y =(uchar)((pPal[i].y*(100-perc)+g*perc)/100);
		pPal[i].x =(uchar)((pPal[i].x*(100-perc)+r*perc)/100);
	}
}
COLOR* CoImage::GetPalette() const
{
	return (COLOR*)m_pPalette;
}
int	CoImage::CreateInfo (int nRows, int nCols, int wBpp, int nImageType)
{
	// prevent further actions if width or height are not vaild <Balabasnia>
	if ((nCols == 0) || (nRows == 0))
	{
		strcpy(m_Info.szLastError,"CoImage::create : width and height must be greater than zero");
		return 0;
	}

    // Make sure bits per pixel is valid
    if		(wBpp <= 1)	wBpp = 1;
    else if (wBpp <= 4)	wBpp = 4;
    else if (wBpp <= 8)	wBpp = 8;
    else				wBpp = 24;
	
	// set the correct bpp value
    switch (wBpp)
	{
	case 1:
		m_Head.biClrUsed = 2;	break;
	case 4:
		m_Head.biClrUsed = 16; break;
	case 8:
		m_Head.biClrUsed = 256; break;
	default:
		m_Head.biClrUsed = 0;
    }
	
	//set the common image informations
    m_Info.dwEffWidth = ((((wBpp * nCols) + 31) / 32) * 4);
    m_Info.dwType = nImageType;
	
    // initialize BITMAPINFOHEADER1
	m_Head.biSize = sizeof(BITMAPINFOHEADER1); //<ralphw>
    m_Head.biWidth = nCols;		// fill in width from parameter
    m_Head.biHeight = nRows;	// fill in height from parameter
    m_Head.biPlanes = 1;			// must be 1
    m_Head.biBitCount = (ushort)wBpp;		// from parameter
    m_Head.biCompression = BI_RGB;    
    m_Head.biSizeImage = m_Info.dwEffWidth * nRows;
// 	m_Head.biXPelsPerMeter = 0; See SetXDPI
// 	m_Head.biYPelsPerMeter = 0; See SetYDPI
    m_Head.biClrImportant = 0;

	int nSize = (int)GetPaletteSize();
	if (nSize)
	{
		m_pPalette = (COLOR*)malloc(nSize);
		if (!m_pPalette)
		{
			strcpy(m_Info.szLastError,"CoImage::create can't allocate memory");
			return NULL;
		}
		//clear the palette
		memset(m_pPalette,0,nSize);
	}
	return 1;
}
void CoImage::SetBpp24(int nh, int nw)
{
	int wBPP = 24;
    m_Info.dwEffWidth = ((((wBPP * nw) + 31) / 32) * 4);
    m_Info.dwType = CVLIB_IMG_FORMAT_UNKNOWN;

	m_Head.biClrUsed = 0;
	int dwEffWidth = ((((wBPP * nw) + 31) / 32) * 4);
    // initialize BITMAPINFOHEADER1
	m_Head.biSize = sizeof(BITMAPINFOHEADER1); //<ralphw>
    m_Head.biWidth = nw;		// fill in width from parameter
    m_Head.biHeight = nh;	// fill in height from parameter
    m_Head.biPlanes = 1;			// must be 1
    m_Head.biBitCount = (ushort)wBPP;		// from parameter
    m_Head.biCompression = BI_RGB;    
    m_Head.biSizeImage = dwEffWidth * nh;
	// 	m_Head.biXPelsPerMeter = 0; See SetXDPI
	// 	m_Head.biYPelsPerMeter = 0; See SetYDPI
    m_Head.biClrImportant = 0;
}
void CoImage::SetPixelIndex(long x,long y,uchar i)
{
	if ((GetPalette() == NULL) || (m_Head.biClrUsed == 0) ||
		(x<0) || (y<0)||(x >= m_Head.biWidth) || (y >= m_Head.biHeight)) return ;
	
	if (m_Head.biBitCount == 8)
	{
		m_Info.pImage[y*m_Info.dwEffWidth + x]=i;
		return;
	} 
	else
	{
		uchar pos;
		uchar* iDst= m_Info.pImage + y*m_Info.dwEffWidth + (x*m_Head.biBitCount >> 3);
		if (m_Head.biBitCount == 4)
		{
			pos = (uchar)(4*(1-x%2));
			*iDst &= ~(0x0F<<pos);
			*iDst |= ((i & 0x0F)<<pos);
			return;
		}
		else if (m_Head.biBitCount == 1)
		{
			pos = (uchar)(7-x%8);
			*iDst &= ~(0x01<<pos);
			*iDst |= ((i & 0x01)<<pos);
			return;
		}
	}
}
uchar CoImage::GetPixelIndex(long x,long y) const
{
	if ((GetPalette()==NULL)||(m_Head.biClrUsed==0)) return 0;
	
	if ((x<0)||(y<0)||(x>=m_Head.biWidth)||(y>=m_Head.biHeight)) 
	{
		if (m_Info.nBkgndIndex != -1)
			return (uchar)m_Info.nBkgndIndex;
		else
			return *m_Info.pImage;
	}
	if (m_Head.biBitCount==8)
	{
		return m_Info.pImage[y*m_Info.dwEffWidth + x];
	} 
	else
	{
		uchar pos;
		uchar iDst= m_Info.pImage[y*m_Info.dwEffWidth + (x*m_Head.biBitCount >> 3)];
		if (m_Head.biBitCount==4)
		{
			pos = (uchar)(4*(1-x%2));
			iDst &= (0x0F<<pos);
			return (uchar)(iDst >> pos);
		}
		else if (m_Head.biBitCount==1)
		{
			pos = (uchar)(7-x%8);
			iDst &= (0x01<<pos);
			return (uchar)(iDst >> pos);
		}
	}
	return 0;
}

void CoImage::SetPaletteColor(uchar idx, uchar r, uchar g, uchar b, uchar alpha)
{
	if (GetPalette() && (m_Head.biClrUsed))
	{
		uchar* iDst = (uchar*)(m_pPalette);
		if (idx < m_Head.biClrUsed)
		{
			long ldx= idx * sizeof(COLOR);
			iDst[ldx++] = (uchar) b;
			iDst[ldx++] = (uchar) g;
			iDst[ldx++] = (uchar) r;
			iDst[ldx] = (uchar) alpha;
			m_Info.last_c_isvalid = false;
		}
	}
}

void CoImage::SetPaletteColor(uchar idx, COLOR c)
{
	if (GetPalette() && (m_Head.biClrUsed))
	{
		uchar* iDst = (uchar*)(m_pPalette);
		if (idx < m_Head.biClrUsed)
		{
			long ldx=idx * sizeof(COLOR);
			iDst[ldx++] = (uchar) c.z;
			iDst[ldx++] = (uchar) c.y;
			iDst[ldx++] = (uchar) c.x;
			iDst[ldx] = (uchar) c.w;
			m_Info.last_c_isvalid = false;
		}
	}
}

COLOR CoImage::GetPaletteColor(uchar idx) const
{
	COLOR rgb;
	if ( GetPalette() && (m_Head.biClrUsed) )
	{
		uchar* iDst = (uchar*)m_pPalette;
		if ( idx < m_Head.biClrUsed )
		{
			long ldx = idx * sizeof(COLOR);
			rgb.z = iDst[ldx++];
			rgb.y = iDst[ldx++];
			rgb.x = iDst[ldx++];
			rgb.w = iDst[ldx];
		}
	}
	return rgb;
}
uchar CoImage::GetPixelIndex(uchar* pData,long x) const
{
	if ((pData==NULL)||(m_Head.biClrUsed==0))
		return 0;
	
	if (m_Head.biBitCount==8)
	{
		return pData[x];
	}
	else
	{
		uchar pos;
		uchar iDst= pData[x*m_Head.biBitCount >> 3];
		if (m_Head.biBitCount==4)
		{
			pos = (uchar)(4*(1-x%2));
			iDst &= (0x0F<<pos);
			return (uchar)(iDst >> pos);
		}
		else if (m_Head.biBitCount==1)
		{
			pos = (uchar)(7-x%8);
			iDst &= (0x01<<pos);
			return (uchar)(iDst >> pos);
		}
	}
	return 0;
}

COLOR CoImage::getPixelColor(const Mat& image, long x,long y, bool fGetAlpha) const
{
	int cn = image.channels();
	COLOR rgb = m_Info.nBkgndColor; //<mpwolski>
	if ((!image.isValid())||(x<0)||(y<0)||
		(x>=m_Head.biWidth)||(y>=m_Head.biHeight))
	{
		if (m_Info.nBkgndIndex != -1)
		{
			if (m_Head.biBitCount<24) 
				return GetPaletteColor((uchar)m_Info.nBkgndIndex);
			else return m_Info.nBkgndColor;
		} 
		else if (GetPalette()) 
			return getPixelColor(image, 0,0);
		return rgb;
	}
	
	if (m_Head.biClrUsed)
	{
		rgb = GetPaletteColor(GetPixelIndex(x,y));
	}
	else
	{
		rgb.x = image.data.ptr[y][x*cn];
		rgb.y= image.data.ptr[y][x*cn+1];
		rgb.z  = image.data.ptr[y][x*cn+2];
	}
#if CVLIB_IMG_SUPPORT_ALPHA
	if (fGetAlpha) rgb.w = image.data.ptr[y][x*cn+3];
#else
	rgb.w = 0;
#endif //CVLIB_IMG_SUPPORT_ALPHA
	return rgb;
}

void CoImage::setPixelColor(Mat& image, long x,long y,float* color)
{
	int cn = image.channels();
	for (int i = 0; i<3; i ++)
		image.data.ptr[y][x*cn+i] = (uchar)color[i];
}

void CoImage::setPixelColor(Mat& image, long x,long y,COLOR c, bool bSetAlpha)
{
	int cn = image.channels();
	if ((!image.isValid()) || (x<0) || (y<0) ||
		(x>=m_Head.biWidth) || (y>=m_Head.biHeight))
		return;
	if (m_Head.biClrUsed)
		SetPixelIndex(x,y,GetNearestIndex(c));
	else 
	{
		image.data.ptr[y][x*cn] = c.x;
		image.data.ptr[y][x*cn+1] = c.y;
		image.data.ptr[y][x*cn+2] = c.z;
#if CVLIB_IMG_SUPPORT_ALPHA
		if (bSetAlpha) 
			image.data.ptr[y][x*cn+3] = c.w;
#endif //CVLIB_IMG_SUPPORT_ALPHA
	}
}

void CoImage::RGBtoBGR(uchar *buffer, int length, int cn)
{
	if (buffer && (m_Head.biClrUsed==0))
	{
		uchar temp;
		length = MIN(length,(int)m_Info.dwEffWidth);
		for (int i=0;i<length;i+=cn)
		{
			temp = buffer[i]; buffer[i] = buffer[i+2]; buffer[i+2] = temp;
		}
	}
}

uchar CoImage::GetNearestIndex(COLOR c)
{
	if ((GetPalette() == NULL)||(m_Head.biClrUsed==0)) return 0;
	
	// <RJ> check matching with the previous result
	if (m_Info.last_c_isvalid && (*(long*)&m_Info.last_c == *(long*)&c)) return m_Info.last_c_index;
	m_Info.last_c = c;
	m_Info.last_c_isvalid = true;
	
	uchar* iDst = (uchar*)(GetPalette()) + sizeof(BITMAPINFOHEADER1);
	long distance=200000;
	int i,j = 0;
	long k,l;
	int m = (int)(m_Head.biClrImportant==0 ? m_Head.biClrUsed : m_Head.biClrImportant);
	for(i=0,l=0;i<m;i++,l+=sizeof(COLOR))
	{
		k = (iDst[l]-c.z)*(iDst[l]-c.z)+
			(iDst[l+1]-c.y)*(iDst[l+1]-c.y)+
			(iDst[l+2]-c.x)*(iDst[l+2]-c.x);
		//		k = abs(iDst[l]-c.z)+abs(iDst[l+1]-c.y)+abs(iDst[l+2]-c.x);
		if (k==0)
		{
			j=i;
			break;
		}
		if (k<distance)
		{
			distance=k;
			j=i;
		}
	}
	m_Info.last_c_index = (uchar)j;
	return (uchar)j;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
bool	CoImage::Decode (Mat& image, unsigned char* szbuf, int nsize, ulong nImageType)
{
	XFileMem hfile(szbuf, nsize);
	return Decode(image, &hfile, nImageType);
}
bool	CoImage::DecodeUnknown (Mat& image, XFile* pFile, ulong nImageType)
{
	int pos = pFile->tell();
	if (nImageType == CVLIB_IMG_FORMAT_UNKNOWN)
	{
#if CVLIB_IMG_SUPPORT_BMP
		{
			CoImageBMP newImage; 
			if (newImage.Decode (image, pFile))
				return true;
			else 
				pFile->seek(pos, SEEK_SET);
		}
#endif
#ifndef CVLIB_IMG_NOCODEC
#if CVLIB_IMG_SUPPORT_JPG
		{
			CoImageJPG newImage; 
			if (newImage.Decode (image, pFile))
				return true;
			else  
				pFile->seek (pos, SEEK_SET);
		}
#endif
#if CVLIB_IMG_SUPPORT_PNG
		{
			CoImagePNG newImage;
			if (newImage.Decode (image, pFile)) 
				return true;
			else  
				pFile->seek (pos, SEEK_SET);
		}
#endif
#if CVLIB_IMG_SUPPORT_TIF
		{
			CoImageTIF newImage; 
			if (newImage.Decode (image, pFile))
				return true;
			else  
				pFile->seek (pos, SEEK_SET);
		}
#endif
#if CVLIB_IMG_SUPPORT_JP2
		{ 
			CoImageJAS newImage; 
			if (newImage.Decode (image, pFile, CVLIB_IMG_FORMAT_JP2))
				return true;
			else  
				pFile->seek (pos, SEEK_SET);
		}
#endif
#if CVLIB_IMG_SUPPORT_JPC
		{
			CoImageJAS newImage;
			if (newImage.Decode (image, pFile, CVLIB_IMG_FORMAT_JPC))
				return true;
			else  
				pFile->seek (pos, SEEK_SET);
		}
#endif
#if CVLIB_IMG_SUPPORT_PGX
		{ 
			CoImageJAS newImage;
			if (newImage.Decode (image, pFile, CVLIB_IMG_FORMAT_PGX))
				return true;
			else  pFile->seek (pos, SEEK_SET);
		}
#endif
#if CVLIB_IMG_SUPPORT_PNM
		{ 
			CoImageJAS newImage; 
			if (newImage.Decode (image, pFile, CVLIB_IMG_FORMAT_PNM))
				return true;
			else  
				pFile->seek (pos, SEEK_SET);
		}
#endif
#if CVLIB_IMG_SUPPORT_RAS
		{
			CoImageJAS newImage;
			if (newImage.Decode (image, pFile, CVLIB_IMG_FORMAT_RAS))
				return true;
			else  
				pFile->seek (pos, SEEK_SET);
		}
#endif
#if CVLIB_IMG_SUPPORT_J2K
		{
			CoImageJ2K newImage;
			if (newImage.Decode(image, pFile))
				return true;
			else
				pFile->seek(pos, SEEK_SET);
		}
#endif
#endif //CVLIB_IMG_NOCODEC
	}
	return false;
}
bool	CoImage::Decode (Mat& image, XFile* pFile, ulong nImageType)
{
	if (nImageType == CVLIB_IMG_FORMAT_UNKNOWN)
		return DecodeUnknown(image, pFile, nImageType);
	int pos = pFile->tell();
#if CVLIB_IMG_SUPPORT_BMP
	if (nImageType == CVLIB_IMG_FORMAT_BMP)
	{
		CoImageBMP newImage;
		if (newImage.Decode (image, pFile))
			return true;
		else {
			pFile->seek (pos, SEEK_SET);
			return DecodeUnknown(image, pFile, CVLIB_IMG_FORMAT_UNKNOWN);
		}
	}
#endif

#ifndef CVLIB_IMG_NOCODEC

#if CVLIB_IMG_SUPPORT_JPG
	if (nImageType == CVLIB_IMG_FORMAT_JPG)
	{
		CoImageJPG newImage;
		if (newImage.Decode (image, pFile))
			return true;
		else {
			pFile->seek (pos, SEEK_SET);
			return DecodeUnknown(image, pFile, CVLIB_IMG_FORMAT_UNKNOWN);
		}
	}
#endif
#if CVLIB_IMG_SUPPORT_PNG
	if (nImageType == CVLIB_IMG_FORMAT_PNG)
	{
		CoImagePNG newImage;
		if (newImage.Decode (image, pFile))
			return true;
		else {
			pFile->seek (pos, SEEK_SET);
			return DecodeUnknown(image, pFile, CVLIB_IMG_FORMAT_UNKNOWN);
		}
	}
#endif
#if CVLIB_IMG_SUPPORT_TIF
	if (nImageType == CVLIB_IMG_FORMAT_TIF)
	{
		CoImageTIF newImage;
		if (newImage.Decode (image, pFile))
			return true;
		else {
			pFile->seek (pos, SEEK_SET);
			return DecodeUnknown(image, pFile, CVLIB_IMG_FORMAT_UNKNOWN);
		}
	}
#endif
#if CVLIB_IMG_SUPPORT_JAS
	if (nImageType == CVLIB_IMG_FORMAT_JP2 ||
		nImageType == CVLIB_IMG_FORMAT_JPC ||
		nImageType == CVLIB_IMG_FORMAT_PGX ||
		nImageType == CVLIB_IMG_FORMAT_PNM ||
		nImageType == CVLIB_IMG_FORMAT_RAS )
	{
		CoImageJAS newImage;
		if (newImage.Decode (image, pFile, nImageType))
			return true;
		else {
			pFile->seek (pos, SEEK_SET);
			return DecodeUnknown(image, pFile, CVLIB_IMG_FORMAT_UNKNOWN);
		}
	}
#endif
#if CVLIB_IMG_SUPPORT_J2K
	if (nImageType == CVLIB_IMG_FORMAT_J2K)
	{
		CoImageJ2K newImage;
		if (newImage.Decode(image, pFile))
			return true;
		else {
			pFile->seek(pos, SEEK_SET);
			return DecodeUnknown(image, pFile, CVLIB_IMG_FORMAT_UNKNOWN);
		}
	}
#endif

#endif //CVLIB_IMG_NOCODEC

	return false;
}

bool CoImage::Encode(const Mat& image, uchar* &buffer, int &size, ulong imagetype) const
{
	XFileMem xfile(buffer, size);
	return Encode(image, &xfile, imagetype);
}
bool CoImage::Encode(const Mat& image, XFile * pFile, ulong imagetype) const
{
#if CVLIB_IMG_SUPPORT_BMP
	if (imagetype == CVLIB_IMG_FORMAT_BMP)
	{
		CoImageBMP newImage;
		if (newImage.Encode (image, pFile))
			return true;
		else
			return false;
	}
#endif

#ifndef CVLIB_IMG_NOCODEC

#if CVLIB_IMG_SUPPORT_JPG
	if (imagetype == CVLIB_IMG_FORMAT_JPG)
	{
		CoImageJPG newImage;
		if (newImage.Encode (image, pFile))
			return true;
		else
			return false;
	}
#endif
#if CVLIB_IMG_SUPPORT_PNG
	if (imagetype == CVLIB_IMG_FORMAT_PNG)
	{
		CoImagePNG newImage;
		if (newImage.Encode (image, pFile))
			return true;
		else
			return false;
	}
#endif
#if CVLIB_IMG_SUPPORT_TIF
	if (imagetype == CVLIB_IMG_FORMAT_TIF)
	{
		CoImageTIF newImage;
		if (newImage.Encode (image, pFile))
			return true;
		else
			return false;
	}
#endif	
#if CVLIB_IMG_SUPPORT_JAS
	if (imagetype == CVLIB_IMG_FORMAT_JP2 ||
		imagetype == CVLIB_IMG_FORMAT_JPC ||
		imagetype == CVLIB_IMG_FORMAT_PGX ||
		imagetype == CVLIB_IMG_FORMAT_PNM ||
		imagetype == CVLIB_IMG_FORMAT_RAS )
	{
		CoImageJAS newImage;
		if (newImage.Encode (image, pFile, imagetype))
			return true;
		else
			return false;
	}
#endif	
#if CVLIB_IMG_SUPPORT_J2K
	if (imagetype == CVLIB_IMG_FORMAT_J2K)
	{
		CoImageJ2K newImage;
		if (newImage.Encode(image, pFile))
			return true;
		else
			return false;
	}
#endif	

#endif //CVLIB_IMG_NOCODEC

	return false;
}

}

#ifndef CVLIB_NO_PRAGMA

#if CVLIB_OS == CVLIB_OS_WIN32
#ifndef CVLIB_IMG_NOCODEC

#ifdef NDEBUG

#if CVLIB_IMG_SUPPORT_J2K
#pragma comment(lib,"j2k.lib")
#endif //CVLIB_IMG_SUPPORT_J2K
#if CVLIB_IMG_SUPPORT_JAS
#pragma comment(lib,"jasper.lib")
#endif //CVLIB_IMG_SUPPORT_JAS
#if CVLIB_IMG_SUPPORT_JBG
#pragma comment(lib,"jbig.lib")
#endif //CVLIB_IMG_SUPPORT_JBG
#if CVLIB_IMG_SUPPORT_JPG
#pragma comment(lib,"jpeg.lib")
#endif //CVLIB_IMG_SUPPORT_JPG
#if CVLIB_IMG_SUPPORT_PNG
#pragma comment(lib,"png.lib")
#endif //CVLIB_IMG_SUPPORT_PNG
#if CVLIB_IMG_SUPPORT_TIF
#pragma comment(lib,"tiff.lib")
#endif //CVLIB_IMG_SUPPORT_TIF
#if CVLIB_IMG_SUPPORT_PNG || CVLIB_IMG_SUPPORT_TIF
#pragma comment(lib,"zlib.lib")
#endif //CVLIB_IMG_SUPPORT_PNG || CVLIB_IMG_SUPPORT_TIF

#else //DEBUG

#if CVLIB_IMG_SUPPORT_J2K
#pragma comment(lib,"j2kd.lib")
#endif //CVLIB_IMG_SUPPORT_J2K
#if CVLIB_IMG_SUPPORT_JAS
#pragma comment(lib,"jasperd.lib")
#endif //CVLIB_IMG_SUPPORT_JAS
#if CVLIB_IMG_SUPPORT_JBG
#pragma comment(lib,"jbigd.lib")
#endif //CVLIB_IMG_SUPPORT_JBG
#if CVLIB_IMG_SUPPORT_JPG
#pragma comment(lib,"jpegd.lib")
#endif //CVLIB_IMG_SUPPORT_JPG
#if CVLIB_IMG_SUPPORT_PNG
#pragma comment(lib,"pngd.lib")
#endif //CVLIB_IMG_SUPPORT_PNG
#if CVLIB_IMG_SUPPORT_TIF
#pragma comment(lib,"tiffd.lib")
#endif //CVLIB_IMG_SUPPORT_TIF
#if CVLIB_IMG_SUPPORT_PNG || CVLIB_IMG_SUPPORT_TIF
#pragma comment(lib,"zlibd.lib")
#endif //CVLIB_IMG_SUPPORT_PNG || CVLIB_IMG_SUPPORT_TIF

#endif //NDEBUG

#endif //CVLIB_IMG_NOCODEC
#endif // CVLIB_OS

#endif
