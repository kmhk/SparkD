#pragma once

#include "cvlibbase/Inc/cvlibstructs.h"
#include "cvlibbase/Inc/Vec.h"
#include "cvlibbase/Inc/Template.h"

namespace cvlib
{

	class utils
	{
	public:
		static void sortPoints(Vector<Point2i>& points, bool fx);
		static void sortPoints(const Vector<Point2i>& points, Point2i& minPt, Point2i& maxPt);

		static void makeBinomial(int degree, Veci& vbinomial);
		static Rect rotateRegion(const Rect& r);

		static void xyzcolor(int* colors);
		static COLOR xyzcolor(COLOR color);
		static int colordist(int* color1, int* color2);
		static void calcWidthHeight(Point2i* points, float& width, float& height);
		static COLOR	decideColor(const Vecf** pvs, int cn, int startx, int lastx);
		static Vecf makeStdSignal(const Vecf& vin, int side);
	};

}