
/*! 
 * \file cvlibstructs.cpp
 * \ingroup 
 * \brief 
 * \author
 */

#include "cvlibstructs.h"

namespace cvlib
{

	TermCriteria TermCriteria::check(double default_eps, int default_max_iters) const
	{
		TermCriteria crit;

		crit.nType = TermCriteria::MAX_ITER | TermCriteria::EPS;
		crit.nMaxIter = default_max_iters;
		crit.rEpsilon = (float)default_eps;

		if ((nType & ~(TermCriteria::EPS | TermCriteria::MAX_ITER)) != 0)
			assert(0);

		if ((nType & TermCriteria::MAX_ITER) != 0)
		{
			if (nMaxIter <= 0)
				assert(0);
			crit.nMaxIter = nMaxIter;
		}

		if ((nType & TermCriteria::EPS) != 0)
		{
			if (rEpsilon < 0)
				assert(0);

			crit.rEpsilon = rEpsilon;
		}

		if ((nType & (TermCriteria::EPS | TermCriteria::MAX_ITER)) == 0)
			assert(0);

		crit.rEpsilon = (float)MAX(0, crit.rEpsilon);
		crit.nMaxIter = MAX(1, crit.nMaxIter);

		return crit;
	}


	RotatedRect::RotatedRect() :angle(0.0f) {}
	RotatedRect::RotatedRect(const Point2f& _center, const Sizef& _size, float _angle) : center(_center), size(_size), angle(_angle) {}
	RotatedRect::RotatedRect(const RotatedRect& box) : center(box.center), size(box.size), angle(box.angle) {}
	void RotatedRect::points(Point2f pts[]) const
	{
		double _angle = angle*CVLIB_PI / 180.;
		float b = (float)cos(_angle)*0.5f;
		float a = (float)sin(_angle)*0.5f;
		float height = size.height;
		float width = size.width;
		pts[0].x = center.x - a*height - b*width;
		pts[0].y = center.y + b*height - a*width;
		pts[1].x = center.x + a*height - b*width;
		pts[1].y = center.y - b*height - a*width;
		pts[2].x = 2 * center.x - pts[0].x;
		pts[2].y = 2 * center.y - pts[0].y;
		pts[3].x = 2 * center.x - pts[1].x;
		pts[3].y = 2 * center.y - pts[1].y;
	}
	Rect RotatedRect::boundingRect() const
	{
		Point2f pt[4];
		points(pt);
		Rect r((int)floor(MIN(MIN(MIN(pt[0].x, pt[1].x), pt[2].x), pt[3].x)),
			(int)floor(MIN(MIN(MIN(pt[0].y, pt[1].y), pt[2].y), pt[3].y)),
			(int)ceil(MAX(MAX(MAX(pt[0].x, pt[1].x), pt[2].x), pt[3].x)),
			(int)ceil(MAX(MAX(MAX(pt[0].y, pt[1].y), pt[2].y), pt[3].y)));
		r.width -= r.x - 1;
		r.height -= r.y - 1;
		return r;
	}

}