/*
 *  All rights reserved.
 *  , 
 */
/*!
 * \file	LineRansac.h
 * \brief	extract the line that contains the most points among the given points
 * \author	
 */


#pragma once

#include "Mat.h"

class SimpleLine
{
public:
	SimpleLine() { a = b = 0; }
	SimpleLine(const SimpleLine& t) {
		a = t.a; b = t.b;
	}
	SimpleLine& operator=(const SimpleLine& t) {
		a = t.a; b = t.b;
		return *this;
	}
	float ypos(float x) const {
		return x*a + b;
	}
	bool createFrom(const cvlib::vec2f& sPoint1, const cvlib::vec2f& sPoint2) {
		float rDx, rDy;

		rDx = sPoint2.x - sPoint1.x;
		rDy = sPoint2.y - sPoint1.y;

		if (ABS(rDx) < FLT_EPSILON)
		{
			a = 0.f;
			b = sPoint1.x;
			return false;
		}

		a = rDy / rDx;
		b = sPoint1.y - a * sPoint1.x;
		return true;
	}
	bool createFrom(const cvlib::vec2i& sPoint1, const cvlib::vec2i& sPoint2) {
		int rDx, rDy;

		rDx = sPoint2.x - sPoint1.x;
		rDy = sPoint2.y - sPoint1.y;

		if (ABS(rDx) < FLT_EPSILON)
		{
			a = 0.f;
			b = sPoint1.x;
			return false;
		}

		a = (float)rDy / (float)rDx;
		b = sPoint1.y - a * sPoint1.x;
		return true;
	}
	float a, b;
};

class CVLIB_DECLSPEC LineRansac
{
public:
	LineRansac();
	LineRansac(float rErrorTh);
	~LineRansac ();

	bool create(float rErrorTh);
	void release();
	bool process(const cvlib::Point2f* psPoint, int nNum, SimpleLine& line, cvlib::Vector<int>& vInlier, int mode = 0);
	bool processFast(const cvlib::Point2i* psPoint, int nNum, SimpleLine& line);
	bool processFast(const cvlib::Point2f* psPoint, int nNum, SimpleLine& line);

private:
	bool GetLineEquationByLsq(const cvlib::Point2f* psPoint, const cvlib::Vector<int>& vmaxInlier, SimpleLine& line, cvlib::Vector<int>& vInlier) const;
	bool GetLineEquationByLsq(const cvlib::Point2f* psPoint, int num, const SimpleLine& maxline, SimpleLine& line) const;
	bool GetLineEquationByLsq(const cvlib::Point2i* psPoint, int num, const SimpleLine& maxline, SimpleLine& line) const;
	float FindInliers(const cvlib::Point2f* psPoint, int nNum, const SimpleLine& line, cvlib::Vector<int>& vInlier) const;
	float FindInliers(const cvlib::Point2i* psPoint, int nNum, const SimpleLine& line, cvlib::Vector<int>& vInlier) const;
	float FindInliers(const cvlib::Point2f* psPoint, int nNum, const SimpleLine& line, int& inlier_count) const;
	float FindInliers(const cvlib::Point2i* psPoint, int nNum, const SimpleLine& line, int& inlier_count) const;
private:
	float m_rErrorTh;
};

CVLIB_DECLSPEC SimpleLine createSimpleLineFromRegions(const cvlib::Vector<cvlib::vec2f>& vpoints, float rerr = 5.f, int* pvalid = 0, int mode = 0);
CVLIB_DECLSPEC SimpleLine createSimpleLineFromRegions(const cvlib::Vector<cvlib::vec2i>& vpoints, float rerr = 5.f, int* pvalid = 0, int mode = 0);
CVLIB_DECLSPEC SimpleLine createSimpleLineFromRegions(const cvlib::Vector<cvlib::Rect>& vregions, int* pvalid, int mode = 0);
CVLIB_DECLSPEC SimpleLine createFastSimpleLineFromRegions(const cvlib::Vector<cvlib::vec2f>& vpoints, float rerr = 5.f);
CVLIB_DECLSPEC SimpleLine createFastSimpleLineFromRegions(const cvlib::Vector<cvlib::vec2i>& vpoints, float rerr = 5.f);
