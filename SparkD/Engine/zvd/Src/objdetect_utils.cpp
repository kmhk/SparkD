#include "objdetect_utils.h"
#include "ScaleXY.h"

namespace cvlib
{
	namespace objutils
	{
		static int		pyramidLayerNum(const Mat& image, float rScale, int nMinSize)
		{
			return (int)(log(MIN(image.cols(), image.rows()) / (float)nMinSize) / log(rScale)) + 1;
		}

		int 	pyramidOfImage(const Mat& image, Mat** ppmatOut, int* pnCount, float rScale, int nMinSize, int object_size, int max_size)
		{
			assert(image.type() == MAT_Tuchar);

			int max_size_image = MIN(image.rows(), image.cols());
			if (max_size == -1)
				max_size = max_size_image;
			//float max_scale = (float)max_size / (float)object_size;

			int		nInWidth = image.cols();
			int		nInHeight = image.rows();
			int		nNum = pyramidLayerNum(image, rScale, nMinSize);
			Mat*	pmatOut;

			max_size = MIN(max_size, max_size_image);

			if (nNum > 0)
			{
				float aspect_ratio = (float)nInHeight / (float)nInWidth;
				float start_scale = (float)max_size / (float)max_size_image;
				float min_scale = (float)nMinSize / (float)max_size_image;

				pmatOut = new Mat[nNum];

				int i = nNum - 1;
				while (i >= 0)
				{
					float r = object_size / start_scale;
					int	nShrH, nShrW;
					if (nInWidth > nInHeight) {
						nShrH = cvutil::round(r);
						nShrW = cvutil::round(r / aspect_ratio);
					}
					else {
						nShrH = cvutil::round(r / aspect_ratio);
						nShrW = cvutil::round(r);
					}
					nShrW = (nShrW + 1) / 2 * 2;
					nShrH = (nShrH + 1) / 2 * 2;
					ip::resize(image, pmatOut[i], nShrW, nShrH, ip::INTER_LINEAR);
					start_scale = start_scale / rScale;
					i--;
				}

				*ppmatOut = pmatOut;
				*pnCount = nNum;
				return 1;
			}
			*ppmatOut = NULL;
			*pnCount = 0;

			return 0;
		}

		ImagePyramid::ImagePyramid()
		{
			m_psubimages = 0;
			m_countImages = 0;
			m_points = 0;
		}
		ImagePyramid::~ImagePyramid()
		{
			release();
		}
		void ImagePyramid::release()
		{
			m_countImages = 0;
			m_sizeImage = Size();
			if (m_psubimages != 0) {
				delete[]m_psubimages;
				m_psubimages = 0;
			}
			if (m_points != 0) {
				delete[]m_points;
				m_points = 0;
			}
		}

		int 	pyramidOfImage(const Mat& image, ImagePyramid& pyramid, float rScale, int nMinSize, int object_size, int max_size)
		{
			assert(image.type() == MAT_Tuchar);

			int		nInWidth = image.cols();
			int		nInHeight = image.rows();
			int		nNum = pyramidLayerNum(image, rScale, nMinSize);

			if (nNum <= 0) {
				pyramid.release();
				return 0;
			}

			if (pyramid.m_countImages != nNum || pyramid.m_sizeImage == image.size())
			{
				pyramid.release();
				pyramid.m_psubimages = new Mat[nNum];
				pyramid.m_countImages = nNum;
				pyramid.m_sizeImage = image.size();
			}

			if (max_size == -1)
				max_size = MIN(image.rows(), image.cols());
			float max_scale = (float)max_size / (float)object_size;

			int i = nNum;
			float	rShrRate = (float)(object_size) / (float)nMinSize;
			int	nShrW = cvutil::round(nInWidth * rShrRate);
			int nShrH = cvutil::round(nInHeight * rShrRate);
			while (i--)
			{
				float img_scale = (float)image.cols() / (float)nShrW;
				if (max_scale >= img_scale)
				{
					ip::resize(image, pyramid.m_psubimages[i], nShrW, nShrH, ip::INTER_LINEAR);
				}
				rShrRate /= rScale;
				nShrW = cvutil::round(nInWidth * rShrRate);
				nShrH = cvutil::round(nInHeight * rShrRate);
			}

			return 1;
		}

	}
}