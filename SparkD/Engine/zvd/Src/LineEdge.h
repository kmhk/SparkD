#pragma once

#include "cvlibbase/Inc/_cvlibbase.h"

namespace cvlib
{

	class CVLIB_DECLSPEC LineEdge
	{
	public:
		Point2i firstPt, lastPt;
		int angle;
		int classno;
		const LineEdge *p1;
		const LineEdge *p2;
	public:
		LineEdge(int n = 0);
		~LineEdge();
		LineEdge& operator=(const LineEdge& t);
		int xpos() const { return (firstPt.x + lastPt.x) / 2; }
		int ypos() const { return (firstPt.y + lastPt.y) / 2; }
		int length() { return (int)(firstPt.distTo(lastPt) + 0.5f); }
		bool createFromPoints(const Vector<Point2i>& points, bool fx);
		Point2i Cross(const LineEdge& t) const;
		int Angle() const;
		int BetweenAngle(const LineEdge& t) const;
	};

	LineEdge createLineEdgeFromPoints(const Vector<Point2i>& points, bool fx);
	void sortPoints(Vector<Point2i>& points, bool fx);

}