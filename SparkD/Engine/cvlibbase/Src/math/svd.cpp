#include "AutoBuffer.h"
#include "Random.h"
#include "svd.h"

namespace cvlib
{

template<typename _Tp> struct VBLAS
{
    int dot(const _Tp*, const _Tp*, int, _Tp*) const { return 0; }
    int givens(_Tp*, _Tp*, int, _Tp, _Tp) const { return 0; }
    int givensx(_Tp*, _Tp*, int, _Tp, _Tp, _Tp*, _Tp*) const { return 0; }
};

#if CVLIB_SSE2
template<> inline int VBLAS<float>::dot(const float* a, const float* b, int n, float* result) const
{
    if( n < 8 )
        return 0;
    int k = 0;
    __m128 s0 = _mm_setzero_ps(), s1 = _mm_setzero_ps();
    for( ; k <= n - 8; k += 8 )
    {
        __m128 a0 = _mm_load_ps(a + k), a1 = _mm_load_ps(a + k + 4);
        __m128 b0 = _mm_load_ps(b + k), b1 = _mm_load_ps(b + k + 4);

        s0 = _mm_add_ps(s0, _mm_mul_ps(a0, b0));
        s1 = _mm_add_ps(s1, _mm_mul_ps(a1, b1));
    }
    s0 = _mm_add_ps(s0, s1);
    float sbuf[4];
    _mm_storeu_ps(sbuf, s0);
    *result = sbuf[0] + sbuf[1] + sbuf[2] + sbuf[3];
    return k;
}


template<> inline int VBLAS<float>::givens(float* a, float* b, int n, float c, float s) const
{
    if( n < 4 )
        return 0;
    int k = 0;
    __m128 c4 = _mm_set1_ps(c), s4 = _mm_set1_ps(s);
    for( ; k <= n - 4; k += 4 )
    {
        __m128 a0 = _mm_load_ps(a + k);
        __m128 b0 = _mm_load_ps(b + k);
        __m128 t0 = _mm_add_ps(_mm_mul_ps(a0, c4), _mm_mul_ps(b0, s4));
        __m128 t1 = _mm_sub_ps(_mm_mul_ps(b0, c4), _mm_mul_ps(a0, s4));
        _mm_store_ps(a + k, t0);
        _mm_store_ps(b + k, t1);
    }
    return k;
}


template<> inline int VBLAS<float>::givensx(float* a, float* b, int n, float c, float s,
                                             float* anorm, float* bnorm) const
{
    if( n < 4 )
        return 0;
    int k = 0;
    __m128 c4 = _mm_set1_ps(c), s4 = _mm_set1_ps(s);
    __m128 sa = _mm_setzero_ps(), sb = _mm_setzero_ps();
    for( ; k <= n - 4; k += 4 )
    {
        __m128 a0 = _mm_load_ps(a + k);
        __m128 b0 = _mm_load_ps(b + k);
        __m128 t0 = _mm_add_ps(_mm_mul_ps(a0, c4), _mm_mul_ps(b0, s4));
        __m128 t1 = _mm_sub_ps(_mm_mul_ps(b0, c4), _mm_mul_ps(a0, s4));
        _mm_store_ps(a + k, t0);
        _mm_store_ps(b + k, t1);
        sa = _mm_add_ps(sa, _mm_mul_ps(t0, t0));
        sb = _mm_add_ps(sb, _mm_mul_ps(t1, t1));
    }
    float abuf[4], bbuf[4];
    _mm_storeu_ps(abuf, sa);
    _mm_storeu_ps(bbuf, sb);
    *anorm = abuf[0] + abuf[1] + abuf[2] + abuf[3];
    *bnorm = bbuf[0] + bbuf[1] + bbuf[2] + bbuf[3];
    return k;
}


template<> inline int VBLAS<double>::dot(const double* a, const double* b, int n, double* result) const
{
    if( n < 4 )
        return 0;
    int k = 0;
    __m128d s0 = _mm_setzero_pd(), s1 = _mm_setzero_pd();
    for( ; k <= n - 4; k += 4 )
    {
        __m128d a0 = _mm_load_pd(a + k), a1 = _mm_load_pd(a + k + 2);
        __m128d b0 = _mm_load_pd(b + k), b1 = _mm_load_pd(b + k + 2);

        s0 = _mm_add_pd(s0, _mm_mul_pd(a0, b0));
        s1 = _mm_add_pd(s1, _mm_mul_pd(a1, b1));
    }
    s0 = _mm_add_pd(s0, s1);
    double sbuf[2];
    _mm_storeu_pd(sbuf, s0);
    *result = sbuf[0] + sbuf[1];
    return k;
}


template<> inline int VBLAS<double>::givens(double* a, double* b, int n, double c, double s) const
{
    int k = 0;
    __m128d c2 = _mm_set1_pd(c), s2 = _mm_set1_pd(s);
    for( ; k <= n - 2; k += 2 )
    {
        __m128d a0 = _mm_load_pd(a + k);
        __m128d b0 = _mm_load_pd(b + k);
        __m128d t0 = _mm_add_pd(_mm_mul_pd(a0, c2), _mm_mul_pd(b0, s2));
        __m128d t1 = _mm_sub_pd(_mm_mul_pd(b0, c2), _mm_mul_pd(a0, s2));
        _mm_store_pd(a + k, t0);
        _mm_store_pd(b + k, t1);
    }
    return k;
}


template<> inline int VBLAS<double>::givensx(double* a, double* b, int n, double c, double s,
                                              double* anorm, double* bnorm) const
{
    int k = 0;
    __m128d c2 = _mm_set1_pd(c), s2 = _mm_set1_pd(s);
    __m128d sa = _mm_setzero_pd(), sb = _mm_setzero_pd();
    for( ; k <= n - 2; k += 2 )
    {
        __m128d a0 = _mm_load_pd(a + k);
        __m128d b0 = _mm_load_pd(b + k);
        __m128d t0 = _mm_add_pd(_mm_mul_pd(a0, c2), _mm_mul_pd(b0, s2));
        __m128d t1 = _mm_sub_pd(_mm_mul_pd(b0, c2), _mm_mul_pd(a0, s2));
        _mm_store_pd(a + k, t0);
        _mm_store_pd(b + k, t1);
        sa = _mm_add_pd(sa, _mm_mul_pd(t0, t0));
        sb = _mm_add_pd(sb, _mm_mul_pd(t1, t1));
    }
    double abuf[2], bbuf[2];
    _mm_storeu_pd(abuf, sa);
    _mm_storeu_pd(bbuf, sb);
    *anorm = abuf[0] + abuf[1];
    *bnorm = bbuf[0] + bbuf[1];
    return k;
}
#endif


template<typename _Tp> void
jacobiSVDImpl_(_Tp* At, size_t astep, _Tp* _W, _Tp* Vt, size_t vstep,
               int m, int n, int n1, double minval, _Tp eps)
{
    VBLAS<_Tp> vblas;
    AutoBuffer<double> Wbuf(n);
    double* W = Wbuf;
    int i, j, k, iter, max_iter = MAX(m, 30);
    _Tp c, s;
    double sd;
    astep /= sizeof(At[0]);
    vstep /= sizeof(Vt[0]);

    for( i = 0; i < n; i++ )
    {
        for( k = 0, sd = 0; k < m; k++ )
        {
            _Tp t = At[i*astep + k];
            sd += (double)t*t;
        }
        W[i] = sd;

        if( Vt )
        {
            for( k = 0; k < n; k++ )
                Vt[i*vstep + k] = 0;
            Vt[i*vstep + i] = 1;
        }
    }

    for( iter = 0; iter < max_iter; iter++ )
    {
        bool changed = false;

        for( i = 0; i < n-1; i++ )
            for( j = i+1; j < n; j++ )
            {
                _Tp *Ai = At + i*astep, *Aj = At + j*astep;
                double a = W[i], p = 0, b = W[j];

                for( k = 0; k < m; k++ )
                    p += (double)Ai[k]*Aj[k];

                if( ABS(p) <= eps*sqrt((double)a*b) )
                    continue;

                p *= 2;
                double beta = a - b, gamma = hypot((double)p, beta);
                if( beta < 0 )
                {
                    double delta = (gamma - beta)*0.5;
                    s = (_Tp)sqrt(delta/gamma);
                    c = (_Tp)(p/(gamma*s*2));
                }
                else
                {
                    c = (_Tp)sqrt((gamma + beta)/(gamma*2));
                    s = (_Tp)(p/(gamma*c*2));
                }

                a = b = 0;
                for( k = 0; k < m; k++ )
                {
                    _Tp t0 = c*Ai[k] + s*Aj[k];
                    _Tp t1 = -s*Ai[k] + c*Aj[k];
                    Ai[k] = t0; Aj[k] = t1;

                    a += (double)t0*t0; b += (double)t1*t1;
                }
                W[i] = a; W[j] = b;

                changed = true;

                if( Vt )
                {
                    _Tp *Vi = Vt + i*vstep, *Vj = Vt + j*vstep;
                    k = vblas.givens(Vi, Vj, n, c, s);

                    for( ; k < n; k++ )
                    {
                        _Tp t0 = c*Vi[k] + s*Vj[k];
                        _Tp t1 = -s*Vi[k] + c*Vj[k];
                        Vi[k] = t0; Vj[k] = t1;
                    }
                }
            }
        if( !changed )
            break;
    }

    for( i = 0; i < n; i++ )
    {
        for( k = 0, sd = 0; k < m; k++ )
        {
            _Tp t = At[i*astep + k];
            sd += (double)t*t;
        }
        W[i] = sqrt(sd);
    }

    for( i = 0; i < n-1; i++ )
    {
        j = i;
        for( k = i+1; k < n; k++ )
        {
            if( W[j] < W[k] )
                j = k;
        }
        if( i != j )
        {
            std::swap(W[i], W[j]);
            if( Vt )
            {
                for( k = 0; k < m; k++ )
                    std::swap(At[i*astep + k], At[j*astep + k]);

                for( k = 0; k < n; k++ )
                    std::swap(Vt[i*vstep + k], Vt[j*vstep + k]);
            }
        }
    }

    for( i = 0; i < n; i++ )
        _W[i] = (_Tp)W[i];

    if( !Vt )
        return;

	Random::init();
    for( i = 0; i < n1; i++ )
    {
        sd = i < n ? W[i] : 0;

        while( sd <= minval )
        {
            // if we got a zero singular value, then in order to get the corresponding left singular vector
            // we generate a random vector, project it to the previously computed left singular vectors,
            // subtract the projection and normalize the difference.
            const _Tp val0 = (_Tp)(1./m);
            for( k = 0; k < m; k++ )
            {
//                _Tp val = (rng.next() & 256) != 0 ? val0 : -val0;
                _Tp val = (Random::uniform() & 256) != 0 ? val0 : -val0;
                At[i*astep + k] = val;
            }
            for( iter = 0; iter < 2; iter++ )
            {
                for( j = 0; j < i; j++ )
                {
                    sd = 0;
                    for( k = 0; k < m; k++ )
                        sd += At[i*astep + k]*At[j*astep + k];
                    _Tp asum = 0;
                    for( k = 0; k < m; k++ )
                    {
                        _Tp t = (_Tp)(At[i*astep + k] - sd*At[j*astep + k]);
                        At[i*astep + k] = t;
                        asum += ABS(t);
                    }
                    asum = asum ? 1/asum : 0;
                    for( k = 0; k < m; k++ )
                        At[i*astep + k] *= asum;
                }
            }
            sd = 0;
            for( k = 0; k < m; k++ )
            {
                _Tp t = At[i*astep + k];
                sd += (double)t*t;
            }
            sd = sqrt(sd);
        }

        s = (_Tp)(1/sd);
        for( k = 0; k < m; k++ )
            At[i*astep + k] *= s;
    }
}

static void JacobiSVD(double* At, size_t astep, double* W, double* Vt, size_t vstep, int m, int n, int n1=-1)
{
    jacobiSVDImpl_(At, astep, W, Vt, vstep, m, n, !Vt ? 0 : n1 < 0 ? n : n1, DBL_MIN, DBL_EPSILON*10);
}

void icomputeSVD( const Mat& src, Mat& w, Mat& u, Mat& vt, int )
{
    int m = src.rows(), n = src.cols();
    TYPE type = src.type1();
    bool compute_uv = true;//_u.needed() || _vt.needed();
    bool full_uv = true;// = (flags & SVD::FULL_UV) != 0;

    assert( type == MAT_Tfloat || type == MAT_Tdouble );
	/*
    if( flags & SVD::NO_UV )
    {
        _u.release();
        _vt.release();
        compute_uv = full_uv = false;
    }*/

    bool at = false;
    if( m < n )
    {
        std::swap(m, n);
        at = true;
    }

    int urows = full_uv ? m : n;
    size_t esz = src.elemSize(), astep = cvlib::alignSize(m*esz, 16), vstep = cvlib::alignSize(n*esz, 16);
    AutoBuffer<uchar> _buf(urows*astep + n*vstep + n*esz + 32);
	uchar* buf = (uchar*)cvlib::alignPtr((uchar*)_buf, 16);

	int m1 = (m+1)/2*2;
    Mat temp_a(buf, n, m1, type);
    Mat temp_w(buf+urows*astep, n, 1, type);
    Mat temp_u(buf, urows, m1, type);
	Mat temp_v;

    if( compute_uv )
	{
		int n1 = (n+1)/2*2;
		uchar* ptemp = (uchar*)alignPtr(buf + urows*astep + n*esz, 16);
		temp_v.create(ptemp, n, n1, type);
	}

    if( urows > n )
        temp_u = 0;

    if( !at ){
		//temp_a = src.transposed();
		int i, j;
		for (i=0; i<src.rows(); i++)
		{
			for (j=0; j<src.cols(); j++)
			{
				temp_a.data.db[j][i] = src.data.db[i][j];
			}
		}
	}
    else {
		//temp_a = src;
		int i, j;
		for (i=0; i<src.rows(); i++)
		{
			for (j=0; j<src.cols(); j++)
			{
				temp_a.data.db[i][j] = src.data.db[i][j];
			}
		}
	}

    if( type == MAT_Tfloat )
    {
        //JacobiSVD(temp_a.data.fl[0], temp_u.step, temp_w.ptr<float>(), temp_v.ptr<float>(), temp_v.step, m, n, compute_uv ? urows : 0);
    }
    else
    {
		JacobiSVD(temp_a.data.db[0], temp_u.cols()*sizeof(double), temp_w.data.db[0],
              temp_v.data.db[0], temp_v.cols()*sizeof(double), m, n, compute_uv ? urows : 0);
    }

    w = temp_w;
	temp_u.resizeCols (m);
	temp_v.resizeCols (n);
    if( compute_uv )
    {
        if( !at )
        {
			u = temp_u.transposed();//transpose(temp_u, _u);
			/*u.create (urows, m, type);
			for (i=0; i<n; i++)
			{
				for (j=0; j<n; j++)
				{
					vt.data.db[i][j] = temp_v.data.db[i][j];
				}
			}*/
			/*
			vt.create (n, n, type);
			for (i=0; i<n; i++)
			{
				for (j=0; j<n; j++)
				{
					vt.data.db[i][j] = temp_v.data.db[i][j];
				}
			}*/
			vt = temp_v;//temp_v.copyTo(_vt);
        }
        else
        {
			u = temp_v.transposed();//transpose(temp_v, _u);
			vt = temp_u;//.copyTo(_vt);
        }
    }
}

}