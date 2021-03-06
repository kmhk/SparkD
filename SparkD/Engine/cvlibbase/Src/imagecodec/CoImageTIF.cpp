
/*! 
 * \file	CoImageTIF.cpp
 * \ingroup base
 * \brief	TIFF
 * \author
 */
#include "cvlibmacros.h"

#ifdef _MSC_VER
#include <windows.h>
#include "TCHAR.h"
#endif

#include "CoImageTIF.h"

#ifndef CVLIB_IMG_NOCODEC

#if CVLIB_IMG_SUPPORT_TIF

#include <stdio.h>
#if CVLIB_OS==CVLIB_OS_WIN32
#include <io.h>
#elif defined(linux) || defined(__linux) || defined(__CYGWIN__)
//#include <sys/io.h>
#endif
#include <sys/stat.h>

#include "cvlibbaseDef.h"
#include "XFileDisk.h"
#include "ImageIter.h"

#include "tiffiop.h"

#define CVT(x)			(((x) * 255L) / ((1L<<16)-1))
#define	SCALE(x)		(((x)*((1L<<16)-1))/255)
#define CalculateLine(width,bitdepth)	(((width * bitdepth) + 7) / 8)
#define CalculatePitch(line)	(line + 3 & ~3)

#ifdef __GNUC__
extern	char* malloc();
extern	char* realloc();
#else
#include <malloc.h>
#endif

tdata_t
_TIFFmalloc(tsize_t s)
{
	return (malloc((size_t) s));
}

void
_TIFFfree(tdata_t p)
{
	free(p);
}

tdata_t
_TIFFrealloc(tdata_t p, tsize_t s)
{
	return (realloc(p, (size_t) s));
}

void
_TIFFmemset(tdata_t p, int v, tsize_t c)
{
	memset(p, v, (size_t) c);
}

void
_TIFFmemcpy(tdata_t d, const tdata_t s, tsize_t c)
{
	memcpy(d, s, (size_t) c);
}

int
_TIFFmemcmp(const tdata_t p1, const tdata_t p2, tsize_t c)
{
	return (memcmp(p1, p2, (size_t) c));
}

static void
Win32WarningHandler(const char* module, const char* fmt, va_list ap)
{
#ifdef _DEBUG
#if (!defined(_CONSOLE) && defined(WIN32))
#ifndef __QT__
    LPTSTR szTitle;
	LPTSTR szTmp;
	LPCTSTR szTitleText = "%s Warning";
	LPCTSTR szDefaultModule = "TIFFLIB";
	szTmp = (module == NULL) ? (LPTSTR)szDefaultModule : (LPTSTR)module;
	if ((szTitle = (LPTSTR)LocalAlloc(LMEM_FIXED, (lstrlen(szTmp) +
		lstrlen(szTitleText) + lstrlen(fmt) + 128)*sizeof(char))) == NULL)
		return;
	wsprintf(szTitle, szTitleText, szTmp);
	szTmp = szTitle + (lstrlen(szTitle)+2)*sizeof(char);
	wvsprintf(szTmp, fmt, ap);
	//MessageBox(GetFocus(), szTmp, szTitle, MB_OK | MB_ICONINFORMATION);
	LocalFree(szTitle);
#endif
	return;
#else
	if (module != NULL)
		fprintf(stderr, "%s: ", module);
	fprintf(stderr, "Warning, ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, ".\n");
#endif
#endif
}
TIFFErrorHandler _TIFFwarningHandler = Win32WarningHandler;

static void
Win32ErrorHandler(const char* module, const char* fmt, va_list ap)
{
#ifdef _DEBUG
#if (!defined(_CONSOLE) && defined(WIN32))
#ifndef __QT__
    LPTSTR szTitle;
	LPTSTR szTmp;
	LPCTSTR szTitleText = "%s Error";
	LPCTSTR szDefaultModule = "TIFFLIB";
	szTmp = (module == NULL) ? (LPTSTR)szDefaultModule : (LPTSTR)module;
	if ((szTitle = (LPTSTR)LocalAlloc(LMEM_FIXED, (lstrlen(szTmp) +
		lstrlen(szTitleText) + lstrlen(fmt) + 128)*sizeof(char))) == NULL)
		return;
	wsprintf(szTitle, szTitleText, szTmp);
	szTmp = szTitle + (lstrlen(szTitle)+2)*sizeof(char);
	wvsprintf(szTmp, fmt, ap);
//	MessageBox(GetFocus(), szTmp, szTitle, MB_OK | MB_ICONEXCLAMATION);
	char szlf[] = {0x0d, 0x0a, 0};
	printf (szTmp);
	printf (szlf);

	LocalFree(szTitle);
#endif
	return;
#else
	if (module != NULL)
		fprintf(stderr, "%s: ", module);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, ".\n");
#endif
#endif
}
TIFFErrorHandler _TIFFerrorHandler = Win32ErrorHandler;





namespace cvlib 
{

static tsize_t _tiffReadProcEx(thandle_t fd, tdata_t buf, tsize_t size)
{
	return (tsize_t)((XFile*)fd)->read(buf, 1, (size_t)size);
}

static tsize_t _tiffWriteProcEx(thandle_t fd, tdata_t buf, tsize_t size)
{
	return (tsize_t)((XFile*)fd)->write(buf, 1, (size_t)size);
}

static toff_t _tiffSeekProcEx(thandle_t fd, toff_t off, int whence)
{
	if ( off == 0xFFFFFFFF ) 
		return 0xFFFFFFFF;
	if (!((XFile*)fd)->seek(off, whence))
		return 0xFFFFFFFF;
	if (whence == SEEK_SET)
		return off;
	
	return (toff_t)((XFile*)fd)->tell();
}

// Return nonzero if error
static int _tiffCloseProcEx(thandle_t fd)
{
	return !((XFile*)fd)->close(); // "//" needed for memory files <DP>
//	return 0;
}


static toff_t _tiffSizeProcEx(thandle_t fd)
{
	return ((XFile*)fd)->size();
}

static int _tiffMapProcEx(thandle_t /*fd*/, tdata_t* /*pbase*/, toff_t* /*psize*/)
{
	return (0);
}

static void _tiffUnmapProcEx(thandle_t /*fd*/, tdata_t /*base*/, toff_t /*size*/)
{
}

TIFF* _TIFFFdOpen(intptr_t fd, const char* name, const char* mode)
{
	TIFF* tif;
	
	tif = TIFFClientOpen(name, mode,
		(thandle_t) fd,
		_tiffReadProcEx, _tiffWriteProcEx, _tiffSeekProcEx, _tiffCloseProcEx,
		_tiffSizeProcEx, _tiffMapProcEx, _tiffUnmapProcEx);
	if (tif)
		tif->tif_fd = (int)fd;
	return (tif);
}

extern "C" TIFF* _TIFFOpenEx(XFile* stream, const char* mode)
{
    return (_TIFFFdOpen((intptr_t)stream, "TIFF IMAGE", mode));
}

void CoImageTIF::TileToStrip(unsigned char* out, unsigned char* in,	unsigned int rows, unsigned int cols, int outskew, int inskew)
{
	while (rows-- > 0) 
	{
		uint32 j = cols;
		while (j-- > 0)
			*out++ = *in++;
		out += outskew;
		in += inskew;
	}
}

CoImageTIF::~CoImageTIF()
{
	if (m_tif2) TIFFClose((TIFF*)m_tif2);
}

bool CoImageTIF::Decode(Mat& image, XFile* pFile)
{
	TIFF* m_tif = _TIFFOpenEx(pFile, "rb");

	uint32 height = 0;
	uint32 width = 0;
	uint16 bitspersample = 1;
	uint16 samplesperpixel = 1;
	uint32 rowsperstrip = (ulong)-1;
	uint16 photometric = 0;
	uint16 compression = 1;
	uint16 orientation = ORIENTATION_TOPLEFT; //<vho>
	uint16 res_unit; //<Trifon>
	uint32 x, y;
	float resolution, offset;
	bool isRGB;
	uchar *bits;		//pointer to source data
	uchar *bits2;	//pointer to destination data

	try
	{
		//check if it's a tiff file
		if (!m_tif)
			throw "Error encountered while opening TIFF file";

		// <Robert Abram> - 12/2002 : get NumFrames directly, instead of looping
		// m_Info.nNumFrames=0;
		// while(TIFFSetDirectory(m_tif,(uint16)m_Info.nNumFrames)) m_Info.nNumFrames++;
		m_Info.nNumFrames = TIFFNumberOfDirectories(m_tif);

		if (!TIFFSetDirectory(m_tif, (uint16)m_Info.nFrame))
			throw "Error: page not present in TIFF file";

		//get image info
		TIFFGetField(m_tif, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(m_tif, TIFFTAG_IMAGELENGTH, &height);
		TIFFGetField(m_tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
		TIFFGetField(m_tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
		TIFFGetField(m_tif, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);
		TIFFGetField(m_tif, TIFFTAG_PHOTOMETRIC, &photometric);
		TIFFGetField(m_tif, TIFFTAG_ORIENTATION, &orientation);

		if (m_Info.nEscape == -1)
		{
			// Return output dimensions only
			m_Head.biWidth = width;
			m_Head.biHeight = height;
			throw "output dimensions returned";
		}

		TIFFGetFieldDefaulted(m_tif, TIFFTAG_RESOLUTIONUNIT, &res_unit);
		if (TIFFGetField(m_tif, TIFFTAG_XRESOLUTION, &resolution))
		{
			if (res_unit == RESUNIT_CENTIMETER) resolution = (float)(resolution*2.54f + 0.5f);
			SetXDPI((long)resolution);
		}
		if (TIFFGetField(m_tif, TIFFTAG_YRESOLUTION, &resolution))
		{
			if (res_unit == RESUNIT_CENTIMETER) resolution = (float)(resolution*2.54f + 0.5f);
			SetYDPI((long)resolution);
		}

		if (TIFFGetField(m_tif, TIFFTAG_XPOSITION, &offset))	m_Info.xOffset = (long)offset;
		if (TIFFGetField(m_tif, TIFFTAG_YPOSITION, &offset))	m_Info.yOffset = (long)offset;

		m_Head.biClrUsed = 0;
		m_Info.nBkgndIndex = -1;

		if (rowsperstrip > height)
		{
			rowsperstrip = height;
			TIFFSetField(m_tif, TIFFTAG_ROWSPERSTRIP, rowsperstrip);
		}

		isRGB = (bitspersample >= 8) &&
			(photometric == PHOTOMETRIC_RGB) ||
			(photometric == PHOTOMETRIC_YCBCR) ||
			(photometric == PHOTOMETRIC_SEPARATED) ||
			(photometric == PHOTOMETRIC_LOGL) ||
			(photometric == PHOTOMETRIC_LOGLUV);

		if (isRGB)
		{
			m_Head.biBitCount = 24;
		}
		else
		{
			if ((photometric == PHOTOMETRIC_MINISBLACK) || (photometric == PHOTOMETRIC_MINISWHITE))
			{
				if (bitspersample == 1) {
					m_Head.biBitCount = 1;		//B&W image
					m_Head.biClrUsed = 2;
				}
				else if (bitspersample == 4)
				{
					m_Head.biBitCount = 4;		//16 colors gray scale
					m_Head.biClrUsed = 16;
				}
				else
				{
					m_Head.biBitCount = 8;		//gray scale
					m_Head.biClrUsed = 256;
				}
			}
			else if (bitspersample == 4)
			{
				m_Head.biBitCount = 4;			// 16 colors
				m_Head.biClrUsed = 16;
			}
			else
			{
				m_Head.biBitCount = 8;			//256 colors
				m_Head.biClrUsed = 256;
			}
		}

		if (m_Info.nEscape)  throw "Cancelled"; // <vho> - cancel decoding

		CreateInfo(height, width, m_Head.biBitCount, CVLIB_IMG_FORMAT_TIF);
		TYPE type = MAT_Tbyte3;
#if CVLIB_IMG_SUPPORT_ALPHA
		if (samplesperpixel == 4)
			type = MAT_Tbyte4;
		if (samplesperpixel == 2 && bitspersample == 8)
			type = MAT_Tbyte4;
#endif //CVLIB_IMG_SUPPORT_ALPHA
		image.create(height, width, type);
		if (m_Info.pImage == NULL)
			m_Info.pImage = (uchar*)malloc(m_Head.biSizeImage);

		//if (!GetPalette())
		//	throw "CoImageTIF can't create image";


		TIFFGetField(m_tif, TIFFTAG_COMPRESSION, &compression);
		SetCodecOption(compression, CVLIB_IMG_FORMAT_TIF); // <DPR> save original compression type

		if (isRGB)
		{
			// fromFile the whole image into one big RGBA buffer using
			// the traditional TIFFReadRGBAImage() API that we trust.
			uint32* raster;		// retrieve RGBA image
			uint32 *row;

			raster = (uint32*)_TIFFmalloc(width * height * sizeof(uint32));
			if (raster == NULL) throw "No space for raster buffer";

			// fromFile the image in one chunk into an RGBA array
			if (!TIFFReadRGBAImage(m_tif, width, height, raster, 1))
			{
				_TIFFfree(raster);
				throw "Corrupted TIFF file!";
			}

			// read the raster lines and save them in the DIB
			// with COLOR mode, we have to change the order of the 3 samples COLOR
			row = &raster[0];
			bits2 = m_Info.pImage;
			for (y = 0; y < height; y++)
			{

				if (m_Info.nEscape)
				{ // <vho> - cancel decoding
					_TIFFfree(raster);
					throw "Cancelled";
				}

				bits = bits2;
				for (x = 0; x < width; x++)
				{
					*bits++ = (uchar)TIFFGetR(row[x]);
					*bits++ = (uchar)TIFFGetG(row[x]);
					*bits++ = (uchar)TIFFGetB(row[x]);
#if CVLIB_IMG_SUPPORT_ALPHA
					if (samplesperpixel == 4)
						image.data.ptr[y][x * 4 + 3] = (uchar)TIFFGetA(row[x]);
#endif //CVLIB_IMG_SUPPORT_ALPHA
				}
				row += width;
				bits2 += m_Info.dwEffWidth;
			}
			_TIFFfree(raster);
		}
		else
		{
			COLOR *pal;
			pal = (COLOR*)calloc(256, sizeof(COLOR));
			if (pal == NULL) throw "Unable to allocate TIFF palette";

			// set up the colormap based on photometric	
			switch (photometric)
			{
			case PHOTOMETRIC_MINISBLACK:	// bitmap and greyscale image types
			case PHOTOMETRIC_MINISWHITE:
				if (bitspersample == 1)
				{	// Monochrome image
					if (photometric == PHOTOMETRIC_MINISBLACK)
						pal[1].x = pal[1].y = pal[1].z = 255;
					else
						pal[0].x = pal[0].y = pal[0].z = 255;
				}
				else
				{		// need to build the scale for greyscale images
					if (photometric == PHOTOMETRIC_MINISBLACK)
					{
						for (ulong i = 0; i < m_Head.biClrUsed; i++)
							pal[i].x = pal[i].y = pal[i].z = (uchar)(i*(255 / (m_Head.biClrUsed - 1)));
					}
					else
					{
						for (ulong i = 0; i < m_Head.biClrUsed; i++)
							pal[i].x = pal[i].y = pal[i].z = (uchar)(255 - i*(255 / (m_Head.biClrUsed - 1)));
					}
				}
				break;
			case PHOTOMETRIC_PALETTE:	// color map indexed
				uint16 *red;
				uint16 *green;
				uint16 *blue;
				TIFFGetField(m_tif, TIFFTAG_COLORMAP, &red, &green, &blue);

				// Is the palette 16 or 8 bits ?
				bool Palette16Bits = false;
				int n = 1 << bitspersample;
				while (n-- > 0) {
					if (red[n] >= 256 || green[n] >= 256 || blue[n] >= 256)
					{
						Palette16Bits = TRUE;
						break;
					}
				}

				// load the palette in the DIB
				for (int i = (1 << bitspersample) - 1; i >= 0; i--)
				{
					if (Palette16Bits)
					{
						pal[i].x = (uchar)CVT(red[i]);
						pal[i].y = (uchar)CVT(green[i]);
						pal[i].z = (uchar)CVT(blue[i]);
					}
					else
					{
						pal[i].x = (uchar)red[i];
						pal[i].y = (uchar)green[i];
						pal[i].z = (uchar)blue[i];
					}
				}
				break;
			}
			SetPalette(pal, m_Head.biClrUsed);	//palette assign
			free(pal);

			// read the tiff lines and save them in the DIB
			uint32 nrow;
			uint32 ys;
			int line = CalculateLine(width, bitspersample * samplesperpixel);
			long bitsize = TIFFStripSize(m_tif);
			//verify bitsize: could be wrong if StripByteCounts is missing.
			if (bitsize > (long)(m_Head.biSizeImage*samplesperpixel))
				bitsize = m_Head.biSizeImage*samplesperpixel;

			int tiled_image = TIFFIsTiled(m_tif);
			uint32 tw = 0, tl;
			uchar* tilebuf = 0;
			if (tiled_image)
			{
				TIFFGetField(m_tif, TIFFTAG_TILEWIDTH, &tw);
				TIFFGetField(m_tif, TIFFTAG_TILELENGTH, &tl);
				rowsperstrip = tl;
				bitsize = TIFFTileSize(m_tif) * (int)(1 + width / tw);
				tilebuf = (uchar*)malloc(TIFFTileSize(m_tif));
			}

			bits = (uchar*)malloc(bitsize);
			if (bits == NULL)
			{
				throw "CoImageTIF can't allocate memory";
			}

			for (ys = 0; ys < height; ys += rowsperstrip)
			{
				if (m_Info.nEscape)
				{ // <vho> - cancel decoding
					free(bits);
					throw "Cancelled";
				}

				nrow = (ys + rowsperstrip > height ? height - ys : rowsperstrip);

				if (tiled_image)
				{
					uint32 imagew = TIFFScanlineSize(m_tif);
					uint32 tilew = TIFFTileRowSize(m_tif);
					int iskew = imagew - tilew;
					uint8* bufp = (uint8*)bits;

					uint32 colb = 0;
					for (uint32 col = 0; col < width; col += tw)
					{
						if (TIFFReadTile(m_tif, tilebuf, col, ys, 0, 0) < 0)
						{
							free(tilebuf);
							free(bits);
							throw "Corrupted tiled TIFF file!";
						}

						if (colb + tw > imagew)
						{
							uint32 owidth = imagew - colb;
							uint32 oskew = tilew - owidth;
							TileToStrip(bufp + colb, tilebuf, nrow, owidth, oskew + iskew, oskew);
						}
						else
						{
							TileToStrip(bufp + colb, tilebuf, nrow, tilew, iskew, 0);
						}
						colb += tilew;
					}

				}
				else
				{
					if (TIFFReadEncodedStrip(m_tif, TIFFComputeStrip(m_tif, ys, 0), bits, nrow * line) == -1)
					{
						free(bits);
						throw "Corrupted TIFF file!";
					}
				}

				for (y = 0; y < nrow; y++)
				{
					long offset2 = (nrow - y - 1)*line;
					if (bitspersample == 16)
						for (ulong xi = 0; xi < width; xi++)
							bits[xi + offset2] = bits[xi * 2 + offset2 + 1];
					if (samplesperpixel == 1)
					{ //simple 8bpp image
						memcpy(
							m_Info.pImage + m_Info.dwEffWidth*(height - ys - nrow + y),
							bits + offset2,
							m_Info.dwEffWidth);
					}
					else if (samplesperpixel == 2)  //8bpp image with alpha layer
					{
						int xi = 0;
						int ii = 0;
						int yi = height - ys - nrow + y;
						while (ii < line)
						{
							SetPixelIndex(xi, yi, bits[ii + offset2]);
#if CVLIB_IMG_SUPPORT_ALPHA
							image.data.ptr[yi][xi * 4 + 3] = bits[ii + offset2 + 1];
#endif //CVLIB_IMG_SUPPORT_ALPHA
							ii += 2;
							xi++;
							if (xi >= (int)width)
							{
								yi--;
								xi = 0;
							}
						}
					}
					else
					{ //photometric==PHOTOMETRIC_CIELAB
						int cn = 3;
						if (m_Head.biBitCount != 24)
						{ //fix image
							CreateInfo(height, width, 24, CVLIB_IMG_FORMAT_BMP);
#if CVLIB_IMG_SUPPORT_ALPHA
							if (samplesperpixel == 4)
								image.create(height, width, MAT_Tbyte4);
							else
#endif //CVLIB_IMG_SUPPORT_ALPHA
								image.create(height, width, MAT_Tbyte3);
							cn = image.channels();
						}

						int xi = 0;
						int ii = 0;
						int yi = height - ys - nrow + y;
						int l, a, b, bitsoffset;
						double p, cx, cy, cz, cr, cg, cb;
						while (ii < line) {
							bitsoffset = ii*samplesperpixel + offset2;
							l = bits[bitsoffset];
							a = bits[bitsoffset + 1];
							b = bits[bitsoffset + 2];
							if (a > 127) a -= 256;
							if (b > 127) b -= 256;
							// lab to xyz
							p = (l / 2.55 + 16) / 116.0;
							cx = pow(p + a * 0.002, 3);
							cy = pow(p, 3);
							cz = pow(p - b * 0.005, 3);
							// white point
							cx *= 0.95047;
							//cy*=1.000;
							cz *= 1.0883;
							// xyz to rgb
							cr = 3.240479 * cx - 1.537150 * cy - 0.498535 * cz;
							cg = -0.969256 * cx + 1.875992 * cy + 0.041556 * cz;
							cb = 0.055648 * cx - 0.204043 * cy + 1.057311 * cz;

							if (cr > 0.00304) cr = 1.055 * pow(cr, 0.41667) - 0.055;
							else            cr = 12.92 * cr;
							if (cg > 0.00304) cg = 1.055 * pow(cg, 0.41667) - 0.055;
							else            cg = 12.92 * cg;
							if (cb > 0.00304) cb = 1.055 * pow(cb, 0.41667) - 0.055;
							else            cb = 12.92 * cb;

							image.data.ptr[yi][xi*cn + 0] = (uchar)MAX(0, MIN(255, (int)(cr * 255)));
							image.data.ptr[yi][xi*cn + 1] = (uchar)MAX(0, MIN(255, (int)(cg * 255)));
							image.data.ptr[yi][xi*cn + 2] = (uchar)MAX(0, MIN(255, (int)(cb * 255)));
#if CVLIB_IMG_SUPPORT_ALPHA
							if (samplesperpixel == 4)
								image.data.ptr[yi][xi*cn + 3] = bits[bitsoffset + 3];
#endif //CVLIB_IMG_SUPPORT_ALPHA
							ii++;
							xi++;
							if (xi >= (int)width)
							{
								yi--;
								xi = 0;
							}
						}
					}
				}
			}
			free(bits);
			if (tiled_image) free(tilebuf);

			// 		switch(orientation)
			// 		{
			// 		case ORIENTATION_TOPRIGHT: /* row 0 top, col 0 rhs */
			// 			Mirror();
			// 			break;
			// 		case ORIENTATION_BOTRIGHT: /* row 0 bottom, col 0 rhs */
			// 			Flip();
			// 			Mirror();
			// 			break;
			// 		case ORIENTATION_BOTLEFT: /* row 0 bottom, col 0 lhs */
			// 			Flip();
			// 			break;
			// 		case ORIENTATION_LEFTTOP: /* row 0 lhs, col 0 top */
			// 			RotateRight();
			// 			Mirror();
			// 			break;
			// 		case ORIENTATION_RIGHTTOP: /* row 0 rhs, col 0 top */
			// 			RotateLeft();
			// 			break;
			// 		case ORIENTATION_RIGHTBOT: /* row 0 rhs, col 0 bottom */
			// 			RotateLeft();
			// 			Mirror();
			// 			break;
			// 		case ORIENTATION_LEFTBOT: /* row 0 lhs, col 0 bottom */
			// 			RotateRight();
			// 			break;
			// 		}

		}
	}
	catch (char *message)
	{
		strncpy(m_Info.szLastError, message, 255);
		if (m_tif) TIFFClose(m_tif);
		if (m_Info.nEscape == -1) return true;
		return false;
	}
	TIFFClose(m_tif);

	ImageIterator iter(&image, this);
	iter.BMP2XYZ(m_Info.pImage);

	if (m_Info.pImage)
		free(m_Info.pImage);
	m_Info.pImage = NULL;
	return true;
}

#if CVLIB_IMG_SUPPORT_ENCODE
bool CoImageTIF::Encode(const Mat& image, XFile* pFile, bool bAppend)
{
	try
	{
		if (pFile==NULL) throw CVLIB_IMG_ERR_NOFILE;
// 		if (GetPalette() == NULL) throw CVLIB_IMG_ERR_NOIMAGE;
		
		// <RJ> replaced "w+b" with "a", to append an image directly on an existing file
		if (m_tif2==NULL) m_tif2 =(void*)_TIFFOpenEx(pFile, "a");
		if (m_tif2==NULL) throw "initialization fail";
		
		if (bAppend || m_pages) m_multipage=true;
		m_pages++;
		
		if (!EncodeBody(image, m_tif2,m_multipage,m_pages,m_pages)) throw "Error saving TIFF file";
		if (bAppend) 
		{
			if (!TIFFWriteDirectory((TIFF*)m_tif2)) throw "Error saving TIFF directory";
		}
	} 
	catch (char *message) 
	{
		strncpy(m_Info.szLastError,message,255);
		if (m_tif2)
		{
			TIFFClose((TIFF*)m_tif2);
			m_tif2=NULL;
			m_multipage=false;
			m_pages=0;
		}
		return false;
	}
	if (!bAppend)
	{
		TIFFClose((TIFF*)m_tif2);
		m_tif2=NULL;
		m_multipage=false;
		m_pages=0;
	}
	return true;
}

bool CoImageTIF::EncodeBody(const Mat& image, void* m_tif, bool multipage, int page, int pagecount)
{
	TIFF* pTIF = (TIFF*)m_tif;

	SetBpp24(image.rows(), image.cols());
	uint32 height=m_Head.biHeight;
	uint32 width=m_Head.biWidth;
	uint16 bitcount=m_Head.biBitCount;
	uint16 bitspersample;
	uint16 samplesperpixel;
	uint16 photometric=0;
	uint16 compression;
//	uint16 pitch;
//	int line;
	uint32 x, y;

	samplesperpixel = ((bitcount == 24) || (bitcount == 32)) ? (uchar)3 : (uchar)1;
#if CVLIB_IMG_SUPPORT_ALPHA
	if (image.channels() == 4) { bitcount=32; samplesperpixel=4; }
#endif //CVLIB_IMG_SUPPORT_ALPHA

	bitspersample = bitcount / samplesperpixel;

// 	if (m_Info.pImage == NULL)
// 		m_Info.pImage = (uchar*)malloc (m_Head.biSizeImage);
	ImageIterator iter((Mat*)&image, this);
	iter.XYZ2BMP();

	//set the PHOTOMETRIC tag
//	COLOR *rgb = GetPalette();
	switch (bitcount) 
	{
// 		case 1:
// 			if (CompareColors(&rgb[0],&rgb[1])<0) 
// 			{
// 				/* <abe> some viewers do not handle PHOTOMETRIC_MINISBLACK:
// 				 * let's transform the image in PHOTOMETRIC_MINISWHITE
// 				 */
// 				//invert the colors
// 				COLOR tempRGB=GetPaletteColor(0);
// 				SetPaletteColor(0,GetPaletteColor(1));
// 				SetPaletteColor(1,tempRGB);
// 				//invert the pixels
// 				uchar *iSrc=iter.m_pbTemp;/*m_Info.pImage*/;
// 				for (unsigned long i=0;i<m_Head.biSizeImage;i++)
// 				{
// 					*iSrc=(uchar)~(*(iSrc));
// 					iSrc++;
// 				}
// 				photometric = PHOTOMETRIC_MINISWHITE;
// 				//photometric = PHOTOMETRIC_MINISBLACK;
// 			} 
// 			else
// 			{
// 				photometric = PHOTOMETRIC_MINISWHITE;
// 			}
// 			break;
// 		case 4:	// Check if the DIB has a color or a greyscale palette
// 		case 8:
// 			photometric = PHOTOMETRIC_MINISBLACK; //default to gray scale
// 			for (x = 0; x < m_Head.biClrUsed; x++) 
// 			{
// 				if ((rgb->x != x)||(rgb->x != rgb->y)||(rgb->x != rgb->z))
// 				{
// 					photometric = PHOTOMETRIC_PALETTE;
// 					break;
// 				}
// 				rgb++;
// 			}
// 			break;
		case 24:
		case 32:
			photometric = PHOTOMETRIC_RGB;			
			break;
	}

#if CVLIB_IMG_SUPPORT_ALPHA
	if (image.channels()==4 && bitcount==8) samplesperpixel=2; //8bpp + alpha layer
#endif //CVLIB_IMG_SUPPORT_ALPHA

//	line = CalculateLine(width, bitspersample * samplesperpixel);
//	pitch = (uint16)CalculatePitch(line);

	//prepare the palette struct
// 	COLOR pal[256];
// 	if (GetPalette())
// 	{
// 		uchar b;
// 		memcpy(pal,GetPalette(),GetPaletteSize());
// 		for(ushort a=0;a<m_Head.biClrUsed;a++){	//swap blue and red components
// 			b=pal[a].z; pal[a].z=pal[a].x; pal[a].x=b;
// 		}
// 	}

	// handle standard width/height/bpp stuff
	TIFFSetField(pTIF, TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(pTIF, TIFFTAG_IMAGELENGTH, height);
	TIFFSetField(pTIF, TIFFTAG_SAMPLESPERPIXEL, samplesperpixel);
	TIFFSetField(pTIF, TIFFTAG_BITSPERSAMPLE, bitspersample);
	TIFFSetField(pTIF, TIFFTAG_PHOTOMETRIC, photometric);
	TIFFSetField(pTIF, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);	// single image plane 
	TIFFSetField(pTIF, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

	uint32 rowsperstrip = TIFFDefaultStripSize(pTIF, (uint32) -1);  //<REC> gives better compression
	TIFFSetField(pTIF, TIFFTAG_ROWSPERSTRIP, rowsperstrip);

	// handle metrics
	TIFFSetField(pTIF, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
	TIFFSetField(pTIF, TIFFTAG_XRESOLUTION, (float)m_Info.xDPI);
	TIFFSetField(pTIF, TIFFTAG_YRESOLUTION, (float)m_Info.yDPI);
//	TIFFSetField(pTIF, TIFFTAG_XPOSITION, (float)m_Info.xOffset);
//	TIFFSetField(pTIF, TIFFTAG_YPOSITION, (float)m_Info.yOffset);

	// multi-paging - Thanks to Abe <God(dot)bless(at)marihuana(dot)com>
	if (multipage)
	{
		char page_number[20];
		sprintf(page_number, "Page %d", page);

		TIFFSetField(pTIF, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
		TIFFSetField(pTIF, TIFFTAG_PAGENUMBER, page,pagecount);
		TIFFSetField(pTIF, TIFFTAG_PAGENAME, page_number);
	}
	else 
	{
		TIFFSetField(pTIF, TIFFTAG_SUBFILETYPE, 0);
	}

	// palettes (image colormaps are automatically scaled to 16-bits)
// 	if (photometric == PHOTOMETRIC_PALETTE) 
// 	{
// 		uint16 *r, *g, *b;
// 		r = (uint16 *) _TIFFmalloc(sizeof(uint16) * 3 * 256);
// 		g = r + 256;
// 		b = g + 256;
// 
// 		for (int i = 255; i >= 0; i--) 
// 		{
// 			b[i] = (uint16)SCALE((uint16)pal[i].x);
// 			g[i] = (uint16)SCALE((uint16)pal[i].y);
// 			r[i] = (uint16)SCALE((uint16)pal[i].z);
// 		}
// 
// 		TIFFSetField(pTIF, TIFFTAG_COLORMAP, r, g, b);
// 		_TIFFfree(r);
// 	}

	// compression
	if (GetCodecOption(CVLIB_IMG_FORMAT_TIF)) 
	{
		compression = (ushort)GetCodecOption(CVLIB_IMG_FORMAT_TIF);
	}
	else 
	{
		switch (bitcount) 
		{
			case 1 :
				compression = COMPRESSION_CCITTFAX4;
				break;
			case 4 :
			case 8 :
				compression = COMPRESSION_LZW;
				break;
			case 24 :
			case 32 :
				compression = COMPRESSION_JPEG;
				break;
			default :
				compression = COMPRESSION_NONE;
				break;
		}
	}
	TIFFSetField(pTIF, TIFFTAG_COMPRESSION, compression);

	switch (compression) 
	{
	case COMPRESSION_JPEG:
		TIFFSetField(pTIF, TIFFTAG_JPEGQUALITY, m_Info.nQuality);
		TIFFSetField(pTIF, TIFFTAG_ROWSPERSTRIP, ((7+rowsperstrip)>>3)<<3);
   		break;
	case COMPRESSION_LZW:
		if (bitcount>=8) TIFFSetField(pTIF, TIFFTAG_PREDICTOR, 2);
		break;
	}

	// read the DIB lines from bottom to top and save them in the TIF

	uchar *bits;
	switch(bitcount) 
	{				
		case 1 :
		case 4 :
		case 8 :
		{
			if (samplesperpixel==1)
			{
				for (y = 0; y < height; y++) 
				{
					bits= iter.m_pbTemp + (height - y - 1)*m_Info.dwEffWidth;
					if (TIFFWriteScanline(pTIF,bits, y, 0)==-1) return false;
				}
			}
#if CVLIB_IMG_SUPPORT_ALPHA
			else 
			{ //8bpp + alpha layer
				bits = (uchar*)malloc(2*width);
				if (!bits) return false;
				for (y = 0; y < height; y++) 
				{
					for (x=0;x<width;x++)
					{
						bits[2*x]=GetPixelIndex(x,height - y - 1);
						bits[2*x+1]=image.data.ptr[height - y - 1][x*4+3];
					}
					if (TIFFWriteScanline(pTIF,bits, y, 0)==-1) 
					{
						free(bits);
						return false;
					}
				}
				free(bits);
			}
#endif //CVLIB_IMG_SUPPORT_ALPHA
			break;
		}				
		case 24:
		{
			uchar *buffer = (uchar *)malloc(m_Info.dwEffWidth);
			if (!buffer) return false;
			for (y = 0; y < height; y++) 
			{
				// get a pointer to the scanline
				memcpy(buffer, iter.m_pbTemp + (height - y - 1)*m_Info.dwEffWidth, m_Info.dwEffWidth);
				// TIFFs store color data COLOR instead of BGR
				uchar *pBuf = buffer;
				for (x = 0; x < width; x++) 
				{
					uchar tmp = pBuf[0];
					pBuf[0] = pBuf[2];
					pBuf[2] = tmp;
					pBuf += 3;
				}
				// write the scanline to disc
				if (TIFFWriteScanline(pTIF, buffer, y, 0)==-1)
				{
					free(buffer);
					return false;
				}
			}
			free(buffer);
			break;
		}				
		case 32 :
		{
#if CVLIB_IMG_SUPPORT_ALPHA
			uchar *buffer = (uchar *)malloc((m_Info.dwEffWidth*4)/3);
			if (!buffer) return false;
			for (y = 0; y < height; y++) 
			{
				// get a pointer to the scanline
				memcpy(buffer, iter.m_pbTemp + (height - y - 1)*m_Info.dwEffWidth, m_Info.dwEffWidth);
				// TIFFs store color data COLOR instead of BGR
				uchar *pSrc = buffer + 3 * width;
				uchar *pDst = buffer + 4 * width;
				for (x = 0; x < width; x++) 
				{
					pDst-=4;
					pSrc-=3;
					pDst[3] = image.data.ptr[height-y-1][(width-x-1)*4+3];
					pDst[2] = pSrc[0];
					pDst[1] = pSrc[1];
					pDst[0] = pSrc[2];
				}
				// write the scanline to disc
				if (TIFFWriteScanline(pTIF, buffer, y, 0)==-1)
				{
					free(buffer);
					return false;
				}
			}
			free(buffer);
#endif //CVLIB_IMG_SUPPORT_ALPHA
			break;
		}				
	}
	return true;
}
#endif // CVLIB_IMG_SUPPORT_ENCODE

}

#endif //CVLIB_IMG_SUPPORT_TIF

#endif //CVLIB_IMG_NOCODEC
