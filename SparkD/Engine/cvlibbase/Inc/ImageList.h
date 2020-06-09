/*!
 * \file	ipCoreABC.h
 * \ingroup cvlibip
 * \brief   
 * \author  
 */

#pragma once

#include "Template.h"
#include "StringArray.h"
#include "Mat.h"

namespace cvlib
{

	enum enAlignType
	{
		AT_Center,
		AT_Up,
		AT_Down,
		AT_Left,
		AT_Right
	};
	enum enAxeType
	{
		AxeX,
		AxeY,
		AxeZ
	};

	class CVLIB_DECLSPEC ImageList
	{
	public:
	protected:
		//! This variable represents the number of images in the image list.
		/**
		   \note if \c size==0, the image list is empty.
		**/
		unsigned int m_nSize;

		//! This variable represents a pointer to the first \c Mat image of the list.
		/**
		   \note the images are stored continuously in memory.
		   \note If the list is empty, \c data=NULL.
		**/
		Mat *m_pData;                      //!< Pointer to the first image of the image list.
	public:
		//! create a list of \p n new images, each having size (\p width,\p height,\p depth,\p dim).
		ImageList(const unsigned int n = 0, const unsigned int width = 0, const unsigned int height = 1, TYPE type = MAT_Tbyte);

		// ! create a list of \p n copy of the input image.
		ImageList(const unsigned int n, const Mat& img);

		//! copy constructor.
		ImageList(const ImageList& list);

		//! create a list from a single image \p img.
		ImageList(const Mat& img);

		//! create a list from two images \p img1 and \p img2 (images are copied).
		ImageList(const Mat& img1, const Mat& img2);

		//! create a list from three images \p img1,\p img2 and \p img3 (images are copied).
		ImageList(const Mat& img1, const Mat& img2, const Mat& img3);

		//! create a list from four images \p img1,\p img2,\p img3 and \p img4 (images are copied).
		ImageList(const Mat& img1, const Mat& img2, const Mat& img3, const Mat& img4);

		//! copy a list into another one.
		ImageList& operator=(const ImageList& list);

		//! Destructor
		~ImageList()
		{
			if (m_pData)
			{
				for (unsigned int i = 0; i < m_nSize; i++)
					m_pData[i].release();
				delete[] m_pData;
				m_pData = NULL;
			}
		}

		//! empty list
		ImageList& empty()
		{
			return ImageList().swapList(*this);
		}

		//@}
		//------------------------------------------
		//------------------------------------------
		//
		//! \name List operations
		//@{
		//------------------------------------------
		//------------------------------------------

		int count() const { return m_nSize; }
		//! Return a reference to the i-th element of the image list.
		Mat& operator[](const unsigned int pos) const
		{
			return m_pData[pos];
		}

		//! Equivalent to ImageList::operator[]
		Mat& operator()(const unsigned int pos) const
		{
			return (*this)[pos];
		}

		//! insert a copy of the image \p img into the current image list, at position \p pos.
		bool insert(const Mat& img, const unsigned int nPos);
		bool insert(const Mat& img);

		void remove(const unsigned int nPos);
		void remove();

		//! append images of a list into a single image (which is returned), by concatenating them along the specified axe \p axe,
		// centering them using the alignment \p align.
		Mat* getAppend(const int nAxe, const int nAlign);
		Mat* getAppendAlign(const int nAxe, const int nAlign = 1);

		//! Same as \ref cimg::wait()
		// 	const ImageList& Wait(const unsigned int milliseconds) const { cimg::wait(milliseconds); return *this;  }
		ImageList& swapList(ImageList& list)
		{
			int tmp = list.m_nSize; list.m_nSize = m_nSize; m_nSize = tmp;
			Mat* ptr = list.m_pData; list.m_pData = m_pData; m_pData = ptr;
			return list;
		}
	};

	class CVLIB_DECLSPEC ImagePtrList : public Array<Mat*>
	{
	public:
		//! create a list from a single image \p img.
		ImagePtrList() {}
		ImagePtrList(Mat* pImg);
		ImagePtrList(Mat* pImg1, Mat* pImg2);
		ImagePtrList(Mat* pImg1, Mat* pImg2, Mat* pImg3);
		ImagePtrList(Mat* pImg1, Mat* pImg2, Mat* pImg3, Mat* pImg4);

		//! Destructor
		~ImagePtrList()
		{
			release();
		}

		void release()
		{
			for (int i = 0; i < getSize(); i++)
				delete getAt(i);
			removeAll();
		}

		void detachAll() {
			removeAll();
		}
		//! append images of a list into a single image (which is returned), by concatenating them along the specified axe \p axe,
		// centering them using the alignment \p align.
		void getAppend(const int nAxe, const int nAlign, Mat* pImage);
	};

	class CVLIB_DECLSPEC ImageDescPtrList
	{
	public:
		//! create a list from a single image \p img.
		ImageDescPtrList() {}

		//! Destructor
		~ImageDescPtrList()
		{
			release();
		}

		int count() { return m_images.getSize(); }

		void release()
		{
			for (int i = 0; i < m_images.getSize(); i++)
				delete m_images.getAt(i);
			m_images.removeAll();
			m_szDescArray.removeAll();
		}

		void detachAll()
		{
			m_images.removeAll();
			m_szDescArray.removeAll();
		}

		int add(Mat* pImage, const String& szDesc = String("noname"))
		{
			m_szDescArray.add(szDesc);
			return m_images.add(pImage);
		}

		Mat* getImage(int nIdx) { return m_images[nIdx]; }
		String getDesc(int nIdx) { return m_szDescArray[nIdx]; }
	public:
		Vector<Mat*> m_images;
		StringArray		m_szDescArray;
	};

	class CVLIB_DECLSPEC MatDescPtrList : public Vector<Mat*>
	{
	public:
		MatDescPtrList() {}

		//! Destructor
		~MatDescPtrList()
		{
			release();
		}

		void release()
		{
			for (int i = 0; i < getSize(); i++)
				delete getAt(i);
			removeAll();
		}

		void detachAll();
		//! append images of a list into a single image (which is returned), by concatenating them along the specified axe \p axe,
		// centering them using the alignment \p align.
		void getAppend(const int nAxe, const int nAlign, Mat& image);
	};

	CVLIB_DECLSPEC Mat imageGroup(const Mat& img1, const Mat& img2 = Mat(), const Mat& img3 = Mat(), const Mat& img4 = Mat());
	CVLIB_DECLSPEC Mat imageGroup(const Vector<Mat>& images);
	Mat makeImageWithComment(const Mat& image, const char* comment);
}