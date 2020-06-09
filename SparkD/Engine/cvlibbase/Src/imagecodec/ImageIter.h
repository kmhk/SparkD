/*! 
 * \file    ImageIter.h
 * \ingroup base
 * \brief   
 * \author  
 */

#pragma once

#include "Mat.h"
#include "CoImage.h"

namespace cvlib
{
/**
 * @brief  
 */
class CVLIB_DECLSPEC ImageIterator
{
	friend class CoImage;
public:
	
	ImageIterator ( Mat* pImage, CoImage* pCoImage );
	virtual ~ImageIterator ();

	void  Reset ();
	void  Upset ();
	void  setRow(uchar *buf, int n);
	void  GetRow(uchar *buf, int n);
	uchar  GetByte( ) { return m_pbIterImage[m_nItx]; }
	void  SetByte(uchar b) { m_pbIterImage[m_nItx] = b; }
	uchar* GetRow(void);
	uchar* GetRow(int n);
	bool  NextRow();
	bool  PrevRow();
	bool  NextByte();
	bool  PrevByte();
	void  SetSteps(int x, int y=0) {  m_nStepx = x; m_nStepy = y; }
	void  GetSteps(int *x, int *y) {  *x = m_nStepx; *y = m_nStepy; }
	bool  NextStep();
	bool  PrevStep();
	void  SetY(int y);	/* AD - for interlace */
	int   GetY() {return m_nIty;}
	void  BMP2XYZ(uchar* pbData = NULL);
	void  XYZ2BMP();
	uchar* m_pbTemp;

protected:
	int  m_nItx, m_nIty;
	int  m_nStepx, m_nStepy;
	uchar*    m_pbIterImage;
	Mat* m_pImage;
	CoImage*	m_pCoImage;
};

inline
ImageIterator::ImageIterator(Mat *pImage, CoImage* pCoImage): m_pImage(pImage),m_pCoImage(pCoImage)
{
	m_pbTemp = new uchar[m_pImage->rows() * m_pCoImage->GetEffWidth()];
	if (m_pImage) 
		m_pbIterImage = m_pbTemp;
	m_nItx = m_nIty = 0;
	m_nStepx = m_nStepy = 0;
}

inline
ImageIterator::~ImageIterator ()
{
	delete []m_pbTemp;
}

inline void ImageIterator::Reset()
{
	if (m_pImage) m_pbIterImage = m_pbTemp;
	else	 m_pbIterImage=0;
	m_nItx = m_nIty = 0;
}

inline void ImageIterator::Upset()
{
	m_nItx = 0;
	m_nIty = m_pImage->rows()-1;
	m_pbIterImage = m_pbTemp + m_pCoImage->GetEffWidth()*(m_pImage->rows()-1);
}

inline bool ImageIterator::NextRow()
{
	if (++m_nIty >= (int)m_pImage->rows()) 
		return 0;
	m_pbIterImage += m_pCoImage->GetEffWidth();
	return 1;
}

inline bool ImageIterator::PrevRow()
{
	if (--m_nIty < 0) return 0;
	m_pbIterImage -= m_pCoImage->GetEffWidth();
	return 1;
}
/* AD - for interlace */
inline void ImageIterator::SetY(int y)
{
	if ((y < 0) || (y > (int)m_pImage->rows())) 
		return;
	m_nIty = y;
	m_pbIterImage = m_pbTemp + m_pCoImage->GetEffWidth()*y;
}

inline void ImageIterator::setRow(uchar *buf, int n)
{
	if (n<0) n = (int)m_pCoImage->GetEffWidth();
	else n = MIN(n,(int)m_pCoImage->GetEffWidth());

	if ((m_pbIterImage!=NULL)&&(buf!=NULL)&&(n>0)) memcpy(m_pbIterImage,buf,n);
}

inline void ImageIterator::GetRow(uchar *buf, int n)
{
	if ((m_pbIterImage!=NULL)&&(buf!=NULL)&&(n>0)) memcpy(buf,m_pbIterImage,n);
}

inline uchar* ImageIterator::GetRow()
{
	return m_pbIterImage;
}

inline uchar* ImageIterator::GetRow(int n)
{
	SetY(n);
	return m_pbIterImage;
}

inline bool ImageIterator::NextByte()
{
	if (++m_nItx < (int)m_pCoImage->GetEffWidth()) 
		return 1;
	else
		if (++m_nIty < (int)m_pImage->rows())
		{
			m_pbIterImage += m_pCoImage->GetEffWidth();
			m_nItx = 0;
			return 1;
		}
		else
			return 0;
}

inline bool ImageIterator::PrevByte()
{
  if (--m_nItx >= 0) 
	  return 1;
  else
	  if (--m_nIty >= 0)
	  {
		  m_pbIterImage -= m_pCoImage->GetEffWidth();
		  m_nItx = 0;
		  return 1;
	  } 
	  else
		  return 0;
}

inline bool ImageIterator::NextStep()
{
	m_nItx += m_nStepx;
	if (m_nItx < (int)m_pCoImage->GetEffWidth()) 
		return 1;
	else 
	{
		m_nIty += m_nStepy;
		if (m_nIty < (int)m_pImage->rows())
		{
			m_pbIterImage += m_pCoImage->GetEffWidth();
			m_nItx = 0;
			return 1;
		}
		else
			return 0;
	}
}

inline bool ImageIterator::PrevStep()
{
	m_nItx -= m_nStepx;
	if (m_nItx >= 0) 
		return 1;
	else 
	{       
		m_nIty -= m_nStepy;
		if (m_nIty >= 0 && m_nIty < (int)m_pImage->rows()) 
		{
			m_pbIterImage -= m_pCoImage->GetEffWidth();
			m_nItx = 0;
			return 1;
		}
		else
			return 0;
	}
}

inline void ImageIterator::BMP2XYZ(uchar* pbData)
{
	uchar* pbTemp;
	if (pbData)
		pbTemp = pbData;
	else
		pbTemp = m_pbTemp;
	int cn = m_pImage->channels();
	for (int iH = 0; iH < m_pImage->rows(); iH ++)
	{
		uchar* pdst = m_pImage->data.ptr[m_pImage->rows() - iH - 1];
		if (m_pCoImage->m_Head.biClrUsed)
		{
			for (int iW = 0; iW < m_pImage->cols(); iW ++)
				m_pCoImage->GetPaletteColor(m_pCoImage->GetPixelIndex(pbTemp, iW), &pdst[cn*iW]);
		}
		else
		{
			for (int iW = 0, iEffW = 0, idw=0; iW < m_pImage->cols(); iW ++, idw+=cn)
			{
				pdst[idw] = pbTemp[iEffW ++];
				pdst[idw+1] = pbTemp[iEffW ++];
				pdst[idw+2] = pbTemp[iEffW ++];
			}
		}
		pbTemp += m_pCoImage->GetEffWidth();
	}
}

inline void ImageIterator::XYZ2BMP()
{
//	SetBpp24(m_pImage->cols(), m_pImage->rows());
	if (m_pbTemp)
		delete []m_pbTemp;
	m_pbTemp = new uchar[m_pImage->rows() * m_pCoImage->GetEffWidth()];

	uchar* pbTemp  = m_pbTemp;
	if (m_pImage) 
		m_pbIterImage = m_pbTemp;
	
	m_nItx = m_nIty = 0;
	m_nStepx = m_nStepy = 0;
	
	int cn = m_pImage->channels();
	if (cn <= 3)
	{
		for (int iH = 0; iH < m_pImage->rows(); iH ++)
		{
			uchar* psrc = m_pImage->data.ptr[m_pImage->rows() - iH - 1];
			memcpy (pbTemp, psrc, m_pImage->cols()*cn);
			pbTemp += m_pCoImage->GetEffWidth();
		}
	}
	else
	{
		for (int iH = 0; iH < m_pImage->rows(); iH ++)
		{
			uchar* psrc = m_pImage->data.ptr[m_pImage->rows() - iH - 1];
			for (int iW = 0, dstcn=0, srccn = 0; iW < m_pImage->cols(); iW ++, dstcn+=3, srccn+=4)
			{
				pbTemp[dstcn] = psrc[srccn];
				pbTemp[dstcn+1] = psrc[srccn+1];
				pbTemp[dstcn+2] = psrc[srccn+2];
			}
			pbTemp += m_pCoImage->GetEffWidth();
		}
	}
}

}