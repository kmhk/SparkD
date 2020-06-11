/*!
 * \file	ImgProc.cpp
 * \ingroup VOR
 * \brief	
 * \author	
 */

#include "ImgProc.h"
#include "cvlibbase/Inc/_cvlibbase.h"
#include "ImgFilter.h"

namespace cvlib { namespace ip{

#define SHIFT             16
#define SCALE(rX)          (rX)
//#define SCALE(rX)          cvutil::round((rX)*(1 << SHIFT))
#define NOP(ppnA)      (ppnA)

/* lightweight convolution with 3x3 kernel */
void sepConvSmall( float*  prSrc, int nSrc_step,
						  float*  prDst, int nDst_step,
						  Size src_size,
						  const float* prKx, const float* prKy,
						  float*  prBuffer )
{
    int  nDst_width, nBuffer_step = 0;
    int  nX, nY;
	
    assert( prSrc && prDst && src_size.width > 2 && src_size.height > 2 &&
		(nSrc_step & 3) == 0 && (nDst_step & 3) == 0 &&
		(prKx || prKy) && (prBuffer || !prKx || !prKy));
	
    nSrc_step >>= 2;
    nDst_step >>= 2;
	
    nDst_width = src_size.width - 2;
	
    if( !prKx )
    {
	/* set vars, so that vertical convolution
	will write results into destination ROI and
		horizontal convolution won't run */
        src_size.width = nDst_width;
        nBuffer_step = nDst_step;
        prBuffer = prDst;
        nDst_width = 0;
    }
	
    assert( nSrc_step >= src_size.width && nDst_step >= nDst_width );
	
    src_size.height -= 3;
    if( !prKy )
    {
	/* set vars, so that vertical convolution won't run and
		horizontal convolution will write results into destination ROI and */
        src_size.height += 3;
        nBuffer_step = nSrc_step;
        prBuffer = prSrc;
        src_size.width = 0;
    }
	
    for( nY = 0; nY <= src_size.height; nY++, prSrc += nSrc_step,
		prDst += nDst_step,
		prBuffer += nBuffer_step )
    {
        float* src2 = prSrc + nSrc_step;
        float* src3 = prSrc + nSrc_step*2;
        for( nX = 0; nX < src_size.width; nX++ )
        {
            prBuffer[nX] = (float)(prKy[0]*prSrc[nX] + prKy[1]*src2[nX] + prKy[2]*src3[nX]);
        }
		
        for( nX = 0; nX < nDst_width; nX++ )
        {
            prDst[nX] = (float)(prKx[0]*prBuffer[nX] + prKx[1]*prBuffer[nX+1] + prKx[2]*prBuffer[nX+2]);
        }
    }
}

void* adjustRect( const void* pSrcptr, int nSrc_step, int nPix_size,
		Size src_size, Size win_size, Point2i ip, Rect* pRect )
{
	Rect rect;
	const char* pchSrc = (const char*)pSrcptr;

	if( ip.x >= 0 )
	{
		pchSrc += ip.x*nPix_size;
		rect.x = 0;
	}
	else
	{
		rect.x = -ip.x;
		if( rect.x > win_size.width )
			rect.x = win_size.width;
	}

	if( ip.x + win_size.width < src_size.width )
		rect.width = win_size.width;
	else
	{
		rect.width = src_size.width - ip.x - 1;
		if( rect.width < 0 )
		{
			pchSrc += rect.width*nPix_size;
			rect.width = 0;
		}
		assert( rect.width <= win_size.width );
	}

	if( ip.y >= 0 )
	{
		pchSrc += ip.y * nSrc_step;
		rect.y = 0;
	}
	else
		rect.y = -ip.y;

	if( ip.y + win_size.height < src_size.height )
		rect.height = win_size.height;
	else
	{
		rect.height = src_size.height - ip.y - 1;
		if( rect.height < 0 )
		{
			pchSrc += rect.height*nSrc_step;
			rect.height = 0;
		}
	}

	*pRect = rect;

	return (void*)(pchSrc - rect.x*nPix_size);
};


bool getRectSubPixel(const uchar* pbSrc, int nSrc_step, Size src_size, float* prDst,
										 int nDst_step, Size win_size, Point2f center)
{                                                                           
    Point2i ip;                                                             
    float  rA11, rA12, rA21, rA22, rB1, rB2;                                   
    float rA, rB;                                                             
    int i, j;                                                               
                                                                            
    center.x -= (win_size.width - 1) * 0.5f;                                    
    center.y -= (win_size.height - 1) * 0.5f;                                   
                                                                            
    ip.x = cvutil::floor( (float)center.x );                                             
    ip.y = cvutil::floor( (float)center.y );
                                                                            
    rA = (float)(center.x - ip.x);                                                    
    rB = (float)(center.y - ip.y);                                                    
    rA11 = (float)SCALE((1.f-rA)*(1.f-rB));                                     
    rA12 = (float)SCALE(rA*(1.f-rB));                                           
    rA21 = (float)SCALE((1.f-rA)*rB);                                           
    rA22 = (float)SCALE(rA*rB);                                                 
    rB1 = (float)SCALE(1.f - rB);                                               
    rB2 = (float)SCALE(rB);                                                    
                                                                            
    nSrc_step /= sizeof( pbSrc[0] );                                           
                                                                            
    if( 0 <= ip.x && ip.x + win_size.width < src_size.width &&              
        0 <= ip.y && ip.y + win_size.height < src_size.height )             
    {                                                                       
        /* extracted rectangle is totally inside the image */               
        pbSrc += ip.y * nSrc_step + ip.x;                                      
                                                              
        for( i = 0; i < win_size.height; i++, pbSrc += nSrc_step,              
                                              (char*&)prDst += nDst_step )     
        {                                                                   
            for( j = 0; j <= win_size.width - 2; j += 2 )                   
            {                                                               
                float s0 = NOP(pbSrc[j])*rA11 +                      
                              NOP(pbSrc[j+1])*rA12 +                    
                              NOP(pbSrc[j+nSrc_step])*rA21 +             
                              NOP(pbSrc[j+nSrc_step+1])*rA22;            
                float s1 = NOP(pbSrc[j+1])*rA11 +                    
                              NOP(pbSrc[j+2])*rA12 +                    
                              NOP(pbSrc[j+nSrc_step+1])*rA21 +           
                              NOP(pbSrc[j+nSrc_step+2])*rA22;            
                                                                            
                prDst[j] = (float)NOP(s0);                          
                prDst[j+1] = (float)NOP(s1);                        
            }                                                               
                                                                            
            for( ; j < win_size.width; j++ )                                
            {                                                               
                float s0 = NOP(pbSrc[j])*rA11 +                      
                              NOP(pbSrc[j+1])*rA12 +                    
                              NOP(pbSrc[j+nSrc_step])*rA21 +             
                              NOP(pbSrc[j+nSrc_step+1])*rA22;            
                                                                            
                prDst[j] = (float)NOP(s0);                          
            }                                                               
        }                                                                   
    }                                                                       
    else                                                                    
    {                                                                       
        Rect r;                                                           
                                                                            
        pbSrc = (const uchar*)adjustRect( pbSrc, nSrc_step*sizeof(*pbSrc),    
                               sizeof(*pbSrc), src_size, win_size,ip, &r);    
                                                                            
        for( i = 0; i < win_size.height; i++, (char*&)prDst += nDst_step )     
        {                                                                   
            const uchar *pbSrc2 = pbSrc + nSrc_step;                           
                                                                            
            if( i < r.y || i >= r.height )                                  
                pbSrc2 -= nSrc_step;                                           
                                                                            
            for( j = 0; j < r.x; j++ )                                      
            {                                                               
                float rS0 = NOP(pbSrc[r.x])*rB1 +                     
                              NOP(pbSrc2[r.x])*rB2;                     
                                                                            
                prDst[j] = (float)NOP(rS0);                          
            }                                                               
                                                                            
            for( ; j < r.width; j++ )                                       
            {                                                               
                float rS0 = NOP(pbSrc[j])*rA11 +                      
                              NOP(pbSrc[j+1])*rA12 +                    
                              NOP(pbSrc2[j])*rA21 +                     
                              NOP(pbSrc2[j+1])*rA22;                    
                                                                            
                prDst[j] = (float)NOP(rS0);                          
            }                                                               
                                                                            
            for( ; j < win_size.width; j++ )                                
            {                                                               
                float rS0 = NOP(pbSrc[r.width])*rB1 +                 
                              NOP(pbSrc2[r.width])*rB2;                 
                                                                            
                prDst[j] = (float)NOP(rS0);                          
            }                                                               
                                                                            
            if( i < r.height )                                              
                pbSrc = pbSrc2;                                                 
        }                                                                   
    }                                                                       
                                                                            
    return true;                                                           
}

void findCornerSubPix(const Mat& src, Point2f* corners,
	int count, Size win, Size zeroZone,
	TermCriteria criteria)
{
	AutoBuffer<float> buffer;

	const int MAX_ITERS2 = 100;
	const float drv[] = { -1.f, 0.f, 1.f };
	float *maskX;
	float *maskY;
	float *mask;
	float *src_buffer;
	float *gx_buffer;
	float *gy_buffer;
	int win_w = win.width * 2 + 1, win_h = win.height * 2 + 1;
	int win_rect_size = (win_w + 4) * (win_h + 4);
	double coeff;
	Size size, src_buf_size;
	int i, j, k, pt_i;
	int max_iters = 10;
	double eps = 0;

	if (src.type1() != MAT_Tuchar)
		assert(false);//"The source image must be 8-bit single-channel (CVLIB_8UC1)";

	if (!corners)
		assert(false);

	if (count < 0)
		assert(false);

	if (count == 0)
		return;

	if (win.width <= 0 || win.height <= 0)
		assert(false);

	size = src.size();

	if (size.width < win_w + 4 || size.height < win_h + 4)
		assert(false);

	/* initialize variables, controlling loop termination */
	switch (criteria.nType)
	{
	case TermCriteria::MAX_ITER:
		eps = 0.f;
		max_iters = criteria.nMaxIter;
		break;
	case TermCriteria::EPS:
		eps = criteria.rEpsilon;
		max_iters = MAX_ITERS2;
		break;
	case TermCriteria::MAX_ITER | TermCriteria::EPS:
		eps = criteria.rEpsilon;
		max_iters = criteria.nMaxIter;
		break;
	default:
		assert(0);
	}

	eps = MAX(eps, 0);
	eps *= eps;                 /* use square of error in comparsion operations. */

	max_iters = MAX(max_iters, 1);
	max_iters = MIN(max_iters, MAX_ITERS2);

	buffer.alloc(win_rect_size * 5 + win_w + win_h + 32);

	/* assign pointers */
	maskX = buffer;
	maskY = maskX + win_w + 4;
	mask = maskY + win_h + 4;
	src_buffer = mask + win_w * win_h;
	gx_buffer = src_buffer + win_rect_size;
	gy_buffer = gx_buffer + win_rect_size;

	coeff = 1. / (win.width * win.width);

	/* calculate mask */
	for (i = -win.width, k = 0; i <= win.width; i++, k++)
	{
		maskX[k] = (float)exp(-i * i * coeff);
	}

	if (win.width == win.height)
	{
		maskY = maskX;
	}
	else
	{
		coeff = 1. / (win.height * win.height);
		for (i = -win.height, k = 0; i <= win.height; i++, k++)
		{
			maskY[k] = (float)exp(-i * i * coeff);
		}
	}

	for (i = 0; i < win_h; i++)
	{
		for (j = 0; j < win_w; j++)
		{
			mask[i * win_w + j] = maskX[j] * maskY[i];
		}
	}


	/* make zero_zone */
	if (zeroZone.width >= 0 && zeroZone.height >= 0 &&
		zeroZone.width * 2 + 1 < win_w && zeroZone.height * 2 + 1 < win_h)
	{
		for (i = win.height - zeroZone.height; i <= win.height + zeroZone.height; i++)
		{
			for (j = win.width - zeroZone.width; j <= win.width + zeroZone.width; j++)
			{
				mask[i * win_w + j] = 0;
			}
		}
	}

	/* set sizes of image rectangles, used in convolutions */
	src_buf_size.width = win_w + 2;
	src_buf_size.height = win_h + 2;

	/* do optimization loop for all the points */
	for (pt_i = 0; pt_i < count; pt_i++)
	{
		Point2f cT = corners[pt_i], cI = cT;
		int iter = 0;
		double err;

		do
		{
			Point2f cI2;
			double a, b, c, bb1, bb2;

			getRectSubPixel((uchar*)src.data.ptr[0], src.cols(), size,
				src_buffer, (win_w + 2) * sizeof(src_buffer[0]),
				Size(win_w + 2, win_h + 2), cI);

			/* calc derivatives */
			sepConvSmall(src_buffer + src_buf_size.width, src_buf_size.width * sizeof(src_buffer[0]),
				gx_buffer, win_w * sizeof(gx_buffer[0]),
				src_buf_size, drv, NULL, NULL);
			sepConvSmall(src_buffer + 1, src_buf_size.width * sizeof(src_buffer[0]),
				gy_buffer, win_w * sizeof(gy_buffer[0]),
				src_buf_size, NULL, drv, NULL);

			a = b = c = bb1 = bb2 = 0;

			/* process gradient */
			for (i = 0, k = 0; i < win_h; i++)
			{
				double py = i - win.height;

				for (j = 0; j < win_w; j++, k++)
				{
					double m = mask[k];
					double tgx = gx_buffer[k];
					double tgy = gy_buffer[k];
					double gxx = tgx * tgx * m;
					double gxy = tgx * tgy * m;
					double gyy = tgy * tgy * m;
					double px = j - win.width;

					a += gxx;
					b += gxy;
					c += gyy;

					bb1 += gxx * px + gxy * py;
					bb2 += gxy * px + gyy * py;
				}
			}

			double det = a*c - b*b;
			if (fabs(det) > DBL_EPSILON*DBL_EPSILON)
			{
				// 2x2 matrix inversion
				double scale = 1.0 / det;
				cI2.x = (float)(cI.x + c*scale*bb1 - b*scale*bb2);
				cI2.y = (float)(cI.y - b*scale*bb1 + a*scale*bb2);
			}
			else
			{
				cI2 = cI;
			}

			err = (cI2.x - cI.x) * (cI2.x - cI.x) + (cI2.y - cI.y) * (cI2.y - cI.y);
			cI = cI2;
		} while (++iter < max_iters && err > eps);

		/* if new point is too far from initial, it means poor convergence.
		   leave initial point as the result */
		if (fabs(cI.x - cT.x) > win.width || fabs(cI.y - cT.y) > win.height)
		{
			cI = cT;
		}

		corners[pt_i] = cI;     /* store result */
	}
}

Rect getBoundRect(const Mat& src, int value)
{
	int rows = src.rows();
	int cols = src.cols();
	Rect region(cols - 1, rows - 1, 0, 0);
	for (int y = 0; y < rows; y++) {
		const uchar* pdata = src.data.ptr[y];
		for (int x = 0; x < cols; x++) {
			if (pdata[x] == value) {
				region.x = MIN(region.x, x);
				region.y = MIN(region.y, y);
				region.width = MAX(region.width, x);
				region.height = MAX(region.height, y);
			}
		}
	}
	region.width = MAX(0, region.width - region.x + 1);
	region.height = MAX(0, region.height - region.y + 1);
	return region;
}
Rect getBoundRect(const Vector<Point2i>& points)
{
	int count = points.getSize();
	if (count == 0)
		return Rect();
	Rect r(points[0].x, points[0].y, points[0].x, points[0].y);
	for (int i = 1; i < count; i++) {
		r.x = MIN(r.x, points[i].x);
		r.y = MIN(r.y, points[i].y);
		r.width = MAX(r.width, points[i].x);
		r.height = MAX(r.height, points[i].y);
	}
	r.width -= r.x;
	r.height -= r.y;
	return r;
}

}}