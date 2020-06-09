
/*!
 * \file	HaarFeature.h
 * \ingroup 
 * \brief	
 * \author	
 */

#pragma once

#include "PtrArray.h"
#include "cvlibstructs.h"

namespace cvlib
{

#define CVLIB_HAAR_FEATURE_DESC_MAX 20
#define CVLIB_HAAR_FEATURE_MAX  3

	struct STHaarFeature
	{
		char desc[CVLIB_HAAR_FEATURE_DESC_MAX];
		int  tilted;
		struct STHaarFeature_internal
		{
			Rect r;
			float weight;
		} rect[CVLIB_HAAR_FEATURE_MAX];
	};

	/**
	 * @brief
	 */
	struct SFastHaarFeature
	{
		int tilted;
		struct SFastHaarFeature_internal
		{
			int p0, p1, p2, p3;
			float weight;
		} rect[CVLIB_HAAR_FEATURE_MAX];
	};

	struct SHaarFeatures
	{
		Size winsize;
		int count;
		STHaarFeature* feature;
		SFastHaarFeature* fastfeature;
	};

	class CVLIB_DECLSPEC HAARLIST
	{
	public:
		PtrArray HaarFeatureList;
	public:
		HAARLIST() {};
		~HAARLIST() {};

		void	Push_back(STHaarFeature* pTHaarFeature);
		STHaarFeature* getAt(int nIdx);
		void	release();
		int		getSize() { return HaarFeatureList.getSize(); }
	};

	/*
	 * get sum image offsets for <rect> corner points
	 * step - row step (measured in image pixels!) of sum image
	 */
#define CVLib_SUM_OFFSETS( p0, p1, p2, p3, rect, step )                      \
    /* (x, y) */                                                          \
    (p0) = (rect).x + (step) * (rect).y;                                  \
    /* (x + w, y) */                                                      \
    (p1) = (rect).x + (rect).width + (step) * (rect).y;                   \
    /* (x, y + h) */                                                      \
    (p2) = (rect).x + (step) * ((rect).y + (rect).height);                \
    /* (x + w, y + h) */                                                  \
    (p3) = (rect).x + (rect).width + (step) * ((rect).y + (rect).height);

	 /*
	  * get tilted image offsets for <rect> corner points
	  * step - row step (measured in image pixels!) of tilted image
	  */
#define CVLib_TILTED_OFFSETS( p0, p1, p2, p3, rect, step )                   \
    /* (x, y) */                                                          \
    (p0) = (rect).x + (step) * (rect).y;                                  \
    /* (x - h, y + h) */                                                  \
    (p1) = (rect).x - (rect).height + (step) * ((rect).y + (rect).height);\
    /* (x + w, y + w) */                                                  \
    (p2) = (rect).x + (rect).width + (step) * ((rect).y + (rect).width);  \
    /* (x + w - h, y + w + h) */                                          \
    (p3) = (rect).x + (rect).width - (rect).height                        \
           + (step) * ((rect).y + (rect).width + (rect).height);

	CVLIB_DECLSPEC SHaarFeatures*	initPossibleFeatures(Size winsize, int nMode, int nSymmetric);

	CVLIB_DECLSPEC void		saveHaarFeature(STHaarFeature* feature, FILE* fp);
	CVLIB_DECLSPEC void		loadHaarFeature(STHaarFeature* feature, FILE* fp);
	CVLIB_DECLSPEC float		evalFastHaarFeature(const SFastHaarFeature* feature,
		int* sum, int* tilted);
	CVLIB_DECLSPEC STHaarFeature*	haarFeature(const char* desc,
		int x0, int y0, int w0, int h0, float wt0,
		int x1, int y1, int w1, int h1, float wt1,
		int x2 = 0, int y2 = 0, int w2 = 0, int h2 = 0, float wt2 = 0);


	CVLIB_DECLSPEC void	convertToFastHaarFeature(STHaarFeature* feature, SFastHaarFeature* pFastHF,
		int size, int stepx, int stepy);

}
