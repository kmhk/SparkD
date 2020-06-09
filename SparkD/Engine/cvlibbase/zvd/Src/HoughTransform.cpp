#include "HoughTransform.h"
#include "ipCoreABC.h"

namespace cvlib
{

class HoughTransform
{
public:
	HoughTransform();
	~HoughTransform();

	bool Process_Circle (Mat* pmatOrg, Mat* pmatImg, Point2i* pCenterPt, int* pnRadius, Rect* pRect = NULL);
	bool Process_Circle (Mat* pmatImg, Point2i* pCenterPt, int* pnRadius, Rect* pRect =  NULL);

private:
	
};

class ipHoughLines : public ipCoreFeaturePump
{
public:

	ipHoughLines();
	ipHoughLines(const ipHoughLines& from);
	ipHoughLines(float rho, float rtheta, int nThreshold, int nlinesMax);
	virtual ~ipHoughLines();

	virtual bool process (Mat* pmSrc);
	virtual bool getResultImage(Mat* pSrc);

	bool ProcessPoints(const Vector<Point2i>& points, int nNeighbors, Veci& vOIdx);
public:
	float	m_rho;			// step of line
	float	m_rtheta;		// step of angle
	int		m_nThreshold;	// edge threshold
	int		m_nlinesMax;	// max lines	
	int		m_nlinesTotal;	// returned total lines
	Point2i* m_pStart;
	Point2i* m_pEnd;

private:
	void iFilteringCycle (const Veci& vIn, Veci& vOut, int* pnFilter, int nFilterlen);
	int iOrientation(const Point2i& pt1, const Point2i& pt2);
};

HoughTransform::HoughTransform()
{
}
HoughTransform::~HoughTransform()
{
}

/**
 * @brief Search Circle
 *
 * @param pmatOrg	[in]:	Original Gray Image
 * @param pmatImg	[in]:	Edge Image
 * @param pCenterPt [out]:	Circle Center Point
 * @param pnRadius	[out]:	Circle Radius
 * @param pRect		[in]:	Search Rect
 * @return bool 
 */
bool HoughTransform::Process_Circle (Mat* pmatOrg, Mat* pmatImg, Point2i* pCenterPt, int* pnRadius, Rect* pRect/* =  NULL*/)
{
	int nImgWidth = pmatImg->cols();
	int nImgHeight = pmatImg->rows();
	int nSmallRadius = nImgWidth / 8;
	if (*pnRadius != -1)
		nSmallRadius = *pnRadius/* * 2*/;
	int nRadiusOffset = 3;

	Rect rcSearchRect;
	if (pRect == NULL)
	{
		if (pCenterPt->x != -1 && pCenterPt->y != -1)
		{
			rcSearchRect.x = MAX((pCenterPt->x - nSmallRadius), 0);
			rcSearchRect.y = MAX((pCenterPt->y - nSmallRadius / 2), 0);
			rcSearchRect.width = nSmallRadius * 2;
			rcSearchRect.height = nSmallRadius;
			rcSearchRect.width = MIN(rcSearchRect.x + rcSearchRect.width, nImgWidth) - rcSearchRect.x;
			rcSearchRect.height = MIN (rcSearchRect.y + rcSearchRect.height, nImgHeight) - rcSearchRect.y;
		}
		else
		{
			rcSearchRect.x = rcSearchRect.y = nSmallRadius;
			rcSearchRect.width = nImgWidth - nSmallRadius;
			rcSearchRect.height = nImgHeight - nSmallRadius;
		}
	}
	else
		rcSearchRect = *pRect;

	Mat* pmatCenterRadius = new Mat(nImgHeight, nImgWidth, MAT_Tint);
	Mat* pmatCenterCount = new Mat(nImgHeight, nImgWidth, MAT_Tint);
	pmatCenterRadius->zero();
	pmatCenterCount->zero();

	uchar** ppbData = pmatImg->data.ptr;
	for (int iOffset = -/*nRadiusOffset*/0 ; iOffset <= nRadiusOffset ; iOffset ++)
	{
		int nRadius = nSmallRadius + iOffset; // Search Radius
		for (int iCenterY = rcSearchRect.y ; iCenterY < rcSearchRect.y + rcSearchRect.height ; iCenterY ++)
		{
			for (int iCenterX = rcSearchRect.x ; iCenterX < rcSearchRect.x + rcSearchRect.width ; iCenterX ++)
			{
				if (pmatOrg->data.ptr[iCenterY][iCenterX] > 100)
					continue;
				
				/* Circular points */
				for (int iY = MAX((iCenterY - nRadius), 0) ; iY < MIN((iCenterY + nRadius), nImgHeight - 1) ; iY ++)
				{
					for (int iX = MAX((iCenterX - nRadius), 0) ; iX < MIN((iCenterX + nRadius), nImgWidth - 1) ; iX ++)
					{
						/* Is not Edge point? */
						if (ppbData[iY][iX] < 100)
							continue;
// 						if (ppbData[iY][iX] > 50)
// 							continue;
						if ((SQR(iY - iCenterY) + SQR(iX - iCenterX)) == SQR(nRadius))
						{
							pmatCenterCount->data.i[iCenterY][iCenterX] ++;
							pmatCenterRadius->data.i[iCenterY][iCenterX] += nRadius;
						}
					}
				}
			}
		}
	}

	int nMaxPoint = 0;
	pCenterPt->x = pCenterPt->y = -1;
	*pnRadius = -1;
	for (int iRow = 0 ; iRow < nImgHeight ; iRow ++)
	{
		for (int iCol = 0 ; iCol < nImgWidth ; iCol ++)
		{
			if (nMaxPoint < pmatCenterCount->data.i[iRow][iCol])
			{
				nMaxPoint = pmatCenterCount->data.i[iRow][iCol];
				pCenterPt->y = iRow;
				pCenterPt->x = iCol;
				*pnRadius = pmatCenterRadius->data.i[iRow][iCol] / pmatCenterCount->data.i[iRow][iCol];
			}
			else if (nMaxPoint > 0 && nMaxPoint == pmatCenterCount->data.i[iRow][iCol])
			{
				if (pCenterPt->y < iRow)
				{
					pCenterPt->y = iRow;
					pCenterPt->x = iCol;
					*pnRadius = pmatCenterRadius->data.i[iRow][iCol] / pmatCenterCount->data.i[iRow][iCol];
				}
			}
		}
	}

	pmatCenterRadius->release();
	pmatCenterCount->release();
	delete pmatCenterRadius;
	delete pmatCenterCount;

	return true;
}

/**
 * @brief
 *
 * @param pmatImg	[in]:	Original Gray Image
 * @param pCenterPt [out]:	Circle Center Point
 * @param pnRadius	[out]:	Circle Radius
 * @param pRect		[in]:	Search Rect
 * @return bool 
 */
bool HoughTransform::Process_Circle (Mat* pmatImg, Point2i* pCenterPt, int* pnRadius, Rect* pRect/* =  NULL*/)
{
	int nImgWidth = pmatImg->cols();
	int nImgHeight = pmatImg->rows();
	int nSmallRadius = nImgWidth / 8;
	if (*pnRadius != -1)
		nSmallRadius = *pnRadius;
	int nRadiusOffset = *pnRadius/3;

	/* Compute Search Rect */
	Rect rcSearchRect;
	if (pRect == NULL)
	{
		if (pCenterPt->x != -1 && pCenterPt->y != -1)
		{
			rcSearchRect.x = MAX((pCenterPt->x - nSmallRadius), 0);
			rcSearchRect.y = MAX((pCenterPt->y - nSmallRadius / 2), 0);
			rcSearchRect.width = nSmallRadius * 2;
			rcSearchRect.height = nSmallRadius;
			rcSearchRect.width = MIN(rcSearchRect.x + rcSearchRect.width, nImgWidth) - rcSearchRect.x;
			rcSearchRect.height = MIN (rcSearchRect.y + rcSearchRect.height, nImgHeight) - rcSearchRect.y;
		}
		else
		{
			rcSearchRect.x = rcSearchRect.y = nSmallRadius;
			rcSearchRect.width = nImgWidth - nSmallRadius;
			rcSearchRect.height = nImgHeight - nSmallRadius;
		}
	}
	else
		rcSearchRect = *pRect;

	Mat* pmatCenterRadius = new Mat(nImgHeight, nImgWidth, MAT_Tint);
	Mat* pmatCenterCount = new Mat(nImgHeight, nImgWidth, MAT_Tint);
	pmatCenterRadius->zero();
	pmatCenterCount->zero();

	float rMinValue = MAX_FLOAT;
	pCenterPt->x = pCenterPt->y = -1;
	*pnRadius = -1;
	for (int iOffset = 0/*-nRadiusOffset*/ ; iOffset <= nRadiusOffset ; iOffset ++)
	{
		int nRadius = nSmallRadius + iOffset; // Search Radius
		for (int iCenterY = rcSearchRect.y ; iCenterY < rcSearchRect.y + rcSearchRect.height ; iCenterY ++)
		{
			for (int iCenterX = rcSearchRect.x ; iCenterX < rcSearchRect.x + rcSearchRect.width ; iCenterX ++)
			{
				if (pmatImg->data.ptr[iCenterY][iCenterX] > 100)
					continue;
				
				/* Search Circular points */
				float rCircularPtValue = 0.0f;
				int nPtCount = 0;
				for (int iY = MAX((iCenterY - nRadius), 0) ; iY < MIN((iCenterY + nRadius), nImgHeight - 1) ; iY ++)
				{
					for (int iX = MAX((iCenterX - nRadius), 0) ; iX < MIN((iCenterX + nRadius), nImgWidth - 1) ; iX ++)
					{
						/* Is not Edge point? */
// 						if (ppbData[iY][iX] > 100)
// 							continue;
						if ((SQR(iY - iCenterY) + SQR(iX - iCenterX) + 3) < SQR(nRadius))
						{
							nPtCount ++;
							rCircularPtValue += (float)pmatImg->data.ptr[iY][iX];
						}
					}
				}
				rCircularPtValue /= nPtCount;
				if (rMinValue > rCircularPtValue)
				{
					rMinValue = rCircularPtValue;
					pCenterPt->y = iCenterY;
					pCenterPt->x = iCenterX;
					*pnRadius = nRadius;
				}
			}
		}
	}

	pmatCenterRadius->release();
	pmatCenterCount->release();
	delete pmatCenterRadius;
	delete pmatCenterCount;

	return true;
}

	ipHoughLines::ipHoughLines()
	{
		m_rho = 0;
		m_rtheta = 0;
		m_nThreshold = 0;
		m_nlinesMax = 0;
		m_nlinesTotal = 0;
		m_pEnd = NULL;
		m_pStart = NULL;
	}

	ipHoughLines::ipHoughLines(const ipHoughLines& from)
	{
		m_rho=from.m_rho;
		m_rtheta=from.m_rtheta;
		m_nThreshold=from.m_nThreshold;
		m_nlinesMax=from.m_nlinesMax;
	}

	ipHoughLines::~ipHoughLines()
	{
		if (m_pStart)
			delete m_pStart;
		if (m_pEnd)
			delete m_pEnd;


	}

	ipHoughLines::ipHoughLines(float rho, float rtheta, int nThreshold, int nlinesMax)
	{
		m_rho = rho;
		m_rtheta = rtheta;
		m_nThreshold = nThreshold;
		m_nlinesMax = nlinesMax;
		m_nlinesTotal = 0;
		m_pEnd = NULL;
		m_pStart = NULL;

	}

	bool ipHoughLines::process (Mat* pmSrc)
	{
		if ( pmSrc == NULL )
			return false;

		struct sLinePolar{ float rho; float angle; };

		Mati	mAccum;
		int *sort_buf_r =0;
		int *sort_buf_n = 0;
		float *tabSin = 0;
		float *tabCos = 0;
		float ang;
		int r, n;
		int i, j;
		int ri, rj, ni, nj, temp;
		int width, height;
		int numangle, numrho;
		float irho = 1 / m_rho;
		uchar** const image = pmSrc->data.ptr;
		
		m_nlinesTotal = 0;
		height = pmSrc->rows();
		width = pmSrc->cols();

		numangle = (int)(CVLIB_PI / m_rtheta);
		numrho = (int)(((width + height) * 2 + 1) / m_rho);

		mAccum.create (numangle + 2, numrho + 2);
		mAccum.zero();
	
		sort_buf_r = (int *)malloc(sizeof(int) * numangle * numrho);
		if (sort_buf_r == NULL) return false;

		sort_buf_n = (int *)malloc(sizeof(int) * numangle * numrho);
		if (sort_buf_n == NULL) return false;

		tabSin = (float *)malloc(sizeof(int) * numangle);
		if (tabSin == NULL) return false;

		tabCos = (float *)malloc(sizeof(int) * numangle);
		if (tabCos == NULL) return false;
		
		for( ang = 0, n = 0; n < numangle ; ang += m_rtheta, n++ )
		{
			tabSin[n] = (float)(sin(ang) * irho);
			tabCos[n] = (float)(cos(ang) * irho);
		}

		// stage 1. fill accumulator
		for( i = 0; i < height; i++ )
			for( j = 0; j < width; j++ )
			{
				if (image[i][j] == 0)
					for( n = 0; n < numangle; n++ )
					{
						r = (int)( j * tabCos[n] + i * tabSin[n]);
						mAccum[n + 1][r + 1]++;
					}
			}

		{
			Mat mt;
			mAccum.convertTo(mt, MAT_Tuchar);
			for (i = 0; i < mt.rows()*mt.cols()*mt.channels(); i++) {
				mAccum.data.i[0][i] = mt.data.ptr[0][i];
			}
		}
		
		// stage 2. find local maximums 
		for( r = 0; r <= numrho; r++)
			for( n = 0; n < numangle; n++)
			{
				if ( mAccum[n][r] > m_nThreshold &&
					 mAccum[n][r] > mAccum[n - 1][r] && mAccum[n][r] > mAccum[n + 1][r] &&
					 mAccum[n][r] > mAccum[n][r - 1] && mAccum[n][r] > mAccum[n][r + 1])
				{
					 sort_buf_r[m_nlinesTotal] = r;
					 sort_buf_n[m_nlinesTotal] = n;
					 m_nlinesTotal++;
				}
			}
		// stage 3. sort the detected lines by accumulator value
		for( i = 0; i < m_nlinesTotal - 1; i++)
			for( j = i + 1; j < m_nlinesTotal; j++)
			{
				ri = sort_buf_r[i];
				ni = sort_buf_n[i];

				rj = sort_buf_r[j];
				nj = sort_buf_n[j];

				if (mAccum[ni][ri] < mAccum[nj][rj])
				{
					temp = mAccum[ni][ri];
					mAccum[ni][ri] = mAccum[nj][rj];
					mAccum[nj][rj] = temp;

					r = sort_buf_r[i];
					sort_buf_r[i] = sort_buf_r[j];
					sort_buf_r[j] = r;
					
					n = sort_buf_n[i];
					sort_buf_n[i] = sort_buf_n[j];
					sort_buf_n[j] = n;
				}
			}

		// stage 4. store the first min(total,linesMax) lines to the output buffer
		m_nlinesMax = MIN(m_nlinesMax, m_nlinesTotal);

		int x, y;
		bool flag;
		m_pStart = new Point2i[m_nlinesMax];
		m_pEnd = new Point2i[m_nlinesMax];
		for(i = 0; i < m_nlinesMax; i++)
		{
			sLinePolar line;
			n = sort_buf_n[i] - 1;
			r = sort_buf_r[i] - 1;
			line.rho = r * m_rho;
			line.angle = n * m_rtheta;

			double a = cos(line.angle), b = sin(line.angle);
			double x0 = a * line.rho, y0 = b * line.rho;

			m_pStart[i].x = 0;
			m_pStart[i].y = 0;

			m_pEnd[i].x = 0;
			m_pEnd[i].y = 0;

			flag = false;
			for(j = 1000; j >= -1000; j--)
			{
				x = (int)(x0 + j * (-b));
				y = (int)(y0 + j * (a));
				if ((x < 0) || (x >= width) ||
					(y < 0) || (y >= height)) continue;

				if (!flag && !image[y][x])				
				{
					flag = true;
					m_pStart[i].x = x;
					m_pStart[i].y = y;
				}
				if (flag && image[y][x])				
				{
					m_pEnd[i].x = x;
					m_pEnd[i].y = y;
					break;
				}
				
			}

			
		}
		
		for( r = 0; r <= numrho; r++)
			for( n = 0; n < numangle; n++)
			{
				if (mAccum[n][r] > m_nThreshold)
				{
					mAccum[n][r] = 255;
				}
				else
				{
					mAccum[n][r] = 0;
				}
			
			}		
	
		free(sort_buf_r);
		free(sort_buf_n);
		free(tabSin);
		free(tabCos);
		mAccum.release();
		
		return true;
	}

	bool ipHoughLines::getResultImage(Mat* pSrc)
	{
		int i;
		if (!m_nlinesMax) 
			return false;
		for( i = 0; i < m_nlinesMax; i++)
		{
			pSrc->drawLine(Point(m_pStart[i].x, m_pStart[i].y), Point(m_pEnd[i].x, m_pEnd[i].y), COLOR(255, 0, 0));
		}
		return true;
	}

	int ipHoughLines::iOrientation(const Point2i& pt1, const Point2i& pt2)
	{
		return (int)(atanf((float)(pt1.y-pt2.y) / (float)(pt1.x-pt2.x)) / CVLIB_PI * 180+180) % 180;
	}
	bool ipHoughLines::ProcessPoints(const Vector<Point2i>& points, int nNeighbors, Veci& vOIdx)
	{
		int i, j;
		int nPointNum=points.getSize();
		Matf mDist(nPointNum, nPointNum);
		Mati mOrient(nPointNum, nPointNum);
		mDist=0;
		mOrient=0;
		for (i=0; i<nPointNum-1; i++)
		{
			for (j=i+1; j<nPointNum; j++)
			{
				mDist[i][j] = (float)points[i].distTo(points[j]);
				mDist[j][i] = mDist[i][j];
				
				mOrient[i][j] = iOrientation(points[i], points[j]);
				mOrient[j][i] = mOrient[i][j];
			}
		}
		
		Veci vIdx(nNeighbors+1);
		Veci vOrientHistogram(180);
		vOrientHistogram=0;
		for (i=0; i<nPointNum; i++)
		{
			cvutil::sortIdx(mDist.data.fl[i], nPointNum, vIdx, nNeighbors + 1, cvutil::SORT_INC);
			for (int k=1; k<nNeighbors+1; k++)
			{
				vOrientHistogram[mOrient[i][vIdx[k]]] ++;
			}
		}
		vIdx.release();
		
		Veci vFiltered(180);
		int anFilter[] = {1, 4, 6, 4, 1};
//		int anFilter[] = {1, 6, 15, 20, 15, 6, 1};
		int nFilterlen=sizeof(anFilter)/sizeof(anFilter[0]);
		iFilteringCycle (vOrientHistogram, vFiltered, anFilter, nFilterlen);
		vOrientHistogram.release();
		
		Veci vTemp(180);
		for (i=0; i<180; i++)
		{
			int nPrev=(i-1+180)%180;
			int nNext=(i+1)%180;
			if (vFiltered[i] <= vFiltered[nPrev] || vFiltered[i] <= vFiltered[nNext])
				vTemp[i]=0;
			else
				vTemp[i]=vFiltered[i];
		}
		vFiltered.release();
		vOIdx.resize(180);
		cvutil::sortIdx (vTemp, vTemp.length(), vOIdx, 180, cvutil::SORT_DEC);
		vTemp.release();
		return true;
	}
	void ipHoughLines::iFilteringCycle (const Veci& vIn, Veci& vOut, int* pnFilter, int nFilterlen)
	{
		int nVecLen=vIn.length();
		int i;
		vOut=0;
		int nSide=nFilterlen/2;
		for (i=0; i<nFilterlen/2; i++)
		{
			for (int k=0; k<nFilterlen; k++)
				vOut[i]+=pnFilter[k]*vIn[ (nVecLen+i-nSide+k) % nVecLen];
		}
		for (i=nFilterlen/2; i<nVecLen-nFilterlen/2; i++)
		{
			for (int k=0; k<nFilterlen; k++)
				vOut[i]+=pnFilter[k]*vIn[i-nSide+k];
		}
		for (i=nVecLen-nFilterlen/2; i<nVecLen; i++)
		{
			for (int k=0; k<nFilterlen; k++)
				vOut[i]+=pnFilter[k]*vIn[ (nVecLen+i-nSide+k) % nVecLen];
		}
	}

/************************************************************************/
/*                                                                      */
/************************************************************************/

/*
	ipHoughCircles::ipHoughCircles()
	{

	}

	ipHoughCircles::ipHoughCircles(const ipHoughCircles& from)
	{
		m_nMinRadius=from.m_nMinRadius;
		m_nMaxRadius=from.m_nMaxRadius;
		m_nStep=from.m_nStep;
		m_nAccthreshold=from.m_nAccthreshold;
		m_nlimitR=from.m_nlimitR;
	}

	ipHoughCircles::~ipHoughCircles()
	{

	}

	ipHoughCircles::ipHoughCircles(int nMinRadius, int nMaxRadius, int nStep, int nAccthreshold, int nlimitR)
	{
		m_nMinRadius = nMinRadius;
		m_nMaxRadius = nMaxRadius;
		m_nStep = nStep;
		m_nAccthreshold = nAccthreshold;
		m_nlimitR = nlimitR;
	}

	bool ipHoughCircles::process(CoImage* pSrc)
	{
		
		const int SHIFT = 10, ONE = 1 << SHIFT, R_THRESH = 100;

		int height = pSrc->height();
		int width = pSrc->width();

		int MinDist = 1;

		m_nMinRadius = MAX( m_nMinRadius , 0 );

		m_nMinRadius = MAX( m_nMinRadius, 0 );
		if( m_nMaxRadius <= 0 )
			m_nMaxRadius = MAX( height, width );
		else if( m_nMaxRadius <= m_nMinRadius )
			m_nMaxRadius = m_nMinRadius + 2;
		
		int nMaxCircle = height + width - 1;
		int row = (int)(height / m_nStep);
		int col = (int)(width / m_nStep);
		int nz_count = 0, center_count = 0;
		int max_nz_count = 100;
		
		Mati nAccum;
		nAccum.create(row + 2, col + 2);
		nAccum.zero();
		
		int* nz;
		MALLOC(nz, 2 * max_nz_count * sizeof(int), int)
		float rStep = 1.f/m_nStep, rSwap;
		int y, x, i, j, sx, sy, x0, y0, x1, y1, x2, y2, k, r, nSwap, vx, vy;
		
		Mat* dx = new Mat(height, width, MAT_Tint);
		Mat* dy= new Mat(height, width, MAT_Tint); 
		Mat* pmGray = new Mat(height, width, MAT_Tbyte);
		CoImage* EdgeImg = new CoImage(height, width, pSrc->GetType(), CoImage::enCOLOR_MODE);

		ipCanny canny;
		canny.process(pSrc, EdgeImg);

		pSrc->Gray(pmGray);

		int Kernel1[] = {-1, 0, 1, -2, 0 , 2, -1, 0, 1};
		int Kernel2[] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
		
		uchar **ppbgray = pmGray->data.ptr;

		for( y = 0; y < height; y++ )
			for( x = 0 ; x < width; x++)
			{
				if ( (y - 1) >= 0 && (y + 1) < height && (x - 1) >= 0 && (x + 1) < width )
				{
					int b = 0;
					int c = 0;
					int iCount = 0;
					for( i = -1; i <= 1; i++ )
					{
						for( j = -1; j <= 1; j++ )
						{
							b += ppbgray[y + i][x + j] * Kernel1[iCount];
							c += ppbgray[y + i][x + j] * Kernel2[iCount];
 							iCount++;
						}
					}
					dx->data.ptr[y][x] = b;
					dy->data.ptr[y][x] = c;
				}
				else
				{
					dx->data.ptr[y][x] = 0;
					dy->data.ptr[y][x] = 0;
				}
			}

		Mati mdx;
		mdx.create(height, width);
			
		for (j = 0 ; j < height ; j ++)
		{
			for (i = 0 ; i < width ; i++)
			{
				mdx[j][i] = dy->data.ptr[j][i];
			}
		}

		for (j = 0 ; j < height ; j ++)
		{
			for (i = 0 ; i < width ; i ++)
			{
				vx = dx->data.ptr[j][i];
				vy = dy->data.ptr[j][i];				
				
				if( EdgeImg->m_matX.data.ptr[j][i] || vx == 0 && vy == 0 )
					continue;
				
				if( ABS(vx) < ABS(vy) )
				{
					sx = (int)(vx*ONE/ABS(vy) + 0.5);
					sy = vy < 0 ? -ONE : ONE;
				}
				else
				{
					assert( vx != 0 );
					sy = (int)(vy*ONE/ABS(vx) + 0.5);
					sx = vx < 0 ? -ONE : ONE;
				}
				
				x0 = (int)((i*rStep)*ONE + 0.5) + ONE + (ONE/2);
				y0 = (int)((j*rStep)*ONE + 0.5) + ONE + (ONE/2);
				
				for( k = 0; k < 2; k++ )
				{
					x0 += m_nMinRadius * sx;
					y0 += m_nMinRadius * sy;
					
					for( x1 = x0, y1 = y0, r = m_nMinRadius; r <= m_nMaxRadius; x1 += sx, y1 += sy, r++ )
					{
						x2 = x1 >> SHIFT, y2 = y1 >> SHIFT;
						if( (unsigned)x2 >= (unsigned)col || (unsigned)y2 >= (unsigned)row )
							break;
						nAccum[y2][x2]++;
					}
					
					x0 -= m_nMinRadius * sx;
					y0 -= m_nMinRadius * sy;
					sx = -sx; sy = -sy;
				}
				
				nz[nz_count * 2] = i ;
				nz[nz_count * 2 + 1] = j ;
				
				nz_count ++ ;
				
				if (nz_count >= max_nz_count )
				{
					max_nz_count = max_nz_count * 2;
					nz = (int*)realloc(nz, sizeof(int) * ( 2 * max_nz_count));
				}
				
			}
		}
		
		if(!nz_count) return 0;
		
	 	for (j = 0 ; j < height ; j ++)
	 	{
	 		for (i = 0 ; i < width ; i++)
	 		{
	  			if (nAccum[j][i] > m_nAccthreshold)
	  			{
	 				mdx[j][i] = 0;
	  			}
	 			else
	 				mdx[j][i] = 255;
	 		}
	 	}
	 	mdx.release();
		
		int* centerx;
		int* centery;
		MALLOC(centerx, row * col * sizeof(int), int);
		MALLOC(centery, row * col * sizeof(int), int);

		for( j = 1; j < row - 1; j++ )
		{
			for( i = 1; i < col - 1; i++ )
			{
				if( nAccum[j][i] > m_nAccthreshold &&
					nAccum[j][i] > nAccum[j][i - 1] && nAccum[j][i] > nAccum[j][i + 1] &&
					nAccum[j][i] > nAccum[j - 1][i] && nAccum[j][i] > nAccum[j + 1][i] )
				{
					centerx[center_count] = i;
					centery[center_count] = j;
					center_count ++;
				}
			}
		}

		if(!center_count) return 0;
		
		for( i = 0; i < center_count - 1; i++ )
			for( j = i + 1; j < center_count; j++ )
			{
				int xi, yi, xj, yj, temp;
				
				xi = centerx[i];
				yi = centery[i];
				xj = centerx[j];
				yj = centery[j];
				
				if (nAccum[yi][xi] < nAccum[yj][xj])
				{
					temp = nAccum[yi][xi];
					nAccum[yi][xi] = nAccum[yj][xj];
					nAccum[yj][xj] = temp;
					
					temp = centerx[i];
					centerx[i] = centerx[j];
					centerx[j] = temp;
					
					temp = centery[i];
					centery[i] = centery[j];
					centery[j] = temp;
				}
			}

	MinDist = MAX( MinDist, m_nStep);
	MinDist *= MinDist;

	float* dist;
	int* sort_buf;
	
	MALLOC(dist, nz_count * sizeof(float), float);
	MALLOC(sort_buf, nz_count * sizeof(int) , int);

	int circle_count = 0 ;
	float* m_szCircle;
	MALLOC(m_szCircle, nMaxCircle * 3 * sizeof(float), float)

	for (k = 0 ; k < center_count ; k ++)
	{
        i = centerx[k];
        j = centery[k];
        x0 = i * m_nStep, y0 = j * m_nStep;
       
		int start_idx = nz_count - 1;
        float start_dist, dist_sum;
        float r_best = 0;
        int max_count = R_THRESH;
		
        for( r = 0; r < circle_count; r++ )
        {
            x1 = (int)m_szCircle[3 * r];
			y1 = (int)m_szCircle[3 * r + 1];
            
			if( (x1 - x0)*(x1 - x0) + (y1 - y0)*(y1 - y0) < MinDist )
                break;
        }
		
        if( r < circle_count )
            continue;

		for( r = 0; r < nz_count; r++ )
        {
            x1 = x0 - nz[2 * r];
			y1 = y0 - nz[2 * r + 1];
            dist[r] = powf( (float)(x1 * x1 + y1 * y1), 0.5f);
			sort_buf[r] = r ;
        }

		for( i = 0; i < nz_count - 1; i++ )
			for( j = i + 1; j < nz_count; j++ )
				if ( dist[i] < dist[j])
				{
					rSwap = dist[i];
					dist[i] = dist[j];
					dist[j] = rSwap;

					nSwap = sort_buf[i];
					sort_buf[i] = sort_buf[j];
					sort_buf[j] = nSwap;
				}


		dist_sum = start_dist = dist[sort_buf[nz_count-1]];
	
		for( j = nz_count - 2; j >= 0; j-- )
        {
            float d = dist[sort_buf[j]];
			
            if( d > m_nMaxRadius )
                break;
			
            if( d - start_dist > m_nStep )
            {
                float r_cur = dist[sort_buf[(j + start_idx)/2]];
                if( (start_idx - j)*r_best >= max_count*r_cur ||
                    r_best < FLT_EPSILON && start_idx - j >= max_count )
                {
                    r_best = r_cur;
                    max_count = start_idx - j;
                }
                start_dist = d;
                start_idx = j;
                dist_sum = 0;
            }
            dist_sum += d;
        }
		
        if( max_count > R_THRESH )
        {
            m_szCircle[3 * circle_count] = (float)x0;
            m_szCircle[3 * circle_count + 1] = (float)y0;
            m_szCircle[3 * circle_count + 2] = (float)r_best;
		
			circle_count++;
           
			if( circle_count > nMaxCircle )
                break;
        }
	}

 	int nXcir = 0, nYcir = 0, nRad = 0;
 
 	nXcir = (int)m_szCircle[0];
 	nYcir = (int)m_szCircle[1];
 	nRad = (int)m_szCircle[2];

	m_nXCir = nXcir;
	m_nYCir = nYcir;
	m_nRad = nRad;

	nAccum.release();
	free((void*)nz);
	nz = NULL;
	FREE((void*)centerx);
	centerx = NULL;
	FREE((void*)centery);
	centery = NULL;
	FREE((void*)dist);
	dist = NULL;
	FREE((void*)sort_buf);
	sort_buf = NULL;
	
	delete pmGray;
	delete dx;
	delete dy;
	
	m_nCircleCount = circle_count;

	return true;

	}

	bool ipHoughCircles::process(Mat* pmSrc)
	{
		return true;
	}

	bool ipHoughCircles::getResultImage(CoImage* pSrc)
	{
		if (m_nCircleCount == 0) return false;

		Rect rect ;
		rect.height = rect.width = 2 * m_nRad;
		rect.x = m_nXCir - m_nRad ;
		rect.y = m_nYCir - m_nRad ;

		if (rect.y < 0) rect.y = 0;

		pSrc->drawEllipse(&rect, COLOR(255, 0, 0), 1);
		return true;

	}
	*/
namespace ip
{

void houghLines( const Mat& image, Vector<Point2i>& lines, float rho, float theta, int threshold, 
				int lineLength, int lineGap )
{
    Mat accum, mask;
    Vector<float> trigtab;

    int width, height;
    int numangle, numrho;
    float ang;
    int r, n, count;
    Point2i pt;
    float irho = 1 / rho;
    const float* ttab;
    uchar** mdata0;
	const int linesMax = image.cols()*image.rows();

	assert( image.type() == MAT_Tbyte);

    width = image.cols();
    height = image.rows();

    numangle = cvutil::round(CVLIB_PI / theta);
    numrho = cvutil::round(((width + height) * 2 + 1) / rho);

    accum.create( numangle, numrho, MAT_Tint );
    mask.create( height, width, MAT_Tbyte );
    trigtab.setSize(numangle*2);
	accum.zero();

    for( ang = 0, n = 0; n < numangle; ang += theta, n++ )
    {
        trigtab[n*2] = (float)(cos(ang) * irho);
        trigtab[n*2+1] = (float)(sin(ang) * irho);
    }
    ttab = &trigtab[0];
    mdata0 = mask.data.ptr;

	Vector<Point2i> seq;

    // stage 1. collect non-zero image points
    for( pt.y = 0; pt.y < height; pt.y++ )
    {
        const uchar* data = image.data.ptr[pt.y];
        uchar* mdata = mdata0[pt.y];
        for( pt.x = 0; pt.x < width; pt.x++ )
        {
            if( data[pt.x] == 0 ) // EDGE_PIXEL
            {
                mdata[pt.x] = (uchar)1;
				seq.add(pt);
            }
            else
                mdata[pt.x] = 0;
        }
    }

	count = seq.getSize();
	Random::init();
    // stage 2. process all the points in random order
    for(; count > 0; count--)
    {
        // choose random point out of the remaining ones
		uint64 t=Random::uniform();
		int idx = (t*t) % count;
        int max_val = threshold-1, max_n = 0;
        Point2i* point=&seq[idx];
        Point2i line_end[2];
        float a, b;
        int** adata = accum.data.i;
        int i, j, k, x0, y0, dx0, dy0, xflag;
        int good_line;
        const int shift = 16;

        i = point->y;
        j = point->x;

        // "remove" it by overriding it with the last element
        *point = seq[count-1];

        // check if it has been excluded already (i.e. belongs to some other line)
        if( !mdata0[i][j] )
            continue;

        // update accumulator, find the most probable line
        for( n = 0; n < numangle; n++)
        {
            r = cvutil::round( j * ttab[n*2] + i * ttab[n*2+1] );
            r += (numrho - 1) / 2;
            int val = ++adata[n][r];
            if( max_val < val )
            {
                max_val = val;
                max_n = n;
            }
        }

        // if it is too "weak" candidate, continue with another point
        if( max_val < threshold )
            continue;

        // from the current point walk in each direction
        // along the found line and extract the line segment
        a = -ttab[max_n*2+1];
        b = ttab[max_n*2];
        x0 = j;
        y0 = i;
        if( fabs(a) > fabs(b) )
        {
            xflag = 1;
            dx0 = a > 0 ? 1 : -1;
            dy0 = cvutil::round( b*(1 << shift)/fabs(a) );
            y0 = (y0 << shift) + (1 << (shift-1));
        }
        else
        {
            xflag = 0;
            dy0 = b > 0 ? 1 : -1;
            dx0 = cvutil::round( a*(1 << shift)/fabs(b) );
            x0 = (x0 << shift) + (1 << (shift-1));
        }

        for( k = 0; k < 2; k++ )
        {
            int gap = 0, x = x0, y = y0, dx = dx0, dy = dy0;

            if( k > 0 )
                dx = -dx, dy = -dy;

            // walk along the line using fixed-point arithmetics,
            // stop at the image border or in case of too big gap
            for( ;; x += dx, y += dy )
            {
                int i1, j1;

                if( xflag )
                {
                    j1 = x;
                    i1 = y >> shift;
                }
                else
                {
                    j1 = x >> shift;
                    i1 = y;
                }

                if( j1 < 0 || j1 >= width || i1 < 0 || i1 >= height )
                    break;

                // for each non-zero point:
                //    update line end,
                //    clear the mask element
                //    reset the gap
                if( mdata0[i1][j1] )
                {
                    gap = 0;
                    line_end[k].y = i1;
                    line_end[k].x = j1;
                }
                else if( ++gap > lineGap )
                    break;
            }
        }

        good_line = ABS(line_end[1].x - line_end[0].x) >= lineLength ||
                    ABS(line_end[1].y - line_end[0].y) >= lineLength;

        for( k = 0; k < 2; k++ )
        {
            int x = x0, y = y0, dx = dx0, dy = dy0;

            if( k > 0 )
                dx = -dx, dy = -dy;

            // walk along the line using fixed-point arithmetics,
            // stop at the image border or in case of too big gap
            for( ;; x += dx, y += dy )
            {
                int i1, j1;

                if( xflag )
                {
                    j1 = x;
                    i1 = y >> shift;
                }
                else
                {
                    j1 = x >> shift;
                    i1 = y;
                }

                // for each non-zero point:
                //    update line end,
                //    clear the mask element
                //    reset the gap
                if( mdata0[i1][j1] )
                {
                    if( good_line )
                    {
                        for( n = 0; n < numangle; n++)
                        {
                            r = cvutil::round( j1 * ttab[n*2] + i1 * ttab[n*2+1] );
                            r += (numrho - 1) / 2;
                            accum.data.i[n][r]--;
                        }
                    }
                    mdata0[i1][j1] = 0;
                }

                if( i1 == line_end[k].y && j1 == line_end[k].x )
                    break;
            }
        }

        if( good_line )
        {
			lines.add (line_end[0]);
			lines.add (line_end[1]);
			if( lines.getSize() >= linesMax)
                return;
        }
    }
}

void houghCircle( const Mat&  image, Point2i& center, int& radius)
{
	HoughTransform t;
	t.Process_Circle ((Mat*)&image, &center, &radius);
}

void houghLinePoints(const Vector<Point2i>& points, int nNeighbors, Veci& vOIdx)
{
	ipHoughLines houghTransform;
	houghTransform.ProcessPoints (points, nNeighbors, vOIdx);
}

void houghLinesStd(const Mat& image, Vector<Point2i>& lines, float rho, float theta, int threshold, int linesMax)
{
	lines.removeAll();
	ipHoughLines hough(rho, theta, threshold, linesMax);
	hough.process((Mat*)&image);// points, nNeighbors, vOIdx);
	for (int i = 0; i < hough.m_nlinesMax; i++) {
		lines.add(hough.m_pStart[i]);
		lines.add(hough.m_pEnd[i]);
	}
}

}}
