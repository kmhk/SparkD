#include "cvlibbase/Inc/_cvlibbase.h"
#include "signalprocess.h"

namespace cvlib
{

void autocorrelation(const Vecf& vs, int len, Vecd& vcorr)
{
	vcorr.resize(vs.length());
	vcorr = 0;
	for (int i=0; i<len; i++)
	{
		vcorr[i] = (float)vs.autoCorrelation(i+1);
	}
}

void drawSignal(const Vecf& v, Mat& drawImage, COLOR color, bool fnormal)
{
	int len = v.length();
	if (!fnormal)
	{
		for (int i = 1; i < len; i++)
			drawImage.drawLine(Point2i(i - 1, (int)v[i - 1]), Point2i(i, (int)v[i]), color);
	}
	else
	{
		Vecf v_(len);
		double minv = v.min();
		double maxv = v.max();
		if (maxv - minv < 0.000001)
			return;
		double scale = 255.0f / (maxv - minv);
		for (int i = 0; i < len; i++)
			v_[i] = (float)((v[i] - minv)*scale);
		for (int i = 1; i < len; i++)
			drawImage.drawLine(Point2i(i - 1, (int)v_[i - 1]), Point2i(i, (int)v_[i]), color);
	}
}
void drawSignal(const Vecf& v, Mat& drawImage, COLOR color, const Vecf& v2, COLOR color2, bool fnormal)
{
	int len = v.length();
	if (!fnormal)
	{
		for (int i = 1; i < len; i++)
			drawImage.drawLine(Point2i(i - 1, (int)v[i - 1]), Point2i(i, (int)v[i]), color);
	}
	else
	{
		Vecf v_(len);
		Vecf v2_(len);
		double minv = v.min();
		double maxv = v.max();
		if (maxv - minv < 0.000001)
			return;
		double scale = 255.0f / (maxv - minv);
		for (int i = 0; i < len; i++) {
			v_[i] = (float)((v[i] - minv)*scale);
			v2_[i] = (float)((v2[i] - minv)*scale);
			if (v2_[i] < 0) v2_[i] = 0;
			if (v2_[i] > 255) v2_[i] = 255;
		}
		for (int i = 1; i < len; i++)
			drawImage.drawLine(Point2i(i - 1, (int)v_[i - 1]), Point2i(i, (int)v_[i]), color);
		for (int i = 1; i < len; i++)
			drawImage.drawLine(Point2i(i - 1, (int)v2_[i - 1]), Point2i(i, (int)v2_[i]), color2);
	}
}
void drawSignal(const Vecf& v, Mat& drawImage, COLOR color, int start, int end, COLOR color2, bool fnormal)
{
	int len = v.length();
	if (!fnormal)
	{
		int i;
		for (i = 1; i < start + 1; i++)
			drawImage.drawLine(Point2i(i - 1, (int)v[i - 1]), Point2i(i, (int)v[i]), color);
		for (; i < end; i++)
			drawImage.drawLine(Point2i(i - 1, (int)v[i - 1]), Point2i(i, (int)v[i]), color2);
		for (; i < len; i++)
			drawImage.drawLine(Point2i(i - 1, (int)v[i - 1]), Point2i(i, (int)v[i]), color);
	}
	else
	{
		Vecf v_(len);
		double minv = v.min();
		double maxv = v.max();
		if (maxv - minv < 0.000001)
			return;
		double scale = 255.0f / (maxv - minv);
		for (int i = 0; i < len; i++)
			v_[i] = (float)((v[i] - minv)*scale);

		int i;
		for (i = 1; i < start + 1; i++)
			drawImage.drawLine(Point2i(i - 1, (int)v_[i - 1]), Point2i(i, (int)v_[i]), color);
		for (; i < end; i++)
			drawImage.drawLine(Point2i(i - 1, (int)v_[i - 1]), Point2i(i, (int)v_[i]), color2);
		for (; i < len; i++)
			drawImage.drawLine(Point2i(i - 1, (int)v_[i - 1]), Point2i(i, (int)v_[i]), color);
	}
}

void drawSignal(const Vecd& v, Mat& drawImage, COLOR color, bool fnormal)
{
	double minv, maxv, scale;
	int len = v.length();
	if (!fnormal)
	{
		for (int i=1; i<len; i++)
			drawImage.drawLine(Point2i(i-1,(int)v[i-1]), Point2i(i,(int)v[i]), color);
	}
	else
	{
		Vecf v_(len);
		minv = v.min();
		maxv = v.max();
		if (maxv-minv < 0.000001)
			return;
		scale = 255.0f/(maxv-minv);
		for (int i=0; i<len; i++)
			v_[i] = (float)((v[i]-minv)*scale);
		for (int i=1; i<len; i++)
			drawImage.drawLine(Point2i(i-1,(int)v_[i-1]), Point2i(i,(int)v_[i]), color);
	}
}

int detectCycle(const Vecf& v, int startx, int endx)
{
	Vecd vcorr;
	autocorrelation(v, endx + 1, vcorr);
	Vec* vsub = vcorr.subVec(startx, endx);
	int iposmax;
	vsub->max(iposmax);
	iposmax += startx;
	delete vsub;
#ifdef IPDEBUG
	Mat mdraw = Mat::zeros(256, v.length(), MAT_Tuchar3);
	drawSignal(vcorr, mdraw, COLOR(255, 255, 255), true);
	mdraw.drawLine(Point2i(iposmax, 0), Point2i(iposmax, mdraw.rows()), COLOR(255, 0, 0));
	IPDebug::addDebug(mdraw);
#endif
	return iposmax;
}

void makeTriangleFileter(Vecf& v, int len)
{
	v.resize(len);
	int side = len/2;
	int i;
	for (i=0; i<side; i++)
		v[i] = (float)(i+1);
	for (;i<len; i++)
		v[i] = (float)(len-i);
	double sum = v.sum();
	v = v*(1.0/sum);
}
void makeTriangleFileter2(Vecf& v, int len)
{
	v.resize(len);
	int side = len / 2;
	int i;
	for (i = 0; i<side; i++)
		v[i] = (float)(i + 1);
	for (; i<len; i++)
		v[i] = (float)(len - i);
	double sum = v.sum();
	v = v*(1.0 / sum);
}
void filteringTriangle (const Vecf& vIn, Vecf& vOut, const float* pnFilter, int nFilterlen)
{
	int nVecLen=vIn.length();
	int i;
	vOut=vIn;
	vOut = 0;
	int nSide=nFilterlen/2;
	for (i = 0; i < nSide; i++)
	{
		for (int k = 0; k < nFilterlen; k++)
			vOut[i] += pnFilter[k] * vIn[MAX(0, i - nSide + k)];
	}
	for (i = nSide; i < nVecLen - nSide; i++)
	{
		for (int k = 0; k < nFilterlen; k++)
			vOut[i] += pnFilter[k] * vIn[i - nSide + k];
	}
	for (; i < nVecLen; i++)
	{
		for (int k = 0; k < nFilterlen; k++)
			vOut[i] += pnFilter[k] * vIn[MIN(i - nSide + k, nVecLen - 1)];
	}
}
/*void filtering (const Vecf& vin, Vecf& vout, int filterlen)
{
	vout = vin;
	int len=vin.length();
	int i;
	vout=0;
	int side=filterlen/2;
	for (i=side; i<len-side; i++)
	{
		for (int k=0; k<filterlen; k++)
			vut[i]+=pnFilter[k]*vIn[i-nSide+k];
	}
}*/

}