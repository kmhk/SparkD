#pragma once

#include "zvd.h"
#include "ObjectBoostDetectorHaar.h"
#include "CascadeClassifierObjectHaar.h"
#include "strip_predictor.h"

namespace cvlib
{

	class ZVDRegionDetector
	{
	public:
		ZVDRegionDetector();
		~ZVDRegionDetector();

		bool			loadData(const char* szpath);
		bool			loadData(XFile* pfile);
		void			saveData(const char* szpath);
		void			saveData(XFile* pfile);
		bool			isLoaded() const;

		bool			detect(const Mat& orgimage, const Mat& image, const Rect& workregion, ZVDRegion& vdregion);
		bool			detectCTRegion(const Mat& image, const Vector<vec2f>& srcpoints, ZVDRegion& vdregion);
		void			makeImage(const Mat& image, Mat& mimg, const TiltedRect& tregion);

	private:
		ObjectBoostDetectorHaar			m_ctside_detector;
		CascadeClassifierObjectHaar		m_ctside_cascadeclassifier;
		//ObjectBoostDetectorHaar			m_idline_detector;
		//CascadeClassifierObjectHaar		m_idline_cascadeclassifier;
		
		strip_predictor					m_strip_predictor;
		bool		m_floaded;

		Mat				m_matControlC;
		Mat				m_matTestT;
		Mat				m_matBetweenCTline;

		bool detectBoundary(const Mat& image, ZVDRegion& vdregion);
		//bool detectIDLineByCascade(const Mat& image, ZVDRegion& vdregion);
		bool detectIDLineByLine(const Mat& image, vec2i& pt_first, vec2i& pt_last);
	};

}