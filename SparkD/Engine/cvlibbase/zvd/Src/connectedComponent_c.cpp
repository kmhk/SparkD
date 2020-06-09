#include "connectedComponent_c.h"

namespace cvlib { namespace ip{

#pragma warning (push)
#pragma warning (disable : 4127)

static Point2i	NextPixel(int nX,int nY,int nDir);
static int		ComparePtr(const void* pEl1, const void* pEl2);
static int		compare(const void* pEl1, const void* pEl2);

//////////////////////////////////////////////////////////////////////
void	extractConnectComponent(Mat* pxImg, PtrArray& asConnetInfo, uchar bVal)
{
	if (!pxImg->isValid())
		return;

	RECT1 sRect;
	int cnRealPixel;
	PSConnectInfo psConnInfo;
	
	int nW = pxImg->cols();
	int nH = pxImg->rows();
	
	Mat xCopyImg = *pxImg;
	uchar* pbImg = xCopyImg.data.ptr[0];
	for(int iH = 0; iH < nH; iH++)
	{
		uchar* pbLine = pbImg + iH * nW;
		for(int iW = 0; iW < nW; iW++)
		{
			if (*pbLine++ == bVal)
			{
				sRect = extractOneConnectComponent(&xCopyImg, iW, iH, bVal);
				cnRealPixel = eraseRect(&xCopyImg, sRect, bVal);

				psConnInfo = new SConnectInfo;
				psConnInfo->sRect.x1 = sRect.x1;
				psConnInfo->sRect.y1 = sRect.y1;
				psConnInfo->sRect.x2 = sRect.x2;
				psConnInfo->sRect.y2 = sRect.y2;
				psConnInfo->cnPixel = cnRealPixel;
				asConnetInfo.add(psConnInfo);
			}
		}
	}
	qsort(asConnetInfo.getData(), asConnetInfo.getSize(), sizeof(void*), compare);
	
}

static int ComparePtr(const void* pEl1, const void* pEl2)
{
	PSConnectInfo psEl1 = *(PSConnectInfo*)pEl1;
	PSConnectInfo psEl2 = *(PSConnectInfo*)pEl2;
	int cnPixel1 = psEl1->cnPixel;
	int cnPixel2 = psEl2->cnPixel;
	
	if (cnPixel1 < cnPixel2)
		return 1;
	else if (cnPixel1 > cnPixel2)
		return -1;
	else
		return 0;
}
static int compare(const void* pEl1, const void* pEl2)
{
	PSConnectInfo psEl1 = (PSConnectInfo)pEl1;
	PSConnectInfo psEl2 = (PSConnectInfo)pEl2;
	int cnPixel1 = psEl1->cnPixel;
	int cnPixel2 = psEl2->cnPixel;
	
	if (cnPixel1 < cnPixel2)
		return 1;
	else if (cnPixel1 > cnPixel2)
		return -1;
	else
		return 0;
}

RECT1 extractOneConnectComponent(Mat* pxImg, int nX, int nY, uchar bVal)
{
	RECT1 sRect;
	Point2i sPt;
	int nDir = 0;
	int nMaxX, nMinX, nMaxY, nMinY, nTmpX, nTmpY;
	
	nTmpX = nMaxX = nMinX = nX; 
	nTmpY = nMaxY = nMinY = nY; 

	uchar** ppbImg = pxImg->data.ptr;
	int nW = pxImg->cols();
	int nH = pxImg->rows();

	do 
	{
		for (int iDir = nDir; iDir < nDir + 8; iDir++) 
		{
			sPt  = NextPixel(nTmpX, nTmpY, iDir);

			if ((sPt.x >= 0 && sPt.x < nW) && (sPt.y >= 0 && sPt.y < nH))
			{
				if (ppbImg[sPt.y][sPt.x] == bVal)
				{
					if (sPt.x > nMaxX) 
						nMaxX = (int)sPt.x; 
					if (sPt.x < nMinX) 
						nMinX = (int)sPt.x;
					if (sPt.y > nMaxY)
						nMaxY = (int)sPt.y;
					if (sPt.y < nMinY)
						nMaxY = (int)sPt.y;

					nDir = iDir + 5; 
					nTmpX = sPt.x; 
					nTmpY = sPt.y;
					break;
				}
			}
		}
	} while (nTmpX != nX || nTmpY != nY);

	sRect.x1 = nMinX; 
	sRect.y1 = nMinY; 
	sRect.x2 = nMaxX;
	sRect.y2 = nMaxY;
	return sRect;
}
static Point2i NextPixel(int nX,int nY,int nDir)
{
	Point2i sP;
	
	switch ( nDir % 8 ) 
	{
	case 0: 
		sP.x = nX + 1; sP.y = nY; 
		break;
	case 1: 
		sP.x = nX + 1; sP.y = nY + 1;
		break;
	case 2: 
		sP.x = nX; sP.y = nY + 1; 
		break;
	case 3: 
		sP.x = nX - 1; sP.y = nY + 1; 
		break;
	case 4:
		sP.x = nX - 1; sP.y = nY; 
		break;
	case 5: 
		sP.x = nX - 1; sP.y = nY - 1; 
		break;
	case 6: 
		sP.x = nX; sP.y = nY - 1;
		break;
	case 7: 
		sP.x = nX + 1; sP.y = nY - 1;
		break;
	}
	
	return sP;
}
int eraseRect(Mat* pxImg, RECT1& sRect, uchar bVal)
{
	int nRealPixelNum = 0;
	int nW = pxImg->cols();
	uchar* pbImg = pxImg->data.ptr[0];

	for (int iH = sRect.y1; iH <= sRect.y2; iH++)
	{
		uchar* pbLine = pbImg + iH * nW ;
		for (int iW = sRect.x1; iW <= sRect.x2; iW++)
		{
			if (pbLine[iW] == bVal)
			{
				nRealPixelNum++;
				pbLine[iW] = 255 - bVal;
			}
		}
	}
	return nRealPixelNum;
}
void extractConnectComponent(Mat* pxImg, Vector<PSConnectInfo>& asConnectInfo, Mat* pxIndex, const Rect& region, uchar bEraseVal, 
						int nMinW, int nMinH, int nMinPixelNum, bool fErase, bool fSort, Mat* pxCopyImg, int* pnQueue)
{
	if (!pxImg->isValid())
		return;

	int nLeft=region.x, nTop=region.y, nRight=region.limx()-1, nBottom=region.limy()-1;
	int anOffX[8] = {-1, 0, 1, -1, 1, 0, 1, -1};
	int anOffY[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
	int nNX, nNY;
	uchar bVal;
	Mat xWorkImg, xIndex;
	Mat* pxWorkImg;
	int iX, iY;
	int* pnQueue1 = NULL;
	int nConnW, nConnH;
	int nConnLeft, nConnTop, nConnRight, nConnBottom;
	int nQueueNum;
	int nPixelNum;
	int iRemarkX, iRemarkY;
	
	int nW = pxImg->cols();
	int nH = pxImg->rows();
	uchar** ppbSrcImg = pxImg->data.ptr;

	if (pxIndex == NULL)
	{
		xIndex.create(nH, nW, MAT_Tint);
		pxIndex = &xIndex;
	}
	else
	{
		if (!pxIndex->isValid())
			pxIndex->create(nH, nW, MAT_Tint);
	}

	if (pxCopyImg == NULL)
	{
		xWorkImg.create(nH, nW, MAT_Tbyte);
		MatOp::copyMat(xWorkImg, *pxImg, Rect(nLeft, nTop, nRight-nLeft+1, nBottom-nTop+1), Point2i(nLeft, nTop));
		pxWorkImg = &xWorkImg;
	}
	else
	{
		*pxCopyImg = *pxImg;
		pxWorkImg = pxCopyImg;
	}

	if (pnQueue == NULL)
	{
		pnQueue = pnQueue1 = (int*)malloc(nW * nH * sizeof(int));
	}
	
	uchar** ppbImg = pxWorkImg->data.ptr;
	int* pnIndex = pxIndex->data.i[0];
	
	for (iX = nLeft; iX <= nRight; iX++)
	{
		for (iY = nTop; iY <= nBottom; iY++)
		{
			
			bVal = ppbImg[iY][iX];
			if (bVal == bEraseVal)
				continue;
			ppbImg[iY][iX] = bEraseVal;
			nConnLeft = nConnRight = iX;
			nConnTop = nConnBottom = iY;
			nQueueNum = 0;
			nPixelNum = 1;
			iRemarkX = iX;
			iRemarkY = iY;
			pnIndex[iRemarkY * nW + iRemarkX] = -1;
			do 
			{
				for (int i = 0; i < 8; i++)
				{
					nNX = iRemarkX + anOffX[i];
					nNY = iRemarkY + anOffY[i];
					if (nNX < nLeft || nNX > nRight)
						continue;
					if (nNY < nTop || nNY > nBottom)
						continue;
					
					if (ppbImg[nNY][nNX] == bEraseVal)
						continue;

					if (nConnLeft > nNX)
						nConnLeft = nNX;
					if (nConnRight < nNX)
						nConnRight = nNX;
					if (nConnTop > nNY)
						nConnTop = nNY;
					if (nConnBottom < nNY)
						nConnBottom = nNY;
					
					ppbImg[nNY][nNX] = bEraseVal;
					pnQueue[nQueueNum] = nNY * nW + nNX;
					nQueueNum++;
					nPixelNum++;
				}

				nQueueNum--;
				if (nQueueNum < 0)
					break;
				pnIndex[pnQueue[nQueueNum]] = iRemarkY * nW + iRemarkX;
				iRemarkX = pnQueue[nQueueNum] % nW;
				iRemarkY = pnQueue[nQueueNum] / nW;

			} while (true);
			nConnW = nConnRight - nConnLeft + 1;
			nConnH = nConnBottom - nConnTop + 1;	
			 
			int nPos = iRemarkY * nW + iRemarkX;
			if (nPixelNum >= nMinPixelNum && nConnW >= nMinW && nConnH >= nMinH)
			{
				PSConnectInfo psConnInfo = new SConnectInfo;
				psConnInfo->sRect.x1 = nConnLeft;
				psConnInfo->sRect.y1 = nConnTop;
				psConnInfo->sRect.x2 = nConnRight;
				psConnInfo->sRect.y2 = nConnBottom;
				psConnInfo->bVal = bVal;
				psConnInfo->cnPixel = nPixelNum;
				psConnInfo->nLastPos = nPos;
				asConnectInfo.add(psConnInfo);
			}
			else
			{
				if (fErase)
				{
					do 
					{
						ppbSrcImg[nPos/nW][nPos%nW] = bEraseVal;
						nPos = pnIndex[nPos];
					} while (nPos != -1);
				}
			}
		}
	}
	if (fSort)
		qsort(asConnectInfo.getData(), asConnectInfo.getSize(), sizeof(void*), ComparePtr);
	if (pnQueue1)
		free(pnQueue1);
}
void	releaseConnectComponent(Vector<PSConnectInfo>& asConnectInfo)
{
	for (int i=0; i<asConnectInfo.getSize(); i++)
		delete asConnectInfo[i];
	asConnectInfo.removeAll();
}

int extractConnectComponent(Mat* pxImg, PSConnectInfo psConnInfo, int nMaxConnNum, Mat* pxIndex, 
	const Rect& region, uchar bEraseVal, int nMinW, int nMinH, int nMinPixelNum, bool fErase, bool fSort,Mat* pxCopyImg, int* pnQueue)
{
	int nLeft=region.x, nTop=region.y, nRight=region.limx()-1, nBottom=region.limy()-1;
	int anOffX[8] = {-1, 0, 1, -1, 1, 0, 1, -1};
	int anOffY[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
	int nNX, nNY;
	uchar bVal;
	Mat xWorkImg, xIndex;
	Mat* pxWorkImg;
	int iX, iY;
	int* pnQueue1 = NULL;
	int nConnW, nConnH;
	int nConnLeft, nConnTop, nConnRight, nConnBottom;
	int nQueueNum;
	int nPixelNum;
	int iRemarkX, iRemarkY;
	int nConnNum = 0;

	assert(psConnInfo != NULL);
	
	int nW = pxImg->cols();
	int nH = pxImg->rows();
	uchar** ppbSrcImg = pxImg->data.ptr;

	if (pxIndex == NULL)
	{
		xIndex.create(nH, nW, MAT_Tint);
		pxIndex = &xIndex;
	}
	else
	{
		if (!pxIndex->isValid())
			pxIndex->create(nH, nW, MAT_Tint);
	}

	if (pxCopyImg == NULL)
	{
		xWorkImg.create(nH, nW, MAT_Tbyte);
		MatOp::copyMat(xWorkImg, *pxImg, Rect(nLeft, nTop, nRight-nLeft, nBottom-nTop), Point2i(nLeft, nTop));
		pxWorkImg = &xWorkImg;
	}
	else
	{
		*pxCopyImg = *pxImg;
		pxWorkImg = pxCopyImg;
	}

	if (pnQueue == NULL)
	{
		pnQueue = pnQueue1 = (int*)malloc(nW * nH * sizeof(int));
	}
	
	uchar** ppbImg = pxWorkImg->data.ptr;
	int* pnIndex = pxIndex->data.i[0];
	
	for (iX = nLeft; iX <= nRight; iX++)
	{
		for (iY = nTop; iY <= nBottom; iY++)
		{
			
			bVal = ppbImg[iY][iX];
			if (bVal == bEraseVal)
				continue;

			ppbImg[iY][iX] = bEraseVal;
			nConnLeft = nConnRight = iX;
			nConnTop = nConnBottom = iY;
			nQueueNum = 0;
			nPixelNum = 1;
			iRemarkX = iX;
			iRemarkY = iY;
			pnIndex[iRemarkY * nW + iRemarkX] = -1;
			do 
			{
				for (int i = 0; i < 8; i++)
				{
					nNX = iRemarkX + anOffX[i];
					nNY = iRemarkY + anOffY[i];
					if (nNX < nLeft || nNX > nRight)
						continue;
					if (nNY < nTop || nNY > nBottom)
						continue;
					
					if (ppbImg[nNY][nNX] == bEraseVal)
						continue;

					if (nConnLeft > nNX)
						nConnLeft = nNX;
					if (nConnRight < nNX)
						nConnRight = nNX;
					if (nConnTop > nNY)
						nConnTop = nNY;
					if (nConnBottom < nNY)
						nConnBottom = nNY;
					
					ppbImg[nNY][nNX] = bEraseVal;
					pnQueue[nQueueNum] = nNY * nW + nNX;
					nQueueNum++;
					nPixelNum++;
				}

				nQueueNum--;
				if (nQueueNum < 0)
					break;
				pnIndex[pnQueue[nQueueNum]] = iRemarkY * nW + iRemarkX;
				iRemarkX = pnQueue[nQueueNum] % nW;
				iRemarkY = pnQueue[nQueueNum] / nW;

			} while (true);
			nConnW = nConnRight - nConnLeft + 1;
			nConnH = nConnBottom - nConnTop + 1;	
			 
			int nPos = iRemarkY * nW + iRemarkX;
			if (nPixelNum >= nMinPixelNum && nConnW >= nMinW && nConnH >= nMinH)
			{
				if (nConnNum < nMaxConnNum)
				{
					psConnInfo[nConnNum].sRect.x1 = nConnLeft;
					psConnInfo[nConnNum].sRect.y1 = nConnTop;
					psConnInfo[nConnNum].sRect.x2 = nConnRight;
					psConnInfo[nConnNum].sRect.y2 = nConnBottom;
					psConnInfo[nConnNum].bVal = bVal;
					psConnInfo[nConnNum].cnPixel = nPixelNum;
					psConnInfo[nConnNum].nLastPos = nPos;
					nConnNum++;
					
				}
			}
			else
			{
				if (fErase)
				{
					do 
					{
						ppbSrcImg[nPos/nW][nPos%nW] = bEraseVal;
						nPos = pnIndex[nPos];
					} while (nPos != -1);
				}
			}
		}
	}
	if (fSort)
		qsort(psConnInfo, nConnNum, sizeof(SConnectInfo), compare);
	if (pnQueue1)
		free(pnQueue1);
	return nConnNum;
}
static int intersectRect(RECT1* psInterRect, RECT1* psRect1, RECT1* psRect2)
{
	psInterRect->x1 = MAX(psRect1->x1, psRect2->x1);	
	psInterRect->x2 = MIN(psRect1->x2, psRect2->x2);	
	psInterRect->y1 = MAX(psRect1->y1, psRect2->y1);	
	psInterRect->y2 = MIN(psRect1->y2, psRect2->y2);	
	
	if (psInterRect->x2 < psInterRect->x1 || psInterRect->y2 < psInterRect->y1)
		return 0;
	return 1;
}
int intersectRect(PSConnectInfo psInterRect, PSConnectInfo psRect1, PSConnectInfo psRect2)
{
	return intersectRect(&psInterRect->sRect, &psRect1->sRect, &psRect2->sRect);
}
static void unionRect(RECT1* psUnionRect, RECT1* psRect1, RECT1* psRect2)
{
	psUnionRect->x1 = MIN(psRect1->x1, psRect2->x1);	
	psUnionRect->x2 = MAX(psRect1->x2, psRect2->x2);	
	psUnionRect->y1 = MIN(psRect1->y1, psRect2->y1);	
	psUnionRect->y2 = MAX(psRect1->y2, psRect2->y2);	
}
void unionRect(PSConnectInfo psInterRect, PSConnectInfo psRect1, PSConnectInfo psRect2)
{
	unionRect(&psInterRect->sRect, &psRect1->sRect, &psRect2->sRect);
	psInterRect->cnPixel = psRect1->cnPixel + psRect2->cnPixel;
}
void createImageFromConnect(PSConnectInfo psConn, Mat* pxIndexImg, Mat* pxImg, void* pvBuffer/* = NULL*/)
{
	assert (pxIndexImg->isValid());

	int nConnW, nConnH;
	nConnW = psConn->sRect.x2-psConn->sRect.x1+1;
	nConnH = psConn->sRect.y2-psConn->sRect.y1+1;

	if (pvBuffer == NULL)
		pxImg->create(nConnH, nConnW, MAT_Tbyte);
	else
		pxImg->create(pvBuffer, nConnH, nConnW, MAT_Tbyte, false);

	int nW = pxIndexImg->cols();

	int nX, nY, nPos;
	nPos = psConn->nLastPos;
	do 
	{
		nX = nPos % nW;
		nY = nPos / nW;
		pxImg->data.ptr[nY - psConn->sRect.y1][nX - psConn->sRect.x1] = 0;
		nPos = pxIndexImg->data.i[nY][nX];

	}while (nPos != -1);
}
void	createImageFromConnect(const PSConnectInfo psConn, const Mat& indexImage, Mat& img)
{
	assert (indexImage.isValid());

	int nConnW, nConnH;
	nConnW = psConn->sRect.width();
	nConnH = psConn->sRect.height();
	img.create(nConnH, nConnW, MAT_Tbyte);
	img.setValue(255);

	int nW, nH;
	nW = indexImage.cols();
	nH = indexImage.rows();

	int nX, nY, nPos;
	nPos = psConn->nLastPos;
	do 
	{
		nX = nPos % nW;
		nY = nPos / nW;
		img.data.ptr[nY - psConn->sRect.y1][nX - psConn->sRect.x1] = 0;
		nPos = indexImage.data.i[nY][nX];
	} while (nPos != -1);
}

void createPointArrayFromConnect(const PSConnectInfo psConn, const Mat& indexImage, Vector<Point2i>& points)
{
	points.setSize(psConn->cnPixel);
	assert (indexImage.isValid());
	int ipixel = 0;
	int nW = indexImage.cols(), nX, nY, nPos = psConn->nLastPos;
	do 
	{
		nX = nPos % nW;
		nY = nPos / nW;
		points[ipixel++] = Point2i(nX, nY);
		nPos = indexImage.data.i[nY][nX];
	} while (nPos != -1);
	assert(ipixel == psConn->cnPixel);
}

void extractBoundaryPointsFromConnect(const Mat* pxImg, int nX, int nY, uchar bVal, int max_pixels, Vector<vec2i>& points)
{
	vec2i sPt;
	int nDir = 0;
	int nTmpX, nTmpY;

	nTmpX = nX;
	nTmpY = nY;

	uchar** const ppbImg = pxImg->data.ptr;
	int nW = pxImg->cols();
	int nH = pxImg->rows();
	points.setSize(max_pixels);
	int ipixel = 0;
	points[ipixel++] = vec2i(nX, nY);

	uchar values[8];
	vec2i first_points[8];
	for (int i = 0; i < 8; i++)
	{
		sPt = NextPixel(nTmpX, nTmpY, i);
		first_points[i] = sPt;
		if ((sPt.x >= 0 && sPt.x < nW) && (sPt.y >= 0 && sPt.y < nH))
			values[i] = ppbImg[sPt.y][sPt.x];
		else
			values[i] = 128;
	}
	for (int i = 7; i >= 0; i--) {
		int nextidx = (i + 1) & 7;
		if (values[i] != values[nextidx] && values[nextidx] == 255) {
			nTmpX = first_points[nextidx].x;
			nTmpY = first_points[nextidx].y;
			nDir = (nextidx + 5);
			break;
		}
	}

	do
	{
		for (int iDir = nDir; iDir < nDir + 8; iDir++)
		{
			sPt = NextPixel(nTmpX, nTmpY, iDir);

			if ((sPt.x >= 0 && sPt.x < nW) && (sPt.y >= 0 && sPt.y < nH))
			{
				if (ppbImg[sPt.y][sPt.x] == bVal)
				{

					nDir = iDir + 5;
					nTmpX = sPt.x;
					nTmpY = sPt.y;
					points[ipixel++] = sPt;
					break;
				}
			}
		}
	} while (nTmpX != nX || nTmpY != nY);
	assert(ipixel <= max_pixels);
	points.resize(ipixel);
}

void eraseConnect(Mat* pxImg, PSConnectInfo psConn, Mat* pxIndexImg, uchar bEraseVal /*= 0xff*/)
{
	int nW, nH;
	nW = pxImg->cols();
	nH = pxImg->rows();
	
	int nX, nY, nPos;
	nPos = psConn->nLastPos;
	if (pxImg->isContinuous())
	{
		uchar* pimage = pxImg->data.ptr[0];
		const int* pindex = pxIndexImg->data.i[0];
		do
		{
			pimage[nPos] = bEraseVal;
			nPos = pindex[nPos];
		} while (nPos != -1);
	}
	else
	{
		do
		{
			nX = nPos % nW;
			nY = nPos / nW;
			pxImg->data.ptr[nY][nX] = bEraseVal;
			nPos = pxIndexImg->data.i[nY][nX];

		} while (nPos != -1);
	}
}

#pragma warning (pop)

}}
