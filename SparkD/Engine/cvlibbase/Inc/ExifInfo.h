#pragma once

//#include "CoImageCFG.h"
#include "cvlibbaseDef.h"	//<vho> adjust some #define

namespace cvlib
{

	//--------------------------------------------------------------------------
	// JPEG markers consist of one or more 0xFF bytes, followed by a marker
	// code byte (which is not an FF).  Here are the marker codes of interest
	// in this program.  (See jdmarker.c for a more complete list.)
	//--------------------------------------------------------------------------

#define M_SOF0  0xC0            // Start Of Frame N
#define M_SOF1  0xC1            // N indicates which compression process
#define M_SOF2  0xC2            // Only SOF0-SOF2 are now in common use
#define M_SOF3  0xC3
#define M_SOF5  0xC5            // NB: codes C4 and CC are NOT SOF markers
#define M_SOF6  0xC6
#define M_SOF7  0xC7
#define M_SOF9  0xC9
#define M_SOF10 0xCA
#define M_SOF11 0xCB
#define M_SOF13 0xCD
#define M_SOF14 0xCE
#define M_SOF15 0xCF
#define M_SOI   0xD8            // Start Of Image (beginning of datastream)
#define M_EOI   0xD9            // End Of Image (end of datastream)
#define M_SOS   0xDA            // Start Of Scan (begins compressed data)
#define M_JFIF  0xE0            // Jfif marker
#define M_EXIF  0xE1            // Exif marker
#define M_COM   0xFE            // COMment 

#define PSEUDO_IMAGE_MARKER 0x123; // Extra value.

#define EXIF_READ_EXIF  0x01
#define EXIF_READ_IMAGE 0x02
#define EXIF_READ_ALL   0x03


#define MAX_COMMENT 255
#define MAX_SECTIONS 20

	typedef struct tag_ExifInfo {
		char  Version[5];
		char  CameraMake[32];
		char  CameraModel[40];
		char  DateTime[20];
		int   Height, Width;
		int   Orientation;
		int   IsColor;
		int   process;
		int   FlashUsed;
		float FocalLength;
		float ExposureTime;
		float ApertureFNumber;
		float Distance;
		float CCDWidth;
		float ExposureBias;
		int   Whitebalance;
		int   MeteringMode;
		int   ExposureProgram;
		int   ISOequivalent;
		int   CompressionLevel;
		float FocalplaneXRes;
		float FocalplaneYRes;
		float FocalplaneUnits;
		float Xresolution;
		float Yresolution;
		float ResolutionUnit;
		float Brightness;
		char  Comments[MAX_COMMENT + 1];

		uchar * ThumbnailPointer;  /* Pointer at the thumbnail */
		unsigned ThumbnailSize;     /* Size of thumbnail. */

		bool  IsExif;
	} ExifInfo;

	class CVLIB_DECLSPEC CxExifInfo
	{

		typedef struct tag_Section_t {
			uchar*    data;
			int      type;
			unsigned Size;
		} Section_t;

	public:
		ExifInfo* m_exifinfo;
		char m_szLastError[256];
		CxExifInfo(ExifInfo* info = NULL);
		~CxExifInfo();
		bool decodeExif(XFile * hFile, int nReadMode = EXIF_READ_EXIF);
		bool encodeExif(XFile * hFile);
		void discardAllButExif();
	protected:
		bool process_EXIF(uchar * CharBuf, uint length);
		void process_COM(const uchar * data, int length);
		void process_SOFn(const uchar * data, int marker);
		int Get16u(void * Short);
		int Get16m(void * Short);
		int Get32s(void * Long);
		uint Get32u(void * Long);
		double ConvertAnyFormat(void * ValuePtr, int format);
		void* FindSection(int SectionType);
		bool ProcessExifDir(uchar * DirStart, uchar * OffsetBase, unsigned ExifLength,
			ExifInfo * const pInfo, uchar ** const LastExifRefdP, int NestingLevel = 0);
		int ExifImageWidth;
		int MotorolaOrder;
		Section_t Sections[MAX_SECTIONS];
		int SectionsRead;
		bool freeinfo;
	};

	CVLIB_DECLSPEC bool loadEXIF(const char* szfilename, ExifInfo& exifinfo);

}