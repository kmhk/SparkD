#include "cvlibbase/Inc/_cvlibbase.h"

namespace cvlib { namespace ip {

CVLIB_DECLSPEC RotatedRect fitEllipse( const Vector<Point2i>& points )
{
	int i, n = points.getSize();

    RotatedRect box;

    if( n < 5 )
		return box; //"There should be at least 5 points to fit the ellipse"

    // New fitellipse algorithm, contributed by Dr. Daniel Weiss
    Point2f c(0,0);
    double gfp[5], rp[5], t;
    const double min_eps = 1e-8;
	const Point2i* ptsi = points.getData();

    AutoBuffer<double> _Ad(n*5), _bd(n);
    double *Ad = _Ad, *bd = _bd;

    // first fit for parameters A - E
    Mat A( Ad, n, 5, MAT_Tdouble );
    Mat b( bd, n, 1, MAT_Tdouble );
    Mat x( gfp, 5, 1, MAT_Tdouble );

    for( i = 0; i < n; i++ )
    {
        Point2f p = Point2f((float)ptsi[i].x, (float)ptsi[i].y);
        c += p;
    }
    c.x /= n;
    c.y /= n;

    for( i = 0; i < n; i++ )
    {
        Point2f p = Point2f((float)ptsi[i].x, (float)ptsi[i].y);
        p -= c;

        bd[i] = 10000.0; // 1.0?
        Ad[i*5] = -(double)p.x * p.x; // A - C signs inverted as proposed by APP
        Ad[i*5 + 1] = -(double)p.y * p.y;
        Ad[i*5 + 2] = -(double)p.x * p.y;
        Ad[i*5 + 3] = p.x;
        Ad[i*5 + 4] = p.y;
    }

    cvutil::solve(A, b, x, DECOMP_SVD);

    // now use general-form parameters A - E to find the ellipse center:
    // differentiate general form wrt x/y to get two equations for cx and cy
    A.create( Ad, 2, 2, MAT_Tdouble );
    b.create( bd, 2, 1, MAT_Tdouble);
    x.create( rp, 2, 1, MAT_Tdouble);
    Ad[0] = 2 * gfp[0];
    Ad[1] = Ad[2] = gfp[2];
    Ad[3] = 2 * gfp[1];
    bd[0] = gfp[3];
    bd[1] = gfp[4];
    cvutil::solve( A, b, x, DECOMP_SVD );

    // re-fit for parameters A - C with those center coordinates
    A.create( Ad, n, 3, MAT_Tdouble);
    b.create( bd, n, 1, MAT_Tdouble);
    x.create( gfp, 3, 1, MAT_Tdouble);
    for( i = 0; i < n; i++ )
    {
        Point2f p = Point2f((float)ptsi[i].x, (float)ptsi[i].y);
        p -= c;
        bd[i] = 1.0;
        Ad[i * 3] = (p.x - rp[0]) * (p.x - rp[0]);
        Ad[i * 3 + 1] = (p.y - rp[1]) * (p.y - rp[1]);
        Ad[i * 3 + 2] = (p.x - rp[0]) * (p.y - rp[1]);
    }
    cvutil::solve(A, b, x, DECOMP_SVD);

    // store angle and radii
    rp[4] = -0.5 * atan2(gfp[2], gfp[1] - gfp[0]); // convert from APP angle usage
    if( fabs(gfp[2]) > min_eps )
        t = gfp[2]/sin(-2.0 * rp[4]);
    else // ellipse is rotated by an integer multiple of pi/2
        t = gfp[1] - gfp[0];
    rp[2] = fabs(gfp[0] + gfp[1] - t);
    if( rp[2] > min_eps )
        rp[2] = ::sqrt(2.0 / rp[2]);
    rp[3] = fabs(gfp[0] + gfp[1] + t);
    if( rp[3] > min_eps )
        rp[3] = ::sqrt(2.0 / rp[3]);

    box.center.x = (float)rp[0] + c.x;
    box.center.y = (float)rp[1] + c.y;
    box.size.width = (float)(rp[2]*2);
    box.size.height = (float)(rp[3]*2);
    if( box.size.width > box.size.height )
    {
        float tmp = box.size.width;
		box.size.width = box.size.height;
		box.size.height = tmp;
        box.angle = (float)(90 + rp[4]*180/CVLIB_PI);
    }
    if( box.angle < -180 )
        box.angle += 360;
    if( box.angle > 360 )
        box.angle -= 360;

    return box;
}

}}
