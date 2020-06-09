#pragma once

#include "_cvlibbase.h"

namespace cvlib
{

	namespace ip
	{

		class CVLIB_DECLSPEC ConnectInfo
		{
		public:
			ConnectInfo();
			ConnectInfo(const ConnectInfo& t);
			void operator=(const ConnectInfo& t);
			Rect	region;
			uchar	val;			// pixel value
			int		pixels_count;		//number of pixels in the connect component
			int		lastPos;		//position (x*w+h) of last pixel in the search order in the connect component
		};

		class CVLIB_DECLSPEC ConnectedComponent
		{
		public:
			struct CVLIB_DECLSPEC FilterParams
			{
			public:
				FilterParams(int _minW = 1, int _minH = 1, int _minPixelCount = 1) {
					minW = _minW;
					minH = _minH;
					minPixelCount = _minPixelCount;
				}
				FilterParams(const FilterParams& t) {
					minW = t.minW;
					minH = t.minH;
					minPixelCount = t.minPixelCount;
				}
				int minW;
				int minH;
				int minPixelCount;
			};

			ConnectedComponent(const FilterParams& filterParams = FilterParams());
			~ConnectedComponent();

			Vector<ConnectInfo>	apply(Mat& image, uchar eraseVal = 0, bool ferase = false, bool fsort = true);
			void	createImageFromConnect(const ConnectInfo& conn, Mat& image);
			void	createPointArrayFromConnect(const ConnectInfo& conn, Vector<Point2i>& points);
			void	createBoundaryPointsFromConnect(const ConnectInfo& conn, Vector<Point2i>& points);
			void	eraseConnect(const ConnectInfo& conn, uchar val);
			Mat&	getWorkImage() { return *m_pmwork; }
		private:
			FilterParams	m_filterparams;
			Mat*	m_pmwork;
			Mat		m_mindex;
		};

	}

}
