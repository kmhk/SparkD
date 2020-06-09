
/*! 
 * \file	CoImageJ2K.cpp
 * \ingroup base
 * \brief	Jasper
 * \author	
 */

#include "CoImageJ2K.h"

#ifdef _MSC_VER
#include <windows.h>
#endif

#ifndef CVLIB_IMG_NOCODEC

#if CVLIB_IMG_SUPPORT_J2K

#include "XFile.h"
#include "XFileDisk.h"

extern "C"
{
#include "openjpeg.h"
#include "j2k.h"
#include "opj_includes.h"
}

#pragma warning (push)
#pragma warning (disable : 4611)

namespace cvlib
{
	/*class CxFileJas
	{
	public:
		CxFileJas(XFile* pFile,jas_stream_t *stream)
		{
			if (stream->obj_) jas_free(stream->obj_);
			stream->obj_ = pFile;

			// <vho> - cannot set the stream->ops_->functions here,
			// because this overwrites a static structure in the Jasper library.
			// This structure is used by Jasper for internal operations too, e.g. tempfile.
			// However the ops_ pointer in the stream can be overwritten.

			//stream->ops_->close_ = JasClose;
			//stream->ops_->read_  = JasRead;
			//stream->ops_->seek_  = JasSeek;
			//stream->ops_->write_ = JasWrite;

			jas_stream_CxFile.close_ = JasClose;
			jas_stream_CxFile.read_  = JasRead;
			jas_stream_CxFile.seek_  = JasSeek;
			jas_stream_CxFile.write_ = JasWrite;

			stream->ops_ = &jas_stream_CxFile;

			// <vho> - end
		}
		static int JasRead(jas_stream_obj_t *obj, char *buf, int cnt)
		{		return (int)((XFile*)obj)->read(buf,1,(size_t)cnt); }
		static int JasWrite(jas_stream_obj_t *obj, char *buf, int cnt)
		{		return (int)((XFile*)obj)->write(buf,1,(size_t)cnt); }
		static long JasSeek(jas_stream_obj_t *obj, long offset, int origin)
		{		return ((XFile*)obj)->seek(offset,origin); }
		static int JasClose(jas_stream_obj_t *)
		{		return 1; }
	private:
		jas_stream_ops_t jas_stream_CxFile;
	};*/


	CoImageJ2K::CoImageJ2K()
	{
	}

	typedef struct callback_variables {
		//JNIEnv *env;
		/** 'jclass' object used to call a Java method from the C */
		//jobject *jobj;
		/** 'jclass' object used to call a Java method from the C */
		//jmethodID message_mid;
		//jmethodID error_mid;
	} callback_variables_t;
	/**
	error callback returning the message to Java andexpecting a callback_variables_t client object
	*/
	void error_callback(const char *msg, void *client_data) {
		/*callback_variables_t* vars = (callback_variables_t*)client_data;
		JNIEnv *env = vars->env;
		jstring jbuffer;

		jbuffer = (*env)->NewStringUTF(env, msg);
		(*env)->ExceptionClear(env);
		(*env)->CallVoidMethod(env, *(vars->jobj), vars->error_mid, jbuffer);

		if ((*env)->ExceptionOccurred(env)) {
			fprintf(stderr, "C: Exception during call back method\n");
			(*env)->ExceptionDescribe(env);
			(*env)->ExceptionClear(env);
		}
		(*env)->DeleteLocalRef(env, jbuffer);*/
	}
	/**
	warning callback returning the message to Java andexpecting a callback_variables_t client object
	*/
	void warning_callback(const char *msg, void *client_data) {
		/*callback_variables_t* vars = (callback_variables_t*)client_data;
		JNIEnv *env = vars->env;
		jstring jbuffer;

		jbuffer = (*env)->NewStringUTF(env, msg);
		(*env)->ExceptionClear(env);
		(*env)->CallVoidMethod(env, *(vars->jobj), vars->message_mid, jbuffer);

		if ((*env)->ExceptionOccurred(env)) {
			fprintf(stderr, "C: Exception during call back method\n");
			(*env)->ExceptionDescribe(env);
			(*env)->ExceptionClear(env);
		}
		(*env)->DeleteLocalRef(env, jbuffer);*/
	}
	/**
	information callback returning the message to Java andexpecting a callback_variables_t client object
	*/
	void info_callback(const char *msg, void *client_data) {
		/*callback_variables_t* vars = (callback_variables_t*)client_data;
		JNIEnv *env = vars->env;
		jstring jbuffer;

		jbuffer = (*env)->NewStringUTF(env, msg);
		(*env)->ExceptionClear(env);
		(*env)->CallVoidMethod(env, *(vars->jobj), vars->message_mid, jbuffer);

		if ((*env)->ExceptionOccurred(env)) {
			fprintf(stderr, "C: Exception during call back method\n");
			(*env)->ExceptionDescribe(env);
			(*env)->ExceptionClear(env);
		}
		(*env)->DeleteLocalRef(env, jbuffer);*/
	}
	bool CoImageJ2K::Decode(Mat& mat, XFile* fp)
	{
		if (fp == NULL)
			return false;

		int i;
		int file_length = fp->size();
		if (file_length < 0)
			return false;

		unsigned char* src = new unsigned char[file_length];
		fp->read(src, file_length, 1);

		opj_dparameters_t parameters;	/* decompression parameters */
		opj_set_default_decoder_parameters(&parameters);
		parameters.decod_format = 0;// J2K_CFMT;

									/* get a decoder handle */
		opj_dinfo_t* dinfo = NULL;	/* handle to a decompressor */
		opj_cio_t *cio = NULL;
		opj_image_t *image = NULL;

		dinfo = opj_create_decompress(CODEC_JP2);

		opj_event_mgr_t event_mgr;
		memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
		event_mgr.error_handler = error_callback;
		event_mgr.warning_handler = warning_callback;
		event_mgr.info_handler = info_callback;
		callback_variables_t msgErrorCallback_vars;

		/* catch events using our callbacks and give a local context */
		opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, &msgErrorCallback_vars);

		/* setup the decoder decoding parameters using the current image and user parameters */
		opj_setup_decoder(dinfo, &parameters);

		/* open a byte stream */
		cio = opj_cio_open((opj_common_ptr)dinfo, src, file_length);

		/* decode the stream and fill the image structure */
		image = opj_decode(dinfo, cio);
		if (!image) {
			delete[]src;
			opj_destroy_decompress(dinfo);
			opj_cio_close(cio);
			return false;
		}

		/* close the byte stream */
		delete[]src;

		opj_cio_close(cio);

#ifdef CHECK_THRESHOLDS 
		long min_value, max_value;
#endif			
		int *ptr, *ptr1, *ptr2;
		short tempS;
		unsigned char tempUC, tempUC1, tempUC2;
		int w, h;
		w = image->comps[0].w;
		h = image->comps[0].h;
		if (image->numcomps == 3) 
		{
			ptr = image->comps[0].data;
			ptr1 = image->comps[1].data;
			ptr2 = image->comps[2].data;

			mat.create(h, w, MAT_Tuchar3);
#ifdef CHECK_THRESHOLDS 
			if (image->comps[0].sgnd) {
				min_value = -128;
				max_value = 127;
			}
			else {
				min_value = 0;
				max_value = 255;
			}
#endif			
			for (i = 0; i<w*h; i++) {
				tempUC = (unsigned char)(ptr[i]);
				tempUC1 = (unsigned char)(ptr1[i]);
				tempUC2 = (unsigned char)(ptr2[i]);
#ifdef CHECK_THRESHOLDS
				if (tempUC < min_value)
					tempUC = min_value;
				else if (tempUC > max_value)
					tempUC = max_value;
				if (tempUC1 < min_value)
					tempUC1 = min_value;
				else if (tempUC1 > max_value)
					tempUC1 = max_value;
				if (tempUC2 < min_value)
					tempUC2 = min_value;
				else if (tempUC2 > max_value)
					tempUC2 = max_value;
#endif
				mat.data.ptr[0][i * 3] = tempUC;
				mat.data.ptr[0][i * 3 + 1] = tempUC1;
				mat.data.ptr[0][i * 3 + 2] = tempUC2;
			}
		}
		else // 1 component 8 or 16 bpp image
		{
			ptr = image->comps[0].data;
			if (image->comps[0].prec <= 8) {
				mat.create(h, w, MAT_Tuchar);
#ifdef CHECK_THRESHOLDS 
				if (image->comps[0].sgnd) {
					min_value = -128;
					max_value = 127;
				}
				else {
					min_value = 0;
					max_value = 255;
				}
#endif								
				for (i = 0; i<w*h; i++) {
					tempUC = (unsigned char)(ptr[i]);
#ifdef CHECK_THRESHOLDS
					if (tempUC<min_value)
						tempUC = min_value;
					else if (tempUC > max_value)
						tempUC = max_value;
#endif
					mat.data.ptr[0][i] = tempUC;
				}
			}
			else
			{
				mat.create(h, w, MAT_Tuchar2);
#ifdef CHECK_THRESHOLDS 
				if (image->comps[0].sgnd) {
					min_value = -32768;
					max_value = 32767;
				}
				else {
					min_value = 0;
					max_value = 65535;
				}
#endif				
				for (i = 0; i<w*h; i++) {
					tempS = (short)(ptr[i]);
#ifdef CHECK_THRESHOLDS
					if (tempS<min_value) {
						printf("C: value %d truncated to %d\n", tempS, min_value);
						tempS = min_value;
					}
					else if (tempS > max_value) {
						printf("C: value %d truncated to %d\n", tempS, max_value);
						tempS = max_value;
					}
#endif
					mat.data.s[0][i] = tempS;
				}
			}
		}
		// free remaining structures
		if (dinfo) {
			opj_destroy_decompress(dinfo);
		}
		// free image data structure
		opj_image_destroy(image);
		return true;
	}

#if CVLIB_IMG_SUPPORT_ENCODE
	bool CoImageJ2K::Encode(const Mat& mimage, XFile* fp)
	{
		return false;
	}
#endif // COIMAGE_SUPPORT_ENCODE

}

#endif //CVLIB_IMG_SUPPORT_J2K

#endif//CVLIB_IMG_NOCODEC
