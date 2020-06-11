#include "ImageList.h"
#include "Drawing.h"

namespace cvlib
{

	const unsigned int mblock = 1024;

	static Scalar padColor(192, 192, 192, 192);

	ImageList::ImageList(const unsigned int n/*=0*/, const unsigned int width/*=0*/, const unsigned int height/*=1*/,
		TYPE type) :m_nSize(n)
	{
		if (n)
		{
			m_pData = new Mat[(mblock + 1)*mblock];
			for (unsigned int i = 0; i < m_nSize; i++) {
				m_pData[i] = Mat(height, width, type);
				m_pData[i] = 0;
			}
		}
		else
			m_pData = NULL;
	}

	ImageList::ImageList(const unsigned int n, const Mat& img) :m_nSize(n)
	{
		if (n)
		{
			m_pData = new Mat[(n / mblock + 1)*mblock];
			for (unsigned int i = 0; i < m_nSize; i++) {
				m_pData[i] = img;
			}
		}
		else
			m_pData = NULL;
	}

	ImageList::ImageList(const ImageList& list) :m_nSize(list.m_nSize)
	{
		if (m_nSize)
		{
			m_pData = new Mat[(m_nSize / mblock + 1)*mblock];
			for (unsigned int i = 0; i < m_nSize; i++)
				m_pData[i] = list[i];
		}
		else
			m_pData = NULL;
	}

	ImageList::ImageList(const Mat& img) :m_nSize(1), m_pData(NULL)
	{
		m_pData = new Mat[mblock];
		m_pData[0] = img;
	}

	ImageList::ImageList(const Mat& img1, const Mat& img2) :m_nSize(2)
	{
		m_pData = new Mat[mblock];
		m_pData[0] = img1;
		m_pData[1] = img2;
	}

	ImageList::ImageList(const Mat& img1, const Mat& img2, const Mat& img3) :m_nSize(3)
	{
		m_pData = new Mat[mblock];
		m_pData[0] = img1;
		m_pData[1] = img2;
		m_pData[2] = img3;
	}

	ImageList::ImageList(const Mat& img1, const Mat& img2, const Mat& img3, const Mat& img4) :m_nSize(4)
	{
		m_pData = new Mat[mblock];
		m_pData[0] = img1;
		m_pData[1] = img2;
		m_pData[2] = img3;
		m_pData[3] = img4;
	}

	ImageList& ImageList::operator=(const ImageList& list)
	{
		if (&list == this)
			return *this;
		return ImageList(list).swapList(*this);
	}

	bool ImageList::insert(const Mat& img, const unsigned int nPos)
	{
		if (nPos > m_nSize)
			return false;
		Mat empty;
		Mat* pNewImg = (!((++m_nSize) % mblock) || !m_pData) ? new Mat[(m_nSize / mblock + 1) * mblock] : NULL;
		if (!m_pData)
		{
			m_pData = pNewImg;
			*m_pData = img;
		}
		else
		{
			if (pNewImg)
			{
				unsigned int i;
				for (i = 0; i < nPos; i++)
					pNewImg[i] = m_pData[i];
				if (nPos != m_nSize - 1)
				{
					for (i = 0; i < m_nSize - 1 - nPos; i++)
						pNewImg[nPos + 1 + i] = m_pData[nPos + i];
				}
				for (i = 0; i < m_nSize - 1; i++)
					m_pData[i] = empty;
				delete[] m_pData;
				m_pData = pNewImg;
			}
			else if (nPos != m_nSize - 1)
			{
				for (unsigned int i = 0; i < m_nSize - 1 - nPos; i++)
					m_pData[nPos + 1 + i] = m_pData[i + nPos];
			}
			m_pData[nPos] = img;
		}
		empty.release();
		return true;
	}

	//! append a copy of the image \p img at the current image list.
	bool ImageList::insert(const Mat& img)
	{
		return insert(img, m_nSize);
	}

	void ImageList::remove(const unsigned int nPos)
	{
		if (nPos >= m_nSize)
			return;

		m_nSize--;
		if (nPos != m_nSize)
		{
			for (unsigned int i = 0; i < m_nSize - nPos; i++)
				m_pData[nPos + i] = m_pData[nPos + 1 + i];
		}
		m_pData[m_nSize].release();
		return;
	}

	//! remove the last image from the image list.
	void ImageList::remove()
	{
		remove(m_nSize);
	}

	Mat* ImageList::getAppend(const int nAxe, const int nAlign)
	{
		int dx = 0, dy = 0, nPos = 0;
		Mat* pRes = new Mat;
		switch (nAxe)
		{
		case AxeX:
		{
			for (unsigned int i = 0; i < m_nSize; i++)
			{
				const Mat& img = m_pData[i];
				dx += img.cols();
				dy = MAX(dy, img.rows());
			}
			pRes->create(dy, dx, MAT_Tbyte4);
			*pRes = padColor;
			switch (nAlign)
			{
			case AT_Up:
			{
				for (unsigned int j = 0; j < m_nSize; j++)
				{
					pRes->drawMat(m_pData[j], Point(nPos, 0));
					nPos += m_pData[j].cols();
				}
			}
			break;
			case AT_Down:
			{
				for (unsigned int j = 0; j < m_nSize; j++)
				{
					pRes->drawMat(m_pData[j], Point(nPos, dy - m_pData[j].rows()));
					nPos += m_pData[j].cols();
				}
			}
			break;
			case AT_Center:
			{
				for (unsigned int j = 0; j < m_nSize; j++)
				{
					pRes->drawMat(m_pData[j], Point(nPos, (dy - m_pData[j].rows()) / 2));
					nPos += m_pData[j].cols();
				}
			}
			break;
			}
		}
		break;
		case AxeY:
		{
			for (unsigned int i = 0; i < m_nSize; i++)
			{
				const Mat& img = m_pData[i];
				dy += img.rows();
				dx = MAX(dx, img.cols());
			}
			pRes->create(dy, dx, MAT_Tbyte4);
			*pRes = padColor;
			switch (nAlign)
			{
			case AT_Left:
			{
				for (unsigned int j = 0; j < m_nSize; j++)
				{
					pRes->drawMat(m_pData[j], Point(0, nPos));
					nPos += m_pData[j].rows();
				}
			}
			break;
			case AT_Right:
			{
				for (unsigned int j = 0; j < m_nSize; j++)
				{
					pRes->drawMat(m_pData[j], Point(dx - m_pData[j].cols(), nPos));
					nPos += m_pData[j].rows();
				}
			}
			break;
			case AT_Center:
			{
				for (unsigned int j = 0; j < m_nSize; j++)
				{
					pRes->drawMat(m_pData[j], Point((dx - m_pData[j].cols()) / 2, nPos));
					nPos += m_pData[j].rows();
				}
			}
			break;
			}
		}
		break;
		case AxeZ:
		{
		}
		break;
		}
		return pRes;
	}

	Mat* ImageList::getAppendAlign(const int nAxe, const int nAlign/*=1*/)
	{
		int dx = 0, dy = 0;
		Mat* pRes = new Mat;
		switch (nAxe)
		{
		case AxeX:
		{
			unsigned int i;
			int ywid = 0;
			int xwid = 0;
			for (i = 0; i < m_nSize; i++)
			{
				const Mat& img = m_pData[i];
				if ((i + 1) % nAlign == 0)
				{
					xwid = MAX(xwid, img.cols());
					dx += xwid;
					xwid = 0;
					ywid += img.rows();
					dy = MAX(dy, ywid);
					ywid = 0;
				}
				else
				{
					xwid = MAX(xwid, img.cols());
					ywid += img.rows();
				}
			}
			if (m_nSize%nAlign)
			{
				dx += xwid;
				dy = MAX(dy, ywid);
			}
			pRes->create(dy, dx, MAT_Tbyte);
			*pRes = padColor;
			xwid = 0;
			dx = dy = 0;
			for (i = 0; i < m_nSize; i++)
			{
				const Mat& img = m_pData[i];
				if ((i + 1) % nAlign == 0)
				{
					pRes->drawMat(img, Point(dx, dy));
					xwid = MAX(xwid, img.cols());
					dx += xwid;
					xwid = 0;
					dy = 0;
				}
				else
				{
					pRes->drawMat(img, Point(dx, dy));
					xwid = MAX(xwid, img.cols());
					dy += img.rows();
				}
			}
		}
		break;
		case AxeY:
		{
			assert(false);
		}
		break;
		case AxeZ:
		{
		}
		break;
		}
		return pRes;
	}

	ImagePtrList::ImagePtrList(Mat* pImg)
	{
		add(pImg);
	}

	ImagePtrList::ImagePtrList(Mat* pImg1, Mat* pImg2)
	{
		add(pImg1);
		add(pImg2);
	}

	ImagePtrList::ImagePtrList(Mat* pImg1, Mat* pImg2, Mat* pImg3)
	{
		add(pImg1);
		add(pImg2);
		add(pImg3);
	}

	ImagePtrList::ImagePtrList(Mat* pImg1, Mat* pImg2, Mat* pImg3, Mat* pImg4)
	{
		add(pImg1);
		add(pImg2);
		add(pImg3);
		add(pImg4);
	}

	void ImagePtrList::getAppend(const int nAxe, const int nAlign, Mat* pImage)
	{
		int dx = 0, dy = 0, nPos = 0;
		pImage->release();
		switch (nAxe)
		{
		case AxeX:
		{
			for (int i = 0; i < m_nSize; i++)
			{
				const Mat* img = getAt(i);
				dx += img->cols();
				dy = MAX(dy, img->rows());
			}
			pImage->create(dy, dx, MAT_Tbyte3);
			*pImage = padColor;
			switch (nAlign)
			{
			case AT_Up:
			{
				for (int j = 0; j < m_nSize; j++)
				{
					pImage->drawMat(*getAt(j), Point(nPos, 0));
					nPos += getAt(j)->cols();
				}
			}
			break;
			case AT_Down:
			{
				for (int j = 0; j < m_nSize; j++)
				{
					pImage->drawMat(*getAt(j), Point(nPos, dy - getAt(j)->rows()));
					nPos += getAt(j)->cols();
				}
			}
			break;
			case AT_Center:
			{
				for (int j = 0; j < m_nSize; j++)
				{
					pImage->drawMat(*getAt(j), Point(nPos, (dy - getAt(j)->rows()) / 2));
					nPos += getAt(j)->cols();
				}
			}
			break;
			}
		}
		break;
		case AxeY:
		{
			for (int i = 0; i < m_nSize; i++)
			{
				const Mat* img = getAt(i);
				dy += img->rows();
				dx = MAX(dx, img->cols());
			}
			pImage->create(dy, dx, MAT_Tbyte3);
			*pImage = padColor;
			switch (nAlign)
			{
			case AT_Left:
			{
				for (int j = 0; j < m_nSize; j++)
				{
					pImage->drawMat(*getAt(j), Point(0, nPos));
					nPos += getAt(j)->rows();
				}
			}
			break;
			case AT_Right:
			{
				for (int j = 0; j < m_nSize; j++)
				{
					pImage->drawMat(*getAt(j), Point(dx - getAt(j)->cols(), nPos));
					nPos += getAt(j)->rows();
				}
			}
			break;
			case AT_Center:
			{
				for (int j = 0; j < m_nSize; j++)
				{
					pImage->drawMat(*getAt(j), Point((dx - getAt(j)->cols()) / 2, nPos));
					nPos += getAt(j)->rows();
				}
			}
			break;
			}
		}
		break;
		case AxeZ:
		{
		}
		break;
		}
	}


	/************************************************************************************/
	void MatDescPtrList::getAppend(const int nAxe, const int nAlign, Mat& image)
	{
		if (getSize() == 0)
		{
			image.release();
			return;
		}
		int dx = 0, dy = 0, nPos = 0;
		image.release();
		switch (nAxe)
		{
		case AxeX:
		{
			for (int i = 0; i < m_nSize; i++)
			{
				const Mat* img = getAt(i);
				dx += img->cols();
				dy = MAX(dy, img->rows());
			}
			image.create(dy, dx, MAT_Tbyte3);
			image = padColor;
			switch (nAlign)
			{
			case AT_Up:
			{
				for (int j = 0; j < m_nSize; j++)
				{
					image.drawMat(*getAt(j), Point2i(nPos, 0));
					nPos += getAt(j)->cols();
				}
			}
			break;
			case AT_Down:
			{
				for (int j = 0; j < m_nSize; j++)
				{
					image.drawMat(*getAt(j), Point2i(nPos, dy - getAt(j)->rows()));
					nPos += getAt(j)->cols();
				}
			}
			break;
			case AT_Center:
			{
				for (int j = 0; j < m_nSize; j++)
				{
					image.drawMat(*getAt(j), Point2i(nPos, (dy - getAt(j)->rows()) / 2));
					nPos += getAt(j)->cols();
				}
			}
			break;
			}
		}
		break;
		case AxeY:
		{
			for (int i = 0; i < m_nSize; i++)
			{
				const Mat* img = getAt(i);
				dy += img->rows();
				dx = MAX(dx, img->cols());
			}
			image.create(dy, dx, MAT_Tbyte3);
			image = padColor;
			switch (nAlign)
			{
			case AT_Left:
			{
				for (int j = 0; j < m_nSize; j++)
				{
					image.drawMat(*getAt(j), Point2i(0, nPos));
					nPos += getAt(j)->rows();
				}
			}
			break;
			case AT_Right:
			{
				for (int j = 0; j < m_nSize; j++)
				{
					image.drawMat(*getAt(j), Point2i(dx - getAt(j)->cols(), nPos));
					nPos += getAt(j)->rows();
				}
			}
			break;
			case AT_Center:
			{
				for (int j = 0; j < m_nSize; j++)
				{
					image.drawMat(*getAt(j), Point2i((dx - getAt(j)->cols()) / 2, nPos));
					nPos += getAt(j)->rows();
				}
			}
			break;
			}
		}
		break;
		case AxeZ:
		{
		}
		break;
		}
	}

	void MatDescPtrList::detachAll()
	{
		removeAll();
	}

	Mat imageGroup(const Mat& img1, const Mat& img2, const Mat& img3, const Mat& img4)
	{
		ImageList imglist(img1, img2, img3, img4);
		Mat* pmret = imglist.getAppend(AxeX, AT_Up);
		Mat ret = *pmret;
		delete pmret;
		return ret;
	}

	Mat imageGroup(const Vector<Mat>& images)
	{
		ImageList imglist;
		for (int i = 0; i < images.getSize(); i++) {
			imglist.insert(images[i]);
		}
		Mat* pmret = imglist.getAppend(AxeX, AT_Up);
		Mat ret = *pmret;
		delete pmret;
		return ret;
	}

	Mat makeImageWithComment(const Mat& image, const char* comment)
	{
		COLOR margin_color(0, 0, 255);
		int margin = 2;
		int base_line = 0;
		Size textsize;
		if (comment) {
			textsize = getTextSize(comment, FONT_HERSHEY_SIMPLEX, 1.0, 1, &base_line);
		}
		Mat newMat(margin * 3 + image.rows() + textsize.height + base_line, margin * 2 + MAX(image.cols(), textsize.width), image.type1());
		newMat = Scalar(0, 0, 0, 255);
		newMat.drawRect(Rect(0, 0, newMat.cols(), newMat.rows()), margin_color, 2);
		newMat.drawRect(Rect(0, margin + image.rows(), newMat.cols(), textsize.height + base_line), margin_color, 2);
		newMat.drawMat(image, vec2i(margin, margin));
		if (comment) {
			newMat.drawString(comment, vec2i(0, newMat.rows() - base_line), FONT_HERSHEY_SIMPLEX, 1.0, COLOR(255, 255, 255), 1);
		}
		return newMat;
	}

}