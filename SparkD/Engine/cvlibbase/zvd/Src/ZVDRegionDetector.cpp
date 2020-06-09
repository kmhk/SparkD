#include "ZVDRegionDetector.h"
#include "LOGTIME.h"
#include "thresh.h"
#include "connectedComponent.h"
#include "binarizewolfjolion.h"
#include "Morphology.h"
#include "HoughTransform.h"
#include "signalprocess.h"
#include "ImgProc.h"
#include "LineRansac.h"
#include "GaussianFilter.h"
#include "ScaleXY.h"
#include "Warping.h"

namespace cvlib
{

	ZVDRegionDetector::ZVDRegionDetector()
	{
		m_floaded = false;
	}
	ZVDRegionDetector::~ZVDRegionDetector()
	{
	}
	bool ZVDRegionDetector::loadData(const char* szpath)
	{
		if (m_floaded)
			return true;
		int loaded = m_ctside_cascadeclassifier.loadFromFolder("F:/work_jimmy/Jimmy2/train/ctside_train/HaarObject/");
		if (loaded == 0)
			return false;
		m_ctside_cascadeclassifier.fastProcessing();
		m_ctside_detector.create(&m_ctside_cascadeclassifier);

		/*loaded = m_idline_cascadeclassifier.loadFromFolder("F:/work_jimmy/Jimmy2/train/idline_train/HaarObject/");
		if (loaded == 0)
			return false;
		m_idline_cascadeclassifier.fastProcessing();
		m_idline_detector.create(&m_idline_cascadeclassifier);*/

		if (!m_strip_predictor.fromFile("F:/work_jimmy/Jimmy2/train/stripwork_train/strip_predictor_6.dat"))
			return false;

		m_floaded = true;
		return true;
	}
	void ZVDRegionDetector::saveData(const char* szpath)
	{
		m_ctside_cascadeclassifier.toFile(String::format("%s/ctside_cascade", szpath));
	}
	bool ZVDRegionDetector::loadData(XFile* pfile)
	{
		if (m_floaded)
			return true;
		if (!m_ctside_cascadeclassifier.fromFile(pfile))
			return false;
		m_ctside_cascadeclassifier.fastProcessing();
		m_ctside_detector.create(&m_ctside_cascadeclassifier);
		if (!m_strip_predictor.fromFile(pfile))
			return false;
		m_floaded = true;
		return true;
	}
	void ZVDRegionDetector::saveData(XFile* pfile)
	{
		m_ctside_cascadeclassifier.toFile(pfile);
		m_strip_predictor.toFile(pfile);
	}

	bool ZVDRegionDetector::isLoaded() const
	{
		return m_floaded;
	}
	bool ZVDRegionDetector::detect(const Mat& orgimage, const Mat& image, const Rect& workregion, ZVDRegion& vdregion)
	{
		float scale_factor = (float)orgimage.rows() / (float)image.rows();
		
		Mat msub;
		image.subMat(workregion, msub);
		vec2i pt_first, pt_last;
		if (!detectIDLineByLine(msub, pt_first, pt_last))
			return false;

		if (!detectBoundary(msub, vdregion))
			return false;
		TiltedRect ctsideregion = vdregion.getRect(VD_CTSide);
		ctsideregion.moveXY(workregion.x, workregion.y);
		ctsideregion.scale(scale_factor);
		vdregion.setRect(VD_CTSide, ctsideregion);

		float base_line = pt_last.distTo(pt_first);
		vec2f vx = vec2f(pt_last - pt_first) / vec2f(pt_last - pt_first).norm();
		vec2f vy(vx.y, -vx.x);

		vec2f p1 = pt_first + (vx*0.35f + vy*1.6f)*base_line + workregion.tl();
		vec2f p2 = pt_first + (vx*0.85f + vy*1.6f)*base_line + workregion.tl();
		vec2f p3 = pt_first + (vx*0.85f + vy*0.55f)*base_line + workregion.tl();
		vec2f p4 = pt_first + (vx*0.35f + vy*0.55f)*base_line + workregion.tl();
		Vector<vec2f> srcpoints;
		srcpoints.add(p1*scale_factor);
		srcpoints.add(p2*scale_factor);
		srcpoints.add(p3*scale_factor);
		srcpoints.add(p4*scale_factor);
		if (!detectCTRegion(orgimage, srcpoints, vdregion))
			return false;

		vec2f pt_idline_first = pt_first + workregion.tl();
		vec2f pt_idline_last = pt_last + (pt_last - pt_first)*0.25f + workregion.tl();
		TiltedRect idregion;
		idregion.m_pts[0] = pt_idline_first*scale_factor;
		idregion.m_pts[1] = pt_idline_last*scale_factor;
		idregion.m_pts[2] = (pt_idline_last - vy*0.2f*base_line)*scale_factor;
		idregion.m_pts[3] = (pt_idline_first - vy*0.2f*base_line)*scale_factor;
		vdregion.setRect(VD_IDline, idregion);

		// make opposite ctside
		TiltedRect opposite_ctside_region;
		Rect ctside_region = vdregion.getRect(VD_CTSide).getBoundRect();
		vec2i vcenter = ctside_region.centeri();
		vec2i opposite_center = vcenter + vx*base_line*scale_factor*1.1f;
		Rect opposite_region(opposite_center.x, opposite_center.y, 0, 0);
		opposite_region.inflateRect(ctside_region.width / 2, ctside_region.height / 2);
		vdregion.setRect(VD_OppositeCTSide, TiltedRect(opposite_region));
/*#ifdef IPDEBUG
		Mat drawimg = orgimage;
		//drawimg.drawLine(pt_first, pt_last, COLOR(255, 0, 0));
		vdregion.getRect(VD_CTSide).drawTo(drawimg, COLOR(0, 255, 255));
		vdregion.getRect(VD_IDline).drawTo(drawimg, COLOR(255, 255, 0));
		vdregion.getRect(VD_controlC).drawTo(drawimg, COLOR(255, 0, 0));
		vdregion.getRect(VD_BetweenCTline).drawTo(drawimg, COLOR(0, 0, 255));
		vdregion.getRect(VD_TestT).drawTo(drawimg, COLOR(255, 255, 0));
		vdregion.getRect(VD_OppositeCTSide).drawTo(drawimg, COLOR(0, 255, 255));
		IPDEBUG(drawimg);
#endif*/
		return true;
	}
	bool ZVDRegionDetector::detectBoundary(const Mat& image, ZVDRegion& vdregion)
	{
		Mat msub;
		image.subRefMat(Rect(0, 0, image.cols() / 2, image.rows() / 2), msub);

		m_ctside_detector.m_Param.rStepX = 0.15f;
		m_ctside_detector.m_Param.rStepY = 0.05f;
		m_ctside_detector.m_Param.nMinSize = std::min(image.cols(), image.rows()) / 7;
		m_ctside_detector.m_Param.nMaxSize = std::min(image.cols(), image.rows()) / 2;
		m_ctside_detector.m_Param.rScale = 1.1f;
		m_ctside_detector.m_Param.nOverlap = 0;
		m_ctside_detector.m_Param.nOnlyOne = 0;

		Mat heatmap, sizeheatmap;
		Vector<Rect> regions;
		m_ctside_detector.extractHeatmap(msub, &heatmap, &sizeheatmap);

		Mat heatmap_bin;
		ip::Threshold(heatmap, heatmap_bin, 150, 255, ip::THRESH_BINARY);

		ip::ConnectedComponent cc;
		Vector<ip::ConnectInfo>	connectVector = cc.apply(heatmap_bin, 0, false, true);
		vec2f vcenter;
		float rwidth = 0;
		if (connectVector.getSize() > 0)
		{
			const ip::ConnectInfo& connectinfo = connectVector[0];
			Vector<vec2i> points;
			cc.createPointArrayFromConnect(connectinfo, points);
			for (int k = 0; k < points.getSize(); k++) {
				vcenter.x += points[k].x;
				vcenter.y += points[k].y;
				rwidth += sizeheatmap.data.fl[points[k].y][points[k].x];
			}
			if (points.getSize() > 0) {
				vcenter.x /= points.getSize();
				vcenter.y /= points.getSize();
				rwidth /= points.getSize();
			}
		}
		if (rwidth == 0)
			return false;

		rwidth *= 1.6f;
		Size stdsize(m_ctside_detector.getCascadeClassifier()->m_nObjectWidth,
			m_ctside_detector.getCascadeClassifier()->m_nObjectHeight);
		float rheight = rwidth / stdsize.width*stdsize.height;

		int nh = cvutil::round(rheight);
		int nw = cvutil::round(rwidth);
		connectVector.removeAll();

		vdregion.setRect(VD_CTSide, Rect(vcenter.x - nw / 2, vcenter.y - nh / 2, nw, nh));

		return true;
	}
	/*bool ZVDRegionDetector::detectIDLineByCascade(const Mat& image, ZVDRegion& vdregion)
	{
		vec2i ptanchor(0, image.rows() * 2 / 3);

		Mat msub;
		image.subRefMat(Rect(0, image.rows() * 2 / 3, image.cols(), image.rows() / 3), msub);

		m_idline_detector.m_Param.rStepX = 0.05f;
		m_idline_detector.m_Param.rStepY = 0.1f;
		m_idline_detector.m_Param.nMinSize = std::max(image.cols(), image.rows()) / 5;
		m_idline_detector.m_Param.nMaxSize = std::max(image.cols(), image.rows());
		m_idline_detector.m_Param.rScale = 1.1f;
		m_idline_detector.m_Param.nOverlap = 0;
		m_idline_detector.m_Param.nOnlyOne = 0;

		Mat heatmap, sizeheatmap;
		Vector<Rect> regions;

		m_idline_detector.extractHeatmap(msub, &heatmap, &sizeheatmap);

		Mat heatmap_bin;
		ip::Threshold(heatmap, heatmap_bin, 150, 255, ip::THRESH_BINARY);

		ip::ConnectedComponent cc;
		Vector<ip::ConnectInfo>	connectVector = cc.apply(heatmap_bin, 0, false, true);
		vec2f vcenter;
		float rwidth = 0;
		if (connectVector.getSize() > 0)
		{
			const ip::ConnectInfo& connectinfo = connectVector[0];
			Vector<vec2i> points;
			cc.createPointArrayFromConnect(connectinfo, points);
			for (int k = 0; k < points.getSize(); k++) {
				vcenter.x += points[k].x;
				vcenter.y += points[k].y;
				rwidth += sizeheatmap.data.fl[points[k].y][points[k].x];
			}
			if (points.getSize() > 0) {
				vcenter.x /= points.getSize();
				vcenter.y /= points.getSize();
				rwidth /= points.getSize();
			}
		}
		if (rwidth == 0)
			return false;

		rwidth *= 1.4f;
		Size stdsize(m_idline_detector.getCascadeClassifier()->m_nObjectWidth,
			m_idline_detector.getCascadeClassifier()->m_nObjectHeight);
		float rheight = rwidth / stdsize.width*stdsize.height;

		int nh = cvutil::round(rheight);
		int nw = cvutil::round(rwidth);
		connectVector.removeAll();

		vcenter += ptanchor;
		Mat t = image;
		t.drawCircle(vcenter, 4, COLOR(0, 255, 0));
		t.drawRect(Rect(vcenter.x - nw / 2, vcenter.y - nh / 2, nw, nh), COLOR(255, 0, 0));
		IPDEBUG(imageGroup(heatmap_bin, t));

		return true;
	}*/
	bool ZVDRegionDetector::detectIDLineByLine(const Mat& image, vec2i& pt_first, vec2i& pt_last)
	{
		vec2i ptanchor(0, image.rows() * 2 / 3);

		Mat msub;
		image.subRefMat(Rect(0, image.rows() * 2 / 3, image.cols(), image.rows() / 3), msub);
		Mat gray;
		ColorSpace::RGBtoGray(msub, gray);

		Mat bin;
		int win_w, win_h;
		win_w = win_h = gray.rows() / 2;
		cvlib::binarization(gray, bin, PB_WOLFJOLION, win_w, win_h, 0.3);

		ip::ConnectedComponent cc;
		Vector<ip::ConnectInfo>	connectVector = cc.apply(bin, 0, false, true);
		if (connectVector.getSize() == 0)
			return false;
		Mat subbin;
		bin.subMat(connectVector[0].region, subbin);

		ptanchor += connectVector[0].region.tl();

		Mat subbin2 = 255 - subbin;
		ip::ConnectedComponent cc2;
		connectVector = cc2.apply(subbin2, 0, false, true);
		Vecf vconnect(connectVector.getSize());
		vconnect = subbin2.rows();
		Mat subbin2_main;
		for (int i = 0; i < connectVector.getSize(); i++)
		{
			const Rect& r = connectVector[i].region;
			if (r.width < subbin2.cols() / 2)
				continue;
			if (r.height > r.width / 2)
				continue;
			vconnect[i] = (float)connectVector[i].pixels_count / (float)r.width;
		}
		int imin_connect;
		vconnect.min(imin_connect);
		if (imin_connect < 0)
			return false;
		subbin2.subMat(connectVector[imin_connect].region, subbin2_main);
		if (!subbin2_main.isValid())
			return false;

		ptanchor += connectVector[imin_connect].region.tl();
		Vecf vhistogram(subbin2_main.cols());
		{
			Mat mhist(32, subbin2_main.cols(), MAT_Tuchar3);
			mhist.zero();
			vhistogram = 0;
			for (int ix = 0; ix < subbin2_main.cols(); ix++) {
				for (int iy = 0; iy < subbin2_main.rows(); iy++) {
					if (subbin2_main.data.ptr[iy][ix]) {
						vhistogram[ix] ++;// = 255;
					}
				}
			}
			const float arfilter[] = {0.1, 0.2, 0.4, 0.2, 0.1};
			Vecf vfiltered;
			filteringTriangle(vhistogram, vfiltered, arfilter, 5);

			float mean1 = Vecf(vfiltered.data.fl, vfiltered.length() / 2).mean();
			float mean2 = Vecf(vfiltered.data.fl + (vfiltered.length() - 4), 4).mean();
			float rthresh = mean1 + 5.f;
			int endpos = subbin2_main.cols() / 2;
			for (; endpos < subbin2_main.cols(); endpos++) {
				if (vhistogram[endpos] > rthresh) {
					break;
				}
			}
			if (endpos < subbin2_main.cols() - 1)
				endpos -= 2;
			Mat mainmat;
			subbin2_main.subRefMat(Rect(0, 0, endpos, subbin2_main.rows()), mainmat);
			Rect main_region = ip::getBoundRect(mainmat, 255);
			if (main_region.width < image.cols() / 4)
				return false;
			main_region.x += ptanchor.x;
			main_region.y += ptanchor.y;
			Vector<vec2i> vpoints(mainmat.rows()*mainmat.cols());
			int ipt = 0;
			for (int iy = 0; iy < mainmat.rows(); iy++) {
				for (int ix = 0; ix < mainmat.cols(); ix++) {
					if (mainmat.data.ptr[iy][ix] != 0)
						vpoints[ipt++] = vec2i(ix, iy);
				}
			}
			vpoints.setSize(ipt);

			SimpleLine simpleline = createFastSimpleLineFromRegions(vpoints);
			pt_first = ptanchor + vec2i(0, simpleline.ypos(0));
			pt_last = ptanchor + vec2i(main_region.width - 1, simpleline.ypos(main_region.width - 1));
			//region.width = endpos;
			/*for (int ix = 0; ix < subbin2_main.cols(); ix++) {
				//vhistogram[ix] /= 8.f;
				if (ix > 0) {
					vec2i prevpt(ix - 1, vhistogram[ix - 1]);
					vec2i currpt(ix, vhistogram[ix]);
					mhist.drawLine(prevpt, currpt, COLOR(255, 0, 0));
				}
			}
			mhist.drawLine(vec2i(0, rthresh), vec2i(mhist.cols(), rthresh), COLOR(0, 255, 0));
			IPDEBUG(mhist);*/

		}

		// extract ID locaiton


		/*Vector<vec2i> vlines;
		ip::houghLines(bin, vlines, 2.f, CVLIB_PI / 180.f, bin.cols() / 3, bin.cols() / 3, 1);
		IPDebug::logging("hough line count: %d", vlines.getSize() / 2);


		Mat t = msub;
		for (int i = 0; i < vlines.getSize(); i += 2) {
			t.drawLine(vlines[i], vlines[i + 1], COLOR(255, 0, 0));
		}
		IPDEBUG(t);*/

		return true;
	}

	static void enhanceHorizen(const cvlib::Mat& src, const cvlib::Mat& mask, cvlib::Mat& dst)
	{
		int cols = src.cols();
		int rows = src.rows();

		const cvlib::Mat& in = src;
		//int anx[] = { 1,2,1 };
		int anx[] = { 1,4,6,4,1 };
		//int any[] = { -1,0,1 };
		int any[] = { -1,-4,0,4,1 };
		//int any[] = { -1,-3,-2,0,2,3,1 };
		cvlib::Veci vxdir(anx, sizeof(anx) / sizeof(anx[0]));
		cvlib::Veci vydir(any, sizeof(any) / sizeof(any[0]));
		cvlib::Mat enhancedY(src.size(), cvlib::MAT_Tint);
		cvlib::ip::enhanceEdgeX(in, enhancedY, vxdir, vydir, false);
		if (enhancedY.isValid()) {
			int margin = 5;
			for (int y = margin - 1; y >= 0; y--)
			{
				memcpy(enhancedY.data.i[y], enhancedY.data.i[margin], sizeof(int)*cols);
				memcpy(enhancedY.data.i[rows - y - 1], enhancedY.data.i[rows - margin - 1], sizeof(int)*cols);
			}
			for (int x = margin - 1; x >= 0; x--)
			{
				for (int y = 0; y < rows; y++) {
					enhancedY.data.i[y][x] = enhancedY.data.i[y][margin];
					enhancedY.data.i[y][cols - x - 1] = enhancedY.data.i[y][cols - margin - 1];
				}
			}
		}
		if (mask.isValid()) {
			for (int y = 0; y < rows; y++)
			{
				for (int x = 0; x < cols; x++)
				{
					if (mask.data.ptr[y][x] == 255) {
						enhancedY.data.i[y][x] = 0;
					}
				}
			}
		}
		int elemcount = enhancedY.rows()*enhancedY.cols();

		dst.create(rows, cols, cvlib::MAT_Tint);

		int* pn = dst.data.i[0];
		int* const pedgey = enhancedY.data.i[0];
		for (int j = 0; j < elemcount; j++)
			pn[j] = std::abs(pedgey[j]);

		dst.convert(cvlib::MAT_Tbyte);
	}
	static Mat warpImage(const Mat& src, Mat& dst, const Vector<vec2f>& vsrcCorners)
	{
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
		vdstCorners[1] = cvlib::vec2f(0, plate_height);
		vdstCorners[2] = cvlib::vec2f(plate_width, plate_height);
		vdstCorners[3] = cvlib::vec2f(plate_width, 0);

		cvlib::Mat mwarp_plate = cvlib::ip::getPerspectiveTransform(vsrcCorners, vdstCorners);
		dst.create(plate_height, plate_width, /*cvlib::MAT_Tuchar3*/src.type1());
		dst.zero();
		cvlib::ip::warpPerspective(src, dst, mwarp_plate);
		return mwarp_plate;
	}
	bool ZVDRegionDetector::detectCTRegion(const Mat& _image, const Vector<vec2f>& srcpoints, ZVDRegion& vdregion)
	{
		Mat image;
		Mat mwarp = warpImage(_image, image, srcpoints);
		IPDEBUG(image);

		cvlib::Mat src_gray, mhorizen, plateMask;
		cvlib::ColorSpace::RGBtoGray(image, src_gray);
		{
			cvlib::Mat src_gray_work;
			int newx = 128;
			int newy = cvlib::cvutil::round((float)newx * src_gray.rows() / src_gray.cols());
			cvlib::ip::resize(src_gray, src_gray_work, newx, newy);
			enhanceHorizen(src_gray_work, cvlib::Mat(), mhorizen);
			ip::Threshold(mhorizen, plateMask, 5, 255, ip::THRESH_TILE);
			plateMask = 255 - plateMask;

			IPDEBUG(imageGroup(src_gray_work, mhorizen));
			IPDEBUG(plateMask);

			cvlib::ip::resize(plateMask, plateMask, image.cols(), image.rows());
		}

		IPDEBUG(imageGroup(mhorizen, plateMask), "menhanced");

		cvlib::Vector<cvlib::vec2f> up_points(plateMask.cols()), down_points(plateMask.cols());
		int point_count = 0;
		int point_step = MAX(1, plateMask.cols() / 70);
		for (int i = 0; i < plateMask.cols(); i += point_step)
		{
			int up = 0, down = plateMask.rows() - 1;
			bool fup = false, fdown = false;
			for (int k = 0; k < plateMask.rows(); k++) {
				if (plateMask.data.ptr[k][i] == 255) {
					up = k;
					fup = true;
					break;
				}
			}
			if (fup) {
				up_points[point_count] = cvlib::vec2i(i, up);
				point_count++;
			}
		}
		up_points.resize(point_count);
		SimpleLine line = createSimpleLineFromRegions(up_points, 2.f);
		SimpleLine upLine, downLine;
		if (line.ypos(image.cols() / 2) > image.rows() / 2) {
			downLine = line;
			downLine.b += image.rows() / 20;
			upLine = downLine;
			upLine.b -= image.rows() / 2;
		}
		else {
			upLine = line;
			downLine = upLine;
			downLine.b += image.rows() / 2;
		}
		SimpleLine centerline = upLine;
		centerline.b += image.rows() / 4;

		Vecf vgray(image.cols());
		for (int x = 0; x < image.cols(); x++) {
			int y = centerline.ypos(x);
			if (y < 0 || y >= image.rows())
				continue;
			int value = (image.data.ptr[y][x * 3] + image.data.ptr[y][x * 3 + 1] + image.data.ptr[y][x * 3 + 2]) / 3;
			vgray[x] = value;
		}
		int cols = image.cols();
		Range range(cols / 20, cols - cols / 20);
		full_strip_detection strip = m_strip_predictor.detect(vgray, range);
#ifdef IPDEBUG
		for (int i = 0; i < 6; i++) {
			int x = strip.parts[i];
			image.drawLine(vec2i(x, 0), vec2i(x, image.rows() - 1), COLOR(255, 0, 0));
		}
		IPDEBUG(image);
#endif

		mwarp.invert();
		TiltedRect tregion;
		tregion.m_pts[0] = vec2i(strip.parts[4], upLine.ypos(strip.parts[4]));
		tregion.m_pts[1] = vec2i(strip.parts[4], downLine.ypos(strip.parts[4]));
		tregion.m_pts[2] = vec2i(strip.parts[3], downLine.ypos(strip.parts[3]));
		tregion.m_pts[3] = vec2i(strip.parts[3], upLine.ypos(strip.parts[3]));
		tregion.transform(mwarp);
		vdregion.setRect(VD_TestT, tregion);
		TiltedRect betweenCTline_region;
		betweenCTline_region.m_pts[0] = vec2i(strip.parts[3]-1, upLine.ypos(strip.parts[3]-1));
		betweenCTline_region.m_pts[1] = vec2i(strip.parts[3]-1, downLine.ypos(strip.parts[3]-1));
		betweenCTline_region.m_pts[2] = vec2i(strip.parts[2], downLine.ypos(strip.parts[2]));
		betweenCTline_region.m_pts[3] = vec2i(strip.parts[2], upLine.ypos(strip.parts[2]));
		betweenCTline_region.transform(mwarp);
		vdregion.setRect(VD_BetweenCTline, betweenCTline_region);
		TiltedRect cregion;
		cregion.m_pts[0] = vec2i(strip.parts[2]-1, upLine.ypos(strip.parts[2]-1));
		cregion.m_pts[1] = vec2i(strip.parts[2]-1, downLine.ypos(strip.parts[2]-1));
		cregion.m_pts[2] = vec2i(strip.parts[1], downLine.ypos(strip.parts[1]));
		cregion.m_pts[3] = vec2i(strip.parts[1], upLine.ypos(strip.parts[1]));
		cregion.transform(mwarp);
		vdregion.setRect(VD_controlC, cregion);
		return true;
	}
	void ZVDRegionDetector::makeImage(const Mat& image, Mat& mimg, const TiltedRect& tregion)
	{
		Vector<vec2f> srcpoints(4);
		srcpoints[0] = tregion.m_pts[0];
		srcpoints[1] = tregion.m_pts[1];
		srcpoints[2] = tregion.m_pts[2];
		srcpoints[3] = tregion.m_pts[3];
		warpImage(image, mimg, srcpoints);
	}

}