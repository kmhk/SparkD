#include "FindSquares.h"
#include "iFindContour.h"
#include "Canny.h"
#include "Sobel.h"
#include "Contours.h"
#include "ScaleXY.h"
#include "thresh.h"
#include "Morphology.h"
#include "ipFilter.h"

namespace cvlib
{

	/*************************************************************************************/
	void findSquares4(Mat* pmSrc, Matrix<Point2i>& squares, float t_angle, bool bMultiple)
	{
		Matrix<Point2i>& temp_Squares = squares;
		Sequence* contoursSeq = 0;
		int i, l, N = 11;
		Sequence* result;
		double s, t;

		if (t_angle < 0 || t_angle >= 1)
			t_angle = 0.3f;

		int nH = pmSrc->rows();
		int nW = pmSrc->cols();

		int lim_value = std::max(nH, nW);

		MemStorage* storage = createMemStorage(0);

		int nScale = 1;

		/*if (nH > 1000 || nW > 1000)
			nScale = 5;
		else
			nScale = 1;

		nH /= nScale;
		nW /= nScale;*/

		Mat matSrc(nH, nW, MAT_Tuchar);
		Mat matTemp(nH, nW, MAT_Tuchar);

		//ip::resize(*pmSrc, matSrc, nW, nH);
		ip::median(*pmSrc, matSrc);

		for (l = 1; l < N; l++)
		{
			//Mat mdraw;
			if (l == 0)
			{
				ip::canny(matSrc, matTemp, 0, 50);
				ip::dilate(matTemp, 2, 2);
			}
			else
			{
				ip::Threshold(matSrc, matTemp, (l + 1) * 255 / N, 255, ip::THRESH_BINARY);
			}

			//ColorSpace::GraytoRGB(matTemp, mdraw);

			findContours(&matTemp, storage, &contoursSeq, 1, 2, Point2i(0, 0));

			while (contoursSeq)
			{
				double contourPerimeter = cvlibContourPerimeter(contoursSeq);

				if (contourPerimeter < lim_value) {
					result = approxPoly(contoursSeq, sizeof(IPContour), storage,
						CVLIB_POLY_APPROX_DP, contourPerimeter*0.15, 0);

					if (result->total == 4 && fabs(contourArea(result, CVLIB_WHOLE_SEQ)) > 100 &&
						checkContourConvexity(result))
					{
						s = 0;

						for (i = 0; i < 5; i++)
						{
							if (i >= 2)
							{
								t = fabs(AngleBetWeen2Lines(
									(Point2i*)getSeqElem(result, i),
									(Point2i*)getSeqElem(result, i - 2),
									(Point2i*)getSeqElem(result, i - 1)));
								s = s > t ? s : t;
							}
						}

						//if (s < t_angle)
						{
							Vector<Point2i> square;
							for (i = 0; i < 4; i++)
							{
								Point2i* pPt = (Point2i*)getSeqElem(result, i);
								square.add(*pPt);
							}
							//mdraw.drawPolygon(square.getData(), square.getSize(), COLOR(255, 0, 0), 1, false);
							temp_Squares.add(square);
						}
					}
				}
				contoursSeq = contoursSeq->h_next;
			}
			///IPDebug::addDebug(mdraw);
		}
		releaseMemStorage(&storage);

		//convert Point2i to Point2i
		/*Vector<Point2i> temp_s;
		Vector<Point2i> temp_2i;
		Point2i sp_2i;

		for (i = 0; i < temp_Squares.getSize(); i++)
		{
			temp_s = temp_Squares[i];
			for (int j = 0; j < 4; j++)
			{
				sp_2i.x = temp_s[j].x;
				sp_2i.y = temp_s[j].y;
				temp_2i.add(sp_2i);
			}
			squares.add(temp_2i);
			temp_2i.removeAll();
		}//end for loop i
		*/
		//endSquares(pmSrc, squares, nScale, bMultiple);
	}

	/*************************************************************************************/
	void endSquares(Mat* graySrc, Matrix<Point2i>& squares, int nScale, bool bMulti)
	{
		int i, j;
		Sobel sobel;
		Matrix<Point2i> temp_squares = squares;
		squares.removeAll();

		int nH = graySrc->rows() / nScale;
		int nW = graySrc->cols() / nScale;

		Mat matSrc(nH, nW, MAT_Tuchar);
		ip::resize(*graySrc, matSrc, nW, nH); //	scale.FastProcess( graySrc, &matSrc ) ;

		float rMax = 0;
		float rMean = 0;
		int nMaxid = -1;
		int nSqCount = temp_squares.getSize();

		if (nSqCount < 2)
			return;

		float rTheshold = 0;
		Vector<float> rvalue;
		float buf;
		Matrix<Point2i> tempSq = temp_squares;
		int nDelta = (int)(MAX(nW, nH) * 0.025f);
		float rDistance = (float)(nDelta * sqrtf((float)nDelta));
		Vector<Point2i> pt, tempPt;
		Point2i src[4], comp[4];

		for (i = 0; i < nSqCount; i++)
		{
			tempPt = tempSq[i];
			buf = getEdgeValue(&matSrc, tempPt);
			rMean += buf;
			rvalue.add(buf);
			if (rvalue[i] > rMax)
			{
				rMax = rvalue[i];
				nMaxid = i;
			}
		}//end for loop i
		rMean /= nSqCount;
		rTheshold = rMean + (rMax - rMean) * 0.1f;

		//Single Mode process
		int nSqH, nSqW;
		int nPerArea = 0;

		if (!bMulti)
		{
			squares.removeAll();

			int nMaxArea = -1;
			int nAreaIdx = -1;
			for (i = 0; i < tempSq.getSize(); i++)
			{
				pt = tempSq[i];
				nSqH = (int)pt[0].distTo(pt[1]);
				nSqW = (int)pt[1].distTo(pt[2]);
				nPerArea = (nSqH * nSqW * 100) / (nH * nW);

				if (2 <= nPerArea && nPerArea < 85)
				{
					if (nMaxArea < nPerArea)
					{
						nMaxArea = nPerArea;
						nAreaIdx = i;
						continue;
					}
				}
			}

			if (nAreaIdx != -1)
			{
				pt = tempSq[nAreaIdx];
				for (j = 0; j < 4; j++)
					pt[j] *= (float)nScale;
				squares.add(pt);
				return;
			}
			return;
		}

		if (rMax == 0)
		{
			temp_squares.add(tempSq[0]);
			return;
		}

		int nSize = tempSq.getSize();
		temp_squares.removeAll();

		while (nSize > 1)
		{
			for (i = 0; i < nSize; i++)
			{
				if (rvalue[i] > rMax)
				{
					rMax = rvalue[i];
					nMaxid = i;
				}
			}//end for loop i

	//  		if( rMax < rTheshold )
	// 			break ;

			if (nMaxid != -1)
			{
				temp_squares.add(tempSq[nMaxid]);
				pt = tempSq.getAt(nMaxid);
				tempSq.removeAt(nMaxid);
				rvalue.removeAt(nMaxid);
				nSize -= 1;
				nMaxid = -1;
				rMax = 0;
			}
			else
				break;

			//range filter
			src[0] = pt[0], src[1] = pt[1], src[2] = pt[2], src[3] = pt[3];
			double dis_src;
			double dis_buf = 1000;
			int nIdx = -1;

			for (i = 0; i < nSize; i++)
			{
				tempPt = tempSq.getAt(i);
				comp[0] = tempPt[0], comp[1] = tempPt[1], comp[2] = tempPt[2], comp[3] = tempPt[3];

				//find start point
				for (j = 0; j < 4; j++)
				{
					dis_src = src[0].distTo(comp[j]);
					if (dis_src < dis_buf)
					{
						dis_buf = dis_src;
						nIdx = j;
					}
				}//end for loop j

				int nState = 0;

				if (dis_buf == 1000 || nIdx == -1) continue;
				else if (dis_buf <= rDistance)
				{
					for (j = 1; j < 4; j++)
					{
						dis_buf = src[j].distTo(comp[(j + nIdx) % 4]);

						if (dis_buf > rDistance)
						{
							nState = 1;
							break;
						}
					}//end for loop j			
				}

				if (nState == 0 && dis_buf < rDistance)
				{
					tempSq.removeAt(i);
					rvalue.removeAt(i);
					nSize -= 1;
				}
			}//end for loop i		
		}//end while loop

		Point2i pBuf[4];
		Vector<Point2i> buf_2i;

		for (i = 0; i < temp_squares.getSize(); i++)
		{
			for (j = 0; j < 4; j++)
			{
				src[j] = temp_squares[i][j];
				pBuf[j].x = src[j].x * nScale;
				pBuf[j].y = src[j].y * nScale;
				buf_2i.add(pBuf[j]);
			}
			squares.add(buf_2i);
			buf_2i.removeAll();
		}//end for loop i
	}

}
