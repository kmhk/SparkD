#pragma once

#include "_cvlibbase.h"

namespace cvlib
{

	enum VDElemType
	{
		VD_controlC,	// 0
		VD_TestT,		// 1
		VD_CTSide,			// 2
		VD_OppositeCTSide,	// 3
		VD_BetweenCTline,	// 4
		VD_IDline,		// 5
		VD_max
	};

	class CVLIB_DECLSPEC TiltedRect
	{
	public:
		TiltedRect();
		TiltedRect(const Rect& r);
		TiltedRect(const TiltedRect& r);
		TiltedRect(const Point2i _pts[4]);
		~TiltedRect();

		Rect getBoundRect() const;
		void drawTo(Mat& image, COLOR color) const;
		void rotateRegion();
		void scale(float s);
		void moveXY(int x, int y);
		void transform(const Mat& w);
		void flipLR(int w);

		Point2i m_pts[4];
	};

	class CVLIB_DECLSPEC ZVDRegion
	{
	public:
		ZVDRegion() {
			m_regions.resize((int)VD_max);
		}
		ZVDRegion(const ZVDRegion& t) {
			m_regions = t.m_regions;
		}
		virtual ~ZVDRegion() {}
		ZVDRegion& operator=(const ZVDRegion& t) {
			m_regions = t.m_regions;
			return *this;
		}
		TiltedRect getRect(VDElemType type) const {
			return m_regions[(int)type];
		};
		void setRect(VDElemType type, const TiltedRect& r) {
			m_regions[(int)type] = r;
		};
		void transform(const Mat& mwarp) {
			for (int i = 0; i < m_regions.size(); i++) {
				m_regions[i].transform(mwarp);
			}
		}

	private:

		std::vector<TiltedRect>	m_regions;
	};

	class CVLIB_DECLSPEC ZVD
	{
	public:
		ZVD();
		virtual ~ZVD();

		bool		detect(const Mat& image);
		void		getVDRegion(ZVDRegion& region);

		void		getControlCImage(Mat& mat);
		void		getTestTImage(Mat& mat);
		void		getBetweenCTlineImage(Mat& mat);
        COLOR       getDominantColor(const Mat& image);
	private:
		bool		loadData(const char* szpath);
		bool		isLoaded() const;

		void*		m_pimpl;
	};

}
