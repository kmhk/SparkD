/*!
 * \file	ipMorphology.cpp
 * \ingroup ipCore
 * \brief   
 * \author  
 */
#include "MatOperation.h"
#include "ipCoreABC.h"
#include "Morphology.h"
#include <algorithm>
#include <cstdlib>
#include <cmath>

extern cvlib::uchar icvSaturate8u_cv[];


namespace cvlib
{

/************************************************************************/
/*                                                                      */
/************************************************************************/
namespace ip
{

CVLIB_DECLSPEC void erode(const Mat& src, Mat& dst, int nkx, int nky)
{
//	if (nkx == 3 && nky == 3)
	{
		// binary mode
		int rows = src.rows();
		int cols = src.cols();
		int kx2 = nkx / 2;
		int ky2 = nky / 2;
		Mat temp;
		dst.create(rows, cols, MAT_Tbyte);
		dst = 0;//src;
		temp = dst;
		for (int y = ky2; y < rows - ky2; y++) {
			uchar* pdst = temp.data.ptr[y];
			for (int x = 0; x < cols; x++) {
				int nmin = 255;
				for (int iy = y - ky2; iy <= y + ky2; iy++) {
					if (src.data.ptr[iy][x] < nmin) {
						nmin = 0;
						break;
					}
				}
				pdst[x] = (uchar)nmin;
			}
		}
		for (int x = kx2; x < cols - kx2; x++) {
			for (int y = 0; y < rows; y++) {
				const uchar* psrc = temp.data.ptr[y];
				int nmin = 255;
				for (int ix = x - kx2; ix <= x + kx2; ix++) {
					if (psrc[ix] < nmin) {
						nmin = 0;
						break;
					}
				}
				dst.data.ptr[y][x] = (uchar)nmin;
			}
		}
	}
// 	else {
// 		int rows=src.rows();
// 		int cols=src.cols();
// 		dst.create (rows, cols, MAT_Tbyte);
// 		int kx2 = nkx/2;
// 		int ky2 = nky/2;
// 		uchar** ppsrc=src.data.ptr;
// 		uchar** ppdst=dst.data.ptr;
// 		for (int y=0; y<rows; y++)
// 		{
// 			int starty=MAX(0,y-ky2);
// 			int endy=MIN(rows-1,y+ky2);
// 			uchar* pdst=ppdst[y];
// 			for (int x=0; x<cols; x++)
// 			{
// 				int startx=MAX(0,x-kx2);
// 				int endx=MIN(cols-1,x+kx2);
// 				int nmin=255;
// 				for (int iy=starty; iy<=endy; iy++) for (int ix=startx; ix<=endx; ix++)
// 				{
// 					if (ppsrc[iy][ix] < nmin)
// 						nmin=ppsrc[iy][ix];
// 				}
// 				pdst[x]=nmin;
// 			}
// 		}
// 	}
}
CVLIB_DECLSPEC void erode(Mat& src, int nkx, int nky)
{
	Mat temp=src;
	erode (temp, src, nkx, nky);
}

CVLIB_DECLSPEC void dilate(const Mat& src, Mat& dst, int nkx, int nky)
{
//	if (nkx == 3 && nky == 3)
	{
		int rows = src.rows();
		int cols = src.cols();
		int kx2 = nkx / 2;
		int ky2 = nky / 2;
		Mat temp;
		dst.create(rows, cols, MAT_Tbyte);
		dst = 0;//src;
		temp = dst;
		for (int y = ky2; y < rows - ky2; y++) {
			uchar* pdst = temp.data.ptr[y];
			for (int x = 0; x < cols; x++) {
				int nmax = 0;
				for (int iy = y - ky2; iy <= y + ky2; iy++) {
					if (src.data.ptr[iy][x] > nmax) {
						nmax = 255;
						break;
					}
				}
				pdst[x] = (uchar)nmax;
			}
		}
		for (int x = kx2; x < cols - kx2; x++) {
			for (int y = 0; y < rows; y++) {
				const uchar* psrc = temp.data.ptr[y];
				int nmax = 0;
				for (int ix = x - kx2; ix <= x + kx2; ix++) {
					if (psrc[ix] > nmax) {
						nmax = 255;
						break;
					}
				}
				dst.data.ptr[y][x] = (uchar)nmax;
			}
		}
	}
// 	else {
// 		int rows=src.rows();
// 		int cols=src.cols();
// 		dst.create (rows, cols, MAT_Tbyte);
// 		int kx2 = nkx/2;
// 		int ky2 = nky/2;
// 		uchar** ppsrc=src.data.ptr;
// 		uchar** ppdst=dst.data.ptr;
// 		for (int y=0; y<rows; y++)
// 		{
// 			int starty=MAX(0,y-ky2);
// 			int endy=MIN(rows-1,y+ky2);
// 			uchar* pdst=ppdst[y];
// 			for (int x=0; x<cols; x++)
// 			{
// 				int startx=MAX(0,x-kx2);
// 				int endx=MIN(cols-1,x+kx2);
// 				int nmax=0;
// 				for (int iy=starty; iy<=endy; iy++) for (int ix=startx; ix<=endx; ix++)
// 				{
// 					if (ppsrc[iy][ix] > nmax)
// 						nmax=ppsrc[iy][ix];
// 				}
// 				pdst[x]=nmax;
// 			}
// 		}
// 	}
}
CVLIB_DECLSPEC void dilate(Mat& src, int nkx, int nky)
{
	Mat temp=src;
	dilate (temp, src, nkx, nky);
}
CVLIB_DECLSPEC void opening(const Mat& src, Mat& dst, int nkx, int nky)
{
	Mat temp;
	erode (src, temp, nkx, nky);
	dilate (temp, dst, nkx, nky);
}
CVLIB_DECLSPEC void opening(Mat& src, int nkx, int nky)
{
	Mat temp=src;
	opening (temp, src, nkx, nky);
}
CVLIB_DECLSPEC void closing(const Mat& src, Mat& dst, int nkx, int nky)
{
	Mat temp;
	dilate (src, temp, nkx, nky);
	erode (temp, dst, nkx, nky);
}
CVLIB_DECLSPEC void closing(Mat& src, int nkx, int nky)
{
	Mat temp=src;
	closing (temp, src, nkx, nky);
}

}
}
