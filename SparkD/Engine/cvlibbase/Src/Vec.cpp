/*!
 * \file Vec.cpp
 * \ingroup base
 * \brief
 * \author
 */

#include "Vec.h"
#include "cvlibutil.h"
#include "MatOperation.h"
#include "XFileDisk.h"
#include <cmath>
#include <climits>
#include <cfloat>
#include <cstdlib>

#pragma warning (push)
#pragma warning (disable : 4702)

namespace cvlib 
{

#define ExternalStorage 0
#define SharedStorage 1

	Vec::Vec() :m_len(0), m_nMaxLen(0), m_step(0), m_depend(0), m_type(MAT_Tbyte)
	{
		data.ptr = NULL;
	}
Vec::Vec(int m_len, TYPE type)
{
	data.ptr=0;
	create (m_len, type);
}

Vec::Vec(void* ptr, int m_len, TYPE type)
{
	data.ptr=0;
	create (ptr, m_len, type);
}

Vec::Vec (const Vec& vec)
{
	data.ptr=0;
	if (this == &vec)
		return;
	create (vec, true);
}
Vec::Vec(const Mat& image, bool fcopydata) 
{
	if (fcopydata==true)
		*this = Vec(image.data.ptr[0], image.total()*image.channels(), image.type());
	else
		create(image.data.ptr[0], image.total()*image.channels(), image.type());
}

Vec::~Vec()
{
	release();
}

int Vec::create(int _len, TYPE _type)
{
	if (data.ptr && this->m_type == _type && m_depend != SharedStorage && this->m_nMaxLen >= _len)
	{
		this->m_len = _len;
		m_depend = ExternalStorage;
		return 1;
	}

	release();

	this->m_len = _len;
	this->m_type = _type;
	this->m_nMaxLen = _len;
	m_step = CVLIB_ELEM_SIZE(type());
	data.ptr = new uchar[_len * m_step];
	m_depend = ExternalStorage;

	return 1;
}

int Vec::create(void* ptr, int _len, TYPE _type)
{
	release();
	this->m_len = _len;
	this->m_type = _type;
	this->m_nMaxLen = _len;
	m_step = CVLIB_ELEM_SIZE(type());
	m_depend = SharedStorage;
	data.ptr = (uchar*)ptr;
	return 1;
}

int	Vec::create(const Vec& vec, bool fCopy/* = false*/)
{
	int nRet = create(vec.m_len, vec.m_type);
	if (nRet)
	{
		if (fCopy)
			memcpy (data.ptr, vec.data.ptr, step() * length());
		return 1;
	}
	return 0;
}

void Vec::release ()
{
	if (data.ptr)
	{
		if (m_depend == ExternalStorage)
			delete [] data.ptr;
		data.ptr = 0;
		m_len=0;
	}
}

void Vec::resize(int num)
{
	if (m_nMaxLen >= num) m_len = num; 
	else if (m_depend == ExternalStorage)
	{
		if (isValid())
		{
			Vec v(num, type());
			memcpy (v.data.ptr, data.ptr, m_len * m_step);
			release();
			create (v, true);
		}
		else
			assert(false);
	}
}

Vec& Vec::operator=(const Vec& vecSrc)
{
	if (this == &vecSrc)
		return *this;
	if (length() == vecSrc.length() && type() == vecSrc.type())
	{
		memcpy(data.ptr, vecSrc.data.ptr, length() * step());
	}
	else
	{
		release();
		create(vecSrc, true);
	}
	return *this;
}

Vec& Vec::operator=(double value)
{
	int j;
	switch(type())
	{
	case MAT_Tbyte:
		for(j=0;j<length();j++)
			data.ptr[j] = (uchar)value;
		break;
	case MAT_Tshort:
		for(j=0;j<length();j++)
			data.s[j] = (short)value;
		break;
	case MAT_Tint:
		for(j=0;j<length();j++)
			data.i[j] = (int)value;
		break;
	case MAT_Tfloat:
		for(j=0;j<length();j++)
			data.fl[j] = (float)value;
		break;
	case MAT_Tdouble:
		for(j=0;j<length();j++)
			data.db[j] = (double)value;
		break;
        default:
            assert (false);
	}
	return *this;
}

void Vec::zero ()
{
	memset (data.ptr, 0, m_step * length());
}

void Vec::convertTo(Vec& dst, TYPE _type) const
{
	int i;
	int nLen = length();

	if (!dst.isValid())
		dst.create(nLen, _type);

	assert(dst.length() == nLen);

	if (_type == type())
	{
		memcpy(dst.data.ptr, data.ptr, nLen * step());
	}
	else
	{
		switch (dst.type())
		{
		case MAT_Tbyte:
			for (i = 0; i < nLen; i++)
				dst.data.ptr[i] = (uchar)value(i);
			break;
		case MAT_Tshort:
			for (i = 0; i < nLen; i++)
				dst.data.s[i] = (short)value(i);
			break;
		case MAT_Tint:
			for (i = 0; i < nLen; i++)
				dst.data.i[i] = (int)value(i);
			break;
		case MAT_Tfloat:
			for (i = 0; i < nLen; i++)
				dst.data.fl[i] = (float)value(i);
			break;
		case MAT_Tdouble:
			for (i = 0; i < nLen; i++)
				dst.data.db[i] = (double)value(i);
			break;
		default:
			assert(false);
		}
	}
}

double	Vec::value(int i) const
{
	switch (type())
	{
	case MAT_Tbyte:
		return data.ptr[i];
	case MAT_Tshort:
		return data.s[i];
	case MAT_Tint:
		return data.i[i];
	case MAT_Tfloat:
		return data.fl[i];
	case MAT_Tdouble:
		return data.db[i];
	default:
		assert(false);
	}
	return 0.0;
}

double Vec::norm1() const
{
	double rSum = 0;
	switch (type())
	{
	case MAT_Tbyte:
		{
			uchar *ptr_x = data.ptr;
			for (int i = 0; i < length(); i++, ptr_x++)
				rSum += ABS(*ptr_x);
		}
		break;
	case MAT_Tshort:
		{
			short *ptr_x = data.s;
			for (int i = 0; i < length(); i++, ptr_x++)
				rSum += ABS(*ptr_x);
		}
		break;
	case MAT_Tint:
		{
			int *ptr_x = data.i;
			for (int i = 0; i < length(); i++, ptr_x++)
				rSum += ABS(*ptr_x);
		}
		break;
	case MAT_Tfloat:
		{
			float *ptr_x = data.fl;
			for (int i = 0; i < length(); i++, ptr_x++)
				rSum += fabsf(*ptr_x);
		}
		break;
	case MAT_Tdouble:
		{
			double *ptr_x = data.db;
			for (int i = 0; i < length(); i++, ptr_x++)
				rSum += fabs(*ptr_x);
		}
		break;
        default:
            assert (false);
	}
	
	return rSum;
}

double Vec::norm2() const
{
	return std::sqrt(sq_norm());
}

double Vec::sq_norm() const
{
	double rSum = 0;
	switch (type())
	{
	case MAT_Tbyte:
		{
			uchar *ptr_x = data.ptr;
			for (int i = 0; i < length(); i++, ptr_x++)
				rSum += (*ptr_x) * (*ptr_x);
		}
		break;
	case MAT_Tshort:
		{
			short *ptr_x = data.s;
			for (int i = 0; i < length(); i++, ptr_x++)
				rSum += (*ptr_x) * (*ptr_x);
		}
		break;
	case MAT_Tint:
		{
			int *ptr_x = data.i;
			for (int i = 0; i < length(); i++, ptr_x++)
				rSum += (*ptr_x) * (*ptr_x);
		}
		break;
	case MAT_Tfloat:
		{
			float *ptr_x = data.fl;
			for (int i = 0; i < length(); i++, ptr_x++)
				rSum += (*ptr_x) * (*ptr_x);
		}
		break;
	case MAT_Tdouble:
		{
			double *ptr_x = data.db;
			for (int i = 0; i < length(); i++, ptr_x++)
				rSum += (*ptr_x) * (*ptr_x);
		}
		break;
        default:
            assert (false);
	}
	
	return rSum;
}

double Vec::normInf() const
{
	double rMax_val=0;
	switch (type())
	{
	case MAT_Tbyte:
		{
			uchar *ptr_x = data.ptr;
			rMax_val = (uchar)ABS(*ptr_x); ptr_x++;
			for (int i = 1; i < length(); i++, ptr_x++)
			{
				int rZ = (uchar)ABS(*ptr_x);
				if (rMax_val < rZ)
					rMax_val = rZ;
			}
		}
		break;
	case MAT_Tshort:
		{
			short *ptr_x = data.s;
			rMax_val = (short)ABS(*ptr_x); ptr_x++;
			for (int i = 1; i < length(); i++, ptr_x++)
			{
				int rZ = (short)ABS(*ptr_x);
				if (rMax_val < rZ)
					rMax_val = rZ;
			}
		}
		break;
	case MAT_Tint:
		{
			int *ptr_x = data.i;
			rMax_val = (int)ABS(*ptr_x); ptr_x++;
			for (int i = 1; i < length(); i++, ptr_x++)
			{
				int rZ = (int)ABS(*ptr_x);
				if (rMax_val < rZ)
					rMax_val = rZ;
			}
		}
		break;
	case MAT_Tfloat:
		{
			float *ptr_x = data.fl;
			rMax_val = (float)fabs(*ptr_x); ptr_x++;
			for (int i = 1; i < length(); i++, ptr_x++)
			{
				float rZ = (float)fabs(*ptr_x);
				if (rMax_val < rZ)
					rMax_val = rZ;
			}
		}
		break;
	case MAT_Tdouble:
		{
			double *ptr_x = data.db;
			rMax_val = (double)fabs(*ptr_x); ptr_x++;
			for (int i = 1; i < length(); i++, ptr_x++)
			{
				double rZ = (double)fabs(*ptr_x);
				if (rMax_val < rZ)
					rMax_val = rZ;
			}
		}
		break;
        default:
            assert (false);
	}

	return rMax_val;
}

Vec* Vec::subVec(int nFirst, int nLast, bool fShare /*= false*/) const
{
	Vec* pvRet = NULL;
	if (fShare)
	{
		pvRet = new Vec(data.ptr + step() * nFirst, nLast - nFirst + 1, type());
	}
	else
	{
		pvRet = new Vec(nLast - nFirst + 1, type());
		memcpy (pvRet->data.ptr, data.ptr + step() * nFirst, step() * (nLast - nFirst + 1));
	}
	return pvRet;
}

int Vec::fromFile (const char* szFilename)
{
	return Object::fromFile(szFilename);
}
int Vec::fromFile(XFile* pFile)
{
	int nLen, nType;
	if (pFile->read(&nLen, sizeof(int), 1) != 1)
		return 0;
	if (pFile->read(&nType, sizeof(int), 1) != 1)
		return 0;

	create(nLen, (TYPE)nType);

	if (pFile->read(data.ptr, nLen * step(), 1) != 1)
		return 0;

	return 1;
}

int Vec::toFile (const char* szFilename) const
{
	return Object::toFile(szFilename);
}
int Vec::toFile (XFile* pFile) const
{
	if (!isValid())
		return 0;
	int nLen, nType;
	nLen = length();
	pFile->write(&nLen, sizeof(int), 1);
	nType = type();
	pFile->write(&nType, sizeof(int), 1);
	pFile->write(data.ptr, nLen * step(), 1);
	return 1;
}

double Vec::unit()
{
	double rNorm2 = norm2();
	double rinv = 1. / rNorm2;
	switch (type())
	{
	case MAT_Tfloat:
		{
			float* ptr_x = data.fl;
			for (int i = 0; i < length(); i++)
				*ptr_x++ *= (float)rinv;
		}
		break;
	case MAT_Tdouble:
		{
			double* ptr_x = data.db;
			for (int i = 0; i < length(); i++)
				*ptr_x++ *= rinv;
		}
		break;
        default:
            assert (false);
	}
	return rNorm2;
}

/**
	@memo     Takes the absolute valueof each element.
	@doc      Takes the absolute valueof each element.
	@return   Nothing.
*/
void Vec::abs()
{
	int j;
	switch(type())
	{
	case MAT_Tbyte:
		for(j=0;j<length();j++)
			data.ptr[j] = (uchar)ABS(data.ptr[j]);
		break;
	case MAT_Tshort:
		for(j=0;j<length();j++)
			data.s[j] = (short)ABS(data.s[j]);
		break;
	case MAT_Tint:
		for(j=0;j<length();j++)
			data.i[j] = (int)ABS(data.i[j]);
		break;
	case MAT_Tfloat:
		for(j=0;j<length();j++)
			data.fl[j] = (float)fabs(data.fl[j]);
		break;
	case MAT_Tdouble:
		for(j=0;j<length();j++)
			data.db[j] = (double)fabs(data.db[j]);
		break;
        default:
            assert (false);
	}
}

/**
	@memo     Calculates the autocorrelation of the vector.
	@doc      Calculates the autocorrelation of the vector with
			a given lag (default lag is 1).
	@param    lag     The lag.  
	@return   The autocorrelation.
*/
double Vec::autoCorrelation( const int lag /*= 1*/ ) const
{
    int n = this->length();
    double mean = .0;
    double d = this->var( &mean );    
    double s = .0;
	int i;
	switch (type())
	{
	case MAT_Tbyte:
		for(i=0;i<n-lag;i++) 
			s += ( data.ptr[i]-mean )*( data.ptr[i+lag]-mean ); 
		break;
	case MAT_Tshort:
		for(i=0;i<n-lag;i++) 
			s += ( data.s[i]-mean )*( data.s[i+lag]-mean ); 
		break;
	case MAT_Tint:
		for(i=0;i<n-lag;i++) 
			s += ( data.i[i]-mean )*( data.i[i+lag]-mean ); 
		break;
	case MAT_Tfloat:
		for(i=0;i<n-lag;i++) 
			s += ( data.fl[i]-mean )*( data.fl[i+lag]-mean ); 
		break;
	case MAT_Tdouble:
		for(i=0;i<n-lag;i++) 
			s += ( data.db[i]-mean )*( data.db[i+lag]-mean ); 
		break;
        default:
            assert (false);
	}
    return s/d;
}
/**
	@memo     Finds the maximum element in the vector.
	@doc      Finds the maximum element in the vector.  
	@return   The maximum value of the elements.
*/
double Vec::max() const
{
	int i;
	switch (type())
	{
	case MAT_Tbyte:
	{
		uchar dMax = 0;
		for (i = 0; i < length(); i++)
			dMax = MAX(dMax, data.ptr[i]);
		return dMax;
	}
	break;
	case MAT_Tshort:
	{
		short dMax = SHRT_MIN;
		for (i = 0; i < length(); i++)
			dMax = MAX(dMax, data.s[i]);
		return dMax;
	}
	break;
	case MAT_Tint:
	{
		int dMax = INT_MIN;
		for (i = 0; i < length(); i++)
			dMax = MAX(dMax, data.i[i]);
		return dMax;
	}
	break;
	case MAT_Tfloat:
	{
		float dMax = -FLT_MAX;
		for (i = 0; i < length(); i++)
			dMax = MAX(dMax, data.fl[i]);
		return dMax;
	}
	break;
	case MAT_Tdouble:
	{
		double dMax = -DBL_MAX;
		for (i = 0; i < length(); i++)
			dMax = MAX(dMax, data.db[i]);
		return dMax;
	}
	break;
	default:
		assert(false);
	}
	return 0;
}
double Vec::max(int& iPos) const
{
	int i;
	iPos = -1;
	switch (type())
	{
	case MAT_Tbyte:
		{
			uchar dMax = 0;
			for (i = 0; i < length(); i++)
			{
				if (dMax < data.ptr[i])
				{
					dMax = data.ptr[i];
					iPos=i;
				}
			}
			return dMax;
		}
		break;
	case MAT_Tshort:
		{
			short dMax = SHRT_MIN;
			for (i = 0; i < length(); i++)
			{
				if (dMax < data.s[i])
				{
					dMax = data.s[i];
					iPos=i;
				}
			}
			return dMax;
		}
		break;
	case MAT_Tint:
		{
			int dMax = INT_MIN;
			for (i = 0; i < length(); i++)
			{
				if (dMax < data.i[i])
				{
					dMax = data.i[i];
					iPos=i;
				}
			}
			return dMax;
		}
		break;
	case MAT_Tfloat:
		{
			float dMax = -FLT_MAX;
			for (i = 0; i < length(); i++)
			{
				if (dMax < data.fl[i])
				{
					dMax = data.fl[i];
					iPos=i;
				}
			}
			return dMax;
		}
		break;
	case MAT_Tdouble:
		{
			double dMax = -DBL_MAX;
			for (i = 0; i < length(); i++)
			{
				if (dMax < data.db[i])
				{
					dMax = data.db[i];
					iPos=i;
				}
			}
			return dMax;
		}
		break;
	default:
		assert (false);
	}
	return 0;
}
double Vec::mean() const
{
	assert(length() > 0);
	int i;
	double sum=0;
	switch (type())
	{
	case MAT_Tbyte:
		{
			for (i=0;i < length(); i++)
				sum += data.ptr[i];
		}
		break;
	case MAT_Tshort:
		{
			for (i=0;i < length(); i++)
				sum += data.s[i];
		}
		break;
	case MAT_Tint:
		{
			for (i=0;i < length(); i++)
				sum += data.i[i];
		}
		break;
	case MAT_Tfloat:
		{
			for (i=0;i < length(); i++)
				sum += data.fl[i];
		}
		break;
	case MAT_Tdouble:
		{
			for (i=0;i < length(); i++)
				sum += data.db[i];
		}
		break;
        default:
            assert (false);
	}
	return sum / length();
}

//////////////////////////////////////////////////////////////////////////
#define ELEM_SWAP_uchar(a,b) { uchar t=(a);(a)=(b);(b)=t; }
#define ELEM_SWAP_short(a,b) { short t=(a);(a)=(b);(b)=t; }
#define ELEM_SWAP_int(a,b) { int t=(a);(a)=(b);(b)=t; }
#define ELEM_SWAP_float(a,b) { float t=(a);(a)=(b);(b)=t; }
#define ELEM_SWAP_double(a,b) { double t=(a);(a)=(b);(b)=t; }

#define I_DEF_quick_select( flavor, SrcType)  \
static SrcType quick_select_##flavor(SrcType arr[], int n) \
{\
    int low, high ;\
    int median;\
    int middle, ll, hh;\
	\
    low = 0 ; high = n-1 ; median = (low + high) / 2;\
    for (;;) {\
        if (high <= low) /* One element only */\
            return arr[median] ;\
		\
        if (high == low + 1) {  /* Two elements only */\
            if (arr[low] > arr[high])\
                ELEM_SWAP_##flavor(arr[low], arr[high]) ;\
            return arr[median] ;\
        }\
		\
		/* find median of low, middle and high items; swap into position low */\
		middle = (low + high) / 2;\
		if (arr[middle] > arr[high])    ELEM_SWAP_##flavor(arr[middle], arr[high]) ;\
		if (arr[low] > arr[high])       ELEM_SWAP_##flavor(arr[low], arr[high]) ;\
		if (arr[middle] > arr[low])     ELEM_SWAP_##flavor(arr[middle], arr[low]) ;\
		\
		/* swap low item (now in position middle) into position (low+1) */\
		ELEM_SWAP_##flavor(arr[middle], arr[low+1]) ;\
		\
		/* Nibble from each end towards middle, swapping items when stuck */\
		ll = low + 1;\
		hh = high;\
		for (;;) {\
			do ll++; while (arr[low] > arr[ll]) ;\
			do hh--; while (arr[hh]  > arr[low]) ;\
			\
			if (hh < ll)\
				break;\
			\
			ELEM_SWAP_##flavor(arr[ll], arr[hh]) ;\
		}\
		\
		/* swap middle item (in position low) back into correct position */\
		ELEM_SWAP_##flavor(arr[low], arr[hh]) ;\
		\
		/* Re-set active partition */\
		if (hh <= median)\
			low = ll;\
        if (hh >= median)\
			high = hh - 1;\
    }\
    return (SrcType)0;\
}

I_DEF_quick_select(uchar, uchar)
I_DEF_quick_select(short, short)
I_DEF_quick_select(int, int)
I_DEF_quick_select(float, float)
I_DEF_quick_select(double, double)

#undef ELEM_SWAP
//////////////////////////////////////////////////////////////////////////
double Vec::median() const
{
	Vec copy(*this);
	double r=0;
	bool doSorting = false;
	if (doSorting)
	{
		copy.sort();
		const int _len = copy.length();
		const bool isOdd = m_len%2!=0;
		switch(type())
		{
		case MAT_Tbyte:
			r = isOdd ? data.ptr[_len /2] : .5*(data.ptr[_len /2-1]+data.ptr[_len /2]);
			break;
		case MAT_Tshort:
			r = isOdd ? data.s[_len /2] : .5*(data.s[_len /2-1]+data.s[_len /2]);
			break;
		case MAT_Tint:
			r = isOdd ? data.i[_len /2] : .5*(data.i[_len /2-1]+data.i[_len /2]);
			break;
		case MAT_Tfloat:
			r = isOdd ? data.fl[_len /2] : .5*(data.fl[_len /2-1]+data.fl[_len /2]);
			break;
		case MAT_Tdouble:
			r = isOdd ? data.db[_len /2] : .5*(data.db[_len /2-1]+data.db[_len /2]);
			break;
            default:
                assert (false);
		}
	} 
	else 
	{
		// *much* faster alternative kindly provided by Nicolas Devillard
		// see code snip above
		switch(type())
		{
		case MAT_Tbyte:
			r = quick_select_uchar( copy.data.ptr, copy.length() );
			break;
		case MAT_Tshort:
			r = quick_select_short( copy.data.s, copy.length() );
			break;
		case MAT_Tint:
			r = quick_select_int( copy.data.i, copy.length() );
			break;
		case MAT_Tfloat:
			r = quick_select_float( copy.data.fl, copy.length() );
			break;
		case MAT_Tdouble:
			r = quick_select_double( copy.data.db, copy.length() );
			break;
            default:
                assert (false);
		}
	}
	
	return r;
}
Vec	Vec::trim( const double percentage /*= .10*/ ) const
{
	assert( percentage>=.0 && percentage<=1. );
	
	if ( percentage>=1. ) {
		
		// by convention
		Vec v(1, this->type());
		switch(type())
		{
		case MAT_Tbyte:
			v.data.ptr[0]= (uchar)this->median();
			break;
		case MAT_Tshort:
			v.data.s[0]= (short)this->median();
			break;
		case MAT_Tint:
			v.data.i[0]= (int)this->median();
			break;
		case MAT_Tfloat:
			v.data.fl[0]= (float)this->median();
			break;
		case MAT_Tdouble:
			v.data.db[0]= (double)this->median();
			break;
            default:
                assert (false);
		}
		return v;
	} 
	else 
	{
		const int _len = this->length();
		const int head_tail_len = (int)(.5+ _len*percentage/2.);
		
		Vec trimmedVec(*this);
		trimmedVec.sort();
		return trimmedVec.range( head_tail_len, _len -head_tail_len-1 );
	}
}
double Vec::trimmedMean( const double percentage /*= .10*/ ) const
{
	Vec trimmedVec = this->trim( percentage );
	return trimmedVec.mean();
}
double Vec::trimmedVar( const double percentage /*= .10*/ ) const
{
	Vec trimmedVec = this->trim( percentage );
	return trimmedVec.var();
}
double Vec::trimmedStd( const double percentage /*= .10*/ ) const
{
	Vec trimmedVec = this->trim( percentage );
	return trimmedVec.std();
}
double Vec::min() const
{
	int i;
	switch (type())
	{
	case MAT_Tbyte:
		{
			uchar dMin = 255;
			for (i=0;i < length(); i++)
				dMin = MIN(dMin,data.ptr[i]);
			return dMin;
		}
		break;
	case MAT_Tshort:
		{
			short dMin = SHRT_MAX;
			for (i=0; i < length(); i++)
				dMin = MIN(dMin,data.s[i]);
			return dMin;
		}
		break;
	case MAT_Tint:
		{
			int dMin = INT_MAX;
			for (i=0;i < length(); i++)
				dMin = MIN(dMin,data.i[i]);
			return dMin;
		}
		break;
	case MAT_Tfloat:
		{
			float dMin = FLT_MAX;
			for (i=0; i < length(); i++)
				dMin = MIN(dMin,data.fl[i]);
			return dMin;
		}
		break;
	case MAT_Tdouble:
		{
			double dMin = DBL_MAX;
			for (i = 0; i < length(); i++)
				dMin = MIN(dMin,data.db[i]);
			return dMin;
		}
		break;
	default:
            assert (false);
	}
	return 0;
}
double Vec::min(int& iPos) const
{
	int i;
	iPos = -1;
	switch (type())
	{
	case MAT_Tbyte:
		{
			uchar dMin = 255;
			for (i = 0; i < length(); i++)
			{
				if (dMin > data.ptr[i])
				{
					dMin = data.ptr[i];
					iPos=i;
				}
			}
			return dMin;
		}
		break;
	case MAT_Tshort:
		{
			short dMin = SHRT_MAX;
			for (i = 0; i < length(); i++)
			{
				if (dMin > data.s[i])
				{
					dMin = data.s[i];
					iPos=i;
				}
			}
			return dMin;
		}
		break;
	case MAT_Tint:
		{
			int dMin = INT_MAX;
			for (i=0;i < length(); i++)
			{
				if (dMin > data.i[i])
				{
					dMin = data.i[i];
					iPos=i;
				}
			}
			return dMin;
		}
		break;
	case MAT_Tfloat:
		{
			float dMin = FLT_MAX;
			for (i = 0; i < length(); i++)
			{
				if (dMin > data.fl[i])
				{
					dMin = data.fl[i];
					iPos=i;
				}
			}
			return dMin;
		}
		break;
	case MAT_Tdouble:
		{
			double dMin = DBL_MAX;
			for (i = 0; i < length(); i++)
			{
				if (dMin > data.db[i])
				{
					dMin = data.db[i];
					iPos=i;
				}
			}
			return dMin;
		}
		break;
	default:
		assert (false);
	}
	return 0;
}
double Vec::skewness() const
{
	int i;
	double dSkewness = 0, dMean = mean(), dTmp;
	switch (type())
	{
	case MAT_Tbyte:
		for (i=0;i < length(); i++)
		{
			dTmp = (double)data.ptr[i] - dMean;
			dSkewness += dTmp*dTmp*dTmp;
		}
		break;
	case MAT_Tshort:
		for (i=0;i < length(); i++)
		{
			dTmp = (double)data.s[i] - dMean;
			dSkewness += dTmp*dTmp*dTmp;
		}
		break;
	case MAT_Tint:
		for (i=0;i < length(); i++)
		{
			dTmp = (double)data.i[i] - dMean;
			dSkewness += dTmp*dTmp*dTmp;
		}
		break;
	case MAT_Tfloat:
		for (i=0;i < length(); i++)
		{
			dTmp = (double)data.fl[i] - dMean;
			dSkewness += dTmp*dTmp*dTmp;
		}
		break;
	case MAT_Tdouble:
		for (i=0;i < length(); i++)
		{
			dTmp = (double)data.db[i] - dMean;
			dSkewness += dTmp*dTmp*dTmp;
		}
		break;
        default:
            assert (false);
	}
	double dStd = std();
	return dSkewness/(length()*dStd*dStd*dStd);
}
double Vec::std() const
{
	return std::sqrt(var());
}
double Vec::sum() const
{
	int i;
	double dSum=0;
	switch (type())
	{
	case MAT_Tbyte:
		for (i=0;i < length(); i++)
			dSum+=(double)data.ptr[i];
		break;
	case MAT_Tshort:
		for (i=0;i < length(); i++)
			dSum+=(double)data.s[i];
		break;
	case MAT_Tint:
		for (i=0;i < length(); i++)
			dSum+=(double)data.i[i];
		break;
	case MAT_Tfloat:
		for (i=0;i < length(); i++)
			dSum+=(double)data.fl[i];
		break;
	case MAT_Tdouble:
		for (i=0;i < length(); i++)
			dSum+=(double)data.db[i];
		break;
        default:
            assert (false);
	}
	return dSum;
}
double Vec::var() const
{
	int i;
	double dVar = 0, dMean = mean();
	switch (type())
	{
	case MAT_Tbyte:
		for (i=0;i < length(); i++)
			dVar+=SQR((double)data.ptr[i]-dMean);
		break;
	case MAT_Tshort:
		for (i=0;i < length(); i++)
			dVar+=SQR((double)data.s[i]-dMean);
		break;
	case MAT_Tint:
		for (i=0;i < length(); i++)
			dVar+=SQR((double)data.i[i]-dMean);
		break;
	case MAT_Tfloat:
		for (i=0;i < length(); i++)
			dVar+=SQR((double)data.fl[i]-dMean);
		break;
	case MAT_Tdouble:
		for (i=0;i < length(); i++)
			dVar+=SQR((double)data.db[i]-dMean);
		break;
        default:
            assert (false);
	}
	return dVar;
}
double Vec::var( double *pMean ) const
{
	int i;
	double dVar = 0, dMean = mean();
	switch (type())
	{
	case MAT_Tbyte:
		for (i=0;i < length(); i++)
			dVar+=SQR((double)data.ptr[i]-dMean);
		break;
	case MAT_Tshort:
		for (i=0;i < length(); i++)
			dVar+=SQR((double)data.s[i]-dMean);
		break;
	case MAT_Tint:
		for (i=0;i < length(); i++)
			dVar+=SQR((double)data.i[i]-dMean);
		break;
	case MAT_Tfloat:
		for (i=0;i < length(); i++)
			dVar+=SQR((double)data.fl[i]-dMean);
		break;
	case MAT_Tdouble:
		for (i=0;i < length(); i++)
			dVar+=SQR((double)data.db[i]-dMean);
		break;
        default:
            assert (false);
	}
	if (pMean)
		*pMean = dMean;
	return dVar;
}
void Vec::rand()
{
	int i;
    switch (type())
	{
	case MAT_Tfloat:
		for(i=0;i<m_len;i++)
			data.fl[i] = (float)(std::rand()/(double)RAND_MAX);
		break;
	case MAT_Tdouble:
		for(i=0;i<m_len;i++)
			data.db[i] = (double)(std::rand()/(double)RAND_MAX);
        default:
            assert (false);
		break;
	}
}
/**
	@memo     Uniformly distributed integer random numbers.
	@doc      Inserts uniformly distributed integer random numbers in
			the range [st;end].
	@return   Nothing.
*/
void Vec::rand( const int st, const int end )
{
	int i;
    double width = end-st;
    switch (type())
	{
	case MAT_Tbyte:
		for(i=0;i<m_len;i++)
			data.ptr[i] = (uchar)(int)(.5 + width*(std::rand()/(double)RAND_MAX) + st);
		break;
	case MAT_Tshort:
		for(i=0;i<m_len;i++)
			data.s[i] = (short)(int)(.5 + width*(std::rand()/(double)RAND_MAX) + st);
		break;
	case MAT_Tint:
		for(i=0;i<m_len;i++)
			data.i[i] = (int)(int)(.5 + width*(std::rand()/(double)RAND_MAX) + st);
		break;
	case MAT_Tfloat:
		for(i=0;i<m_len;i++)
			data.fl[i] = (float)(int)(.5 + width*(std::rand()/(double)RAND_MAX) + st);
		break;
	case MAT_Tdouble:
		for(i=0;i<m_len;i++)
			data.db[i] = (double)(int)(.5 + width*(std::rand()/(double)RAND_MAX) + st);
		break;
        default:
            assert (false);
	}
}
void Vec::ceil()
{
	int j;
	switch(type())
	{
	case MAT_Tfloat:
		for(j=0;j<length();j++)
			data.fl[j] = (float)std::ceil(data.fl[j]);
		break;
	case MAT_Tdouble:
		for(j=0;j<length();j++)
			data.db[j] = (double)std::ceil(data.db[j]);
		break;
        default:
            assert (false);
	}
}
void Vec::floor()
{
	int j;
	switch(type())
	{
	case MAT_Tfloat:
		for(j=0;j<length();j++)
			data.fl[j] = (float)std::floor(data.fl[j]);
		break;
	case MAT_Tdouble:
		for(j=0;j<length();j++)
			data.db[j] = (double)std::floor(data.db[j]);
		break;
        default:
            assert (false);
	}
}
void Vec::round()
{
	int j;
	switch(type())
	{
	case MAT_Tfloat:
		for(j=0;j<length();j++)
			data.fl[j] = (float)cvutil::round(data.fl[j]);
		break;
	case MAT_Tdouble:
		for(j=0;j<length();j++)
			data.db[j] = (double)cvutil::round(data.db[j]);
		break;
        default:
            assert (false);
	}
}
/**
	@memo     Generates a vector of linearly equally spaced points 
				between x1 and x2 (inclusive).
	@doc      Generates a vector of linearly equally spaced points 
				between x1 and x2 (inclusive).  
	@param    x1  Starting point.
	@param    x2  Ending point.
	@param    n   Number of points.
	@return   Nothing.
*/
void Vec::linspace( const double x1, const double x2, const int n )
{
    assert( n>0 );
    assert( x1<x2 );
	resize(n);
    double mul = (x2-x1)/(n-1.);
	int i;
	switch (type())
	{
	case MAT_Tbyte:
		for(i=0;i<n;i++)
			data.ptr[i] = (uchar)(x1 + i*mul);
		break;
	case MAT_Tshort:
		for(i=0;i<n;i++)
			data.s[i] = (short)(x1 + i*mul);
		break;
	case MAT_Tint:
		for(i=0;i<n;i++)
			data.i[i] = (int)(x1 + i*mul);
		break;
	case MAT_Tfloat:
		for(i=0;i<n;i++)
			data.fl[i] = (float)(x1 + i*mul);
		break;
	case MAT_Tdouble:
		for(i=0;i<n;i++)
			data.db[i] = (double)(x1 + i*mul);
		break;
        default:
            assert (false);
	}
}

#define I_DEF_cmp(flavor, SrcType)  \
static int __cmp_asc_##flavor( const void *arg1, const void *arg2 ) {   \
	\
    SrcType v1 = *(SrcType*)arg1;\
    SrcType v2 = *(SrcType*)arg2;\
	\
    if (v1<v2)\
        return -1;\
    if (v1>v2)\
        return  1;\
	\
    return 0;\
}\
static int __cmp_des_##flavor( const void *arg1, const void *arg2 ) {   \
	\
    SrcType v1 = *(SrcType*)arg1;\
    SrcType v2 = *(SrcType*)arg2;\
	\
    if (v1>v2)\
        return -1;\
    if (v1<v2)\
        return  1;\
	\
    return 0;\
}

I_DEF_cmp(uchar, uchar)
I_DEF_cmp(short, short)
I_DEF_cmp(int, int)
I_DEF_cmp(float, float)
I_DEF_cmp(double, double)

void Vec::sort( bool ascending)
{
	switch (type())
	{
	case MAT_Tbyte:
		qsort(  data.ptr, (size_t)this->length(), sizeof( uchar ), 
			ascending ? __cmp_asc_uchar: __cmp_des_uchar   );
		break;
	case MAT_Tshort:
		qsort(  data.ptr, (size_t)this->length(), sizeof( short ), 
			ascending ? __cmp_asc_short: __cmp_des_short );
		break;
	case MAT_Tint:
		qsort(  data.ptr, (size_t)this->length(), sizeof( int ), 
			ascending ? __cmp_asc_int: __cmp_des_int );
		break;
	case MAT_Tfloat:
		qsort(  data.ptr, (size_t)this->length(), sizeof( float ), 
			ascending ? __cmp_asc_float: __cmp_des_float);
		break;
	case MAT_Tdouble:
		qsort(  data.ptr, (size_t)this->length(), sizeof( double ), 
			ascending ? __cmp_asc_double: __cmp_des_double);
		break;
        default:
            assert (false);
	}
}

void Vec::shuffle()
{
    int n = length();
	
    int shuffle_amount = 2;
	int s, i;
	switch(type())
	{
	case MAT_Tbyte:
		for(s=0;s<shuffle_amount;s++)
		{
			for(i=0;i<n;i++)
			{
				int index = (int)(.5+(n-1)*std::rand()/(double)RAND_MAX);
				ELEM_SWAP_uchar (data.ptr[i], data.ptr[index]);
			}
		}
		break;
	case MAT_Tshort:
		for(s=0;s<shuffle_amount;s++)
		{
			for(i=0;i<n;i++)
			{
				int index = (int)(.5+(n-1)*std::rand()/(double)RAND_MAX);
				ELEM_SWAP_short(data.s[i], data.s[index]);
			}
		}
		break;
	case MAT_Tint:
		for(s=0;s<shuffle_amount;s++)
		{
			for(i=0;i<n;i++)
			{
				int index = (int)(.5+(n-1)*std::rand()/(double)RAND_MAX);
				ELEM_SWAP_int (data.i[i], data.i[index]);
			}
		}
		break;
	case MAT_Tfloat:
		for(s=0;s<shuffle_amount;s++)
		{
			for(i=0;i<n;i++)
			{
				int index = (int)(.5+(n-1)*std::rand()/(double)RAND_MAX);
				ELEM_SWAP_float(data.fl[i], data.fl[index]);
			}
		}
		break;
	case MAT_Tdouble:
		for(s=0;s<shuffle_amount;s++)
		{
			for(i=0;i<n;i++)
			{
				int index = (int)(.5+(n-1)*std::rand()/(double)RAND_MAX);
				ELEM_SWAP_double(data.db[i], data.db[index]);
			}
		}
		break;
        default:
            assert (false);
	}
}
String Vec::toString(const bool fNewline /*= true*/) const
{
	String strOut = "";
	
	char strTmp[512];
	int i;
	switch (type())
	{
	case MAT_Tbyte:
		for(i=0;i<length();i++)
		{
			sprintf (strTmp, "%d ",data.ptr[i]);
			strOut += strTmp;
		}
		break;
	case MAT_Tshort:
		for(i=0;i<length();i++)
		{
            sprintf (strTmp, "%d ",data.s[i]);
			strOut += strTmp;
		}
		break;
	case MAT_Tint:
		for(i=0;i<length();i++)
		{
            sprintf (strTmp, "%d ",data.i[i]);
			strOut += strTmp;
		}
		break;
	case MAT_Tfloat:
		for(i=0;i<length();i++)
		{
            sprintf (strTmp, "%8.3f ",data.fl[i]);
			strOut += strTmp;
		}
		break;
	case MAT_Tdouble:
		for(i=0;i<length();i++)
		{
            sprintf (strTmp, "%8.3f ",data.db[i]);
			strOut += strTmp;
		}
		break;
        default:
            assert (false);
	}
	
	if (fNewline)
		strOut += "\r\n";	
	
    return strOut;
}
void Vec::elementMultiply(const Vec& vector)
{
	int i;
	assert(type()==vector.type() && length() == vector.length());
	switch(type())
	{
	case MAT_Tint:
		for (i=0;i < length(); i++)
			data.i[i] *= vector.data.i[i];
		break;
	case MAT_Tfloat:
		for (i=0;i < length(); i++)
			data.fl[i] *= vector.data.fl[i];
		break;
	case MAT_Tdouble:
		for (i=0;i < length(); i++)
			data.db[i] *= vector.data.db[i];
		break;
        default:
            assert (false);
	}
}
void Vec::elementDivide(const Vec& vector)
{
	int i;
	assert(type()==vector.type() && length() == vector.length());
	switch(type())
	{
	case MAT_Tfloat:
		for (i=0;i < length(); i++)
			data.fl[i] /= vector.data.fl[i];
		break;
	case MAT_Tdouble:
		for (i=0;i < length(); i++)
			data.db[i] /= vector.data.db[i];
		break;
        default:
            assert (false);
	}
}
/**
	@memo     Clamps the vector to [min,max].
	@doc      Clamps the vector to [min,max]. 
	@param    min     Minumim value.
	@param    max     Maximum value.
	@return   Nothing.
*/
void Vec::clamp( const double min, const double max )
{
	int i;
	switch(type())
	{
	case MAT_Tbyte:
		for(i=0;i<m_len;i++)
		{
			uchar *val = data.ptr+i;
			*val = *val<(uchar)min ? (uchar)min : (*val>(uchar)max ? (uchar)max : *val);
		}
		break;
	case MAT_Tshort:
		for(i=0;i<m_len;i++)
		{
			short *val = data.s+i;
			*val = *val<(short)min ? (short)min : (*val>(short)max ? (short)max : *val);
		}
		break;
	case MAT_Tint:
		for(i=0;i<m_len;i++)
		{
			int *val = data.i+i;
			*val = *val<(int)min ? (int)min : (*val>(int)max ? (int)max : *val);
		}
		break;
	case MAT_Tfloat:
		for(i=0;i<m_len;i++)
		{
			float *val = data.fl+i;
			*val = *val<(float)min ? (float)min : (*val>(float)max ? (float)max : *val);
		}
		break;
	case MAT_Tdouble:
		for(i=0;i<m_len;i++)
		{
			double*val = data.db+i;
			*val = *val<(double)min ? (double)min : (*val>(double)max ? (double)max : *val);
		}
		break;
        default:
            assert (false);
	}
}

void Vec::pow(double dP)
{
	int j;
	switch(type())
	{
	case MAT_Tbyte:
		for(j=0;j<length();j++)
			data.ptr[j] = (uchar)std::pow(data.ptr[j], dP);
		break;
	case MAT_Tshort:
		for(j=0;j<length();j++)
			data.s[j] = (short)std::pow(data.s[j], dP);
		break;
	case MAT_Tint:
		for(j=0;j<length();j++)
			data.i[j] = (int)std::pow(data.i[j], dP);
		break;
	case MAT_Tfloat:
		for(j=0;j<length();j++)
			data.fl[j] = (float)powf(data.fl[j], (float)dP);
		break;
	case MAT_Tdouble:
		for(j=0;j<length();j++)
			data.db[j] = (double)std::pow(data.db[j], dP);
		break;
        default:
            assert (false);
	}
}
void Vec::sqrt()
{
	int j;
	switch(type())
	{
	case MAT_Tbyte:
		for(j=0;j<length();j++)
			data.ptr[j] = (uchar)sqrtf(data.ptr[j]);
		break;
	case MAT_Tshort:
		for(j=0;j<length();j++)
			data.s[j] = (short)sqrtf(data.s[j]);
		break;
	case MAT_Tint:
		for(j=0;j<length();j++)
			data.i[j] = (int)sqrtf((float)data.i[j]);
		break;
	case MAT_Tfloat:
		for(j=0;j<length();j++)
			data.fl[j] = (float)sqrtf(data.fl[j]);
		break;
	case MAT_Tdouble:
		for(j=0;j<length();j++)
			data.db[j] = (double)std::sqrt(data.db[j]);
		break;
        default:
            assert (false);
	}
}
void Vec::sqr()
{
	int j;
	switch(type())
	{
	case MAT_Tbyte:
		for(j=0;j<length();j++)
			data.ptr[j] = (uchar)SQR(data.ptr[j]);
		break;
	case MAT_Tshort:
		for(j=0;j<length();j++)
			data.s[j] = (short)SQR(data.s[j]);
		break;
	case MAT_Tint:
		for(j=0;j<length();j++)
			data.i[j] = (int)SQR(data.i[j]);
		break;
	case MAT_Tfloat:
		for(j=0;j<length();j++)
			data.fl[j] = (float)SQR(data.fl[j]);
		break;
	case MAT_Tdouble:
		for(j=0;j<length();j++)
			data.db[j] = (double)SQR(data.db[j]);
		break;
        default:
            assert (false);
	}
}
void Vec::log()
{
	int j;
	switch(type())
	{
	case MAT_Tbyte:
		for(j=0;j<length();j++)
			data.ptr[j] = (uchar)logf(data.ptr[j]);
		break;
	case MAT_Tshort:
		for(j=0;j<length();j++)
			data.s[j] = (short)logf(data.s[j]);
		break;
	case MAT_Tint:
		for(j=0;j<length();j++)
			data.i[j] = (int)logf((float)data.i[j]);
		break;
	case MAT_Tfloat:
		for(j=0;j<length();j++)
			data.fl[j] = (float)logf(data.fl[j]);
		break;
	case MAT_Tdouble:
		for(j=0;j<length();j++)
			data.db[j] = (double)std::log(data.db[j]);
		break;
        default:
            assert (false);
	}
}
void Vec::reverse()
{
	int i;
	switch(type())
	{
	case MAT_Tbyte:
		for (i=0;i<m_len/2;i++)
			ELEM_SWAP_uchar(data.ptr[i], data.ptr[m_len-i-1])
		break;
	case MAT_Tshort:
		for (i=0;i<m_len/2;i++)
			ELEM_SWAP_short(data.s[i], data.s[m_len-i-1])
		break;
	case MAT_Tint:
		for (i=0;i<m_len/2;i++)
			ELEM_SWAP_int(data.i[i], data.i[m_len-i-1])
		break;
	case MAT_Tfloat:
		for (i=0;i<m_len/2;i++)
			ELEM_SWAP_float(data.fl[i], data.fl[m_len-i-1])
		break;
	case MAT_Tdouble:
		for (i=0;i<m_len/2;i++)
			ELEM_SWAP_double(data.db[i], data.db[m_len-i-1])
		break;
        default:
            assert (false);
	}
}
Vec Vec::range( const int st, const int end) const
{
    assert( st<end );
    assert( end<length() );
	
    int _len=end-st+1;
    Vec range(_len, this->type() );
	memcpy(range.data.ptr, this->data.ptr+step()*st, step()*_len);
    return range;
}
void Vec::toMatrix( const int nRows, const int nCols, Mat &m, bool rowWise /*=true*/ )
{
    assert( nRows>0 && nCols>0 );
    assert( nRows*nCols==this->length() );
	
    int i=0;
    m.create( nRows, nCols, type() );
	switch (type())
	{
	case MAT_Tbyte:
		if (rowWise)
		{
			for(int r=0;r<nRows;r++)
			{
				for(int c=0;c<nCols;c++)
					m.data.ptr[r][c] = data.ptr[i++];
			}
		} 
		else 
		{
			for(int c=0;c<nCols;c++) 
			{
				for(int r=0;r<nRows;r++) 
					m.data.ptr[r][c] = data.ptr[i++];
			}
		}
		break;
	case MAT_Tshort:
		if (rowWise)
		{
			for(int r=0;r<nRows;r++)
			{
				for(int c=0;c<nCols;c++)
					m.data.s[r][c] = data.s[i++];
			}
		} 
		else 
		{
			for(int c=0;c<nCols;c++) 
			{
				for(int r=0;r<nRows;r++) 
					m.data.s[r][c] = data.s[i++];
			}
		}
		break;
	case MAT_Tint:
		if (rowWise)
		{
			for(int r=0;r<nRows;r++)
			{
				for(int c=0;c<nCols;c++)
					m.data.i[r][c] = data.i[i++];
			}
		} 
		else 
		{
			for(int c=0;c<nCols;c++) 
			{
				for(int r=0;r<nRows;r++) 
					m.data.i[r][c] = data.i[i++];
			}
		}
		break;
	case MAT_Tfloat:
		if (rowWise)
		{
			for(int r=0;r<nRows;r++)
			{
				for(int c=0;c<nCols;c++)
					m.data.fl[r][c] = data.fl[i++];
			}
		} 
		else 
		{
			for(int c=0;c<nCols;c++) 
			{
				for(int r=0;r<nRows;r++) 
					m.data.fl[r][c] = data.fl[i++];
			}
		}
		break;
	case MAT_Tdouble:
		if (rowWise)
		{
			for(int r=0;r<nRows;r++)
			{
				for(int c=0;c<nCols;c++)
					m.data.db[r][c] = data.db[i++];
			}
		} 
		else 
		{
			for(int c=0;c<nCols;c++) 
			{
				for(int r=0;r<nRows;r++) 
					m.data.db[r][c] = data.db[i++];
			}
		}
		break;
        default:
            assert (false);
	}
}

void Vec::toMatlab(const String& sFilename,const String& sName,const String& sComment,bool fAppend) const
{
	assert(length() > 0);
	
	Mat mOut(data.ptr, length(),1, type());
	
	memcpy(mOut.data.ptr[0], data.ptr, step()*length());
	mOut.toMatlab(sFilename,sName,sComment,fAppend);
}

void Vec::fromMatlab(const String& sFilename,const String& sName)
{
	Mat mOut;
	mOut.fromMatlab(sFilename,sName);
	
	resize(mOut.rows());
	memcpy(data.ptr, mOut.data.ptr[0], mOut.step()*length());
}

void Vec::toC (const String& sFilename, const String& sVarName, const int nElement, bool fAppend) const
{
	if (length() == 0)
		return;
	FILE* pFile = NULL;
	int* pan = new int[length()];
	if(fAppend)
		pFile = fopen(sFilename, "a++");
	else 
		pFile = fopen(sFilename, "w");
	
	fprintf(pFile, "int %s[%d]={", (const char*)sVarName, length());

	int i;
	for(i=0;i<length();i++)
	{
		switch(type())
		{
		case MAT_Tbyte:
			pan[i] = data.ptr[i];
			break;
		case MAT_Tshort:
			pan[i] = data.s[i];
			break;
		case MAT_Tint:
			pan[i] = data.i[i];
			break;
		case MAT_Tfloat:
			pan[i] = cvutil::round(data.fl[i] * 1048576);
			break;
		case MAT_Tdouble:
			pan[i] = cvutil::round(data.db[i] * 1048576);
			break;
            default:
                assert (false);
		}
	}
	for(i=0;i<length()-1;i++)
	{
		if (i % nElement == 0)
			fprintf(pFile, "\n\t");
		fprintf (pFile, "%d,", pan[i]);
	}
	fprintf (pFile, "%d};\n", pan[length()-1]);
	fclose (pFile);
	delete []pan;
}

void Vec::swap(Vec& other)
{
	int ntemp;
	uchar* pbtemp;
	TYPE nType;
	SWAP(m_step, other.m_step, ntemp);
	SWAP(m_depend, other.m_depend, ntemp);
	SWAP(m_len, other.m_len, ntemp);
	SWAP(m_nMaxLen, other.m_nMaxLen, ntemp);
	SWAP(m_type, other.m_type, nType);
	SWAP(data.ptr, other.data.ptr, pbtemp);
}

bool Vec::operator==(const Vec& refvector) const
{
	if (!isValid() || !refvector.isValid())
		return false;
	int countbytes = length()*step();
	int countbytes2 = refvector.length()*refvector.step();
	if (countbytes != countbytes2)
		return false;
	bool fcomp = memcmp (this->data.ptr, refvector.data.ptr, countbytes) ? false : true;
	return fcomp;
}
bool Vec::operator!=(const Vec& refvector) const
{
	return !(operator==(refvector));
}
bool Vec::operator<(const Vec& /*refvector*/) const
{
	assert (false);
	return false;
}
Vec Vec::operator+(const Vec &refvectorB) const
{
	Vec vret(this->length(), this->type());
	MatOp::add (&vret, this, &refvectorB);
	return vret;
}
Vec Vec::operator-(void) const
{
	Vec vret(this->length(), this->type());
	int i;
	switch(type())
	{
	case MAT_Tbyte:
		for (i=0; i<m_len; i++)
			vret.data.ptr[i] = -data.ptr[i];
		break;
	case MAT_Tshort:
		for (i=0; i<m_len; i++)
			vret.data.s[i] = -data.s[i];
		break;
	case MAT_Tint:
		for (i=0; i<m_len; i++)
			vret.data.i[i] = -data.i[i];
		break;
	case MAT_Tfloat:
		for (i=0; i<m_len; i++)
			vret.data.fl[i] = -data.fl[i];
		break;
	case MAT_Tdouble:
		for (i=0; i<m_len; i++)
			vret.data.db[i] = -data.db[i];
		break;
        default:
            assert (false);
	}
	return vret;
}
Vec Vec::operator-(const Vec &refvectorB) const
{
	Vec vret(this->length(), this->type());
	MatOp::sub (&vret, this, &refvectorB);
	return vret;
}
Vec Vec::operator*(double dbl) const
{
	Vec vret;
	MatOp::mul (&vret, dbl, this);
	return vret;
}
double Vec::operator*(const Vec &refvectorB) const
{
	return MatOp::dotProduct (this, &refvectorB);
}
Vec Vec::operator/(double dbl) const
{
	Vec vret;
	MatOp::mul (&vret, 1.0f/dbl, this);
	return vret;
}
Vec& Vec::operator+=(const Vec &refvectorB)
{
	MatOp::add (this, this, &refvectorB);
	return *this;
}
Vec& Vec::operator-=(const Vec &refvectorB)
{
	MatOp::sub (this, this, &refvectorB);
	return *this;
}
Vec& Vec::operator+=(double dbl)
{
	int i;
	switch(type())
	{
	case MAT_Tbyte:
		for (i=0; i<m_len; i++)
			data.ptr[i] += (uchar)dbl;
		break;
	case MAT_Tshort:
		for (i=0; i<m_len; i++)
			data.s[i] += (short)dbl;
		break;
	case MAT_Tint:
		for (i=0; i<m_len; i++)
			data.i[i] += (int)dbl;
		break;
	case MAT_Tfloat:
		for (i=0; i<m_len; i++)
			data.fl[i] += (float)dbl;
		break;
	case MAT_Tdouble:
		for (i=0; i<m_len; i++)
			data.db[i] += (double)dbl;
		break;
        default:
            assert (false);
	}
	return *this;
}
Vec& Vec::operator-=(double dbl)
{
	return operator+=(dbl);
}
Vec& Vec::operator*=(double dbl)
{
	MatOp::mul (this, dbl, this);
	return *this;
}
Vec& Vec::operator/=(double dbl)
{
	return operator*=(1.0/dbl);
}

}

#pragma warning (pop)