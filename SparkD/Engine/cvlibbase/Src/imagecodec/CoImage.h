/*! 
 * \file    CoImage.h
 * \ingroup base
 * \brief   
 * \author  
 */
#pragma once

#include "cvlibbaseDef.h"	//<vho> adjust some #define
#include "Mat.h"
#include "String.hpp"
#include "cvlibstructs.h"
#include "XFileMem.h"
#include "cvlibformats.h"

namespace cvlib
{

	/**
	 * @brief
	 */
	class CoImage
	{
	public:

		/**
		 * @brief
		 */
		typedef struct tagCoImageInfo
		{
			uint	dwEffWidth;
			uchar*	pImage;
			CoImage* pGhost;
			CoImage* pParent;
			uint	dwType;
			char	szLastError[256];
			long	nProgress;
			long	nEscape;
			long	nBkgndIndex;
			COLOR nBkgndColor;
			uchar	nQuality;
			uchar	nJpegScale;
			long	nFrame;
			long	nNumFrames;
			ulong	dwFrameDelay;
			long	xDPI;
			long	yDPI;
			//		RECT	rSelectionBox;
			uchar	nAlphaMax;
			bool	bAlphaPaletteEnabled;
			bool	bEnabled;
			long	xOffset;
			long	yOffset;
			ulong	dwCodecOpt[CVLIB_IMG_MAX_FORMATS];
			COLOR last_c;
			uchar	last_c_index;
			bool	last_c_isvalid;
			long	nNumLayers;
			ulong	dwFlags;
		} COIMAGEINFO;

		/**
		 * @brief
		 */
		struct rgb_color
		{
			uchar r;  /**<  */
			uchar g;  /**<  */
			uchar b;  /**<  */
		};

		//! 
		enum
		{
			enGRAY_MODE = 1,
			enCOLOR_MODE = 3
		};

	public:
		//! Construction and Destruction
		CoImage();
		virtual ~CoImage();

		bool	Decode(Mat& image, XFile* pFile, ulong nImageType);
		bool	Decode(Mat& image, unsigned char* szbuf, int nsize, ulong nImageType);
		bool	Encode(const Mat& image, XFile * hFile, ulong imagetype) const;
		bool	Encode(const Mat& image, uchar* &buffer, int &size, ulong imagetype) const;

		int		GetEffWidth() const { return m_Info.dwEffWidth; }
		long	GetXDPI() const;
		long	GetYDPI() const;
		void	SetXDPI(long dpi);
		void	SetYDPI(long dpi);

		uchar	GetJpegScale() const;
		uchar	GetJpegQuality() const;
		void	SetJpegQuality(uchar q);
		void	InitBMPHeader(int nW, int nH);
		void	Startup(ulong imagetype);
		ulong	GetCodecOption(ulong imagetype) const;
		bool	SetCodecOption(ulong opt, ulong imagetype);

		void	SetPaletteColor(uchar idx, uchar r, uchar g, uchar b, uchar alpha = 0);
		void	SetPaletteColor(uchar idx, COLOR cr);
		bool	GetPaletteColor(uchar i, uchar* r, uchar* g, uchar* b) const;
		bool	GetPaletteColor(uchar i, uchar* rgb) const;
		COLOR	GetPaletteColor(uchar idx) const;

		void	SetPalette(ulong n, uchar *r, uchar *g, uchar *b);
		void	SetPalette(rgb_color *rgb, ulong nColors);
		void	SetPalette(COLOR* pPal, ulong nColors);
		void	SetGrayPalette();
		void	BlendPalette(COLOR cr, long perc) const;
		COLOR* GetPalette() const;
		int		CreateInfo(int nRows, int nCols, int wBpp, int nImageType);
		void	SetBpp24(int nh, int nw);

		void	SetPixelIndex(long x, long y, uchar i);
		uchar	GetPixelIndex(long x, long y) const;
		uchar	GetPixelIndex(uchar* pData, long x) const;

		void	setPixelColor(Mat& image, long x, long y, float* color);
		void	setPixelColor(Mat& image, long x, long y, COLOR c, bool bSetAlpha = false);
		COLOR getPixelColor(const Mat& image, long x, long y, bool fGetAlpha = true) const;

		void	RGBtoBGR(uchar *buffer, int length, int cn);
		uchar	GetNearestIndex(COLOR c);


		void	release();

		COIMAGEINFO	m_Info;
		BITMAPINFOHEADER1 m_Head;
	protected:
		ulong	GetPaletteSize() const;

		COLOR*	m_pPalette;

		bool	DecodeUnknown(Mat& image, XFile* pFile, ulong nImageType);
	};

}