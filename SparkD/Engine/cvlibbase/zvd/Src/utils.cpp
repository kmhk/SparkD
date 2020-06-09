#include "utils.h"

namespace cvlib
{


static int compareX(const void* a, const void* b)
{
	Point2i* p1=(Point2i*)a;
	Point2i* p2=(Point2i*)b;
	return p1->x - p2->x;
}
static int compareY(const void* a, const void* b)
{
	Point2i* p1=(Point2i*)a;
	Point2i* p2=(Point2i*)b;
	return p1->y - p2->y;
}
void utils::sortPoints(Vector<Point2i>& points, bool fx)
{
	if (fx)
	{
		qsort (points.getData(), points.getSize(), sizeof(Point2i), compareX);
	}
	else
	{
		qsort (points.getData(), points.getSize(), sizeof(Point2i), compareY);
	}
}
void utils::sortPoints(const Vector<Point2i>& points, Point2i& minPt, Point2i& maxPt)
{
	int i;
	minPt = points[0];
	maxPt = points[0];
	for (i=1; i<points.getSize(); i++)
	{
		if (minPt.x > points[i].x)
			minPt = points[i];
		if (maxPt.x < points[i].x)
			maxPt = points[i];
	}
}

void utils::makeBinomial(int degree, Veci& vbinomial)
{
	static  int anstd[] = {1,2,1};
	Veci v(anstd, 3);
	vbinomial = v;
	if (degree <= 2)
		return;

	for (int i=3; i<=degree; i++)
	{
		Veci v2(i+1);
		v2[0] = v[0];
		for (int k=1; k<i; k++)
			v2[k] = v[k]+v[k-1];
		v2[i] = v[i-1];
		v = v2;
	}
	vbinomial = v;
}
Rect utils::rotateRegion(const Rect& r)
{
	Rect ret;
	ret.x = r.y;
	ret.y = r.x;
	ret.width = r.height;
	ret.height = r.height;
	return ret;
}
void utils::xyzcolor(int* colors)
{
/*	int i=0;
	int sum = colors[i]+colors[i+1]+colors[i+2];
	colors[i] = (colors[i] * 255 / sum);
	colors[i+1] = (colors[i+1] * 255 / sum);
	colors[i+2] = (colors[i+2] * 255 / sum);*/
}
COLOR utils::xyzcolor(COLOR color)
{
	int colors[3];
	colors[0] = color.x;
	colors[1] = color.y;
	colors[2] = color.z;
	int sum = colors[0]+colors[1]+colors[2];
	if (sum > 0)
	{
		for (int i=0; i<3; i++)
			colors[i] = (colors[i] * 255 / sum);
	}
	return COLOR(colors[0], colors[1], colors[2]);
}
int utils::colordist(int* color1, int* color2)
{
	return ABS(color1[0]-color2[0])+ABS(color1[1]-color2[1])+ABS(color1[2]-color2[2]);
}

void utils::calcWidthHeight(Point2i* points, float& width, float& height)
{
	width = (float)((points[0].distTo(points[1])+points[2].distTo(points[3]))/2.0f);
	height = (float)((points[0].distTo(points[3])+points[2].distTo(points[1]))/2.0f);
}

COLOR	utils::decideColor(const Vecf** pvs, int cn, int startx, int lastx)
{
	// mean color
	int len = lastx - startx;
	Vecf vr(len);
	Vecf vg(len);
	Vecf vb(len);
	for (int i = 0; i < len; i++)
	{
		vr[i] = pvs[0]->data.fl[i + startx];
		vg[i] = pvs[1]->data.fl[i + startx];
		vb[i] = pvs[2]->data.fl[i + startx];
	}
	uchar r = (uchar)vr.median();
	uchar g = (uchar)vg.median();
	uchar b = (uchar)vb.median();
	return COLOR(r, g, b);
}

Vecf utils::makeStdSignal(const Vecf& vin, int side)
{
	Vecf vextent(vin.length() + side * 2);
	int i;
	for (i = 0; i < side; i++)
		vextent[i] = vin[0];
	for (; i < vin.length() + side; i++)
		vextent[i] = vin[i - side];
	for (; i < vin.length() + side * 2; i++)
		vextent[i] = vin[vin.length() - 1];

	int len = vextent.length();
	Vecf vsum(len + 1);
	Vecd vsqsum(len + 1);
	vsum = 0;
	vsqsum = 0;

	for (int k = 0; k < len; k++) {
		vsum[k + 1] = vsum[k] + vextent[k];
		vsqsum[k + 1] = vsqsum[k] + vextent[k] * vextent[k];
	}
	int count = side * 2;
	Vecf vstd(vin.length()); vstd = 0;
	for (int k = side; k < len - side; k++) {
		double rsum = vsum[k + side] - vsum[k - side];
		double rvar = vsqsum[k + side] - vsqsum[k - side];
		rsum /= count;
		rvar /= count;
		vstd[k - side] = (float)sqrt(MAX(0, rvar - rsum*rsum));
	}
	return vstd;
}

}