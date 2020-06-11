#pragma once

#include "zvd.h"
#include "ObjectBoostDetectorHaar.h"
#include "CascadeClassifierObjectHaar.h"
#include "ZVDRegionDetector.h"

namespace cvlib
{

	class points4 {
	public:
		vec2i pts[4];
		points4() {}
		points4(const points4& t) {
			for (int i = 0; i < 4; i++)
				pts[i] = t.pts[i];
		}
		void scale(float r) {
			for (int i = 0; i < 4; i++) {
				pts[i].x = (float)(pts[i].x*r);
				pts[i].y = (float)(pts[i].y*r);
			}
		}
	};

	class ZVDImpl
	{
	public:
		ZVDImpl();
		virtual ~ZVDImpl();

		bool			loadData(const char* szpath);
		void			saveData(const char* szpath);
		bool			isLoaded() const;

		bool			detect(const Mat& image);
		void			getVDRegion(ZVDRegion& region);

		void			getControlCImage(Mat& mat) {
			mat = m_matControlC;
		}
		void			getTestTImage(Mat& mat) {
			mat = m_matTestT;
		}
		void			getBetweenCTlineImage(Mat& mat) {
			mat = m_matBetweenCTline;
		}
		void	getImage(const Mat& image, VDElemType elemtype, Mat& mat);
	private:
		bool		m_fload;
		ZVDRegion		m_vdregion;

		ZVDRegionDetector		m_vdDetecotr;

		Mat				m_matControlC;
		Mat				m_matTestT;
		Mat				m_matBetweenCTline;

		ObjectBoostDetectorHaar			m_ct_detector;
		CascadeClassifierObjectHaar		m_ct_cascadeclassifier;

		bool			detectOne(const Mat& image, ZVDRegion& region);
		bool			detectColorLandmark(const Mat& image, points4& points, points4& qrpoints, points4* region_points);
		bool			detectColorLandmark(const Mat& image, const Vector<vec2i>& vpoints, Vector<vec2i>& landmarks);
		void			detectControlC(const Mat& image, const points4& points);

		void releaseData();

	};

}