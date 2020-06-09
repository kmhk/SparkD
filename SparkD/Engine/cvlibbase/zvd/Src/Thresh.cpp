
#include "thresh.h"
#include "cvlibutil.h"
#include "integralImage.h"

namespace cvlib{ namespace ip{

static double ThresholdTile( const Mat& src, Mat& dst, double percent, double maxval, bool fInverse)
{
	int i;
	dst=src;
	dst.convert(MAT_Tbyte);
	Veci vhist(256);vhist=0;
	int len=dst.rows()*dst.cols();
	for (i=0; i<len; i++)
	{
		vhist[dst.data.ptr[0][i]]++;
	}
	float rThreshsum=(float)(percent*len/100.0f);
	int sum=0;
	for (i=0; i<256; i++)
	{
		sum += vhist[i];
		if (sum > rThreshsum)
			break;
	}
	int thresh=i;
	if (fInverse==false)
	{
		for (i=0; i<len; i++)
		{
			if (dst.data.ptr[0][i] < thresh)
				dst.data.ptr[0][i]=0;
			else
				dst.data.ptr[0][i]=(uchar)maxval;
		}
	}
	else
	{
		for (i=0; i<len; i++)
		{
			if (dst.data.ptr[0][i] < thresh)
				dst.data.ptr[0][i]=(uchar)maxval;
			else
				dst.data.ptr[0][i]=0;
		}
	}
	return (double)thresh;
}

static void thresh_8u( const Mat& _src, Mat& _dst, uchar thresh, uchar maxval, int type )
{
    int i, j, j_scalar = 0;
    uchar tab[256];
    Size roi = _src.size();

    if( _src.isContinuous() && _dst.isContinuous() )
    {
        roi.width *= roi.height;
        roi.height = 1;
    }

    switch( type )
    {
    case THRESH_BINARY:
        for( i = 0; i <= thresh; i++ )
            tab[i] = 0;
        for( ; i < 256; i++ )
            tab[i] = maxval;
        break;
    case THRESH_BINARY_INV:
        for( i = 0; i <= thresh; i++ )
            tab[i] = maxval;
        for( ; i < 256; i++ )
            tab[i] = 0;
        break;
    case THRESH_TRUNC:
        for( i = 0; i <= thresh; i++ )
            tab[i] = (uchar)i;
        for( ; i < 256; i++ )
            tab[i] = thresh;
        break;
    case THRESH_TOZERO:
        for( i = 0; i <= thresh; i++ )
            tab[i] = 0;
        for( ; i < 256; i++ )
            tab[i] = (uchar)i;
        break;
    case THRESH_TOZERO_INV:
        for( i = 0; i <= thresh; i++ )
            tab[i] = (uchar)i;
        for( ; i < 256; i++ )
            tab[i] = 0;
        break;
    default:
        assert(false);
    }

    if( j_scalar < roi.width )
    {
        for( i = 0; i < roi.height; i++ )
        {
            const uchar* src = (const uchar*)_src.data.ptr[i];
            uchar* dst = (uchar*)_dst.data.ptr[i];
			j = j_scalar;
// #if CVLIB_ENABLE_UNROLLED
            for( ; j <= roi.width - 4; j += 4 )
            {
                uchar t0 = tab[src[j]];
                uchar t1 = tab[src[j+1]];

                dst[j] = t0;
                dst[j+1] = t1;

                t0 = tab[src[j+2]];
                t1 = tab[src[j+3]];

                dst[j+2] = t0;
                dst[j+3] = t1;
            }
// #endif
            for( ; j < roi.width; j++ )
                dst[j] = tab[src[j]];
        }
    }
}


static void thresh_16s( const Mat& _src, Mat& _dst, short thresh, short maxval, int type )
{
    int i, j;
    Size roi = _src.size();
    
    if( _src.isContinuous() && _dst.isContinuous() )
    {
        roi.width *= roi.height;
        roi.height = 1;
    }

    switch( type )
    {
    case THRESH_BINARY:
        for( i = 0; i < roi.height; i++)
        {
			const short* src = (const short*)_src.data.ptr[i];
			short* dst = (short*)_dst.data.ptr[i];
            j = 0;
            for( ; j < roi.width; j++ )
                dst[j] = src[j] > thresh ? maxval : 0;
        }
        break;

    case THRESH_BINARY_INV:
        for( i = 0; i < roi.height; i++)
        {
			const short* src = (const short*)_src.data.ptr[i];
			short* dst = (short*)_dst.data.ptr[i];
            j = 0;
            for( ; j < roi.width; j++ )
                dst[j] = src[j] <= thresh ? maxval : 0;
        }
        break;

    case THRESH_TRUNC:
        for( i = 0; i < roi.height; i++)
        {
			const short* src = (const short*)_src.data.ptr[i];
			short* dst = (short*)_dst.data.ptr[i];
            j = 0;
            for( ; j < roi.width; j++ )
                dst[j] = MIN(src[j], thresh);
        }
        break;

    case THRESH_TOZERO:
        for( i = 0; i < roi.height; i++)
        {
			const short* src = (const short*)_src.data.ptr[i];
			short* dst = (short*)_dst.data.ptr[i];
            j = 0;
            for( ; j < roi.width; j++ )
            {
                short v = src[j];
                dst[j] = v > thresh ? v : 0;
            }
        }
        break;

    case THRESH_TOZERO_INV:
        for( i = 0; i < roi.height; i++)
        {
			const short* src = (const short*)_src.data.ptr[i];
			short* dst = (short*)_dst.data.ptr[i];
            j = 0;
            for( ; j < roi.width; j++ )
            {
                short v = src[j];
                dst[j] = v <= thresh ? v : 0;
            }
        }
        break;
    default:
        return;
    }
}

    
static void thresh_32f( const Mat& _src, Mat& _dst, float thresh, float maxval, int type )
{
    int i, j;
    Size roi = _src.size();

	if( _src.isContinuous() && _dst.isContinuous() )
    {
        roi.width *= roi.height;
        roi.height = 1;
    }

	switch( type )
    {
        case THRESH_BINARY:
            for( i = 0; i < roi.height; i++)
            {
				const float* src = (const float*)_src.data.ptr[i];
				float* dst = (float*)_dst.data.ptr[i];
                j = 0;
                for( ; j < roi.width; j++ )
                    dst[j] = src[j] > thresh ? maxval : 0;
            }
            break;
            
        case THRESH_BINARY_INV:
            for( i = 0; i < roi.height; i++)
            {
				const float* src = (const float*)_src.data.ptr[i];
				float* dst = (float*)_dst.data.ptr[i];
               j = 0;
                for( ; j < roi.width; j++ )
                    dst[j] = src[j] <= thresh ? maxval : 0;
            }
            break;
            
        case THRESH_TRUNC:
            for( i = 0; i < roi.height; i++)
            {
				const float* src = (const float*)_src.data.ptr[i];
				float* dst = (float*)_dst.data.ptr[i];
                j = 0;
                for( ; j < roi.width; j++ )
                    dst[j] = MIN(src[j], thresh);
            }
            break;
            
        case THRESH_TOZERO:
            for( i = 0; i < roi.height; i++ )
            {
				const float* src = (const float*)_src.data.ptr[i];
				float* dst = (float*)_dst.data.ptr[i];
                j = 0;
                for( ; j < roi.width; j++ )
                {
                    float v = src[j];
                    dst[j] = v > thresh ? v : 0;
                }
            }
            break;
            
        case THRESH_TOZERO_INV:
            for( i = 0; i < roi.height; i++ )
            {
				const float* src = (const float*)_src.data.ptr[i];
				float* dst = (float*)_dst.data.ptr[i];
               j = 0;
                for( ; j < roi.width; j++ )
                {
                    float v = src[j];
                    dst[j] = v <= thresh ? v : 0;
                }
            }
            break;
        default:
            return;
    }
}
    
static double
getThreshVal_Otsu_8u( const Mat& _src )
{
    Size size = _src.size();
    if( _src.isContinuous() )
    {
        size.width *= size.height;
        size.height = 1;
    }
    const int N = 256;
    int i, j, h[N] = {0};
    for( i = 0; i < size.height; i++ )
    {
        const uchar* src=_src.data.ptr[i];
		j = 0;
// 		#if CVLIB_ENABLE_UNROLLED
        for( ; j <= size.width - 4; j += 4 )
        {
            int v0 = src[j], v1 = src[j+1];
            h[v0]++; h[v1]++;
            v0 = src[j+2]; v1 = src[j+3];
            h[v0]++; h[v1]++;
        }
//         #endif
        for( ; j < size.width; j++ )
            h[src[j]]++;
    }

    double mu = 0, scale = 1./(size.width*size.height);
    for( i = 0; i < N; i++ )
        mu += i*(double)h[i];
    
    mu *= scale;
    double mu1 = 0, q1 = 0;
    double max_sigma = 0, max_val = 0;

    for( i = 0; i < N; i++ )
    {
        double p_i, q2, mu2, sigma;

        p_i = h[i]*scale;
        mu1 *= q1;
        q1 += p_i;
        q2 = 1. - q1;

        if( MIN(q1,q2) < FLT_EPSILON || MAX(q1,q2) > 1. - FLT_EPSILON )
            continue;

        mu1 = (mu1 + i*p_i)/q1;
        mu2 = (mu - q1*mu1)/q2;
        sigma = q1*q2*(mu1 - mu2)*(mu1 - mu2);
        if( sigma > max_sigma )
        {
            max_sigma = sigma;
            max_val = i;
        }
    }

    return max_val;
}

// class ThresholdRunner
// {
// public:
//     ThresholdRunner(Mat& _src, Mat& _dst, int _nStripes, double _thresh, double _maxval, int _thresholdType)
//     {
//         src = _src;
//         dst = _dst;
// 
//         nStripes = _nStripes;
// 
//         thresh = _thresh;
//         maxval = _maxval;
//         thresholdType = _thresholdType;
//     }
// 
//     void operator () ( const BlockedRange& range ) const
//     {
//         int row0 = MIN(cvutil::round(range.begin() * src.rows / nStripes), src.rows);
//         int row1 = MIN(cvutil::round(range.end() * src.rows / nStripes), src.rows);
// 
//         /*if(0)
//             printf("Size = (%d, %d), range[%d,%d), row0 = %d, row1 = %d\n",
//                    src.rows, src.cols, range.begin(), range.end(), row0, row1);*/
// 
//         Mat srcStripe = src.rowRange(row0, row1);
//         Mat dstStripe = dst.rowRange(row0, row1);
// 
//         if (srcStripe.type() == MAT_Tbyte)
//         {
//             thresh_8u( srcStripe, dstStripe, (uchar)thresh, (uchar)maxval, thresholdType );
//         }
//         else if( srcStripe.type() == MAT_Tshort )
//         {
//             thresh_16s( srcStripe, dstStripe, (short)thresh, (short)maxval, thresholdType );
//         }
//         else if( srcStripe.type() == MAT_Tfloat )
//         {
//             thresh_32f( srcStripe, dstStripe, (float)thresh, (float)maxval, thresholdType );
//         }
//     }
// 
// private:
//     Mat src;
//     Mat dst;
//     int nStripes;
// 
//     double thresh;
//     double maxval;
//     int thresholdType;
// };
    
double Threshold( const Mat& src, Mat& dst, double thresh, double maxval, int type )
{
	if (!src.isValid())
		return 0;
	if (type==THRESH_TILE || type==THRESH_TILE_INV)
	{
		if (type==THRESH_TILE)
			return ThresholdTile (src, dst, thresh, maxval, false);
		else
			return ThresholdTile (src, dst, thresh, maxval, true);
	}
    bool use_otsu = (type & THRESH_OTSU) != 0;
    type &= THRESH_MASK;

    if( use_otsu )
    {
        assert( src.type() == MAT_Tbyte );
        thresh = getThreshVal_Otsu_8u(src);
    }
  
	if (!src.isEqualMat(dst))
	{
		dst.release();
		dst.create( src.size(), src.type() );
	}


    if( src.type() == MAT_Tbyte )
    {
        int ithresh = (int)floor(thresh);
        thresh = ithresh;
        int imaxval = cvutil::round(maxval);
        if( type == THRESH_TRUNC )
            imaxval = ithresh;
        imaxval = (uchar)(imaxval);

        if( ithresh < 0 || ithresh >= 255 )
        {
            if( type == THRESH_BINARY || type == THRESH_BINARY_INV ||
                ((type == THRESH_TRUNC || type == THRESH_TOZERO_INV) && ithresh < 0) ||
                (type == THRESH_TOZERO && ithresh >= 255) )
            {
                int v = type == THRESH_BINARY ? (ithresh >= 255 ? 0 : imaxval) :
                        type == THRESH_BINARY_INV ? (ithresh >= 255 ? imaxval : 0) :
                        /*type == THRESH_TRUNC ? imaxval :*/ 0;
                dst.setValue(v);
            }
            else
                src.copyTo(dst, Mat());
        }
        else
        {
            thresh_8u( src, dst, (uchar)ithresh, (uchar)imaxval, type );
//             parallel_for(BlockedRange(0, nStripes),
//                          ThresholdRunner(src, dst, nStripes, (uchar)ithresh, (uchar)imaxval, type));
        }
    }
    else if( src.type() == MAT_Tshort )
    {
        int ithresh = (int)floor(thresh);
        thresh = ithresh;
        int imaxval = cvutil::round(maxval);
        if( type == THRESH_TRUNC )
            imaxval = ithresh;
        imaxval = (short)(imaxval);
        
        if( ithresh < SHRT_MIN || ithresh >= SHRT_MAX )
        {
            if( type == THRESH_BINARY || type == THRESH_BINARY_INV ||
               ((type == THRESH_TRUNC || type == THRESH_TOZERO_INV) && ithresh < SHRT_MIN) ||
               (type == THRESH_TOZERO && ithresh >= SHRT_MAX) )
            {
                int v = type == THRESH_BINARY ? (ithresh >= SHRT_MAX ? 0 : imaxval) :
                type == THRESH_BINARY_INV ? (ithresh >= SHRT_MAX ? imaxval : 0) :
                /*type == THRESH_TRUNC ? imaxval :*/ 0;
                dst.setValue(v);
            }
            else
                src.copyTo(dst, Mat());
        }
        else
        {
            thresh_16s( src, dst, (short)ithresh, (short)imaxval, type );
//             parallel_for(BlockedRange(0, nStripes),
//                          ThresholdRunner(src, dst, nStripes, (short)ithresh, (short)imaxval, type));
        }
    }
    else if( src.type() == MAT_Tfloat )
    {
		thresh_32f( src, dst, (float)thresh, (float)maxval, type );
//         parallel_for(BlockedRange(0, nStripes),
//                      ThresholdRunner(src, dst, nStripes, (float)thresh, (float)maxval, type));
    }
    else
        assert(false);

    return thresh;
}

static void GrayHistogram(const Mat* pxImg, int* & pnHist, const Mat& mask)
{
	assert(pxImg->isValid());

	int nW = pxImg->cols();
	int nH = pxImg->rows();

	pnHist = NULL;
	pnHist = (int*)calloc(256, sizeof(int));
	if (pnHist == NULL)
		return;

	if (!mask.isValid())
	{
		for (int iH = 0; iH < nH; iH++)
		{
			uchar* pbLine = pxImg->data.ptr[iH];
			for (int iW = 0; iW < nW; iW++)
				pnHist[*pbLine++]++;
		}
	}
	else
	{
		for (int iH = 0; iH < nH; iH++)
		{
			uchar* pbLine = pxImg->data.ptr[iH];
			uchar* pbMask = mask.data.ptr[iH];
			for (int iW = 0; iW < nW; iW++)
			{
				if (pbMask[iW])
					pnHist[pbLine[iW]]++;
			}
		}
	}
}

uchar otsuThreshold(const Mat& img, const Mat& mask)
{
	int* pnHist = NULL;
	float rM, rM1, rM2;
	float rP1, rP2;
	float rScale;
	float rpi, rVal;

	int i;
	GrayHistogram(&img, pnHist, mask);

	int ntotal=0;
	rM = 0.f;
	for(i = 0; i < 256; i++)
	{
		rM += i * pnHist[i];
		ntotal += pnHist[i];
	}
	rScale = 1.0f/ntotal;
	rM *= rScale;

	float rMax = 0;
	uchar bMaxIdx = 0;
	rM1 = 0;
	rP1 = 0;
	for( i = 0; i < 256; i++ )
	{
		rpi = pnHist[i] * rScale;
		rM1 *= rP1;
		rP1 += rpi;
		rP2 = 1.f - rP1;

		if (MIN(rP1,rP2) < FLT_EPSILON || MAX(rP1,rP2) > 1. - FLT_EPSILON )
			continue;

		rM1 = (rM1 + i * rpi) / rP1;
		rM2 = (rM - rP1 * rM1) / rP2;
		rVal = rP1 * rP2 * (rM1 - rM2) * (rM1 - rM2);
		if( rVal > rMax )
		{
			rMax = rVal;
			bMaxIdx = (uchar)i;
		}
	}

	free(pnHist);
	return (uchar)(bMaxIdx);
}

int thresholdOfGray(const Mat& gray)
{
	uchar* pBits;
	int nHeight, nWidth;
	int i, h, w;
	pBits=gray.data.ptr[0];
	nHeight=gray.rows();
	nWidth=gray.cols();

	int nTotalPixel = nHeight * nWidth;
	int nFreq[256];
	for (i = 0; i < 256; i++)
		nFreq[i] = 0;
	i=0;
	for (h = 0; h < nHeight; h++)
	{
		for (w = 0; w < nWidth; w++, i++)
		{
			nFreq[pBits[i]]++;
		}
	}
	double dFreq[256];
	for (i = 0; i < 256; i++)
		dFreq[i] = (double)nFreq[i] / (double)nTotalPixel;

	int nThreshold = 0;
	int nThresholdByBest = 0;

	double dMt = 0;
	for (i = 0; i < 256; i++)
	{
		dMt += i * dFreq[i];
	}
	double dWk = 0, dMk = 0;
	double dNumerator, dDenominator, dMax = 0;
	for (i = 0; i < 256; i++)
	{
		dWk += dFreq[i];
		dMk += i * dFreq[i];
		dNumerator = (dMt * dWk - dMk) * (dMt * dWk - dMk);
		dDenominator = dWk * (1 - dWk);
		if (dDenominator == 0)
			continue;
		if (dNumerator / dDenominator > dMax)
		{
			dMax = dNumerator / dDenominator;
		}
	}

	dMax = 0;
	bool bFirst = true;
	double dW0 = 0, dM0 = 0, dM1;
	double dDw;
	for (i = 0; i < 256; i++)
	{
		dM0 *= dW0; 
		dW0 += dFreq[i];
		if (dW0 == 0)
			continue;
		if (dW0 == 1)
			break;
		dM0 = (dM0 + i * dFreq[i]) / dW0;
		dM1 = (dMt - dW0 * dM0) / (1 - dW0);
		dDw = 0;
		for (int j = 0; j < 256; j++)
		{
			if (j <= i)
				dDw += (j - dM0) * (j - dM0) * dFreq[j];
			else
				dDw += (j - dM1) * (j - dM1) * dFreq[j];
		}
		if (bFirst)
		{
			dMax = dW0 * log(dW0) + (1 - dW0) * log(1 - dW0) - log(dDw) / 2;
			nThresholdByBest = i;
			bFirst = false;
		}
		if (dW0 * log(dW0) + (1 - dW0) * log(1 - dW0) - log(dDw) / 2 > dMax)
		{
			dMax = dW0 * log(dW0) + (1 - dW0) * log(1 - dW0) - log(dDw) / 2;
			nThresholdByBest = i;
		}
	}
	return nThreshold = nThresholdByBest;
}

void meanFilter(const Mat& src, Mat& dst, int ddepth, Size ksize)
{
	Mat  sum;
	ip::integralImage(src, sum);
	int kx = ksize.width / 2;
	int ky = ksize.height / 2;

	int area = (kx * 2 + 1)*(ky * 2 + 1);
	if (src.type() == MAT_Tbyte || src.type() == MAT_Tint)
	{
		dst.create(src.size(), MAT_Tint);
		for (int y = ky; y < src.rows() - ky; y++)
		{
			for (int x = kx; x < src.cols() - kx; x++)
			{
				dst.data.i[y][x] = (sum.data.i[y + ky + 1][x + kx + 1] + sum.data.i[y - ky][x - kx] - sum.data.i[y + ky + 1][x - kx] - sum.data.i[y - ky][x + kx + 1]) / area;
			}
		}
	}
	else
	{
		dst.create(src.size(), MAT_Tdouble);
		double r = 1. / (double)area;
		for (int y = ky; y < src.rows() - ky; y++)
		{
			for (int x = kx; x < src.cols() - kx; x++)
			{
				dst.data.db[y][x] = (sum.data.db[y + ky + 1][x + kx + 1] + sum.data.db[y - ky][x - kx] - sum.data.db[y + ky + 1][x - kx] - sum.data.db[y - ky][x + kx + 1]) * r;
			}
		}
	}
	dst.convert((TYPE)ddepth, CT_Cast);
}
void adaptiveThreshold(const Mat& src, Mat& dst, double maxValue, int method, int type, int blockSize, double delta)
{
	assert(src.type1() == MAT_Tuchar);
	assert(blockSize % 2 == 1 && blockSize > 1);
	Size size = src.size();
	dst.create(size, src.type1());
	if (maxValue < 0)
	{
		dst.zero();
		return;
	}
	Mat mean;
	if (method == ADAPTIVE_THRESH_MEAN_C)
	{
		meanFilter(src, mean, MAT_Tuchar, Size(blockSize, blockSize));
	}
	else if (method == ADAPTIVE_THRESH_GAUSSIAN_C)
	{

	}
	else
	{
		assert(false);
		return;
	}
	int i, j;
	uchar imaxval = saturate_cast<uchar>(maxValue);
	int idelta = type == THRESH_BINARY ? cvutil::ceil(delta) : cvutil::floor(delta);
	uchar tab[768];

	if (type == THRESH_BINARY)
		for (i = 0; i < 768; i++)
			tab[i] = (uchar)(i - 255 > -idelta ? imaxval : 0);
	else if (type == THRESH_BINARY_INV)
		for (i = 0; i < 768; i++)
			tab[i] = (uchar)(i - 255 <= -idelta ? imaxval : 0);
	else
		assert(false);

	if (src.isContinuous() && mean.isContinuous() && dst.isContinuous())
	{
		size.width *= size.height;
		size.height = 1;
	}

	for (i = 0; i < size.height; i++)
	{
		const uchar* sdata = src.data.ptr[i];
		const uchar* mdata = mean.data.ptr[i];
		uchar* ddata = dst.data.ptr[i];

		for (j = 0; j < size.width; j++)
			ddata[j] = tab[sdata[j] - mdata[j] + 255];
	}

}

}}
