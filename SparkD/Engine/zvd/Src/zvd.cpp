#include "ZVD.h"
#include "ZVD_impl.h"

namespace cvlib
{

	ZVD::ZVD()
	{
		m_pimpl = new ZVDImpl;
	}
	ZVD::~ZVD()
	{
		delete (ZVDImpl*)m_pimpl;
		IPDebug::resetDebug();
		IPDebug::resetLog();
	}
	bool	ZVD::loadData(const char* szpath)
	{
		return 	((ZVDImpl*)m_pimpl)->loadData(szpath);
	}
	bool	ZVD::isLoaded() const
	{
		return 	((ZVDImpl*)m_pimpl)->isLoaded();
	}
	bool	ZVD::detect(const Mat& image)
	{
		return ((ZVDImpl*)m_pimpl)->detect(image);
	}
	void	ZVD::getVDRegion(ZVDRegion& region)
	{
		return 	((ZVDImpl*)m_pimpl)->getVDRegion(region);
	}
	void	ZVD::getControlCImage(Mat& mat)
	{
		return 	((ZVDImpl*)m_pimpl)->getControlCImage(mat);
	}
	void	ZVD::getTestTImage(Mat& mat)
	{
		return 	((ZVDImpl*)m_pimpl)->getTestTImage(mat);
	}
	void	ZVD::getBetweenCTlineImage(Mat& mat)
	{
		return 	((ZVDImpl*)m_pimpl)->getBetweenCTlineImage(mat);
	}
    static unsigned char getMedian(const Mat& img)
    {
        int len = img.rows()*img.cols();
        Vecf v(len);
        for (int i = 0; i < len; i++)
        {
            v[i] = img.data.ptr[0][i];
        }
        uchar r = (uchar)v.median();
        return r;

    }
    COLOR  ZVD::getDominantColor(const Mat& image)
    {
        COLOR color;
        Mat mr, mg, mb;
        ColorSpace::split(image, mr, mg, mb);
        color.r = getMedian(mr);
        color.g = getMedian(mg);
        color.b = getMedian(mb);
        return color;
    }
	COLOR	ZVD::getDominantColor(const Mat& image, VDElemType elemtype)
	{
		Mat mat;
		((ZVDImpl*)m_pimpl)->getImage(image, elemtype, mat);
		return getDominantColor(mat);
	}

}
