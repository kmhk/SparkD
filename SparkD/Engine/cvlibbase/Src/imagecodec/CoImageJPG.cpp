/*! 
 * \file	CoImageJPG.cpp
 * \ingroup base
 * \brief	JPG
 * \author
 */

#include "CoImageJPG.h"

#ifndef CVLIB_IMG_NOCODEC

#if CVLIB_IMG_SUPPORT_JPG

#if CVLIB_OS==CVLIB_OS_WIN32 && !defined __MINGW32__
#include <io.h>
#elif defined(__ANDROID__)
//#include <io.h>
#elif defined(linux) || defined(__linux) || defined(__CYGWIN__)
#include <sys/io.h>
#endif
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif


#include <setjmp.h>
#include <stdlib.h>

#include "XFile.h"
#include "XFileDisk.h"

extern "C" 
{
#include "jpeglib.h"
#include "jerror.h"
}

#pragma warning (push)
#pragma warning (disable : 4611)

namespace cvlib 
{
class CxFileJpg : public jpeg_destination_mgr, public jpeg_source_mgr
	{
public:
	enum { eBufSize = 4096 };

	CxFileJpg(XFile* pFile)
	{
        m_pFile = pFile;

		init_destination = InitDestination;
		empty_output_buffer = EmptyOutputBuffer;
		term_destination = TermDestination;

		init_source = InitSource;
		fill_input_buffer = FillInputBuffer;
		skip_input_data = SkipInputData;
		resync_to_restart = jpeg_resync_to_restart; // use default method
		term_source = TermSource;
		next_input_byte = NULL; //* => next byte to read from buffer 
		bytes_in_buffer = 0;	//* # of bytes remaining in buffer 

		m_pBuffer = new unsigned char[eBufSize];
	}
	~CxFileJpg()
	{
		delete [] m_pBuffer;
	}

	static void InitDestination(j_compress_ptr cinfo)
	{
		CxFileJpg* pDest = (CxFileJpg*)cinfo->dest;
		pDest->next_output_byte = pDest->m_pBuffer;
		pDest->free_in_buffer = eBufSize;
	}

	static boolean EmptyOutputBuffer(j_compress_ptr cinfo)
	{
		CxFileJpg* pDest = (CxFileJpg*)cinfo->dest;
		if (pDest->m_pFile->write(pDest->m_pBuffer,1,eBufSize)!=(size_t)eBufSize)
			ERREXIT(cinfo, JERR_FILE_WRITE);
		pDest->next_output_byte = pDest->m_pBuffer;
		pDest->free_in_buffer = eBufSize;
		return TRUE;
	}

	static void TermDestination(j_compress_ptr cinfo)
	{
		CxFileJpg* pDest = (CxFileJpg*)cinfo->dest;
		size_t datacount = eBufSize - pDest->free_in_buffer;
		/* write any data remaining in the buffer */
		if (datacount > 0) {
			if (!pDest->m_pFile->write(pDest->m_pBuffer,1,datacount))
				ERREXIT(cinfo, JERR_FILE_WRITE);
		}
		pDest->m_pFile->flush();
		/* Make sure we wrote the output file OK */
		if (pDest->m_pFile->error()) ERREXIT(cinfo, JERR_FILE_WRITE);
		return;
	}

	static void InitSource(j_decompress_ptr cinfo)
	{
		CxFileJpg* pSource = (CxFileJpg*)cinfo->src;
		pSource->m_bStartOfFile = TRUE;
	}

	static boolean FillInputBuffer(j_decompress_ptr cinfo)
	{
		size_t nbytes;
		CxFileJpg* pSource = (CxFileJpg*)cinfo->src;
		nbytes = pSource->m_pFile->read(pSource->m_pBuffer,1,eBufSize);
		if (nbytes <= 0){
			if (pSource->m_bStartOfFile)	//* Treat empty input file as fatal error 
				ERREXIT(cinfo, JERR_INPUT_EMPTY);
			WARNMS(cinfo, JWRN_JPEG_EOF);
			// insert a fake EOI marker 
			pSource->m_pBuffer[0] = (JOCTET) 0xFF;
			pSource->m_pBuffer[1] = (JOCTET) JPEG_EOI;
			nbytes = 2;
		}
		pSource->next_input_byte = pSource->m_pBuffer;
		pSource->bytes_in_buffer = nbytes;
		pSource->m_bStartOfFile = false;
		return TRUE;
	}

	static void SkipInputData(j_decompress_ptr cinfo, long num_bytes)
	{
		CxFileJpg* pSource = (CxFileJpg*)cinfo->src;
		if (num_bytes > 0)
		{
			while (num_bytes > (long)pSource->bytes_in_buffer)
			{
				num_bytes -= (long)pSource->bytes_in_buffer;
				FillInputBuffer(cinfo);
				// note we assume that fill_input_buffer will never return FALSE,
				// so suspension need not be handled.
			}
			pSource->next_input_byte += (size_t) num_bytes;
			pSource->bytes_in_buffer -= (size_t) num_bytes;
		}
	}

	static void TermSource(j_decompress_ptr /*cinfo*/)
	{
		return;
	}
protected:
    XFile  *m_pFile;
	unsigned char *m_pBuffer;
	bool m_bStartOfFile;
};

struct jpg_error_mgr {
	struct jpeg_error_mgr pub;	/* "public" fields */
	jmp_buf setjmp_buffer;		/* for return to caller */
	char* buffer;				/* error message <CSC>*/
};
typedef jpg_error_mgr *jpg_error_ptr;

static void
ima_jpeg_error_exit (j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	jpg_error_ptr myerr = (jpg_error_ptr) cinfo->err;
	/* create the message */
	myerr->pub.format_message (cinfo, myerr->buffer);
	/* Send it to stderr, adding a newline */
	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}

CoImageJPG::CoImageJPG()
{
}

bool CoImageJPG::Decode (Mat& image, XFile* pFile)
{
	bool is_exif = false;
// #if CVLIB_IMG_SUPPORT_JPG
// 	is_exif = DecodeExif(hFile);
// #endif

	if( pFile == NULL )
		return false;


	/* This struct contains the JPEG decompression parameters and pointers to
	* working space (which is allocated as needed by the JPEG library).
	*/
	struct jpeg_decompress_struct cinfo;
	/* We use our private extension JPEG error handler. <CSC> */
	struct jpg_error_mgr jerr;
	jerr.buffer=m_Info.szLastError;
	/* More stuff */
	JSAMPARRAY buffer;	/* Output row buffer */
	int row_stride;		/* physical row width in output buffer */

	/* In this example we want to open the input file before doing anything else,
	* so that the setjmp() error recovery below can assume the file is open.
	* VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	* requires it in order to read binary files.
	*/

	/* step 1: allocate and initialize JPEG decompression object */
	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = ima_jpeg_error_exit;

	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		* We need to clean up the JPEG object, close the input file, and return.
		*/
		jpeg_destroy_decompress(&cinfo);
		return 0;
	}

	
//	jpeg_stdio_src(&cinfo, pFile);
	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);

	CxFileJpg src(pFile);

	/* step 2: specify data source (eg, a file) */
    cinfo.src = &src;

	/* step 3: read file parameters with jpeg_read_header() */
	(void) jpeg_read_header(&cinfo, TRUE);

// 	/* step 4 <chupeev> handle decoder options*/
// 	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & DECODE_GRAYSCALE) != 0)
// 		cinfo.out_color_space = JCS_GRAYSCALE;
// 	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & DECODE_QUANTIZE) != 0) {
// 		cinfo.quantize_colors = TRUE;
// 		cinfo.desired_number_of_colors = info.nQuality;
// 	}
// 	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & DECODE_DITHER) != 0)
// 		cinfo.dither_mode = m_nDither;
// 	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & DECODE_ONEPASS) != 0)
// 		cinfo.two_pass_quantize = FALSE;
// 	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & DECODE_NOSMOOTH) != 0)
// 		cinfo.do_fancy_upsampling = FALSE;

//<DP>: load true color images as COLOR (no quantize) 
/* step 4: set parameters for decompression */
/*  if (cinfo.jpeg_color_space!=JCS_GRAYSCALE) {
 *	cinfo.quantize_colors = TRUE;
 *	cinfo.desired_number_of_colors = 128;
 *}
 */ //</DP>

	// Set the scale <ignacio>
//	cinfo.scale_denom = GetJpegScale();

	// Borrowed the idea from GIF implementation <ignacio>
	if (m_Info.nEscape == -1) 
	{
		// Return output dimensions only
		jpeg_calc_output_dimensions(&cinfo);
		m_Head.biWidth = cinfo.output_width;
		m_Head.biHeight = cinfo.output_height;
		jpeg_destroy_decompress(&cinfo);
		return true;
	}

	/* We may need to do some setup of our own at this point before reading
	* the data.  After jpeg_start_decompress() we have the correct scaled
	* output image dimensions available, as well as the output colormap
	* if we asked for color quantization.
	*/
	//create the image using output dimensions <ignacio>
	//create(cinfo.image_width, cinfo.image_height, 8*cinfo.output_components, CXIMAGE_FORMAT_JPG);
	CreateInfo(cinfo.image_width, cinfo.image_height, 8*cinfo.num_components, CVLIB_IMG_SUPPORT_JPG);
	if (cinfo.num_components == 1)
		image.create(cinfo.image_height, cinfo.image_width, MAT_Tbyte);
	else
		image.create(cinfo.image_height, cinfo.image_width, MAT_Tbyte3);

    bool color = image.channels() > 1;

    if( color )
    {
        if( cinfo.num_components != 4 )
        {
            cinfo.out_color_space = JCS_RGB;
            cinfo.out_color_components = 3;
        }
        else
        {
            cinfo.out_color_space = JCS_CMYK;
            cinfo.out_color_components = 4;
        }
    }
    else
    {
        if( cinfo.num_components != 4 )
        {
            cinfo.out_color_space = JCS_GRAYSCALE;
            cinfo.out_color_components = 1;
        }
        else
        {
            cinfo.out_color_space = JCS_CMYK;
            cinfo.out_color_components = 4;
        }
    }

	/* step 5: Start decompressor */
	jpeg_start_decompress(&cinfo);

	if (is_exif)
	{
// #if CXIMAGEJPG_SUPPORT_EXIF
// 	if ((m_exifinfo.Xresolution != 0.0) && (m_exifinfo.ResolutionUnit != 0))
// 		SetXDPI((long)(m_exifinfo.Xresolution/m_exifinfo.ResolutionUnit));
// 	if ((m_exifinfo.Yresolution != 0.0) && (m_exifinfo.ResolutionUnit != 0))
// 		SetYDPI((long)(m_exifinfo.Yresolution/m_exifinfo.ResolutionUnit));
// #endif
	}
	else
	{
		if (cinfo.density_unit==2)
		{
			SetXDPI((long)floor(cinfo.X_density * 254.0 / 10000.0 + 0.5));
			SetYDPI((long)floor(cinfo.Y_density * 254.0 / 10000.0 + 0.5));
		}
		else
		{
			SetXDPI(cinfo.X_density);
			SetYDPI(cinfo.Y_density);
		}
	}

	if (cinfo.out_color_space==JCS_GRAYSCALE)
	{
 		SetGrayPalette();
		m_Head.biClrUsed =256;
	}
	else
	{
		if (cinfo.quantize_colors==TRUE)
		{
			SetPalette(cinfo.actual_number_of_colors, cinfo.colormap[0], cinfo.colormap[1], cinfo.colormap[2]);
			m_Head.biClrUsed=cinfo.actual_number_of_colors;
		}
		else
		{
			m_Head.biClrUsed=0;
		}
	}

	/* JSAMPLEs per row in output buffer */
	row_stride = cinfo.output_width * cinfo.output_components;

	/* Make a one-row-high sample array that will go away when done with image */
	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	/* step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */
	/* Here we use the library's state variable cinfo.output_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	*/
	int iH = 0;
	while (cinfo.output_scanline < cinfo.output_height) 
	{
		if (m_Info.nEscape)
			longjmp(jerr.setjmp_buffer, 1); // <vho> - cancel decoding
		
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);
		// info.nProgress = (long)(100*cinfo.output_scanline/cinfo.output_height);
		//<DP> step 6a: CMYK->COLOR */ 
		if ((cinfo.num_components==4)&&(cinfo.quantize_colors==FALSE))
		{
			uchar k,*src_uchar=buffer[0];
			uchar *dst = image.data.ptr[iH];
			for(long x3=0,x4=0,iW = 0; x3<(long)m_Info.dwEffWidth && x4<row_stride; x3+=3, x4+=4)
			{
				k= src_uchar[x4+3];
				dst[iW++] =(uchar)((k * src_uchar[x4+2])/255);
				dst[iW++]=(uchar)((k * src_uchar[x4+1])/255);
				dst[iW++]=(uchar)((k * src_uchar[x4+0])/255);
			}
		} 
		else if (cinfo.output_components == 3)
		{
			/* Assume put_scanline_someplace wants a pointer and sample count. */
			uchar *dst = image.data.ptr[iH];
			uchar* src_uchar = buffer[0];
			int m_len = row_stride/3*3;
			memcpy (dst, src_uchar, m_len);
		}
		else if (cinfo.output_components == 1)
		{
			uchar* src_uchar =buffer[0];
			uchar *dst = image.data.ptr[iH];
			memcpy (dst, src_uchar, row_stride);
		}
		iH ++;
	}

	/* step 7: Finish decompression */
	(void) jpeg_finish_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/

	//<DP> step 7A: swap red and blue components
	// not necessary if swapped red and blue definition in jmorecfg.h;ln322 <W. Morrison>

	/* step 8: release JPEG decompression object */
	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);

	/* At this point you may want to check to see whether any corrupt-data
	* warnings occurred (test whether jerr.pub.num_warnings is nonzero).
	*/

	/* And we're done! */
	return true;
}
	
#if CVLIB_IMG_SUPPORT_ENCODE
bool CoImageJPG::Encode(const Mat& image, XFile* pFile)
{
	int cn = image.channels();
	if (m_Head.biClrUsed!=0 && cn==4)
	{
		strcpy(m_Info.szLastError,"JPEG can save only COLOR or GreyScale images");
		return false;
	}

	// necessary for EXIF, and for roll backs
//	long pos=pFile->tell();

	/* This struct contains the JPEG compression parameters and pointers to
	* working space (which is allocated as needed by the JPEG library).
	* It is possible to have several such structures, representing multiple
	* compression/decompression processes, in existence at once.  We refer
	* to any one struct (and its associated working data) as a "JPEG object".
	*/
	struct jpeg_compress_struct cinfo;
	/* This struct represents a JPEG error handler.  It is declared separately
	* because applications often want to supply a specialized error handler
	* (see the second half of this file for an example).  But here we just
	* take the easy way out and use the standard error handler, which will
	* print a message on stderr and call exit() if compression fails.
	* Note that this struct must live as long as the main JPEG parameter
	* struct, to avoid dangling-pointer problems.
	*/
	//struct jpeg_error_mgr jerr;
	/* We use our private extension JPEG error handler. <CSC> */
	struct jpg_error_mgr jerr;
	jerr.buffer=m_Info.szLastError;
	/* More stuff */
	int row_stride;		/* physical row width in image buffer */
	JSAMPARRAY buffer;		/* Output row buffer */

	/* step 1: allocate and initialize JPEG compression object */
	/* We have to set up the error handler first, in case the initialization
	* step fails.  (Unlikely, but it could happen if you are out of memory.)
	* This routine fills in the contents of struct jerr, and returns jerr's
	* address which we place into the link field in cinfo.
	*/
	//cinfo.err = jpeg_std_error(&jerr); <CSC>
	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = ima_jpeg_error_exit;

	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer))
	{
		/* If we get here, the JPEG code has signaled an error.
		* We need to clean up the JPEG object, close the input file, and return.
		*/
		strcpy(m_Info.szLastError, jerr.buffer); //<CSC>
		jpeg_destroy_compress(&cinfo);
		return 0;
	}
	
	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress(&cinfo);
	/* step 2: specify data destination (eg, a file) */
	/* Note: steps 2 and 3 can be done in either order. */
	/* Here we use the library-supplied code to send compressed data to a
	* stdio stream.  You can also write your own code to do something else.
	* VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	* requires it in order to write binary files.
	*/

	//jpeg_stdio_dest(&cinfo, outfile);
	CxFileJpg dest(pFile);
    cinfo.dest = &dest;

	/* step 3: set parameters for compression */
	/* First we supply a description of the input image.
	* Four fields of the cinfo struct must be filled in:
	*/
	cinfo.image_width = image.cols(); 	// image width and height, in pixels
	cinfo.image_height = image.rows();
	if (cn == 1)
	{
		cinfo.input_components = 1;			// # of color components per pixel
		cinfo.in_color_space = JCS_GRAYSCALE; /* colorspace of input image */
	} 
	else
	{
		cinfo.input_components = 3; 	// # of color components per pixel
		cinfo.in_color_space = JCS_RGB; /* colorspace of input image */
	}

	/* Now use the library's routine to set default compression parameters.
	* (You must set at least cinfo.in_color_space before calling this,
	* since the defaults depend on the source color space.)
	*/
	jpeg_set_defaults(&cinfo);
	/* Now you can set any non-default parameters you wish to.
	* Here we just illustrate the use of quality (quantization table) scaling:
	*/

// #ifdef C_ARITH_CODING_SUPPORTED
// 	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & ENCODE_ARITHMETIC) != 0)
// 		cinfo.arith_code = TRUE;
// #endif
// 
// #ifdef ENTROPY_OPT_SUPPORTED
// 	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & ENCODE_OPTIMIZE) != 0)
// 		cinfo.optimize_coding = TRUE;
// #endif
// 
// 	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & ENCODE_GRAYSCALE) != 0)
	if (cn == 1)
		jpeg_set_colorspace(&cinfo, JCS_GRAYSCALE);
// 
// 	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & ENCODE_SMOOTHING) != 0)
// 		cinfo.smoothing_factor = m_nSmoothing;

//	jpeg_set_quality(&cinfo, GetJpegQuality(), (GetCodecOption(CXIMAGE_FORMAT_JPG) & ENCODE_BASELINE) != 0);
	jpeg_set_quality(&cinfo, GetJpegQuality(), TRUE);
	
//#ifdef C_PROGRESSIVE_SUPPORTED
//	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & ENCODE_PROGRESSIVE) != 0)
//		jpeg_simple_progression(&cinfo);
//#endif

#ifdef C_LOSSLESS_SUPPORTED
//	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & ENCODE_LOSSLESS) != 0)
//		jpeg_simple_lossless(&cinfo, m_nPredictor, m_nPointTransform);
#endif

	cinfo.density_unit=1;
	cinfo.X_density=(unsigned short)GetXDPI();
	cinfo.Y_density=(unsigned short)GetYDPI();

	/* step 4: Start compressor */
	/* TRUE ensures that we will write a complete interchange-JPEG file.
	* Pass TRUE unless you are very sure of what you're doing.
	*/
	jpeg_start_compress(&cinfo, TRUE);

	/* step 5: while (scan lines remain to be written) */
	/*           jpeg_write_scanlines(...); */
	/* Here we use the library's state variable cinfo.next_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	* To keep things simple, we pass one scanline per call; you can pass
	* more if you wish, though.
	*/
	if (cn <= 3)
		row_stride = (image.cols()*8*cn+31)/32*4;
	else
		row_stride = (image.cols()*24+31)/32*4;

	//<DP> "8+row_stride" fix heap deallocation problem during debug???
	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, 8+row_stride, 1);

	int iH = 0;
	while (cinfo.next_scanline < cinfo.image_height) 
	{
		uchar* dst = buffer[0];
		uchar *src = image.data.ptr[iH];
		if (cn == 1)
		{
			memcpy (dst, src, image.cols());
		}
		else
		{
			for(long x3=0, scn = 0; x3<row_stride; x3+=3, scn+=cn)
			{
				dst[x3+0] = src[scn+0];
				dst[x3+1] = src[scn+1];
				dst[x3+2] = src[scn+2];
			}
		}
		iH ++;
		(void) jpeg_write_scanlines(&cinfo, buffer, 1);
	}

	/* step 6: Finish compression */
	jpeg_finish_compress(&cinfo);

	/* step 7: release JPEG compression object */
	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_compress(&cinfo);

	return true;
}

#endif // COIMAGE_SUPPORT_ENCODE

}
#pragma warning (pop)

#endif //#if CVLIB_IMG_SUPPORT_JPG

#endif // CVLIB_IMG_NOCODEC
