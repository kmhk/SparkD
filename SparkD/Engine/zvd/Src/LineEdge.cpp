#include "LineEdge.h"
#include "Contours.h"
#include "Regressor.h"
#include "RegressorTrainer.h"

//int gcard_land;

namespace cvlib
{

#define DEF_MINDIST		7.0f
#define DEF_MINDIST2	5.0f
//#define DEF_YPOS		0.07f
#define DEF_YPOS		0.15f
#define DEF_XPOS		0.15f
#define DEF_MINSLOPE	10
#define DEF_CARDRATIO	1.626f
#define MAX_EDGE_COUNT	6

static int compareX(const void* a, const void* b)
{
	Point2i* p1 = (Point2i*)a;
	Point2i* p2 = (Point2i*)b;
	return p1->x - p2->x;
}
static int compareY(const void* a, const void* b)
{
	Point2i* p1 = (Point2i*)a;
	Point2i* p2 = (Point2i*)b;
	return p1->y - p2->y;
}
void sortPoints(Vector<Point2i>& points, bool fx)
{
	if (fx)
	{
		qsort(points.getData(), points.getSize(), sizeof(Point2i), compareX);
	}
	else
	{
		qsort(points.getData(), points.getSize(), sizeof(Point2i), compareY);
	}
}
	
LineEdge::LineEdge(int n) { angle = 0; classno = 0; p1 = 0; p2 = 0; }
LineEdge::~LineEdge() {}
LineEdge& LineEdge::operator=(const LineEdge& t) {
	firstPt = t.firstPt; lastPt = t.lastPt;
	angle = t.angle; classno = t.classno;//pt=t.pt; 
	p1 = t.p1; p2 = t.p2;
	return *this;
}
bool LineEdge::createFromPoints(const Vector<Point2i>& points, bool fX) 
{
	DataSet data(1, points.getSize());
	int i;
	if (fX)
	{
		for (i = 0; i<points.getSize(); i++)
		{
			const Point2i& point = points[i];
			data.m_pprData[i][0] = point.x;
			data.m_prCls[i] = point.y;
		}
	}
	else
	{
		for (i = 0; i<points.getSize(); i++)
		{
			const Point2i& point = points[i];
			data.m_pprData[i][0] = point.y;
			data.m_prCls[i] = point.x;
		}
	}
	LinearRegressorTrainer trainer;
	LinearRegressor* pRegressor = (LinearRegressor*)trainer.train(&data);
	if (!pRegressor)
		return false;
	if (fX)
	{
		Vecf vt(1); vt[0] = 0;
		vt[0] = (float)points[0].x; firstPt.x = (int)vt[0];
		firstPt.y = (int)pRegressor->eval(&vt);
		vt[0] = (float)points[points.getSize() - 1].x; lastPt.x = (int)vt[0];
		lastPt.y = (int)pRegressor->eval(&vt);
	}
	else
	{
		Vecf vt(1); vt[0] = 0;
		vt[0] = (float)points[0].y; firstPt.y = (int)vt[0];
		firstPt.x = (int)pRegressor->eval(&vt);
		vt[0] = (float)points[points.getSize() - 1].y; lastPt.y = (int)vt[0];
		lastPt.x = (int)pRegressor->eval(&vt);
	}
	delete pRegressor;
	return true;
}
Point2i LineEdge::Cross(const LineEdge& t) const
{
	Point2f pt = ip::crossPointTwoLines(firstPt, lastPt, t.firstPt, t.lastPt);
	return Point2i((int)pt.x, (int)pt.y);
}
int LineEdge::Angle() const {
	int a1 = ((int)(atan2((double)lastPt.y - firstPt.y, lastPt.x - firstPt.x) / CVLIB_PI * 180) + 180) % 180;
	return a1;
}
int LineEdge::BetweenAngle(const LineEdge& t) const {
	int n1 = Angle();
	int n2 = t.Angle();
	int n = n1 - n2;
	return MIN(ABS(n), ABS(180 - n));
}

LineEdge createLineEdgeFromPoints(const Vector<Point2i>& points, bool fx)
{
	LineEdge edge;
	edge.createFromPoints(points, fx);
	return edge;
}

}