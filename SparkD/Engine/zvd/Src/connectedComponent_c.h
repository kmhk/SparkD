#pragma once

#include "cvlibbase/Inc/_cvlibbase.h"

namespace cvlib {
	namespace ip {

		//////////////////////////////////////////////////////////////////////
		struct RECT1
		{
			RECT1() {}
			RECT1(const RECT1& t) { x1 = t.x1; y1 = t.y1; x2 = t.x2; y2 = t.y2; }
			RECT1(int _x1, int _y1, int _x2, int _y2) { x1 = _x1; y1 = _y1; x2 = _x2; y2 = _y2; }
			inline void inflate(int nDelta) { x1 -= nDelta; y1 -= nDelta; x2 += nDelta; y2 += nDelta; }
			inline void inflateHeight(int nDelta) { y1 -= nDelta; y2 += nDelta; }
			inline void inflateWidth(int nDelta) { x1 -= nDelta; x2 += nDelta; }
			bool intersect(const RECT1& r)
			{
				x1 = MAX(x1, r.x1);
				x2 = MIN(x2, r.x2);
				y1 = MAX(y1, r.y1);
				y2 = MIN(y2, r.y2);
				if (x2 < x1 || y2 < y1) return false;
				return true;
			}
			bool intersect(const RECT1& psRect1, const RECT1& psRect2)
			{
				x1 = MAX(psRect1.x1, psRect2.x1);
				x2 = MIN(psRect1.x2, psRect2.x2);
				y1 = MAX(psRect1.y1, psRect2.y1);
				y2 = MIN(psRect1.y2, psRect2.y2);
				if (x2 < x1 || y2 < y1) return false;
				return true;
			}
			void unionRect(const RECT1& psRect1, const RECT1& psRect2)
			{
				x1 = MIN(psRect1.x1, psRect2.x1);
				x2 = MAX(psRect1.x2, psRect2.x2);
				y1 = MIN(psRect1.y1, psRect2.y1);
				y2 = MAX(psRect1.y2, psRect2.y2);
			}
			inline int width() const { return x2 - x1 + 1; }
			inline int height() const { return y2 - y1 + 1; }
			bool operator==(const RECT1& t) const {
				if (x1 == t.x1 && y1 == t.y1 && x2 == t.x2 && y2 == t.y2) return true;
				return false;
			}
			inline int area() const { return width()*height(); }

			int x1;
			int y1;
			int x2;
			int y2;
		};

		typedef struct _tagConnectInfo
		{
			RECT1 sRect;
			uchar bVal;			// pixel value
			int cnPixel;		//number of pixels in the connect component
			int nLastPos;		//position (x*w+h) of last pixel in the search order in the connect component
		}SConnectInfo, *PSConnectInfo;

		void		extractConnectComponent(Mat* pxImg, PtrArray& asConnetInfo, uchar bVal = 0);
		RECT1	extractOneConnectComponent(Mat* pxImg, int nX, int nY, uchar bVal = 0);
		int		eraseRect(Mat* pxImg, RECT1& sRect, uchar bVal);

		/**
		* extract the connect component by tree search
		* @param pxImg		[in]	input image(byte array)
		* @param pxIndex	[out]	in every position of pixel in the connect component, contains position(x*w+h) of prev pixel
		in the search order in the same component (int array).
		in the position of root pixel, value is ff.
		From nLastPos member of asConnectInfo and this param, can find the whole pixels in the component
		* @param asConnectInfo	[out]	information of components
		* @param bEraseVal	[in]	extract the component about pixels which don't have this value
		* @param nLeft		[in]	left of the area in which component is extracted
		* @param nTop		[in]	top of the area in which component is extracted
		* @param nRight	[in]	right of the area in which component is extracted
		* @param nBottom	[in]	bottom of the area in which component is extracted
		* @param nMinPixelNum	[in]	min pixel number of component
		* @param nMinW		[in]	min width of component
		* @param nMinW		[in]	min height of component
		* @param fErase	[in]	if true, erase pixels in component which doesn't satisfy nMinPixelNum, nMinW and nMinH condition
		* @param pxCopyImg	[out]	as copy image of pxImg, be used for working buffer
		* @param pnQueue	[out]	queue buffer to be used for search
		*/
		int	extractConnectComponent(Mat* pxImg, PSConnectInfo psConnInfo, int nMaxConnNum, Mat* pxIndex, const Rect& region, uchar bEraseVal,
			int nMinW = 1, int nMinH = 1, int nMinPixelNum = 1, bool fErase = false, bool fSort = false, Mat* pxCopyImg = NULL, int* pnQueue = NULL);
		void	extractConnectComponent(Mat* pxImg, Vector<PSConnectInfo>& asConnectInfo, Mat* pxIndex, const Rect& region, uchar bEraseVal,
			int nMinW = 1, int nMinH = 1, int nMinPixelNum = 1, bool fErase = false, bool fSort = false, Mat* pxCopyImg = NULL, int* pnQueue = NULL);

		int	intersectRect(PSConnectInfo psInterRect, PSConnectInfo psRect1, PSConnectInfo psRect2);
		void	unionRect(PSConnectInfo psInterRect, PSConnectInfo psRect1, PSConnectInfo psRect2);
		void	createImageFromConnect(PSConnectInfo psConn, Mat* pxIndexImg, Mat* pxImg, void* pvBuffer = NULL);
		void	createImageFromConnect(const PSConnectInfo psConn, const Mat& indexImage, Mat& img);
		void	createPointArrayFromConnect(const PSConnectInfo psConn, const Mat& indexImage, Vector<Point2i>& points);
		void	extractBoundaryPointsFromConnect(const Mat* pxImg, int nX, int nY, uchar bVal, int max_pixels, Vector<vec2i>& points);
		void	eraseConnect(Mat* pxImg, PSConnectInfo psConn, Mat* pxIndexImg, uchar bEraseVal = 255);
		void	releaseConnectComponent(Vector<PSConnectInfo>& asConnectInfo);

	}
}