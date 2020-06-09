#include "connectedComponent.h"
#include "connectedComponent_c.h"

namespace cvlib { namespace ip {

/**********************************************************************/
	
ConnectInfo::ConnectInfo() {
	val = 0; pixels_count = 0; lastPos = 0;
}
ConnectInfo::ConnectInfo(const ConnectInfo& t) {
	region = t.region;
	val = t.val; pixels_count = t.pixels_count; lastPos = t.lastPos;
}
void ConnectInfo::operator=(const ConnectInfo& t){
	region = t.region;
	val = t.val; pixels_count = t.pixels_count; lastPos = t.lastPos;
//	return *this;
}

/**********************************************************************/
ConnectedComponent::ConnectedComponent(const ConnectedComponent::FilterParams& filterParams)
{
	m_filterparams = filterParams;
	m_pmwork = 0;
}
ConnectedComponent::~ConnectedComponent()
{
	m_mindex.release();
}

Vector<ConnectInfo> ConnectedComponent::apply(Mat& image, uchar eraseVal, bool ferase, bool fsort)
{
	m_pmwork = &image;
	m_mindex.create(image.size(), MAT_Tint);
	Vector<ip::PSConnectInfo> connects;
	Vector<ConnectInfo> ret_connects;

	ip::extractConnectComponent(&image, connects, &m_mindex, Rect(0, 0, image.cols(), image.rows()), eraseVal,
		m_filterparams.minW, m_filterparams.minH, m_filterparams.minPixelCount, ferase, fsort);

	if (connects.getSize() == 0)
		return ret_connects;

	ret_connects.resize(connects.getSize());
	for (int i = 0; i < connects.getSize(); i++) {
		ip::PSConnectInfo connect = connects[i];
		ConnectInfo& ret_connect = ret_connects[i];
		ret_connect.region = Rect(connect->sRect.x1, connect->sRect.y1, connect->sRect.width(), connect->sRect.height());
		ret_connect.pixels_count = connect->cnPixel;
		ret_connect.val = connect->bVal;
		ret_connect.lastPos = connect->nLastPos;
	}
	ip::releaseConnectComponent(connects);
	return ret_connects;
}

void	ConnectedComponent::createImageFromConnect(const ConnectInfo& conn, Mat& image)
{
	if (m_pmwork == 0)
		return;
	ip::SConnectInfo connectInfo;
	connectInfo.bVal = conn.val;
	connectInfo.cnPixel = conn.pixels_count;
	connectInfo.nLastPos = conn.lastPos;
	connectInfo.sRect.x1 = conn.region.x;
	connectInfo.sRect.y1 = conn.region.y;
	connectInfo.sRect.x2 = conn.region.limx() - 1;
	connectInfo.sRect.y2 = conn.region.limy() - 1;
	ip::createImageFromConnect(&connectInfo, m_mindex, image);
}
void	ConnectedComponent::createPointArrayFromConnect(const ConnectInfo& conn, Vector<Point2i>& points)
{
	points.removeAll();
	if (m_pmwork == 0)
		return;
	ip::SConnectInfo connectInfo;
	connectInfo.bVal = conn.val;
	connectInfo.cnPixel = conn.pixels_count;
	connectInfo.nLastPos = conn.lastPos;
	connectInfo.sRect.x1 = conn.region.x;
	connectInfo.sRect.y1 = conn.region.y;
	connectInfo.sRect.x2 = conn.region.limx() - 1;
	connectInfo.sRect.y2 = conn.region.limy() - 1;
	ip::createPointArrayFromConnect(&connectInfo, m_mindex, points);
}
void	ConnectedComponent::eraseConnect(const ConnectInfo& conn, uchar val)
{
	if (m_pmwork == 0)
		return;
	ip::SConnectInfo connectInfo;
	connectInfo.bVal = conn.val;
	connectInfo.cnPixel = conn.pixels_count;
	connectInfo.nLastPos = conn.lastPos;
	connectInfo.sRect.x1 = conn.region.x;
	connectInfo.sRect.y1 = conn.region.y;
	connectInfo.sRect.x2 = conn.region.limx() - 1;
	connectInfo.sRect.y2 = conn.region.limy() - 1;
	ip::eraseConnect(m_pmwork, &connectInfo, &m_mindex, val);
}

void	ConnectedComponent::createBoundaryPointsFromConnect(const ConnectInfo& conn, Vector<Point2i>& points)
{
	points.removeAll();
	int w = m_pmwork->cols();
	int maxpixels = conn.region.width*conn.region.height;
	int x = conn.lastPos % w;
	int y = conn.lastPos / w;
	assert(m_pmwork->data.ptr[y][x] == 255);
	ip::extractBoundaryPointsFromConnect(m_pmwork, x, y, conn.val, maxpixels * 2, points);

}

}
}
