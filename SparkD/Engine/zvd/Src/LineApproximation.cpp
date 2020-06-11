#include "LineApproximation.h"

namespace cvlib
{

	LineApproximation::LineApproximation() { m_fvalid = false; k = b = 0; }
	LineApproximation::LineApproximation(const LineApproximation& t) {
		m_fvalid = t.m_fvalid;
		k = t.k;
		b = t.b;
	}
	LineApproximation& LineApproximation::operator=(const LineApproximation& t) {
		m_fvalid = t.m_fvalid;
		k = t.k;
		b = t.b;
		return *this;
	}

	bool LineApproximation::buildFrom(const Vector<vec2i>& points, bool fx)
	{
		int _sx = 0, _sy = 0, _sxy = 0;
		int _sumx = 0, _sumy = 0;
		int count = points.getSize();
		for (int i = 0; i < count; i++)
		{
			const vec2i& p = points[i];
			_sumx += p.x;
			_sumy += p.y;
			if (fx)
				_sx += p.x*p.x;
			else
				_sy += p.y*p.y;
			_sxy += p.x*p.y;
		}
		double r = 1.0 / count;
		double sx = _sx - r*_sumx*_sumx;
		double sy = _sy - r*_sumy*_sumy;
		double sxy = _sxy - r*_sumx*_sumy;
		double sumx = _sumx*r;
		double sumy = _sumy*r;
	
		if (fx) {
			if (sx == 0)
				m_fvalid = false;
			else
				m_fvalid = true;

			k = (float)(sxy / sx);
			b = (sumy - k*sumx);
		}
		else {
			if (sy == 0)
				m_fvalid = false;
			else
				m_fvalid = true;

			k = (float)(sxy / sy);
			b = (sumx - k*sumy);
		}
		return m_fvalid;
	}
	bool LineApproximation::buildFrom(const Vector<vec2f>& points, bool fx)
	{
		int _sx = 0, _sy = 0, _sxy = 0;
		int _sumx = 0, _sumy = 0;
		int count = points.getSize();
		for (int i = 0; i < count; i++)
		{
			const vec2f& p = points[i];
			_sumx += p.x;
			_sumy += p.y;
			if (fx)
				_sx += p.x*p.x;
			else
				_sy += p.y*p.y;
			_sxy += p.x*p.y;
		}
		double r = 1.0 / count;
		double sx = _sx - r*_sumx*_sumx;
		double sy = _sy - r*_sumy*_sumy;
		double sxy = _sxy - r*_sumx*_sumy;
		double sumx = _sumx*r;
		double sumy = _sumy*r;

		if (fx) {
			if (sx == 0)
				m_fvalid = false;
			else
				m_fvalid = true;

			k = (float)(sxy / sx);
			b = (sumy - k*sumx);
		}
		else {
			if (sy == 0)
				m_fvalid = false;
			else
				m_fvalid = true;

			k = (float)(sxy / sy);
			b = (sumx - k*sumy);
		}
		return m_fvalid;
	}
	bool LineApproximation::buildFrom(const Vecf& vsignal, int start, int end)
	{
		Vector<vec2f> points;
		for (int i = start; i < end; i++)
		{
			points.add(vec2f(i, vsignal[i]));
		}
		return buildFrom(points, true);
	}

}