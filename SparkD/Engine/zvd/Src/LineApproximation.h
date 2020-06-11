#pragma once

#include "cvlibbase/Inc/_cvlibbase.h"

namespace cvlib
{

	class LineApproximation
	{
	public:
		LineApproximation();
		LineApproximation(const LineApproximation& t);
		LineApproximation& operator=(const LineApproximation& t);

		bool buildFrom(const Vector<vec2i>& points, bool fx);
		bool buildFrom(const Vector<vec2f>& points, bool fx);
		bool buildFrom(const Vecf& vsignal, int start, int end);
		float eval(int pos) const { return (float)(b + k*pos); };
		bool IsValid() { return m_fvalid; }
		float k, b;
	private:
		bool m_fvalid;
	};

}