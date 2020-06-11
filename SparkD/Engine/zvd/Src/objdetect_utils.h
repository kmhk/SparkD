#pragma once

#include "cvlibbase/Inc/_cvlibbase.h"

namespace cvlib
{
	namespace objutils
	{
		
		CVLIB_DECLSPEC int 	pyramidOfImage(const Mat& image, Mat** ppmatOut, int* pnCount, float rScale, int nMinSize, int object_size, int max_size = -1);

		class CVLIB_DECLSPEC ImagePyramid
		{
		public:
			ImagePyramid();
			~ImagePyramid();
			void release();

			Size	m_sizeImage;
			int		m_countImages;

			Mat*	m_psubimages;
			std::vector<vec2i>*	m_points;
		};

		CVLIB_DECLSPEC int 	pyramidOfImage(const Mat& image, ImagePyramid& pyramid, float rScale, int nMinSize, int object_size, int max_size = -1);
	}
}