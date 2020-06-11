#include "zvd_impl.h"
#include <time.h>

#include "ScaleXY.h"
#include "ipTransformation.h"
#include "utils.h"
#include "Warping.h"
#include "thresh.h"
#include "ScaleXY.h"
#include "binarizewolfjolion.h"
#include "Sobel.h"
#include "GaussianFilter.h"
#include "LOGTIME.h"
#include "cvlibbase/Inc/cvlibutil.h"
#include "FindSquares.h"
#include "Contours.h"

#ifdef __ANDROID__
#include <android/log.h>
#define LOG_TAG "zvd_jni/ZVD"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#else
#define LOGD(x) 
#endif 

extern unsigned char gvddata[2823440];

namespace cvlib
{

	ZVDImpl::ZVDImpl()
	{
		m_fload = false;
		//loadData("F:/work_jimmy/Jimmy2/train/vd1.data");
		loadData(0);
	}
	ZVDImpl::~ZVDImpl()
	{
		IPDebug::resetDebug();
		IPDebug::resetLog();
	}
	bool	ZVDImpl::loadData(const char* szpath)
	{
		if (m_fload)
			return true;
		XFileMem xfile(gvddata, 2823440);
		//XFileDisk xfile;
		//if (!xfile.open("F:/work_jimmy/Jimmy2/train/vd1.data", "rb"))
		//	return false;

		//int loaded = m_ct_cascadeclassifier.loadFromFolder("F:/work_jimmy/Jimmy2/train/allregions_train/HaarObject/");
		int loaded = m_ct_cascadeclassifier.fromFile(&xfile);
		if (loaded == 0)
			return false;
		//m_ct_cascadeclassifier.toFile(String::format("%s/cascade_ct.dat", szpath));

		m_ct_cascadeclassifier.fastProcessing();
		m_ct_detector.create(&m_ct_cascadeclassifier);

		//if (!m_vdDetecotr.loadData(szpath))
		//	return false;
		if (!m_vdDetecotr.loadData(&xfile))
			return false;

		//saveData("F:/work_jimmy/Jimmy2/train/vd1.data");
		//cvutil::makeDataCpp("F:/work_jimmy/Jimmy2/train/vd1.data", "F:/work_jimmy/Jimmy2/train/vddata2.cpp", "gvddata");

		m_fload = true;
		return m_fload;
	}
	void	ZVDImpl::saveData(const char* szpath)
	{
		XFileDisk xfile;
		if (xfile.open(szpath, "wb")) {
			m_ct_cascadeclassifier.toFile(&xfile);
			m_vdDetecotr.saveData(&xfile);
		}
		xfile.close();
	}
	bool	ZVDImpl::isLoaded() const
	{
		return m_fload;
	}
	
	void ZVDImpl::releaseData()
	{
	}

	void ZVDImpl::getVDRegion(ZVDRegion& region)
	{
		region = m_vdregion;
	}

	static TiltedRect makeFromPoints4(const points4& points, float rscale)
	{
		TiltedRect t;
		for (int i = 0; i < 4; i++)
			t.m_pts[i] = points.pts[i];
		t.scale(rscale);
		return t;
	}
	bool ZVDImpl::detect(const Mat& _image)
	{
		int newx, newy;
		newx = 480;
		newy = newx*_image.rows() / _image.cols();
		Mat scaled, gray;
		ip::resize(_image, scaled, newx, newy, ip::INTER_AREA);

		LOGD("start detectColorLandmark()");
		points4 points, qr_points, rect_points[4];
		if (!detectColorLandmark(scaled, points, qr_points, rect_points)) {
			LOGD("failed detectColorLandmark()");
			return false;
		}
		LOGD("finished detectColorLandmark()");

		points4 points_ctside;
		{
			const vec2i* pts = points.pts;
			vec2i _vcenter = pts[1] + pts[2];
			_vcenter.x /= 2;
			_vcenter.y /= 2;
			vec2f vcenter(_vcenter.x, _vcenter.y);
			vec2i _dx = pts[2] - pts[1];
			vec2f dx(_dx.x, _dx.y);
			vec2i _dy(dx.y, -dx.x);
			vec2f dy(_dy.x, _dy.y);
			dx *= (1.f / dx.norm());
			dy *= (1.f / dy.norm());
			float width = pts[2].distTo(pts[1])*0.5f;
			float height = width*0.25f;
			points_ctside.pts[0] = vcenter - dx*width*0.5f - dy*height;
			points_ctside.pts[1] = vcenter - dx*width*0.5f + dy*height;
			points_ctside.pts[2] = vcenter + dx*width*0.5f + dy*height;
			points_ctside.pts[3] = vcenter + dx*width*0.5f - dy*height;
		}
		points4 points_ctside_opposite;
		{
			const vec2i* pts = points.pts;
			vec2i _vcenter = pts[0] + pts[3];
			_vcenter.x /= 2;
			_vcenter.y /= 2;
			vec2f vcenter(_vcenter.x, _vcenter.y);
			vec2i _dx = pts[3] - pts[0];
			vec2f dx(_dx.x, _dx.y);
			vec2i _dy(dx.y, -dx.x);
			vec2f dy(_dy.x, _dy.y);
			dx *= (1.f / dx.norm());
			dy *= (1.f / dy.norm());
			float width = pts[3].distTo(pts[0])*0.5f;
			float height = width*0.25f;
			points_ctside_opposite.pts[0] = vcenter - dx*width*0.5f - dy*height;
			points_ctside_opposite.pts[1] = vcenter - dx*width*0.5f + dy*height;
			points_ctside_opposite.pts[2] = vcenter + dx*width*0.5f + dy*height;
			points_ctside_opposite.pts[3] = vcenter + dx*width*0.5f - dy*height;
		}
		points4 points_id;
		{
			const vec2i* pts = points.pts;
			vec2i _vc = pts[0] + pts[1];
			_vc.x /= 2;
			_vc.y /= 2;
			vec2f vc(_vc.x, _vc.y);

			vec2i _vc2 = pts[2] + pts[3];
			_vc2.x /= 2;
			_vc2.y /= 2;
			vec2f vc2(_vc2.x, _vc2.y);

			float r1 = 1.95f;
			float r2 = 1.7f;
			float r3 = 2.0f;
			float r4 = 1.75f;
			points_id.pts[0] = pts[0] * r3 + pts[3] * (1.f - r3);
			points_id.pts[1] = pts[1] * r1 + pts[2] * (1.f - r1);
			points_id.pts[2] = pts[1] * r2 + pts[2] * (1.f - r2);
			points_id.pts[3] = pts[0] * r4 + pts[3] * (1.f - r4);

		}
		points4 points_region;
		{
			const vec2i* pts = points.pts;

			float r = 0.3f;
			points_region.pts[0] = pts[0] * (1.f - r) + pts[1] * r;
			points_region.pts[1] = pts[0] * r + pts[1] * (1.f - r);
			points_region.pts[2] = pts[2] * (1.f - r) + pts[3] * r;
			points_region.pts[3] = pts[2] * r + pts[3] * (1.f - r);

			float rscale = (float)_image.cols() / (float)newx;
			points_region.scale(rscale);
			
			detectControlC(_image, points_region);
		}
		float rscale = (float)_image.cols() / (float)newx;

		TiltedRect controlc_rect = m_vdregion.getRect(VD_TestT);
		TiltedRect testt_rect = m_vdregion.getRect(VD_controlC);
		m_vdregion.setRect(VD_controlC, controlc_rect);
		m_vdregion.setRect(VD_TestT, testt_rect);
		
		m_vdregion.setRect(VD_CTSide, makeFromPoints4(points_ctside, rscale));
		m_vdregion.setRect(VD_OppositeCTSide, makeFromPoints4(points_ctside_opposite, rscale));
		m_vdregion.setRect(VD_IDline, makeFromPoints4(points_id, rscale));
		m_vdregion.setRect(VD_qr, makeFromPoints4(qr_points, rscale));
		m_vdregion.setRect(VD_rect1, makeFromPoints4(rect_points[0], rscale));
		m_vdregion.setRect(VD_rect2, makeFromPoints4(rect_points[1], rscale));
		m_vdregion.setRect(VD_rect3, makeFromPoints4(rect_points[2], rscale));
		m_vdregion.setRect(VD_rect4, makeFromPoints4(rect_points[3], rscale));

		m_vdDetecotr.makeImage(_image, m_matControlC, m_vdregion.getRect(VD_controlC));
		m_vdDetecotr.makeImage(_image, m_matTestT, m_vdregion.getRect(VD_TestT));
		m_vdDetecotr.makeImage(_image, m_matBetweenCTline, m_vdregion.getRect(VD_BetweenCTline));

		return true;

	}

	static double angleBetWeen2Lines(const Point2i& pt1, const Point2i& pt2, const Point2i& pt0)
	{
		double dx1 = pt1.x - pt0.x;
		double dy1 = pt1.y - pt0.y;
		double dx2 = pt2.x - pt0.x;
		double dy2 = pt2.y - pt0.y;
		return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
	}
	static double angleBetWeen2Lines(const vec2f& pt1, const vec2f& pt2, const vec2f& pt0)
	{
		double dx1 = pt1.x - pt0.x;
		double dy1 = pt1.y - pt0.y;
		double dx2 = pt2.x - pt0.x;
		double dy2 = pt2.y - pt0.y;
		return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
	}
	template<typename _T> 
	void clockwisePoints(const Mat& image, Point2_<_T>* src)
	{
		typedef Point2_<_T> vectype;
		vectype d1 = src[1] - src[0];
		vectype d2 = src[2] - src[1];
		float r = d1.x*d2.y - d2.x*d1.y;
		if (r < 0) {
			vectype temp[4];
			for (int i = 0; i < 4; i++)
				temp[i] = src[i];
			src[0] = temp[3];
			src[1] = temp[2];
			src[2] = temp[1];
			src[3] = temp[0];
		}
		int v1 = image.at<vec3b>(src[0].y, src[0].x).r + image.at<vec3b>(src[2].y, src[2].x).r;
		int v2 = image.at<vec3b>(src[1].y, src[1].x).r + image.at<vec3b>(src[3].y, src[3].x).r;
		if (v1 < v2) {
			vectype temp[4];
			for (int i = 0; i < 4; i++)
				temp[i] = src[i];
			src[0] = temp[1];
			src[1] = temp[2];
			src[2] = temp[3];
			src[3] = temp[0];
		}
	}
	template<typename _T>
	float calcStraightRectSide(const Point2_<_T>* points)
	{
		Vecf vlength(4);
		for (int i = 0; i < 4; i++) {
			vlength[i] = points[i].distTo(points[(i + 1) % 4]);
		}
		float rmean = (float)vlength.mean();
		float rval = 0;
		for (int i = 0; i < 4; i++) {
			rval += (float)fabsf(vlength[i] - rmean);
		}
		return rval / (rmean * 4);
	}
	template<typename _T>
	float calcStraightRectDiagonal(const Point2_<_T>* points)
	{
		typedef Point2_<_T> vectype;
		vectype vcenter = points[0] + points[1] + points[2] + points[3];
		vcenter.x /= 4;
		vcenter.y /= 4;

		Vecf vlength(4);
		vlength.resize(4);
		for (int i = 0; i < 4; i++) {
			vlength[i] = points[i].distTo(vcenter);
		}
		float rmean = vlength.mean();
		float rvalue = 0;
		for (int i = 0; i < 4; i++) {
			rvalue += fabsf(vlength[i] - rmean);
		}
		return rvalue / (rmean * 4);
	}
	static void arrangePoints(vec2f* src, int i1, int i2, int i3, int i4)
	{
		vec2f temp[4];
		for (int i = 0; i < 4; i++)
			temp[i] = src[i];

		src[0] = temp[i1];
		src[1] = temp[i2];
		src[2] = temp[i3];
		src[3] = temp[i4];
	}
	static float calcStraightRectCondition(const Mat& image, points4& points, bool farrange = false)
	{
		Vector<vec2f> vpoints(4);
		for (int i = 0; i < 4; i++)
			vpoints[i] = vec2f(points.pts[i].x, points.pts[i].y);
		const float bad_value = 1000.f;
		if (vpoints.getSize() != 4)
			return bad_value;
		Vecf vlength(6);
		for (int i = 0; i < 4; i++) {
			int j = (i + 1) % 4;
			vlength[i] = vpoints[i].distTo(vpoints[j]);
		}
		vlength[4] = vpoints[0].distTo(vpoints[2]);
		vlength[5] = vpoints[1].distTo(vpoints[3]);
		vec2f vcross;
		int imax;
		vlength.max(imax);
		if (imax == 0) {
			if (!ip::intersectPointTwoLines(vpoints[0], vpoints[1], vpoints[2], vpoints[3], vcross))
				return bad_value;
			if (farrange)
				arrangePoints(vpoints, 0, 2, 1, 3);
		}
		else if (imax == 1) {
			if (!ip::intersectPointTwoLines(vpoints[1], vpoints[2], vpoints[3], vpoints[0], vcross))
				return bad_value;
			if (farrange)
				arrangePoints(vpoints, 1, 3, 2, 0);
		}
		else if (imax == 2) {
			if (!ip::intersectPointTwoLines(vpoints[2], vpoints[3], vpoints[1], vpoints[0], vcross))
				return bad_value;
			if (farrange)
				arrangePoints(vpoints, 2, 1, 3, 0);
		}
		else if (imax == 3) {
			if (!ip::intersectPointTwoLines(vpoints[3], vpoints[0], vpoints[1], vpoints[2], vcross))
				return bad_value;
			if (farrange)
				arrangePoints(vpoints, 3, 1, 0, 2);
		}
		else if (imax == 4) {
			if (!ip::intersectPointTwoLines(vpoints[2], vpoints[0], vpoints[1], vpoints[3], vcross))
				return bad_value;
			if (farrange)
				arrangePoints(vpoints, 2, 1, 0, 3);
		}
		else if (imax == 5) {
			if (!ip::intersectPointTwoLines(vpoints[1], vpoints[3], vpoints[2], vpoints[0], vcross))
				return bad_value;
			if (farrange)
				arrangePoints(vpoints, 1, 2, 3, 0);
		}

		/*if (farrange) {
		clockwisePoints<float>(image, vpoints);
		float rvalue1 = calcStraightRectSide<float>(vpoints);
//			if (straightRectangleCondition<float>(vpoints) > 0.2f)
//				return false;
//		}
		*/
		float rvalue2 = calcStraightRectDiagonal<float>(vpoints);
		for (int i = 0; i < 4; i++) {
			points.pts[i] = vpoints[i];
		}
		return rvalue2;
	}
	bool ZVDImpl::detectColorLandmark(const Mat& image, const Vector<vec2i>& vpoints, Vector<vec2i>& landmarks)
	{
		int count = vpoints.getSize();
		points4 cand;
		int allcount = count*(count - 1)*(count - 2)*(count - 3);
		Vector<points4> allcands(allcount);
		Vecf values(allcount);
		int idx = 0;
		for (int i0 = 0; i0 < count; i0++) {
			cand.pts[0] = vpoints[i0];
			for (int i1 = i0 + 1; i1 < count; i1++) {
				cand.pts[1] = vpoints[i1];
				for (int i2 = i1 + 1; i2 < count; i2++) {
					cand.pts[2] = vpoints[i2];
					for (int i3 = i2 + 1; i3 < count; i3++) {
						cand.pts[3] = vpoints[i3];
						float rvalue = calcStraightRectCondition(image, cand, false);
						allcands[idx] = cand;
						values[idx] = rvalue;
						idx++;
					}
				}
			}
		}

		values.resize(idx);
		int imin;
		float rminvalue = values.min(imin);
		if (imin < 0)
			return false;
		//IPDebug::logging("4 rect condition : %.2f", rminvalue);
		calcStraightRectCondition(image, allcands[imin], true);
		clockwisePoints<int>(image, allcands[imin].pts);
		if (calcStraightRectSide<int>(allcands[imin].pts) > 0.2f)
			return false;
		landmarks = Vector<vec2i>(allcands[imin].pts, 4);
		return true;
	}

	static void makeMatFromPoints4(const Mat& _src, Mat& dst, const points4& points, int sidelen) {
		Mat src(_src.size(), MAT_Tuchar);
		for (int y = 0; y < _src.rows(); y++) {
			for (int x = 0; x < _src.cols(); x++) {
				src.data.ptr[y][x] = _src.data.ptr[y][x * 3];
			}
		}
		cvlib::Vector<cvlib::vec2f> vsrcCorners(4);
		for (int i = 0; i < 4; i++)
			vsrcCorners[i] = vec2f(points.pts[i].x, points.pts[i].y);
		cvlib::Vector<cvlib::vec2f> vdstCorners(4);
		float r1 = vsrcCorners[0].distTo(vsrcCorners[1]);
		float r2 = vsrcCorners[2].distTo(vsrcCorners[3]);
		float r3 = vsrcCorners[0].distTo(vsrcCorners[3]);
		float r4 = vsrcCorners[2].distTo(vsrcCorners[1]);

		int plate_height = (vsrcCorners[0].distTo(vsrcCorners[1]) + vsrcCorners[2].distTo(vsrcCorners[3]))*0.5f;
		int plate_width = (vsrcCorners[0].distTo(vsrcCorners[3]) + vsrcCorners[2].distTo(vsrcCorners[1]))*0.5f;
		plate_height = (plate_height + 3) / 4 * 4;
		plate_width = (plate_width + 3) / 4 * 4;
		vdstCorners[0] = cvlib::vec2f(0, 0);
		vdstCorners[1] = cvlib::vec2f(0, sidelen);
		vdstCorners[2] = cvlib::vec2f(sidelen, sidelen);
		vdstCorners[3] = cvlib::vec2f(sidelen, 0);

		cvlib::Mat mwarp = cvlib::ip::getPerspectiveTransform(vsrcCorners, vdstCorners);
		dst.create(sidelen, sidelen, src.type1());
		dst = 255;
		cvlib::ip::warpPerspective(src, dst, mwarp);
	}

	bool ZVDImpl::detectColorLandmark(const Mat& image, points4& vdregion_points, points4& qr_points, points4* region_points)
	{
		Mat gray;
		ColorSpace::RGBtoGray(image, gray);

		LOGTIME("start detectColorLandmark");
		LOGD("start detectColorLandmark");
		Matrix<vec2i> contours;
		findSquares4(&gray, contours, 0, true);
		if (contours.getSize() == 0)
			return false;
		LOGD("end findSquares4");
		LOGTIME("end findSquares4");
		Vector<vec2i> points;
		for (int i = 0; i < contours.getSize(); i++) {
			const Vector<vec2i> contour = contours[i];
			vec2i vcenter = contour[0] + contour[1] + contour[2] + contour[3];
			vcenter.x /= 4;
			vcenter.y /= 4;
			points.add(vcenter);
		}
		Vector<int> vidx;
		Vector<int> vflag(points.getSize());
		for (int i = 0; i < points.getSize(); i++) {
			vflag[i] = 0;
		}
		for (int i = 0; i < points.getSize(); i++) {
			if (vflag[i] != 0)
				continue;
			if (vidx.getSize() == 0) {
				vidx.add(i);
				vflag[i] = 1;
				continue;
			}
			Vecf vdist(vidx.getSize());
			for (int k = 0; k < vidx.getSize(); k++) {
				vdist[k] = (float)points[vidx[k]].distTo(points[i]);
			}
			if (vdist.min() > 5.f) {
				vidx.add(i);
				vflag[i] = 1;
				continue;
			}
		}
		Vector<vec2i> all_landmarks;
		Vector<vec2i> vlandmarks_orange;
		Vector<vec2i> vlandmarks_black;
		for (int i = 0; i < vidx.getSize(); i++) {
			const vec2i& p = points[vidx[i]];
			const uchar* pcolor = &image.data.ptr[p.y][p.x * 3];
			int nv = pcolor[0] + pcolor[1] + pcolor[2];
			if (pcolor[0] > pcolor[1] * 1.5f && pcolor[1] > pcolor[2] * 1.2f && nv > 110)
				vlandmarks_orange.add(p);
			else {
				if (pcolor[0] + pcolor[1] + pcolor[2] < 350)
					vlandmarks_black.add(p);
			}
			all_landmarks.add(p);
		}
		Vector<vec2i> landmarks;
		if (!detectColorLandmark(image, all_landmarks, landmarks)) {
			LOGTIME("failed in detectColorLandmark()");
			return false;
		}
		LOGTIME("end detectColorLandmark");

		// arrange 4 landmarks
		vec2i vcenter = landmarks[0] + landmarks[1] + landmarks[2] + landmarks[3];
		vcenter.x /= 4;
		vcenter.y /= 4;
		vec2i v1 = (landmarks[0] + landmarks[1]) / 2;
		vec2i v2 = (landmarks[2] + landmarks[3]) / 2;

		vec2i qr_center1 = vcenter - (v2 - v1)*0.95f;
		vec2i qr_center2 = vcenter + (v2 - v1)*0.95f;
		points4 qr1;
		for (int i = 0; i < 4; i++)
			qr1.pts[i] = qr_center1 + (landmarks[i] - vcenter)*0.5f;

		points4 qr2;
		for (int i = 0; i < 4; i++)
			qr2.pts[i] = qr_center2 + (landmarks[i] - vcenter)*0.5f;

		points4 qr;
		Mat mqr1, mqr2;
		makeMatFromPoints4(image, mqr1, qr1, 16);
		makeMatFromPoints4(image, mqr2, qr2, 16);
		if (mqr1.var() > mqr2.var()) {
			qr = qr1;
			for (int i = 0; i < 4; i++)
				vdregion_points.pts[i] = landmarks[i];
		}
		else {
			qr = qr2;
			for (int i = 2; i < 6; i++)
				vdregion_points.pts[i - 2] = landmarks[i % 4];
		}
		qr_points = qr;

		for (int i = 0; i < 4; i++) {
			region_points[0].pts[i] = (landmarks[i] - vcenter)*0.24f + landmarks[0];
			region_points[1].pts[i] = (landmarks[i] - vcenter)*0.24f + landmarks[1];
			region_points[2].pts[i] = (landmarks[i] - vcenter)*0.24f + landmarks[3];
			region_points[3].pts[i] = (landmarks[i] - vcenter)*0.24f + landmarks[2];
		}
		return true;
	}

	bool ZVDImpl::detectOne(const Mat& image, ZVDRegion& vdregion)
	{
		int newx, newy;
		newx = 480;
		newy = newx*image.rows() / image.cols();
		Mat scaled, gray;
		ip::resize(image, scaled, newx, newy, ip::INTER_AREA);
		ColorSpace::RGBtoGray(scaled, gray);
#ifdef IPDEBUG
		IPDebug::addDebug(imageGroup(scaled, gray));
#endif
		Vector<Rect> candidate_ct_regions;
		if (m_ct_detector.getCascadeClassifier() != 0)
		{
			m_ct_detector.m_Param.rStepX = 0.05f;
			m_ct_detector.m_Param.rStepY = 0.05f;
			m_ct_detector.m_Param.nMinSize = std::min(scaled.cols(), scaled.rows()) / 6;
			m_ct_detector.m_Param.nMaxSize = std::min(scaled.cols(), scaled.rows());
			m_ct_detector.m_Param.rScale = 1.1f;
			m_ct_detector.m_Param.nOverlap = 1;
			m_ct_detector.m_Param.nOnlyOne = 0;

			Vector<Rect> regions;
			LOGTIME("start extract()");
			m_ct_detector.extract(scaled, regions);
			LOGTIME("end extract()");
			candidate_ct_regions = regions;
		}

		bool fgood = false;

		LOGTIME("start m_vdDetecotr.detect()");
		for (int i = 0; i < candidate_ct_regions.getSize(); i++) {
			Rect workregion = candidate_ct_regions[i];
			int w = workregion.width / 8;
			int h = workregion.height / 8;
			workregion.inflateRect(w, h);
			workregion = workregion.intersectRect(Rect(0, 0, scaled.cols(), scaled.rows()));

			if (m_vdDetecotr.detect(image, scaled, workregion, vdregion)) {
				m_vdDetecotr.makeImage(image, m_matControlC, vdregion.getRect(VD_controlC));
				m_vdDetecotr.makeImage(image, m_matTestT, vdregion.getRect(VD_TestT));
				m_vdDetecotr.makeImage(image, m_matBetweenCTline, vdregion.getRect(VD_BetweenCTline));
				fgood = true;
				break;
			}
		}

		LOGTIME("end m_vdDetecotr.detect()");
		return fgood;
	}
	
	void	ZVDImpl::detectControlC(const Mat& image, const points4& points)
	{
		cvlib::Vector<cvlib::vec2f> vsrcCorners(4);
		for (int i = 0; i < 4; i++)
			vsrcCorners[i] = vec2f(points.pts[(i + 2) % 4].x, points.pts[(i + 2) % 4].y);

		m_vdDetecotr.detectCTRegion(image, vsrcCorners, m_vdregion);
	}

	void	ZVDImpl::getImage(const Mat& image, VDElemType elemtype, Mat& mat)
	{
		TiltedRect tr = m_vdregion.getRect(elemtype);
		m_vdDetecotr.makeImage(image, mat, tr);
	}

}