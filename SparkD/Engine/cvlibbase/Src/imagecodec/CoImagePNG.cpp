
/*! 
 * \file	CoImagePNG.cpp
 * \ingroup base
 * \brief	PNG
 * \author
 */

#include "CoImagePNG.h"
#include "ImageIter.h"
#include <stdlib.h>

#ifndef CVLIB_IMG_NOCODEC

#if CVLIB_IMG_SUPPORT_PNG

extern "C"
{
#include "png.h"
#include "pngstruct.h"
#include "pnginfo.h"
}

#pragma warning (push)
#pragma warning (disable : 4611)

namespace cvlib 
{

static struct user_chunk_data
{
	png_const_infop info_ptr;
	png_uint_32     vpAg_width, vpAg_height;
	png_byte        vpAg_units;
	png_byte        sTER_mode;
	int             location[2];
}
user_chunk_data;

/* Used for location and order; zero means nothing. */
#define have_sTER   0x01
#define have_vpAg   0x02
#define before_PLTE 0x10
#define before_IDAT 0x20
#define after_IDAT  0x40

#define MEMZERO(var) ((void)memset(&var, 0, sizeof var))

static void init_callback_info(png_const_infop info_ptr)
{
	MEMZERO(user_chunk_data);
	user_chunk_data.info_ptr = info_ptr;
}
	
static void user_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	XFile* hFile = (XFile*)png_get_io_ptr(png_ptr);
	if (hFile->read(data,1,length) != (int)length)
		png_error(png_ptr, "read Error");
}

static void user_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	XFile* hFile = (XFile*)png_get_io_ptr(png_ptr);
	if (hFile->write(data, 1, length) != (int)length) 
		png_error(png_ptr, "write Error");
}

static void user_flush_data(png_structp png_ptr)
{
	XFile* hFile = (XFile*)png_get_io_ptr(png_ptr);
	if (!hFile->flush()) png_error(png_ptr, "Flush Error");
}
static void user_error_fn(png_structp png_ptr,png_const_charp error_msg)
{
	strncpy((char*)png_ptr->error_ptr,error_msg,255);
	longjmp(png_jmpbuf(png_ptr), 1);
}

CoImagePNG::CoImagePNG()
{

}

void CoImagePNG::expand2to4bpp(uchar* prow)
{
	uchar *psrc,*pdst;
	uchar pos,idx;
	for(long x=m_Head.biWidth-1;x>=0;x--)
	{
		psrc = prow + ((2*x)>>3);
		pdst = prow + ((4*x)>>3);
		pos = (uchar)(2*(3-x%4));
		idx = (uchar)((*psrc & (0x03<<pos))>>pos);
		pos = (uchar)(4*(1-x%2));
		*pdst &= ~(0x0F<<pos);
		*pdst |= (idx & 0x0F)<<pos;
	}
}
bool CoImagePNG::Decode (Mat& image, XFile* pFile)
{
	release();

	png_structp png_ptr;
	png_infop read_info_ptr, end_info_ptr;

	png_uint_32 width, height;
	volatile int num_passes;

	int bit_depth, color_type;
	uchar *row_pointers = NULL;

	try
	{
		/* create and initialize the png_struct with the desired error handler
		* functions.  If you want to use the default stderr and longjump method,
		* you can supply NULL for the last three parameters.  We also supply the
		* the compiler header file version, so that we know if the application
		* was compiled with a compatible version of the library.  REQUIRED
		*/
		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (void *)NULL, NULL, NULL);
		if (png_ptr == NULL)
			throw "Failed to create PNG structure";
		png_set_error_fn(png_ptr, m_Info.szLastError,/*(png_error_ptr)*/user_error_fn, NULL);

		/* Allocate/initialize the memory for image information.  REQUIRED. */
		read_info_ptr = png_create_info_struct(png_ptr);
		end_info_ptr = png_create_info_struct(png_ptr);
		if (read_info_ptr == NULL)
		{
			png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
			throw "Failed to initialize PNG info structure";
		}

		/* Set error handling if you are using the setjmp/longjmp method (this is
		* the normal method of doing things with libpng).  REQUIRED unless you
		* set up your own error handlers in the png_create_read_struct() earlier.
		*/
		if (setjmp(png_jmpbuf(png_ptr)))
		{
			// Free all of the memory associated with the png_ptr and info_ptr
			if (row_pointers) delete[] row_pointers;
			png_destroy_read_struct(&png_ptr, &read_info_ptr, (png_infopp)&end_info_ptr);
			throw "";
		}
			/* set up the input control */
			//png_init_io(png_ptr, hFile);

			// use custom I/O functions
		png_set_read_fn(png_ptr, pFile, /*(png_rw_ptr)*/user_read_data);
		png_set_read_status_fn(png_ptr, NULL);

		/* read the file information */
		png_read_info(png_ptr, read_info_ptr);

		int interlace_type, compression_type, filter_type;
		{
			if (png_get_IHDR(png_ptr, read_info_ptr, &width, &height, &bit_depth,
				&color_type, &interlace_type, &compression_type, &filter_type) != 0)
			{
				/* num_passes may not be available below if interlace support is not
				* provided by libpng for both read and write.
				*/
				switch (interlace_type)
				{
				case PNG_INTERLACE_NONE:
					num_passes = 1;
					break;

				case PNG_INTERLACE_ADAM7:
					num_passes = 7;
					break;

				default:
					png_error(png_ptr, "invalid interlace type");
					/*NOT REACHED*/
				}
			}

			else
				png_error(png_ptr, "png_get_IHDR failed");
		}
		int channels = 0;
		switch (color_type) {
		case PNG_COLOR_TYPE_GRAY:
		case PNG_COLOR_TYPE_PALETTE:
			channels = 1;
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			channels = 2;
			break;
		case PNG_COLOR_TYPE_RGB:
			channels = 3;
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			channels = 4;
			break;
		default:
			longjmp(png_jmpbuf(png_ptr), 1);
		}

		int pixel_depth = bit_depth;
		if (channels == 1 && pixel_depth > 8) pixel_depth = 8;
		else if (channels == 2) pixel_depth = 8;
		else if (channels >= 3) pixel_depth = 24;
		//if (channels == 4) pixel_depth = 32;

		CreateInfo(height, width, pixel_depth, CVLIB_IMG_FORMAT_PNG);
		int alpha_present = (channels - 1) % 2;
		if (alpha_present)
			image.create(height, width, MAT_Tbyte4);
		else
			image.create(height, width, MAT_Tbyte3);

		png_uint_32 res_x, res_y;
		int phys_unit_type;

		if (png_get_pHYs(png_ptr, read_info_ptr, &res_x, &res_y, &phys_unit_type) != 0)
		{
			switch (phys_unit_type)
			{
			case PNG_RESOLUTION_UNKNOWN:
				SetXDPI(res_x);
				SetYDPI(res_y);
				break;
			case PNG_RESOLUTION_METER:
				SetXDPI((long)floor(res_x * 254.0 / 10000.0 + 0.5));
				SetYDPI((long)floor(res_y * 254.0 / 10000.0 + 0.5));
				break;
			}
		}

		png_colorp palette;
		int num_palette = 0;

		png_get_PLTE(png_ptr, read_info_ptr, &palette, &num_palette);

		if (num_palette > 0)
			SetPalette((rgb_color*)palette, num_palette);
		else if (bit_depth == 2)
		{ //<DP> needed for 2 bpp grayscale PNGs
			SetPaletteColor(0, 0, 0, 0);
			SetPaletteColor(1, 85, 85, 85);
			SetPaletteColor(2, 170, 170, 170);
			SetPaletteColor(3, 255, 255, 255);
		}
		else
			SetGrayPalette(); //<DP> needed for grayscale PNGs

		int nshift = MAX(0, (bit_depth >> 3) - 1) << 3;
		// simple transparency (the real PGN transparency is more complex)
		png_bytep trans_alpha;
		int num_trans;
		png_color_16p trans_color;

		if (png_get_tRNS(png_ptr, read_info_ptr, &trans_alpha, &num_trans, &trans_color) != 0) {
			m_Info.nBkgndColor.x = (uchar)(trans_color->red >> nshift);
			m_Info.nBkgndColor.y = (uchar)(trans_color->green >> nshift);
			m_Info.nBkgndColor.z = (uchar)(trans_color->blue >> nshift);
			m_Info.nBkgndColor.w = 0;
			m_Info.nBkgndIndex = 0;
			if (num_trans != 0)
			{
				//palette transparency
				COLOR* pal = GetPalette();
				if (pal)
				{
					ulong ip;
					for (ip = 0; ip < MIN(m_Head.biClrUsed, (unsigned long)num_trans); ip++)
						pal[ip].w = trans_alpha[ip];
					if (num_trans == 1 && pal[0].w == 0)
					{
						m_Info.nBkgndIndex = 0;
					}
					else
					{
						m_Info.bAlphaPaletteEnabled = true;
						for (; ip < m_Head.biClrUsed; ip++)
							pal[ip].w = 255;
					}
				}
			}
		}

		// <vho> - flip the COLOR pixels to BGR (or RGBA to BGRA)
		if (color_type & PNG_COLOR_MASK_COLOR)
			png_set_bgr(png_ptr);

		// <vho> - handle cancel
		if (m_Info.nEscape)
			longjmp(png_jmpbuf(png_ptr), 1);

		//allocate the buffer
		//int row_stride = width * ((pixel_depth + 7) >> 3);
		size_t row_stride = png_get_rowbytes(png_ptr, read_info_ptr);
		row_pointers = new uchar[8 + row_stride];
		//row_pointers = (png_bytep)png_malloc(png_ptr, png_get_rowbytes(png_ptr, read_info_ptr));

		if (png_set_interlace_handling(png_ptr) != num_passes) {
			assert(false);
			//png_error(write_ptr, "png_set_interlace_handling(read): wrong pass count ");
		}
			// turn on interlace handling
		//num_passes = png_set_interlace_handling(png_ptr);

		if (num_passes > 1)
		{
			SetCodecOption(1, CVLIB_IMG_FORMAT_PNG);
		}
		else
		{
			SetCodecOption(0, CVLIB_IMG_FORMAT_PNG);
		}

		int chan_offset = bit_depth >> 3;
		size_t rowbytes = row_stride;
		ImageIterator iter(&image, this);
		for (int pass = 0; pass < num_passes; pass++)
		{
			iter.Upset();
			int y = 0;
			do
			{
				// <vho> - handle cancel
				if (m_Info.nEscape)
					longjmp(png_jmpbuf(png_ptr), 1);

#if CVLIB_IMG_SUPPORT_ALPHA	// <vho>
				if (image.channels() == 3)
#endif // CVLIB_IMG_SUPPORT_ALPHA
				{
					//recover data from previous scan
					if (interlace_type && pass > 0)
					{
						iter.GetRow(row_pointers, (int)row_stride);
						//re-expand buffer for images with bit depth > 8
						if (bit_depth > 8) {
							for (int ax = (m_Head.biWidth*channels - 1); ax >= 0; ax--)
								row_pointers[ax*chan_offset] = row_pointers[ax];
						}
					}
					//read next row
					png_read_row(png_ptr, row_pointers, NULL);

					//shrink 16 bit depth images down to 8 bits
					if (bit_depth > 8) {
						for (int ax = 0; ax < (m_Head.biWidth*channels); ax++)
							row_pointers[ax] = row_pointers[ax*chan_offset];
					}
					// <vho> - already done by png_set_bgr()
					// <vho>			//HACK BY OP && (<DP> for interlace, swap only in the last pass)
					// <vho>			if (info_ptr->color_type==COLORTYPE_COLOR && pass==(number_passes-1))
					RGBtoBGR(row_pointers, m_Info.dwEffWidth, channels);
					iter.setRow(row_pointers, (int)rowbytes);
					//<DP> expand 2 bpp images only in the last pass
					if (bit_depth == 2 && pass == (num_passes - 1))
						expand2to4bpp(iter.GetRow());

					//go on
					iter.PrevRow();
				}
#if CVLIB_IMG_SUPPORT_ALPHA 	// <vho>
				else
				{ //alpha blend
					long ax, ay;
					ay = m_Head.biHeight - 1 - y;
					uchar* prow = iter.GetRow(ay);

					//recover data from previous scan
					if (interlace_type && pass > 0 && pass != 7)
					{
						for (ax = m_Head.biWidth; ax >= 0; ax--)
						{
							row_pointers[ax * 4] = prow[3 * ax + 2];
							row_pointers[ax * 4 + 1] = prow[3 * ax + 1];
							row_pointers[ax * 4 + 2] = prow[3 * ax + 0];
							row_pointers[ax * 4 + 3] = image.data.ptr[image.rows()-ay-1][ax * 4 + 3];
						}
					}

					//read next row
					png_read_row(png_ptr, row_pointers, NULL);

					//RGBA -> COLOR + A
					for (ax = 0; ax < m_Head.biWidth; ax++)
					{
						int px = ax * channels * chan_offset;// pixel_offset;
						if (channels == 2) {
							prow[ax] = row_pointers[px];
							image.data.ptr[y][ax] = row_pointers[px + chan_offset];
						}
						else {
							int qx = ax * 3;
							prow[qx + 2] = row_pointers[px];
							prow[qx + 1] = row_pointers[px + chan_offset];
							prow[qx + 0] = row_pointers[px + chan_offset * 2];
							image.data.ptr[y][ax * 4 + 3] = row_pointers[px + chan_offset * 3];
						}
					}
				}
#endif // CVLIB_IMG_SUPPORT_ALPHA		// vho
				y++;
			} while (y < m_Head.biHeight);
		}

		/* read the rest of the file, getting any additional chunks in info_ptr */
		png_read_end(png_ptr, end_info_ptr);

		delete[]row_pointers; //png_free(png_ptr, row_pointers);
		row_pointers = 0;

		/* clean up after the read, and free any memory allocated - REQUIRED */
		png_destroy_read_struct(&png_ptr, &read_info_ptr, &end_info_ptr);

		iter.BMP2XYZ();

	}
	catch (char *message)
	{
		if (strcmp(message, "")) 
			strncpy(m_Info.szLastError, message, 255);
		return false;
	}
	/* that's it */
	return true;
}

#if CVLIB_IMG_SUPPORT_ENCODE
static void
write_sTER_chunk(png_structp write_ptr)
{
	png_byte sTER[5] = { 115,  84,  69,  82, '\0' };

	png_write_chunk(write_ptr, sTER, &user_chunk_data.sTER_mode, 1);
}

static void
write_vpAg_chunk(png_structp write_ptr)
{
	png_byte vpAg[5] = { 118, 112,  65, 103, '\0' };

	png_byte vpag_chunk_data[9];

	png_save_uint_32(vpag_chunk_data, user_chunk_data.vpAg_width);
	png_save_uint_32(vpag_chunk_data + 4, user_chunk_data.vpAg_height);
	vpag_chunk_data[8] = user_chunk_data.vpAg_units;
	png_write_chunk(write_ptr, vpAg, vpag_chunk_data, 9);
}

static void write_chunks(png_structp write_ptr, int location)
{
	int i;

	/* Notice that this preserves the original chunk order, however chunks
	* intercepted by the callback will be written *after* chunks passed to
	* libpng.  This will actually reverse a pair of sTER chunks or a pair of
	* vpAg chunks, resulting in an error later.  This is not worth worrying
	* about - the chunks should not be duplicated!
	*/
	for (i = 0; i<2; ++i)
	{
		if (user_chunk_data.location[i] == (location | have_sTER))
			write_sTER_chunk(write_ptr);

		else if (user_chunk_data.location[i] == (location | have_vpAg))
			write_vpAg_chunk(write_ptr);
	}
}

bool CoImagePNG::Encode(const Mat& image, XFile* pFile)
{
	Startup(CVLIB_IMG_FORMAT_PNG);
	InitBMPHeader(image.cols(), image.rows());

//	uchar trans[256];	//for transparency (don't move)
	try
	{
		/* create and initialize the png_struct with the desired error handler
		 * functions.  If you want to use the default stderr and longjump method,
		 * you can supply NULL for the last three parameters.  We also check that
		 * the library version is compatible with the one used at compile time,
		 * in case we are using dynamically linked libraries.  REQUIRED.
		 */
		png_structp png_ptr;
		png_infop info_ptr;
		png_infop end_info_ptr;

		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		if (png_ptr == NULL) throw "Failed to create PNG structure";

		//png_set_error_fn(png_ptr, m_Info.szLastError, pngtest_error, pngtest_warning);

		/* Allocate/initialize the image information data.  REQUIRED */
		info_ptr = png_create_info_struct(png_ptr);
		end_info_ptr = png_create_info_struct(png_ptr);
		if (info_ptr == NULL) {
			png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
			throw "Failed to initialize PNG info structure";
		}

		/* Set error handling.  REQUIRED if you aren't supplying your own
		 * error hadnling functions in the png_create_write_struct() call.
		 */
		if (setjmp(png_jmpbuf(png_ptr)))
		{
			/* If we get here, we had a problem reading the file */
			if (info_ptr->palette)
				free(info_ptr->palette);
			png_destroy_info_struct(png_ptr, &end_info_ptr);
			png_destroy_write_struct(&png_ptr, &info_ptr);
			throw "Error saving PNG file";
		}

		int row_stride = m_Info.dwEffWidth;
		/* set up the output control */
		//png_init_io(png_ptr, hFile);

		// use custom I/O functions
		png_set_write_fn(png_ptr, pFile, (png_rw_ptr)user_write_data, (png_flush_ptr)user_flush_data);
		png_set_write_status_fn(png_ptr, 0);

		/* set the file information here */
		int width = image.cols();
		int height = image.rows();
		int bit_depth = (uchar)8;
		int color_type = PNG_COLOR_MASK_COLOR;
		if (image.channels() == 3) color_type = PNG_COLOR_MASK_COLOR;
		else if (image.channels() == 4) color_type = PNG_COLOR_MASK_ALPHA | PNG_COLOR_MASK_COLOR;
		
		// 1 = indexed, 2 = COLOR, 4 = RGBA
		int compression_type = 0;
		int filter_type = 0;
		int num_passes = 7;
		int interlace_type = PNG_INTERLACE_ADAM7;
		if (num_passes == 7)
			interlace_type = PNG_INTERLACE_ADAM7;
		else if (num_passes == 1)
			interlace_type = PNG_INTERLACE_NONE;
		png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth,
			color_type, interlace_type, compression_type, filter_type);

/*		png_write_info_before_PLTE(png_ptr, info_ptr);
		write_chunks(png_ptr, before_PLTE);
		png_write_info(png_ptr, info_ptr);
		write_chunks(png_ptr, before_IDAT);*/

		switch (GetCodecOption(CVLIB_IMG_FORMAT_PNG))
		{
		case 1:
			info_ptr->interlace_type = PNG_INTERLACE_ADAM7;
			break;
		default:
			info_ptr->interlace_type = PNG_INTERLACE_NONE;
		}

		/* set compression level */
		//png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

		/* set background */
		png_color_16 image_background = { 0, 255, 255, 255, 0 };
		if (m_Info.nBkgndIndex != -1)
		{
			image_background.blue = m_Info.nBkgndColor.z;
			image_background.green = m_Info.nBkgndColor.y;
			image_background.red = m_Info.nBkgndColor.x;
		}
		png_set_bKGD(png_ptr, info_ptr, &image_background);

		/* set metrics */
		png_set_pHYs(png_ptr, info_ptr, m_Head.biXPelsPerMeter, m_Head.biYPelsPerMeter, PNG_RESOLUTION_METER);

#if CVLIB_IMG_SUPPORT_ALPHA	// <vho>
		if (image.channels() == 4)
		{
			row_stride = 4 * m_Head.biWidth;

			info_ptr->pixel_depth = 32;
			info_ptr->channels = 4;
			info_ptr->bit_depth = 8;
			info_ptr->color_type = PNG_COLOR_TYPE_RGB_ALPHA;
			info_ptr->rowbytes = row_stride;

			/* write the file information */
			png_write_info(png_ptr, info_ptr);

			//<Ranger> "10+row_stride" fix heap deallocation problem during debug???
			uchar *row_pointers = new uchar[10 + row_stride];

			//interlace handling
			int num_pass = png_set_interlace_handling(png_ptr);
			for (int pass = 0; pass < num_pass; pass++) {

				//write image
	//  			iter.Upset();
				long ay = 0;
				COLOR c;
				do {
					for (long ax = m_Head.biWidth - 1; ax >= 0; ax--) {
						c = getPixelColor(image, ax, ay);
						row_pointers[ax * 4 + 3] = (uchar)((image.data.ptr[ay][ax * 4 + 3] * m_Info.nAlphaMax) / 255);
						row_pointers[ax * 4 + 2] = c.z;
						row_pointers[ax * 4 + 1] = c.y;
						row_pointers[ax * 4] = c.x;
					}
					png_write_row(png_ptr, row_pointers);
					ay++;
				} while (ay < m_Head.biHeight);
			}

			delete[] row_pointers;
		}
		else
#endif //CVLIB_IMG_SUPPORT_ALPHA	// <vho>
		{
			ImageIterator iter((Mat*)&image, this);
			iter.XYZ2BMP();

			/* write the file information */
			png_write_info(png_ptr, info_ptr);

			/* If you are only writing one row at a time, this works */
			uchar *row_pointers = new uchar[10 + row_stride];

			//interlace handling
			int num_pass = png_set_interlace_handling(png_ptr);
			for (int pass = 0; pass < num_pass; pass++)
			{

				//write image
				iter.Upset();
				do
				{
					iter.GetRow(row_pointers, row_stride);
					//HACK BY OP
					//if (info_ptr->color_type == 2 /*COLORTYPE_COLOR*/)
					//RGBtoBGR(row_pointers, row_stride, 3);
					png_write_row(png_ptr, row_pointers);
				} while (iter.PrevRow());

			}
			delete[] row_pointers;
		}

#if CVLIB_IMG_SUPPORT_ALPHA	// <vho>
		/* remove the temporary alpha channel*/
#endif // CVLIB_IMG_SUPPORT_ALPHA	// <vho>

	/* It is REQUIRED to call this to finish writing the rest of the file */
		png_write_end(png_ptr, info_ptr);

		/* if you malloced the palette, free it here */
		if (info_ptr->palette)	delete[](info_ptr->palette);

		/* clean up after the write, and free any memory allocated */
		png_destroy_info_struct(png_ptr, (png_infopp)&info_ptr);
		png_destroy_info_struct(png_ptr, (png_infopp)&end_info_ptr);
		png_destroy_write_struct(&png_ptr, (png_infopp)&end_info_ptr);
	}
	catch (char *message)
	{
		strncpy(m_Info.szLastError, message, 255);
		return false;
	}
	/* that's it */
	return true;
}
#endif // CVLIB_IMG_SUPPORT_ENCODE

}

#pragma warning (pop)

#endif //CVLIB_IMG_SUPPORT_PNG

#endif //CVLIB_IMG_NOCODEC
