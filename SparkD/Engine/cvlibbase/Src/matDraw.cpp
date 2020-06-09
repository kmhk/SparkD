/*!
 * \file matdraw.cpp
 * \ingroup base
 * \brief 
 * \author 
 */

#include "cvlibmacros.h"

#ifdef _MSC_VER
#include <windows.h>
#include "TCHAR.h"
#endif

#include "Mat.h"
#include "imagecodec/CoImage.h"
#include "imagecodec/BmpStructs.h"
#include "Template.h"
#include "cvlibutil.h"
#include "ColorSpace.h"
#include "Drawing.h"

namespace cvlib 
{

	void putText(Mat& img, const String& text, const Point2i& org,
		int fontFace, double fontScale, Scalar color,
		int thickness, int line_type, bool bottomLeftOrigin);

	BitmapData::BitmapData() :pdata(0),effWidth(0){}
	BitmapData::~BitmapData() {if(pdata) delete[]pdata;}
	BITMAPINFOHEADER1*	BitmapData::getBitmapInfoHeader() { return (BITMAPINFOHEADER1*)(const uchar*)pdata;}
	uchar*				BitmapData::getBuffer() {return (uchar*)pdata+sizeof(BITMAPINFOHEADER1);}
	int					BitmapData::getEffWidth() {return effWidth;}

	int Mat::create(const BITMAPINFOHEADER1* pBI)
	{
		uchar* pBMPData = (uchar*)pBI + sizeof(BITMAPINFOHEADER1);
		int nH = pBI->biHeight;
		int nW = pBI->biWidth;

		if (pBI->biBitCount == 32)
			create(nH, nW, MAT_Tbyte4);
		else
			create(nH, nW, MAT_Tbyte3);

		int cn = channels();
		int nRealWidth = (nW * pBI->biBitCount + 31) / 32 * 4;

		int nTemp;
		ushort wRGB;
		int i, j;
		switch (pBI->biBitCount)
		{
		case 24:
			for (i = 0; i < nH; i++)
			{
				nTemp = (nH - i - 1) * nRealWidth;
				uchar* pdata = data.ptr[i];
				int jcn = 0;
				for (j = 0; j < nW; j++, nTemp += 3)
				{
					pdata[jcn++] = pBMPData[nTemp + 2];
					pdata[jcn++] = pBMPData[nTemp + 1];
					pdata[jcn++] = pBMPData[nTemp];
				}
			}
			break;
		case 32:
		{
			bool bAlphaOk = false;
			for (i = 0; i < nH; i++)
			{
				uchar* pbdata = data.ptr[i];
				nTemp = (nH - i - 1) * nRealWidth;
				int k = 0;
				for (j = 0; j < nW; j++)
				{
					pbdata[k++] = pBMPData[nTemp + 2];
					pbdata[k++] = pBMPData[nTemp + 1];
					pbdata[k++] = pBMPData[nTemp + 0];
					pbdata[k++] = pBMPData[nTemp + 3];
					if (pBMPData[nTemp + 3]) bAlphaOk = true;
					nTemp += 4;
				}
			}
			if (!bAlphaOk)
			{
				for (i = 0; i < nH; i++)
				{
					uchar* pbdata = data.ptr[i];
					int k = 3;
					for (j = 0; j < nW; j++, k += 4)
					{
						pbdata[k] = (uchar)~pbdata[k];
					}
				}
			}
		}
		break;
		case 16:
			for (i = 0; i < nH; i++) for (j = 0; j < nW; j++)
			{
				nTemp = (nH - i - 1) * nRealWidth + j * 2;
				wRGB = (ushort)(pBMPData[nTemp] + (pBMPData[nTemp + 1] << 8));
				data.ptr[i][j*cn + 2] = (wRGB & 0x1F) << 3;
				data.ptr[i][j*cn + 1] = ((wRGB >> 5) & 0x1F) << 3;
				data.ptr[i][j*cn] = ((wRGB >> 10) & 0x1F) << 3;
			}
			break;
		case 12:
			for (i = 0; i < nH; i++) for (j = 0; j < nW; j++)
			{
				nTemp = (nH - i - 1) * nRealWidth + j * 2;
				wRGB = (ushort)(pBMPData[nTemp] + (pBMPData[nTemp + 1] << 8));
				data.ptr[i][j*cn + 2] = (wRGB & 0x1F) << 3;
				data.ptr[i][j*cn + 1] = ((wRGB >> 5) & 0x1F) << 3;
				data.ptr[i][j*cn] = ((wRGB >> 10) & 0x1F) << 3;
			}
			break;
		case 8:
		case 4:
		case 1:
			// 		if (pBI->bfOffBits != 0L)
			// 			hFile->seek(off + bf.bfOffBits,SEEK_SET);
			// 		switch (dwCompression) {
			break;
		}
		return 1;
	}

	void Mat::drawRect(const Rect& rect, COLOR color, int thickness/*=1*/, const float rOpacity/*=1*/)
	{
		if (!isInside(rect.x, rect.y) || rect.limx() > m_cols || rect.limy() > m_rows) 
		{
			vec2i points[] = { rect.tl(), vec2i(rect.x, rect.limy()), rect.br(), vec2i(rect.limx(), rect.y), rect.tl() };
			for (int i = 0; i < 4; i++) 
			{
				vec2i pt1 = points[i], pt2 = points[i+1];
				clipLine(Rect(0, 0, cols(), rows()), pt1, pt2);
				this->drawLine(pt1, pt2, color, thickness, rOpacity);
			}
			return;
		}
		if (rect.width == 0 || rect.height == 0)
			return;
		Rect outRegion(rect.x - thickness / 2, rect.y - thickness / 2, rect.width + thickness, rect.height + thickness);
		if (outRegion.x < 0 || outRegion.y < 0 || outRegion.limx() > m_cols || outRegion.limy() > m_rows)
		{
			Rect workRegion(thickness, thickness, rect.width, rect.height);
			Mat t(m_rows + thickness * 2, m_cols + thickness * 2, type1());
			Mat mref;
			t.subRefMat(workRegion, mref);
			mref = *this;
			vec2i points[5];
			points[0] = vec2i(rect.x + thickness, rect.y + thickness);
			points[1] = vec2i(rect.limx() + thickness, rect.y + thickness);
			points[2] = vec2i(rect.limx() + thickness, rect.limy() + thickness);
			points[3] = vec2i(rect.x + thickness, rect.limy() + thickness);
			points[4] = vec2i(rect.x + thickness, rect.y + thickness);
			t.drawPolygon(points, 5, color, thickness, true, rOpacity);
			*this = mref;
		}
		else
		{
			vec2i points[5];
			points[0] = vec2i(rect.x, rect.y);
			points[1] = vec2i(rect.limx(), rect.y);
			points[2] = vec2i(rect.limx(), rect.limy());
			points[3] = vec2i(rect.x, rect.limy());
			points[4] = vec2i(rect.x, rect.y);
			drawPolygon(points, 5, color, thickness, true, rOpacity);
		}
	}
	void Mat::drawRect(const RotatedRect& rect, COLOR color, int thickness/*=1*/, const float rOpacity/*=1*/)
	{
		vec2f points[4];
		rect.points(points);
		for (int i = 0; i < 4; i++) {
			drawLine(points[i], points[(i + 1) % 4], color, thickness, rOpacity);
		}
	}
	void	Mat::drawCircle(const Point2i& center, int radius, COLOR color, int thickness, const float rOpacity)
	{
		Rect r(center.x - radius, center.y - radius, radius * 2 + 1, radius * 2 + 1);
		drawEllipse(r, color, thickness, rOpacity);
	}
	void	Mat::drawEllipse(const Rect& rect, COLOR color, int thickness, const float rOpacity/*=1*/)
	{
		if (rect.width <= 0 || rect.height <= 0)
			return;

		Vector<vec2i> points;
		circlePoints(rect, points);
		vec2i first = points[0];
		points.add(first);
		drawPolygon(points.getData(), points.getSize(), color, thickness, true, rOpacity);
	}

	void	Mat::drawEllipse(const RotatedRect& rect, COLOR color, int thickness, const float rOpacity/*=1*/)
	{
#define XY_ONE 65536
#define XY_SHIFT 16
		Size axes((int)rect.size.width, (int)rect.size.height);
		axes.width = ABS(axes.width) / 2, axes.height = ABS(axes.height) / 2;
        int delta;// = (MAX(axes.width, axes.height) + (XY_ONE >> 1)) >> XY_SHIFT;
		//		delta = delta < 3 ? 90 : delta < 10 ? 30 : delta < 15 ? 18 : 5;
		delta = 5;
		Point2i center((int)rect.center.x, (int)rect.center.y);
		int arc_start = 0;
		int arc_end = 360;
		Vector<Point2i> v;
		ellipse2Poly(center, axes, (int)rect.angle, arc_start, arc_end, delta, v);
		vec2i first = v[0];
		v.add(first);
		drawPolygon(v.getData(), v.getSize(), color, thickness, false, rOpacity);
	}
	void	Mat::drawPolygon(const Point2i* pts, int num, COLOR color, int thickness, bool open, const float rOpacity)
	{
		if (num < 0)
			return;
		vec2i* points = 0;
		int num_point = num;
		if (open)
		{
			points = new vec2i[num];
			memcpy(points, pts, sizeof(vec2i)*num);
		}
		else
		{
			points = new vec2i[num + 1];
			memcpy(points, pts, sizeof(vec2i)*num);
			points[num] = pts[0];
			num_point = num + 1;
		}
		Mat mselection;
		Rect boundingRegion;
		selectionPolygonLine(*this, mselection, boundingRegion, points, num_point, 255, thickness);
		const float rNopacity = ABS(rOpacity), rCopacity = 1 - MAX(rOpacity, 0.0f);
		if (ABS(rCopacity) < 1E-3)
		{
			int _step = this->channels()*m_step;
			for (int y = boundingRegion.y; y < boundingRegion.limy(); y++)
			{
				const uchar* pmask = mselection.data.ptr[y];
				uchar* dst = data.ptr[y];
				for (int x = boundingRegion.x; x < boundingRegion.limx(); x++)
				{
					if (pmask[x] == 255)
						memcpy(&dst[x*_step], color.vec_array, _step);
				}
			}
		}
		else
		{
			const uchar* pcolor = (const uchar*)color.vec_array;
			int _step = channels()*step();
			for (int y = boundingRegion.y; y < boundingRegion.limy(); y++)
			{
				const uchar* pmask = mselection.data.ptr[y];
				uchar* dst = data.ptr[y];
				for (int x = boundingRegion.x; x < boundingRegion.limx(); x++)
				{
					if (pmask[x] == 255)
					{
						for (int k = 0; k < _step; k++)
						{
							dst[x*_step + k] = (uchar)(rCopacity*dst[x*_step + k] + rNopacity*pcolor[k]);
						}
					}
				}
			}
		}
		delete[]points;
	}
	void	Mat::drawPixel(const Point2i& pt, COLOR color, const float rOpacity/*=1*/)
	{
		drawPixel(pt.x, pt.y, color, rOpacity);
	}
	void	Mat::drawPixel(int x, int y, COLOR color, const float rOpacity)
	{
		int cn = channels();
		const uchar* ucolor = color.vec_array;
		if (isInside(x, y))
		{
			const float rNopacity = ABS(rOpacity), rCopacity = 1 - MAX(rOpacity, 0.0f);

			if (rNopacity >= 1)
			{
				for (int k = 0; k < cn; k++)
					data.ptr[y][x*cn + k] = ucolor[k];
			}
			else
			{
				for (int k = 0; k < cn; k++)
					data.ptr[y][x*cn + k] = (uchar)(data.ptr[y][x*cn + k] * rCopacity + ucolor[k] * rNopacity);
			}
		}
	}
	void	Mat::drawLine(const Point2i& pt1, const Point2i& pt2, COLOR color, int thickness/* = 1*/, const float rOpacity/*=1*/)
	{
		int cn = channels();
		const uchar* ucolor = color.vec_array;
		int* pnX;
		int* pnY;
		int nNum;
		linePoints(pt1.x, pt1.y, pt2.x, pt2.y, pnX, pnY, nNum);
		const float rNopacity = ABS(rOpacity), rCopacity = 1 - MAX(rOpacity, 0.0f);

		if (thickness == 1)
		{
			if (isInside(pt1.x, pt1.y) && isInside(pt2.x, pt2.y))
			{
				if (rNopacity >= 1)
				{
					for (int ii = 0; ii < nNum; ii++)
					{
						uchar* p = data.ptr[pnY[ii]];
						int icn = pnX[ii] * cn;
						for (int k = 0; k < cn; k++, icn++)
							p[icn] = ucolor[k];
					}
				}
				else
				{
					for (int ii = 0; ii < nNum; ii++)
					{
						uchar* p = data.ptr[pnY[ii]];
						for (int k = 0; k < cn; k++)
							p[pnX[ii] * cn + k] = (uchar)(p[pnX[ii] * cn + k] * rCopacity + ucolor[k] * rNopacity);
					}
				}
			}
			else
			{
				if (rNopacity >= 1)
				{
					for (int ii = 0; ii < nNum; ii++)
					{
						if (isInside(pnX[ii], pnY[ii]))
						{
							for (int k = 0; k < cn; k++)
								data.ptr[pnY[ii]][pnX[ii] * cn + k] = ucolor[k];
						}
					}
				}
				else
				{
					for (int ii = 0; ii < nNum; ii++)
					{
						if (isInside(pnX[ii], pnY[ii]))
						{
							for (int k = 0; k < cn; k++)
								data.ptr[pnY[ii]][pnX[ii] * cn + k] = (uchar)(data.ptr[pnY[ii]][pnX[ii] * cn + k] * rCopacity + ucolor[k] * rNopacity);
						}
					}
				}
			}
		}
		else
		{
			vec2f dir;
			{
				dir.x = (float)(pt2.x - pt1.x), dir.y = (float)(pt2.y - pt1.y);
				float r = 1.0f / dir.norm();
				dir.x *= r, dir.y *= r;
			}
			vec2f dir2(dir.y, -dir.x);
			float rthick = thickness*0.5f;
			vec2i points[4];
			points[0].x = cvutil::round(pt1.x + rthick*dir2.x), points[0].y = cvutil::round(pt1.y + rthick*dir2.y);
			points[1].x = cvutil::round(pt2.x + rthick*dir2.x), points[1].y = cvutil::round(pt2.y + rthick*dir2.y);
			points[2].x = cvutil::round(pt2.x - rthick*dir2.x), points[2].y = cvutil::round(pt2.y - rthick*dir2.y);
			points[3].x = cvutil::round(pt1.x - rthick*dir2.x), points[3].y = cvutil::round(pt1.y - rthick*dir2.y);
			this->fillConvexPoly(points, 4, color, rOpacity);
		}
		delete[]pnX;
		delete[]pnY;
	}
	void Mat::drawCross(const Point2i& pt, int nSide, COLOR color, int thickness/*=1*/, const float rOpacity/*=1*/)
	{
		if (isInside(pt.x, pt.y))
		{
			drawLine(Point2i(MAX(0, pt.x - nSide), pt.y), Point2i(MIN(pt.x + nSide + 1, cols() - 1), pt.y), color, thickness, rOpacity);
			drawLine(Point2i(pt.x, MAX(0, pt.y - nSide)), Point2i(pt.x, MIN(rows() - 1, pt.y + nSide + 1)), color, thickness, rOpacity);
		}
	}

	void Mat::drawMat(const Mat& img, const Point2i& pt, const float rOpacity/*=1*/)
	{
		if (!isValid())
			return;

		int cn = channels();
		int scn = img.channels();

		const float rNopacity = ABS(rOpacity), rCopacity = 1 - MAX(rOpacity, 0.0f);
		if (cn == scn)
		{
			if (rNopacity >= 1)
			{
				for (int iY = pt.y, j = 0; iY < MIN(m_rows, pt.y + img.m_rows); iY++, j++)
					for (int iX = pt.x, i = 0; iX < MIN(m_cols, pt.x + img.m_cols); iX++, i++)
					{
						for (int k = 0; k < cn; k++)
							data.ptr[iY][iX*cn + k] = img.data.ptr[j][i*cn + k];
					}
			}
			else
			{
				for (int iY = pt.y, j = 0; iY < MIN(m_rows, pt.y + img.m_rows); iY++, j++)
					for (int iX = pt.x, i = 0; iX < MIN(m_cols, pt.x + img.m_cols); iX++, i++)
					{
						for (int k = 0; k < cn; k++)
							data.ptr[iY][iX*cn + k] = (uchar)(data.ptr[iY][iX*cn + k] * rCopacity + img.data.ptr[j][i*cn + k] * rNopacity);
					}
			}
		}
		else
		{
			if (cn == 3)
			{
				if (rNopacity >= 1)
				{
					if (scn == 1)
					{
						for (int iY = pt.y, j = 0; iY < MIN(m_rows, pt.y + img.m_rows); iY++, j++)
						{
							for (int iX = pt.x, i = 0; iX < MIN(m_cols, pt.x + img.m_cols); iX++, i++)
							{
								for (int k = 0; k < cn; k++)
									data.ptr[iY][iX*cn + k] = img.data.ptr[j][i];
							}
						}
					}
					else
					{
						for (int iY = pt.y, j = 0; iY < MIN(m_rows, pt.y + img.m_rows); iY++, j++)
						{
							for (int iX = pt.x, i = 0; iX < MIN(m_cols, pt.x + img.m_cols); iX++, i++)
							{
								for (int k = 0; k < cn; k++)
									data.ptr[iY][iX*cn + k] = img.data.ptr[j][i*scn + k];
							}
						}
					}
				}
				else
				{
					for (int iY = pt.y, j = 0; iY < MIN(m_rows, pt.y + img.m_rows); iY++, j++)
					{
						for (int iX = pt.x, i = 0; iX < MIN(m_cols, pt.x + img.m_cols); iX++, i++)
						{
							for (int k = 0; k < cn; k++)
								data.ptr[iY][iX*cn + k] = (uchar)(data.ptr[iY][iX*cn + k] * rCopacity + img.data.ptr[j][i] * rNopacity);
						}
					}
				}
			}
			else if (cn == 4)
			{
				Mat image4;
				ColorSpace::toRGBA(img, image4);
				if (rNopacity >= 1)
				{
					for (int iY = pt.y, j = 0; iY < MIN(m_rows, pt.y + image4.m_rows); iY++, j++)
					{
						for (int iX = pt.x, i = 0; iX < MIN(m_cols, pt.x + image4.m_cols); iX++, i++)
						{
							for (int k = 0; k < cn; k++)
								data.ptr[iY][iX*cn + k] = image4.data.ptr[j][i*cn + k];
						}
					}
				}
				else
				{
					for (int iY = pt.y, j = 0; iY < MIN(m_rows, pt.y + image4.m_rows); iY++, j++)
					{
						int dpos = pt.x*cn;
						int spos = 0;
						int lim_pos = (MIN(m_cols, pt.x + image4.m_cols) - 1) * cn + 1;
						for (int iX = dpos, i = spos; iX < lim_pos; iX++, i++)
						{
							data.ptr[iY][iX] = (uchar)(data.ptr[iY][iX] * rCopacity + image4.data.ptr[j][i] * rNopacity);
						}
					}
				}
			}
		}
	}
	void Mat::getBMPInfoHeader(BitmapData& bmp) const
	{
		assert(type() == MAT_Tbyte);
		int nH = m_rows;
		int nW = m_cols;
		int cn = channels();
		ushort biBitCount;
		if (cn == 1)
			biBitCount = 24;
		else
			biBitCount = cn == 4 ? 32 : 24;

		int nRealWidth = bmp.effWidth = (nW * biBitCount + 31) / 32 * 4;

		uchar* pBuf = bmp.pdata = new uchar[sizeof(BITMAPINFOHEADER1) + nH * nRealWidth];
		BITMAPINFOHEADER1* pBMPHeader = (BITMAPINFOHEADER1*)(uchar*)pBuf;
		uchar* pbData = pBuf + sizeof(BITMAPINFOHEADER1);

		memset(pBMPHeader, 0, sizeof(BITMAPINFOHEADER1));
		pBMPHeader->biBitCount = biBitCount;
		pBMPHeader->biHeight = nH;
		pBMPHeader->biWidth = nW;
		pBMPHeader->biSizeImage = nH * nRealWidth;
		pBMPHeader->biSize = sizeof(BITMAPINFOHEADER1);
		pBMPHeader->biPlanes = 1;
		pBMPHeader->biXPelsPerMeter = (int)floor(96 * 10000.0 / 254.0 + 0.5);
		pBMPHeader->biYPelsPerMeter = (int)floor(96 * 10000.0 / 254.0 + 0.5);

		if (cn == 1)
		{
			for (int i = 0; i < nH; i++)
			{
				uchar* pbSrc = data.ptr[i];
				int nTemp = (nH - i - 1) * nRealWidth;
				for (int j = 0; j < nW; j++)
				{
					pbData[nTemp++] = pbSrc[j];
					pbData[nTemp++] = pbSrc[j];
					pbData[nTemp++] = pbSrc[j];
				}
			}
		}
		else if (cn == 3)
		{
			int size = nW*cn;
			for (int i = 0; i < nH; i++)
			{
				uchar* psrc = data.ptr[i];
				int nTemp = (nH - i - 1) * nRealWidth;
				for (int j = 0; j < size; j += cn)
				{
					pbData[nTemp + 2] = psrc[j];
					pbData[nTemp + 1] = psrc[j + 1];
					pbData[nTemp + 0] = psrc[j + 2];
					nTemp += 3;
				}
			}
		}
		else if (cn == 4)
		{
			int size = nW*cn;
			for (int i = 0; i < nH; i++)
			{
				uchar* psrc = data.ptr[i];
				int nTemp = (nH - i - 1) * nRealWidth;
				for (int j = 0; j < size; j += cn)
				{
					pbData[nTemp + 2] = psrc[j];
					pbData[nTemp + 1] = psrc[j + 1];
					pbData[nTemp + 0] = psrc[j + 2];
					pbData[nTemp + 3] = psrc[j + 3];
					nTemp += 4;
				}
			}
		}
		return;
	}

	void	Mat::drawString(const char* szText, const Point2i& pt, int fontFace, double fontScale,
		COLOR color, int thickness, int lineType, bool bottomLeftOrigin)
	{
		Scalar scalar;
		scalar.x = color.x;
		scalar.y = color.y;
		scalar.z = color.z;
		scalar.w = color.w;
		putText(*this, String(szText), pt, fontFace, fontScale, scalar, thickness, lineType, bottomLeftOrigin);
	}

#ifdef _MSC_VER
//#if CVLIB_OS==CVLIB_OS_WIN32
	void Mat::drawTo(Handle hDC, int nDstX, int nDstY, int nDstCX, int nDstCY) const
	{
		drawTo(hDC, nDstX, nDstY, nDstCX, nDstCY, 0, 0, cols(), rows());
	}
	void Mat::drawTo(Handle hDC, int nDstX, int nDstY, int nDstCX, int nDstCY,
		int nSrcX, int nSrcY, int nSrcCX, int nSrcCY) const
	{
		if (!isValid())
			return;
		//required for MM_ANISOTROPIC, MM_HIENGLISH, and similar modes [Greg Peatfield]
		int hdc_Restore = ::SaveDC((HDC)hDC);
		if (!hdc_Restore)
			return;

		/*		RECT clipbox,paintbox;
				GetClipBox((HDC)hDC,&clipbox);

				paintbox.top = MIN(clipbox.bottom,MAX(clipbox.top,y));
				paintbox.left = MIN(clipbox.right,MAX(clipbox.left,x));
				paintbox.right = MAX(clipbox.left,MIN(clipbox.right,x+cx));
				paintbox.bottom = MAX(clipbox.top,MIN(clipbox.bottom,y+cy));*/
		int cn = channels();
		if (cn <= 3)
		{
			BitmapData bmpdata;
			getBMPInfoHeader(bmpdata);

			const BITMAPINFOHEADER1* pBMPHeader = bmpdata.getBitmapInfoHeader();
			int nPrevMode = SetStretchBltMode(*(HDC*)&hDC, HALFTONE);
			RealizePalette(*(HDC*)&hDC);
			BITMAPINFO1 bmpInfo;
			memcpy(&bmpInfo.bmiHeader, pBMPHeader, sizeof(BITMAPINFOHEADER1));
			bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER1);
			bmpInfo.bmiHeader.biHeight = m_rows;
			bmpInfo.bmiHeader.biWidth = m_cols;
			bmpInfo.bmiHeader.biPlanes = 1;
			bmpInfo.bmiHeader.biBitCount = pBMPHeader->biBitCount;
			::StretchDIBits(*(HDC*)&hDC,
				nDstX, nDstY, nDstCX, nDstCY, nSrcX, nSrcY, nSrcCX, nSrcCY, bmpdata.getBuffer(), (BITMAPINFO*)&bmpInfo, DIB_RGB_COLORS, SRCCOPY);
			SetStretchBltMode(*(HDC*)&hDC, nPrevMode);
		}
		else if (cn == 4)
		{
			// 			int x=nSrcX;
			// 			int y=nSrcY;
			int cx = nDstCX;
			int cy = nDstCY;
			int desth = nDstCY;
			// 			int destw = nDstCX;

			RECT clipbox, paintbox;
			GetClipBox((HDC)hDC, &clipbox);
			paintbox.top = MIN(clipbox.bottom, MAX(clipbox.top, nDstY));
			paintbox.left = MIN(clipbox.right, MAX(clipbox.left, nDstX));
			paintbox.right = MAX(clipbox.left, MIN(clipbox.right, nDstX + nDstCX));
			paintbox.bottom = MAX(clipbox.top, MIN(clipbox.bottom, nDstY + nDstCY));


			BITMAPINFO bmInfo;
			memset(&bmInfo.bmiHeader, 0, sizeof(BITMAPINFOHEADER));
			bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmInfo.bmiHeader.biWidth = nDstCX;
			bmInfo.bmiHeader.biHeight = nDstCY;
			bmInfo.bmiHeader.biPlanes = 1;
			bmInfo.bmiHeader.biBitCount = 24;

			uchar *pbase;	//points to the final dib
			uchar *pdst;		//current pixel from pbase

			//get the background
			HDC TmpDC = CreateCompatibleDC((HDC)hDC);
			HBITMAP TmpBmp = CreateDIBSection((HDC)hDC, &bmInfo, DIB_RGB_COLORS, (void**)&pbase, 0, 0);
			HGDIOBJ TmpObj = SelectObject(TmpDC, TmpBmp);
			BitBlt(TmpDC, 0, 0, nDstCX, nDstCY, (HDC)hDC, paintbox.left, paintbox.top, SRCCOPY);

			if (pbase)
			{
				int xx, yy;
				uchar a, a1, *psrc;
				int ew = ((((24 * nDstCX) + 31) / 32) * 4);
				// 				int ymin = paintbox.top;
				// 				int xmin = paintbox.left;
				bool bFlipY = true;

				//				if (nDstCX!=cols || nDstCY!=rows)
				{
					//STRETCH
					float fx = (float)nSrcCX / (float)cx;
					float fy = (float)nSrcCY / (float)cy;
					float dx, dy;
					int sx, sy;
					int x = nDstX;
					int y = nDstY;

					for (yy = 0; yy < desth; yy++)
					{
						dy = (yy - y + paintbox.top)*fy + nSrcY;
						sy = MAX(0L, (int)floor(dy));

						if (sy >= nSrcY + nSrcCY)
							continue;

						if (bFlipY) {
							pdst = pbase + (desth - 1 - yy)*ew;
						}
						else {
							pdst = pbase + yy*ew;
						}
						psrc = data.ptr[sy];

						for (xx = paintbox.left; xx < paintbox.right; xx++/*, psrc+=4*/)
						{
							dx = (xx - x)*fx + nSrcX;
							sx = MAX(0L, (int)floor(dx));
							if (sx >= nSrcX + nSrcCX)
							{
								pdst += 3;
								continue;
							}
							uchar* ppix = psrc + sx * 4;
							a = ppix[3];
							if (a == 0) {			// Transparent, retain dest 
								pdst += 3;
							}
							else if (a == 255) {	// opaque, ignore dest 
								*pdst++ = *(ppix + 2);
								*pdst++ = *(ppix + 1);
								*pdst++ = *(ppix + 0);
							}
							else {				// semi transparent 
								a1 = (uchar)~a;
								*pdst++ = (uchar)((*pdst * a1 + a * *(ppix + 2)) >> 8);
								*pdst++ = (uchar)((*pdst * a1 + a * *(ppix + 1)) >> 8);
								*pdst++ = (uchar)((*pdst * a1 + a * *(ppix + 0)) >> 8);
							}
						} //! end for xx
					} //! end for yy
				}
				//				else
				{
					// 					//NORMAL
					// 					iy=rows-ymax+y;
					// 					for(yy=0;yy<desth;yy++,iy++)
					// 					{
					// 						ix=xmin-x;
					// 						if (bFlipY){
					// 							pdst = pbase+(desth-1-yy)*ew;
					// 						} else {
					// 							pdst = pbase+yy*ew;
					// 						}
					// 						uchar* ppix=&data.ptr[iy][ix*4];
					// 						for(xx=0;xx<destw;xx++,ix++)
					// 						{
					// 							a = data.ptr[];
					// 
					// 							if (head.biClrUsed){
					// 								ci = GetPixelIndex(ix,iy);
					// 								c = GetPaletteColor((uint8_t)ci);
					// 								if (info.bAlphaPaletteEnabled){
					// 									a = (uint8_t)((a*(1+c.w))>>8);
					// 								}
					// 							} else {
					// 								c.z = *ppix++;
					// 								c.y= *ppix++;
					// 								c.x  = *ppix++;
					// 							}
					// 
					// 							//if (*pc!=*pct || !bTransparent){
					// //							if ((head.biClrUsed && ci!=cit) || (!head.biClrUsed && *pc!=*pct) || !bTransparent)
					// 							{
					// 								// DJT, assume many pixels are fully transparent or opaque and thus avoid multiplication
					// 								if (a == 0) {			// Transparent, retain dest 
					// 									pdst+=3; 
					// 								} else if (a == 255) {	// opaque, ignore dest 
					// 									*pdst++= c.z; 
					// 									*pdst++= c.y; 
					// 									*pdst++= c.x; 
					// 								} else {				// semi transparent 
					// 									a1=(uchar)~a;
					// 									*pdst++=(uchar)((*pdst * a1 + a * c.z)>>8); 
					// 									*pdst++=(uchar)((*pdst * a1 + a * c.y)>>8); 
					// 									*pdst++=(uchar)((*pdst * a1 + a * c.x)>>8); 
					// 								} 
					// 							} else {
					// 								pdst+=3;
					// 							}
					// 						}
					// 					}
				}
			}
			//paint the image & cleanup
			SetDIBitsToDevice((HDC)hDC, paintbox.left, paintbox.top, nDstCX, nDstCY, 0, 0, 0, nDstCY, pbase, &bmInfo, 0);
			DeleteObject(SelectObject(TmpDC, TmpObj));
			DeleteDC(TmpDC);
		}
		::RestoreDC((HDC)hDC, hdc_Restore);
	}

	void Mat::drawTo(Handle hDC) const
	{
		drawTo(hDC, 0, 0, m_cols, m_rows, 0, 0, m_cols, m_rows);
	}

	int	Mat::drawString(Handle hdc, long x, long y, const char* szText1, COLOR color, const char* szFont1,
		long lSize, long lWeight, uchar bItalic, uchar bUnderline, const float /*rOpacity*/ /*= 1*/)
	{
		if (!isValid())
			return 1;
#ifndef __QT__
		const TCHAR* szText = (const TCHAR*)szText1;
		const TCHAR* szFont = (const TCHAR*)szFont1;
		//get the background
		HDC TmpDC = CreateCompatibleDC(*(HDC*)&hdc);
		//choose the font
		HFONT m_Font;
		LOGFONT* m_pLF;
		m_pLF = (LOGFONT*)calloc(1, sizeof(LOGFONT));
		_tcsncpy(m_pLF->lfFaceName, szFont, 31);	// For UNICODE support
		//strncpy(m_pLF->lfFaceName,szFont,31);
		m_pLF->lfHeight = lSize;
		m_pLF->lfWeight = lWeight;
		m_pLF->lfItalic = bItalic;
		m_pLF->lfUnderline = bUnderline;
		m_Font = CreateFontIndirect(m_pLF);
		//select the font in the dc
		HFONT pOldFont = NULL;
		if (m_Font)
			pOldFont = (HFONT)SelectObject(TmpDC, m_Font);
		else
			pOldFont = (HFONT)SelectObject(TmpDC, GetStockObject(DEFAULT_GUI_FONT));

		//Set text color
		SetTextColor(TmpDC, RGB(255, 255, 255));
		::SetBkColor(TmpDC, RGB(0, 0, 0));
		//draw the text
		SetBkMode(TmpDC, OPAQUE);
		//Set text position;
		RECT pos = { 0,0,0,0 };
		//long len = (long)strlen(text);
		int len = (int)_tcslen(szText);	// For UNICODE support
		::DrawText(TmpDC, szText, len, &pos, DT_CALCRECT);
		pos.right += pos.bottom; //for italics

		//Preparing Bitmap Info
		long width = pos.right;
		long height = pos.bottom;
		BITMAPINFO bmInfo;
		memset(&bmInfo.bmiHeader, 0, sizeof(BITMAPINFOHEADER1));
		bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER1);
		bmInfo.bmiHeader.biWidth = width;
		bmInfo.bmiHeader.biHeight = height;
		bmInfo.bmiHeader.biPlanes = 1;
		bmInfo.bmiHeader.biBitCount = 24;
		uchar *pbase; //points to the final dib

		HBITMAP TmpBmp = CreateDIBSection(TmpDC, &bmInfo, DIB_RGB_COLORS, (void**)&pbase, 0, 0);
		HGDIOBJ TmpObj = SelectObject(TmpDC, TmpBmp);
		memset(pbase, 0, height*((((24 * width) + 31) / 32) * 4));

		::DrawText(TmpDC, szText, len, &pos, 0);

		Mat itext;
		itext.createFromHBITMAP(*(Handle*)&TmpBmp);

		for (long ix = 0; ix < width; ix++)
		{
			for (long iy = 0; iy < height; iy++)
			{
				if (itext.value(iy, ix, 0))
					drawPixel(x + ix, y + iy, color);
			}
		}

		//cleanup
		if (pOldFont) SelectObject(TmpDC, pOldFont);
		DeleteObject(m_Font);
		free(m_pLF);
		DeleteObject(SelectObject(TmpDC, TmpObj));
		DeleteDC(TmpDC);
#endif
		return 1;
	}
	int	Mat::drawString(Handle hdc, long x, long y, const wchar_t* szText1, COLOR color, const wchar_t* szFont1,
		long lSize, long lWeight, uchar bItalic, uchar bUnderline, const float /*rOpacity*/ /*= 1*/)
	{
		if (!isValid())
			return 1;
#ifndef __QT__
		const wchar_t* szText = (const wchar_t*)szText1;
		const wchar_t* szFont = (const wchar_t*)szFont1;
		//get the background
		HDC TmpDC = CreateCompatibleDC(*(HDC*)&hdc);
		//choose the font
		HFONT m_Font;
		LOGFONTW* m_pLF;
		m_pLF = (LOGFONTW*)calloc(1, sizeof(LOGFONTW));
		wcsncpy(m_pLF->lfFaceName, szFont, 31);	// For UNICODE support
		//strncpy(m_pLF->lfFaceName,szFont,31);
		m_pLF->lfHeight = lSize;
		m_pLF->lfWeight = lWeight;
		m_pLF->lfItalic = bItalic;
		m_pLF->lfUnderline = bUnderline;
		m_Font = CreateFontIndirectW(m_pLF);
		//select the font in the dc
		HFONT pOldFont = NULL;
		if (m_Font)
			pOldFont = (HFONT)SelectObject(TmpDC, m_Font);
		else
			pOldFont = (HFONT)SelectObject(TmpDC, GetStockObject(DEFAULT_GUI_FONT));

		//Set text color
		SetTextColor(TmpDC, RGB(255, 255, 255));
		::SetBkColor(TmpDC, RGB(0, 0, 0));
		//draw the text
		SetBkMode(TmpDC, OPAQUE);
		//Set text position;
		RECT pos = { 0,0,0,0 };
		//long len = (long)strlen(text);
		int len = (int)wcslen(szText);	// For UNICODE support
		::DrawTextW(TmpDC, szText, len, &pos, DT_CALCRECT);
		pos.right += pos.bottom; //for italics

		//Preparing Bitmap Info
		long width = pos.right;
		long height = pos.bottom;
		BITMAPINFO bmInfo;
		memset(&bmInfo.bmiHeader, 0, sizeof(BITMAPINFOHEADER1));
		bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER1);
		bmInfo.bmiHeader.biWidth = width;
		bmInfo.bmiHeader.biHeight = height;
		bmInfo.bmiHeader.biPlanes = 1;
		bmInfo.bmiHeader.biBitCount = 24;
		uchar *pbase; //points to the final dib

		HBITMAP TmpBmp = CreateDIBSection(TmpDC, &bmInfo, DIB_RGB_COLORS, (void**)&pbase, 0, 0);
		HGDIOBJ TmpObj = SelectObject(TmpDC, TmpBmp);
		memset(pbase, 0, height*((((24 * width) + 31) / 32) * 4));

		::DrawTextW(TmpDC, szText, len, &pos, 0);

		Mat itext;
		itext.createFromHBITMAP(*(Handle*)&TmpBmp);

		for (long ix = 0; ix < width; ix++)
		{
			for (long iy = 0; iy < height; iy++)
			{
				if (itext.value(iy, ix, 0))
					drawPixel(x + ix, y + iy, color);
			}
		}

		//cleanup
		if (pOldFont) SelectObject(TmpDC, pOldFont);
		DeleteObject(m_Font);
		free(m_pLF);
		DeleteObject(SelectObject(TmpDC, TmpObj));
		DeleteDC(TmpDC);
#endif
		return 1;
	}
	int	Mat::createFromHBITMAP(Handle hBitmap, Handle hPalette/* = 0*/)
	{
		release();

		if (hBitmap == 0)
			return false;
		BITMAP bm;
		// get informations about the bitmap
		GetObject(*(HBITMAP*)&hBitmap, sizeof(BITMAP), (LPSTR)&bm);
		// create a device context for the bitmap
		HDC dc = ::GetDC(NULL);
		if (!dc)
			return false;

		if (hPalette) {
			SelectObject(dc, *(HPALETTE*)&hPalette); //the palette you should get from the user or have a stock one
			RealizePalette(dc);
		}
		uchar* pBuf = new uchar[bm.bmWidthBytes*bm.bmHeight + sizeof(BITMAPINFOHEADER1)];
		if (pBuf == 0)
			return false;
		BITMAPINFOHEADER1* pBMPHeader = (BITMAPINFOHEADER1*)pBuf;
		memset(pBMPHeader, 0, sizeof(BITMAPINFOHEADER1));
		pBMPHeader->biBitCount = bm.bmBitsPixel;
		pBMPHeader->biHeight = bm.bmHeight;
		pBMPHeader->biWidth = bm.bmWidth;
		pBMPHeader->biSizeImage = bm.bmHeight*bm.bmWidthBytes;
		pBMPHeader->biSize = sizeof(BITMAPINFOHEADER1);
		pBMPHeader->biPlanes = 1;
		pBMPHeader->biXPelsPerMeter = (long)floor(96 * 10000.0 / 254.0 + 0.5);
		pBMPHeader->biYPelsPerMeter = (long)floor(96 * 10000.0 / 254.0 + 0.5);
		// copy the pixels
		if (GetDIBits(dc, (HBITMAP)hBitmap, 0, bm.bmHeight, pBuf + sizeof(BITMAPINFOHEADER1), (LPBITMAPINFO)pBMPHeader, DIB_RGB_COLORS) == 0)
		{
			delete[]pBuf;
			::ReleaseDC(NULL, dc);
			return false;
		}
		::ReleaseDC(NULL, dc);

		create(pBMPHeader);
		delete[]pBuf;
		return true;
	}
	Handle Mat::makeBitmap(Handle hdc, bool bTransparency) const
	{
		if (!isValid())
			return 0;

		int cn = channels();
		// create HBITMAP with Trancparency
		if ((cn == 4) && bTransparency)
		{
			HDC hMemDC;
			if (hdc)
				hMemDC = (HDC)hdc;
			else
				hMemDC = CreateCompatibleDC(NULL);

			BITMAPINFO bi;

			// Fill in the BITMAPINFOHEADER
			bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bi.bmiHeader.biWidth = m_cols;
			bi.bmiHeader.biHeight = m_rows;
			bi.bmiHeader.biPlanes = 1;
			bi.bmiHeader.biBitCount = 32;
			bi.bmiHeader.biCompression = BI_RGB;
			bi.bmiHeader.biSizeImage = 4 * m_rows* m_cols;
			bi.bmiHeader.biXPelsPerMeter = 0;
			bi.bmiHeader.biYPelsPerMeter = 0;
			bi.bmiHeader.biClrUsed = 0;
			bi.bmiHeader.biClrImportant = 0;

			COLOR* pCrBits = NULL;
			HBITMAP hbmp = CreateDIBSection(
				hMemDC, &bi, DIB_RGB_COLORS, (void **)&pCrBits,
				NULL, NULL);

			if (!hdc)
				DeleteDC(hMemDC);

			DIBSECTION ds;
			if (::GetObject(hbmp, sizeof(DIBSECTION), &ds) == 0)
			{
				return 0;
			}

			// transfer Pixels from CxImage to Bitmap
			RGBQUAD* pBit = (RGBQUAD*)ds.dsBm.bmBits;
			int lPx, lPy;
			for (lPy = 0; lPy < bi.bmiHeader.biHeight; ++lPy)
			{
				uchar* ptr = data.ptr[bi.bmiHeader.biHeight - lPy - 1];
				for (lPx = 0; lPx < bi.bmiHeader.biWidth; ++lPx, ptr += cn)
				{
					RGBQUAD lPixel;
					lPixel.rgbRed = ptr[0];
					lPixel.rgbGreen = ptr[1];
					lPixel.rgbBlue = ptr[2];
					lPixel.rgbReserved = ptr[3];
					*pBit = lPixel;
					pBit++;
				}
			}
			return (Handle)hbmp;
		}

		// create HBITMAP without Trancparency
		if (!hdc) {
			// this call to CreateBitmap doesn't create a DIB <jaslet>
			// // create a device-independent bitmap <CSC>
			//  return CreateBitmap(head.biWidth,head.biHeight,	1, head.biBitCount, GetBits());
			// use instead this code
			HDC hMemDC = CreateCompatibleDC(NULL);

			BITMAPINFO bi;
			int bisizeImage = (((24 * m_cols) + 31) / 32) * 4;
			// Fill in the BITMAPINFOHEADER
			bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bi.bmiHeader.biWidth = m_cols;
			bi.bmiHeader.biHeight = m_rows;
			bi.bmiHeader.biPlanes = 1;
			bi.bmiHeader.biBitCount = 24;
			bi.bmiHeader.biCompression = BI_RGB;
			bi.bmiHeader.biSizeImage = bisizeImage * m_rows;
			bi.bmiHeader.biXPelsPerMeter = 0;
			bi.bmiHeader.biYPelsPerMeter = 0;
			bi.bmiHeader.biClrUsed = 0;
			bi.bmiHeader.biClrImportant = 0;

			COLOR* pCrBits = NULL;
			HBITMAP hbmp = CreateDIBSection(
				hMemDC, &bi, DIB_RGB_COLORS, (void **)&pCrBits,
				NULL, NULL);

			DeleteDC(hMemDC);

			DIBSECTION ds;
			if (::GetObject(hbmp, sizeof(DIBSECTION), &ds) == 0)
			{
				return 0;
			}

			uchar* pdst = (uchar*)ds.dsBm.bmBits;
			int lPx, lPy;
			if (cn == 3)
			{
				for (lPy = 0; lPy < bi.bmiHeader.biHeight; ++lPy)
				{
					uchar* ptr = data.ptr[bi.bmiHeader.biHeight - lPy - 1];
					uchar* pdst_2 = pdst;
					for (lPx = 0; lPx < bi.bmiHeader.biWidth; ++lPx, ptr += cn, pdst_2 += cn)
					{
						pdst_2[2] = ptr[0];
						pdst_2[1] = ptr[1];
						pdst_2[0] = ptr[2];
					}
					pdst += bisizeImage;
				}
			}
			else if (cn == 1)
			{
				for (lPy = 0; lPy < bi.bmiHeader.biHeight; ++lPy)
				{
					uchar* ptr = data.ptr[bi.bmiHeader.biHeight - lPy - 1];
					uchar* pdst_2 = pdst;
					for (lPx = 0; lPx < bi.bmiHeader.biWidth; ++lPx, ptr += cn, pdst_2 += 3)
					{
						pdst_2[2] =
							pdst_2[1] =
							pdst_2[0] = ptr[0];
					}
					pdst += bisizeImage;
				}
			}
			return (Handle)hbmp;
		}

		// this single line seems to work very well
		//HBITMAP bmp = CreateDIBitmap(hdc, (LPBITMAPINFOHEADER)pDib, CBM_INIT,
		//	GetBits(), (LPBITMAPINFO)pDib, DIB_RGB_COLORS);
		// this alternative works also with _WIN32_WCE
		BitmapData bmpData; getBMPInfoHeader(bmpData);
		LPVOID pBit32;
		HBITMAP bmp = CreateDIBSection((HDC)hdc, (LPBITMAPINFO)bmpData.getBitmapInfoHeader(), DIB_RGB_COLORS, &pBit32, NULL, 0);
		int sizeImage = ((((24 * m_cols) + 31) / 32) * 4)*m_rows;
		if (pBit32) memcpy(pBit32, bmpData.getBuffer(), sizeImage);
		return (Handle)bmp;
	}

	bool Mat::loadResource(Handle hRes, ulong imagetype, Handle hModule/*=NULL*/)
	{
#ifndef __QT__
		ulong rsize = SizeofResource((HMODULE)hModule, (HRSRC)hRes);
		HGLOBAL hMem = ::LoadResource((HMODULE)hModule, (HRSRC)hRes);
		if (hMem)
		{
			char* lpVoid = (char*)LockResource(hMem);
			if (lpVoid)
			{
				CoImage tempImage;
				bool bOK = tempImage.Decode(*this, (uchar*)lpVoid, rsize, imagetype);
				return bOK;
			}
		}
#endif
		return false;
	}

	bool Mat::tile(Handle hdc, const Rect& rc)
	{
		if (isValid() && (hdc))
		{
			int w = rc.width;
			int h = rc.height;
			int x, y, z;
			int bx = cols();
			int by = rows();
			for (y = 0; y < h; y += by)
			{
				if ((y + by) > h)
				{
					by = h - y;
				}
				z = bx;
				for (x = 0; x < w; x += z)
				{
					if ((x + z) > w)
						z = w - x;
					if (by == rows())
						drawTo(hdc, rc.x + x, rc.y + y, z, by, 0, 0, z, by);
					else
						drawTo(hdc, rc.x + x, rc.y + y, z, by, 0, rows() - by, z, by);
				}
			}
			return 1;
		}
		return 0;
	}

	Handle Mat::copyToHandle() const
	{
		HANDLE hMem = NULL;
		if (isValid())
		{
			int bisize = sizeof(BITMAPINFOHEADER1);
			int dwEffWidth = ((((24 * m_cols) + 31) / 32) * 4);
			int biSizeImage = dwEffWidth * m_rows;
			hMem = GlobalAlloc(GHND, bisize + biSizeImage);
			if (hMem)
			{
				uchar* pDst = (uchar*)GlobalLock(hMem);
				if (pDst)
				{
					BitmapData bmpData; getBMPInfoHeader(bmpData);
					memcpy(pDst, bmpData.getBitmapInfoHeader(), bisize + biSizeImage);
				}
				GlobalUnlock(hMem);
			}
		}
		return (Handle)hMem;
	}

	/**
	* Global object (clipboard paste) constructor
	* \param hMem: source bitmap object, the clipboard format must be CF_DIB
	* \return true if everything is ok
	*/
	bool Mat::createFromHANDLE(Handle hMem)
	{
		release();

		size_t dwSize = GlobalSize((HANDLE)hMem);
		if (!dwSize) return false;

		uchar *lpVoid = (uchar *)GlobalLock((HANDLE)hMem);
		if (lpVoid)
		{
			BITMAPINFOHEADER1 *pHead = (BITMAPINFOHEADER1 *)lpVoid;
			bool fRes;
			if (create(pHead))
				fRes = true;
			else
				fRes = false;
			GlobalUnlock(lpVoid);
			return fRes;
		}
		return false;
	}

#endif

	void Mat::fillConvexPoly(const Point2i* pts, int npts, COLOR color, float rOpacity)
	{
		if (!pts || npts <= 0)
			return;

		Scalar scalar;
		scalar.x = (double)color.x;
		scalar.y = (double)color.y;
		scalar.z = (double)color.z;
		scalar.w = 255.0;
		ifillConvexPoly(*this, pts, npts, scalar, rOpacity);
	}

	void Mat::setPixelColor(int x, int y, float* color)
	{
		int cn = channels();
		for (int i = 0; i < cn; i++)
			data.ptr[y][x*cn + i] = (uchar)color[i];
	}
	COLOR Mat::getPixelColor(int x, int y) const
	{
		int cn = channels();
		if (cn == 4)
		{
			return COLOR(&data.ptr[y][x*cn]);
		}
		else if (cn == 3)
		{
			const uchar* p = &data.ptr[y][x*cn];
			return COLOR(p[0], p[1], p[2], 255);
		}
		else
		{
			uchar p = data.ptr[y][x*cn];
			return COLOR(p, p, p, 255);
		}
	}

	void Mat::setPixelColor(int x, int y, const COLOR& c, bool bSetAlpha)
	{
		if (!isValid())
			return;
		if ((x < 0) || (y < 0) || (x >= m_cols) || (y >= m_rows))
			return;
		const uchar* v = c.vec_array;
		int cn = channels();
		if (cn > 3 && !bSetAlpha)
		{
			for (int i = 0; i < 3; i++)
				data.ptr[y][x*cn + i] = v[i];
		}
		else
		{
			for (int i = 0; i < cn; i++)
				data.ptr[y][x*cn + i] = v[i];
		}
	}

}
