#pragma once

#include "_cvlibbase.h"

namespace cvlib
{

	void autocorrelation(const Vecf& vs, int len, Vecd& vcorr);
	int detectCycle(const Vecf& v, int startx, int endx);
	void drawSignal(const Vecf& v, Mat& drawImage, COLOR color, bool fnormal = false);
	void drawSignal(const Vecf& v, Mat& drawImage, COLOR color, int start, int end, COLOR color2, bool fnormal = false);
	void drawSignal(const Vecf& v, Mat& drawImage, COLOR color, const Vecf& v2, COLOR color2, bool fnormal = false);
	void drawSignal(const Vecd& v, Mat& drawImage, COLOR color, bool fnormal = false);
	void makeTriangleFileter(Vecf& v, int len);
	void makeTriangleFileter2(Vecf& v, int len);
	void filteringTriangle(const Vecf& vIn, Vecf& vOut, const float* pnFilter, int nFilterlen);

}
