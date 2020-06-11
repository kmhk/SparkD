#include "zvd.h"
#include "Warping.h"

namespace cvlib
{


	TiltedRect::TiltedRect() {
	}
	TiltedRect::TiltedRect(const Rect& r) {
		m_pts[0] = Point2i(r.x, r.y);
		m_pts[1] = Point2i(r.limx(), r.y);
		m_pts[2] = Point2i(r.limx(), r.limy());
		m_pts[3] = Point2i(r.x, r.limy());
	}
	TiltedRect::TiltedRect(const TiltedRect& r) {
		m_pts[0] = r.m_pts[0];
		m_pts[1] = r.m_pts[1];
		m_pts[2] = r.m_pts[2];
		m_pts[3] = r.m_pts[3];
	}
	TiltedRect::TiltedRect(const Point2i _pts[4]) {
		m_pts[0] = _pts[0];
		m_pts[1] = _pts[1];
		m_pts[2] = _pts[2];
		m_pts[3] = _pts[3];
	}

	TiltedRect::~TiltedRect() {
	}
	Rect TiltedRect::getBoundRect() const {
		Rect r(m_pts[0].x, m_pts[0].y, m_pts[0].x, m_pts[0].y);
		for (int i = 1; i < 4; i++) {
			r.x = MIN(r.x, m_pts[i].x);
			r.y = MIN(r.y, m_pts[i].y);
			r.width = MAX(r.width, m_pts[i].x);
			r.height = MAX(r.height, m_pts[i].y);
		}
		r.width = r.width - r.x + 1;
		r.height = r.height - r.y + 1;
		return r;
	}

	void TiltedRect::drawTo(Mat& image, COLOR color) const 
	{
		for (int i = 0; i < 4; i++)
		{
			vec2i p = m_pts[i];
			vec2i p2 = m_pts[(i + 1) % 4];
			image.drawLine(p, p2, color);
		}
	}
	void TiltedRect::rotateRegion()
	{
		int  t;
		for (int i = 0; i < 4; i++)
		{
			t = m_pts[i].x; m_pts[i].x = m_pts[i].y; m_pts[i].y = t;
		}
	}
	void TiltedRect::scale(float s) {
		for (int i = 0; i < 4; i++)
		{
			m_pts[i].x = (int)(m_pts[i].x*s);
			m_pts[i].y = (int)(m_pts[i].y*s);
		}
	}
	void TiltedRect::moveXY(int x, int y) {
		for (int i = 0; i < 4; i++)
		{
			m_pts[i].x += x;
			m_pts[i].y += y;
		}
	}
	void TiltedRect::transform(const Mat& w) {
		Vector<Point2i> points(m_pts, 4, false);
		Vector<Point2i> dstPoints;
		ip::warpPoint(points, dstPoints, w);
		memcpy(m_pts, &dstPoints[0], sizeof(Point2i) * 4);
	}
	void TiltedRect::flipLR(int w){
		for (int i = 0; i < 4; i++) {
			m_pts[i].x = w - m_pts[i].x - 1;
		}
	}


}