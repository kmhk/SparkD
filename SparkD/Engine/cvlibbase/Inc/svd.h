#pragma once

#include "Mat.h"

namespace cvlib
{

	template<typename _Tp> void
		jacobiSVDImpl_(_Tp* At, size_t astep, _Tp* _W, _Tp* Vt, size_t vstep,
			int m, int n, int n1, double minval, _Tp eps);

	CVLIB_DECLSPEC void icomputeSVD(const Mat& _aarr, Mat& _w, Mat& _u, Mat& _vt, int flags);

}