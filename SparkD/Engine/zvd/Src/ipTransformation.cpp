/*!
 * \file	ipTransformation.cpp
 * \ingroup ipCore
 * \brief   
 * \author  
 */

#include "ipTransformation.h"
#include "ipCoreABC.h"

namespace cvlib
{

static void setElementOfMat (const Mat& src, Mat& dst, int sx, int sy, int dx, int dy)
{
	int elem=src.elemSize ();
	memcpy (&dst.data.ptr[dy][elem*dx], &src.data.ptr[sy][elem*sx], elem);
}
static void setElementOfMat (Mat& dst, int dx, int dy, void* src)
{
	int elem = CVLIB_ELEM_SIZE(dst.type()) * dst.channels();
	memcpy (&dst.data.ptr[dy][elem*dx], src, elem);
}

template <typename _Tp>
class ipTransformation : public ipBaseCore
{
public:
	ipTransformation() {m_bkColor = Point4_<_Tp>((_Tp)0);m_transColor=Point4_<_Tp>((_Tp)0);}

	static float KernelBSpline(const float x);
	static float KernelLinear(const float t);
	static float KernelCubic(const float t);
	static float KernelGeneralizedCubic(const float t, const float a=-1);
	static float KernelLanczosSinc(const float t, const float r = 3);
	static float KernelBox(const float x);
	static float KernelHermite(const float x);
	static float KernelHamming(const float x);
	static float KernelSinc(const float x);
	static float KernelBlackman(const float x);
	static float KernelBessel_J1(const float x);
	static float KernelBessel_P1(const float x);
	static float KernelBessel_Q1(const float x);
	static float KernelBessel_Order1(float x);
	static float KernelBessel(const float x);
	static float KernelGaussian(const float x);
	static float KernelQuadratic(const float x);
	static float KernelMitchell(const float x);
	static float KernelCatrom(const float x);

	void SetBkcolor(COLOR color) {m_bkColor = color;}
	void SetTranscolor(COLOR color) {m_transColor = color;}
//	void SetBkcolor(uchar gray) {m_bkGray= gray;}
//	void SetTranscolor(uchar gray) {m_btransColor = gray;}
protected:
	void overflowCoordinates(const Mat& src, int &x, int &y, OverflowMethod const ofMethod);
	void overflowCoordinates(const Mat& src, float &x, float &y, OverflowMethod const ofMethod);
	_Tp* const getPixelColorWithOverflow(const Mat& src, int x, int y, OverflowMethod const ofMethod, _Tp* const rplColor);

	void addAveragingCont(_Tp const* color, float const surf, float* rr, int cn);
	Scalar_<_Tp> getAreaColorInterpolated(const Mat& src, float const xc, float const yc, float const w, float const h, 
		InterpolationMethod const inMethod, OverflowMethod const ofMethod=OM_BACKGROUND, _Tp* const rplColor = 0 );
	Scalar_<_Tp> getPixelColorInterpolated(const Mat& src, float x,float y, InterpolationMethod const inMethod=IM_BILINEAR, 
		OverflowMethod const ofMethod=OM_BACKGROUND, _Tp* const rplColor = 0);
	_Tp* BlindGetPixelColor(const Mat& src, const int x,const int y);
protected:
	Scalar_<_Tp> m_bkColor;
	Scalar_<_Tp> m_whiteColor;
	Scalar_<_Tp> m_transColor;
};

template <typename _Tp>
class ipRotate : public ipTransformation <_Tp>
{
public:
	ipRotate(){}
	ipRotate(const ipRotate& from);
	ipRotate(RotateType nType);
	ipRotate(float rAngle);
	ipRotate(float rAngle, InterpolationMethod inMethod/*=IM_BILINEAR*/,
		OverflowMethod ofMethod=OM_BACKGROUND, COLOR *replColor=0,
        bool const optimizeRightAngles=true, bool const bKeepOriginacolSize=false);
	virtual bool process (Mat* pmSrc, Mat* pmDst = NULL);
public:
	RotateType m_type;
	float m_rAngle;
	InterpolationMethod	m_inMethod;
	OverflowMethod m_ofMethod;
	COLOR* m_pRepColor;
	bool m_fOptimizeRightAngles;
	bool m_fKeepOriginalSize;
	int m_nAngle;
private:
	COLOR m_repColor;
};

template <typename _Tp>
class ipResample : public ipTransformation<_Tp>
{
public:
	ipResample(){m_newx=0;m_newy=0;}
	ipResample(const ipResample& from);
	ipResample(int newx, int newy, int mode = 1);
	ipResample(int newx, int newy, InterpolationMethod const inMethod=IM_BICUBIC2,
		OverflowMethod const ofMethod=OM_REPEAT, bool const disableAveraging=false);
	virtual bool process (Mat* pmSrc, Mat* pmDst = NULL);
public:
	int m_newx;
	int m_newy;
	int m_nMode;
	InterpolationMethod	m_inMethod;
	OverflowMethod m_ofMethod;
	bool m_fDisableAveraging;

	int m_nMethod;
};

template <typename _Tp>
class ipCrop : public ipTransformation<_Tp>
{
public:
	ipCrop(){}
	ipCrop(const ipCrop& from);
	ipCrop(int left, int top, int right, int bottom);
	ipCrop(const Rect& rect);
	ipCrop(int topx, int topy, int width, int height, float angle);
	virtual bool process (Mat* pmSrc, Mat* pmDst = NULL);
public:
	Rect m_rect;
	float m_rAngle;
	bool m_fAngle;

	int		m_nMethod;
};

template <typename _Tp>
class ipSkew : public ipTransformation<_Tp>
{
public:
	ipSkew(){}
	ipSkew(const ipSkew& from);
	ipSkew(float xgain, float ygain, int xpivot=0, int ypivot=0, bool bEnableInterpolation = false);
	virtual bool process (Mat* pmSrc, Mat* pmDst = NULL);
public:
	float m_rxgain;
	float m_rygain;
	int m_nxpivot;
	int m_nypivot;
	bool m_fEnableInterpolation;
};

template <typename _Tp>
class ipExpand : public ipTransformation<_Tp>
{
public:
	ipExpand(){}
	ipExpand(const ipExpand& from);
	ipExpand(int left, int top, int right, int bottom, COLOR canvascolor);
	virtual bool process (Mat* pmSrc, Mat* pmDst = NULL);
public:
	int m_nLeft;
	int m_nTop;
	int m_nRight;
	int m_nBottom;
	COLOR m_color;
};

template <typename _Tp>
class ipThumbnail : public ipTransformation<_Tp>
{
public:
	ipThumbnail(){}
	ipThumbnail(const ipThumbnail& from);
	ipThumbnail(int newx, int newy, COLOR canvascolor);
	virtual bool process (Mat* pmSrc, Mat* pmDst = NULL);
public:
	int m_newx;
	int m_newy;
	COLOR m_canvasColor;
};

/**
 * Recalculates coordinates according to specified overflow method.
 * If pixel (x,y) lies within image, nothing changes.
 *
 *  \param x, y - coordinates of pixel
 *  \param ofMethod - overflow method
 * 
 *  \return x, y - new coordinates (pixel (x,y) now lies inside image)
 *
 *  \author ***bd*** 2.2004
 */
template <typename _Tp>
void ipTransformation<_Tp>::overflowCoordinates(const Mat& src, int &x, int &y, OverflowMethod const ofMethod)
{
	if (src.isInside(x,y)) return;  //if pixel is within bounds, no change
	switch (ofMethod)
	{
    case OM_REPEAT:
		//clip coordinates
		x=MAX(x,0); x=MIN(x, src.cols()-1);
		y=MAX(y,0); y=MIN(y, src.rows()-1);
		break;
    case OM_WRAP:
		//wrap coordinates
		x = x % src.cols();
		y = y % src.rows();
		if (x<0) x = src.cols() + x;
		if (y<0) y = src.rows() + y;
		break;
    case OM_MIRROR:
		//mirror pixels near border
		if (x<0) x=((-x) % src.cols());
		else if (x>=src.cols()) x=src.cols()-(x % src.cols() + 1);
		if (y<0) y=((-y) % src.rows());
		else if (y>=src.rows()) y=src.rows()-(y % src.rows() + 1);
		break;
    default:
		return;
	}//switch
}

////////////////////////////////////////////////////////////////////////////////

template <typename _Tp>
void ipTransformation<_Tp>::overflowCoordinates(const Mat& src, float &x, float &y, OverflowMethod const ofMethod)
{
	if (x>=0 && x<src.cols() && y>=0 && y<src.rows()) return;  //if pixel is within bounds, no change
	switch (ofMethod)
	{
    case OM_REPEAT:
		//clip coordinates
		x=MAX(x,0); x=MIN(x, src.cols()-1);
		y=MAX(y,0); y=MIN(y, src.rows()-1);
		break;
    case OM_WRAP:
		//wrap coordinates
		x = (float)fmod(x, (float) src.cols());
		y = (float)fmod(y, (float) src.rows());
		if (x<0) x = src.cols() + x;
		if (y<0) y = src.rows() + y;
		break;
    case OM_MIRROR:
		//mirror pixels near border
		if (x<0) x=(float)fmod(-x, (float) src.cols());
		else if (x>=src.cols()) x=src.cols()-((float)fmod(x, (float) src.cols()) + 1);
		if (y<0) y=(float)fmod(-y, (float) src.rows());
		else if (y>=src.rows()) y=src.rows()-((float)fmod(y, (float) src.rows()) + 1);
		break;
    default:
		return;
	}//switch
}

////////////////////////////////////////////////////////////////////////////////
/**
 * Method return pixel color. Different methods are implemented for out of bounds pixels.
 * If an image has alpha channel, alpha value is returned in .RGBReserved.
 *
 *  \param x,y : pixel coordinates
 *  \param ofMethod : out-of-bounds method:
 *    - OF_WRAP - wrap over to pixels on other side of the image
 *    - OF_REPEAT - repeat last pixel on the edge
 *    - OF_COLOR - return input value of color
 *    - OF_BACKGROUND - return background color (if not set, return input color)
 *    - OF_TRANSPARENT - return transparent pixel
 *
 *  \param rplColor : input color (returned for out-of-bound coordinates in OF_COLOR mode and if other mode is not applicable)
 *
 * \return color : color of pixel
 * \author ***bd*** 2.2004
 */

template<typename _Tp> 
_Tp* const ipTransformation<_Tp>::getPixelColorWithOverflow(const Mat& src, int x, int y, OverflowMethod const ofMethod, _Tp* const rplColor)
{
  _Tp* color=0;          //color to return
  if ((!src.isInside(x,y)))
  {     //is pixel within bouns?:
    //pixel is out of bounds or no DIB
    color=rplColor;
    //pixel is out of bounds:
    switch (ofMethod)
	{
      case OM_TRANSPARENT:
          //no alpha transparency
		  if (rplColor == NULL)
		  {
			  color = (_Tp* const)&m_transColor;
		  }//if
		  return color;
      case OM_BACKGROUND:
		  //return background color (if it exists, otherwise input value)
		  if (rplColor == NULL)
		  {
			  color = (_Tp* const)&m_bkColor;
		  }//if
		  return color;
      case OM_REPEAT:
      case OM_WRAP:
      case OM_MIRROR:
			overflowCoordinates(src, x,y,ofMethod);
        break;
      default:
        //simply return replacement color (OM_COLOR and others)
        return color;
    }//switch
  }//if
  //just return specified pixel (it's within bounds)
  return BlindGetPixelColor(src, x,y);
}

////////////////////////////////////////////////////////////////////////////////
/**
 * This method reconstructs image according to chosen interpolation method and then returns pixel (x,y).
 * (x,y) can lie between actual image pixels. If (x,y) lies outside of image, method returns value
 * according to overflow method.
 * This method is very useful for geometrical image transformations, where destination pixel
 * can often assume color value lying between source pixels.
 *
 *  \param (x,y) - coordinates of pixel to return
 *           GPCI method recreates "analogue" image back from digital data, so x and y
 *           are float values and color value of point (1.1,1) will generally not be same
 *           as (1,1). Center of first pixel is at (0,0) and center of pixel right to it is (1,0).
 *           (0.5,0) is half way between these two pixels.
 *  \param inMethod - interpolation (reconstruction) method (kernel) to use:
 *    - IM_NEAREST_NEIGHBOUR - returns colour of nearest lying pixel (causes stairy look of 
 *                            processed images)
 *    - IM_BILINEAR - interpolates colour from four neighbouring pixels (softens image a bit)
 *    - IM_BICUBIC - interpolates from 16 neighbouring pixels (can produce "halo" artifacts)
 *    - IM_BICUBIC2 - interpolates from 16 neighbouring pixels (perhaps a bit less halo artifacts 
                     than IM_BICUBIC)
 *    - IM_BSPLINE - interpolates from 16 neighbouring pixels (softens image, washes colours)
 *                  (As far as I know, image should be prefiltered for this method to give 
 *                   good results... some other time :) )
 *                  This method uses bicubic interpolation kernel from CXImage 5.99a and older
 *                  versions.
 *    - IM_LANCZOS - interpolates from 12*12 pixels (slow, ringing artifacts)
 *
 *  \param ofMethod - overflow method (see comments at getPixelColorWithOverflow)
 *  \param rplColor - pointer to color used for out of borders pixels in OM_COLOR mode
 *              (and other modes if colour can't calculated in a specified way)
 *
 *  \return interpolated color value (including interpolated alpha value, if image has alpha layer)
 * 
 *  \author ***bd*** 2.2004
 */

template <typename _Tp>
Scalar_<_Tp> ipTransformation<_Tp>::getPixelColorInterpolated( const Mat& src, float x,float y, 
	InterpolationMethod const inMethod, 
	OverflowMethod const ofMethod, 
	_Tp* const rplColor)
{
  //calculate nearest pixel
  int xi=(int)(x); if (x<0) xi--;   //these replace (incredibly slow) floor (Visual c++ 2003, AMD Athlon)
  int yi=(int)(y); if (y<0) yi--;
  Scalar_<_Tp> color;                    //calculated colour
  int cn=src.channels();
	float rr[4];                //accumulated color values
	//some variables needed from here on
	int xii,yii;                      //x any y integer indexes for loops
	float kernel, kernelyc;           //kernel cache
	float kernelx[12], kernely[4];    //precalculated kernel values
  switch (inMethod)
  {
    case IM_NEAREST_NEIGHBOUR:
		{
			_Tp* const pixel = getPixelColorWithOverflow(src, (int)(x+0.5f), (int)(y+0.5f), ofMethod, rplColor);
			memcpy (&color[0], pixel, sizeof(_Tp)*cn);
			return color;
		}
    default:
	{
      //bilinear interpolation
      if (xi<-1 || xi>=src.cols() || yi<-1 || yi>=src.rows()) 
	  {  //all 4 points are outside bounds?:
        switch (ofMethod) 
		{
          case OM_COLOR: case OM_TRANSPARENT: case OM_BACKGROUND:
			  {
            //we don't need to interpolate anything with all points outside in this case
            _Tp* const pixel = getPixelColorWithOverflow(src, -999, -999, ofMethod, rplColor);
			memcpy (&color[0], pixel, sizeof(_Tp)*cn);
			return color;
			  }
          default:
            //recalculate coordinates and use faster method later on
            overflowCoordinates(src, x,y,ofMethod);
            xi=(int)(x); if (x<0) xi--;   //x and/or y have changed ... recalculate xi and yi
            yi=(int)(y); if (y<0) yi--;
        }//switch
      }//if
      //get four neighbouring pixels
      if ((xi+1)<src.cols() && xi>=0 && (yi+1)<src.rows() && yi>=0)
	  {
        //all pixels are inside RGB24 image... optimize reading (and use fixed point arithmetic)
        ushort wt1=(ushort)((x-xi)*256.0f), wt2=(ushort)((y-yi)*256.0f);
        ushort wd=wt1*wt2>>8;
        ushort wb=wt1-wd;
        ushort wc=wt2-wd;
        ushort wa=256-wt1-wc;

		_Tp** ppsrc = (_Tp**)src.data.ptr;
		for (int i=0; i<cn; i++)
			color[i] = (_Tp)(wa*ppsrc[yi][xi*cn] + wb*ppsrc[yi][(xi+1)*cn] + wc*ppsrc[yi+1][xi*cn] + wd*ppsrc[yi+1][(xi+1)*cn]);
        return color;
      } 
	  else 
	  {
        //default (slower) way to get pixels (not RGB24 or some pixels out of borders)
        float t1=x-xi, t2=y-yi;
        float d=t1*t2;
        float b=t1-d;
        float c=t2-d;
        float a=1-t1-c;

        _Tp* const rgb11=getPixelColorWithOverflow(src, xi, yi, ofMethod, rplColor);
        _Tp* const rgb21=getPixelColorWithOverflow(src, xi+1, yi, ofMethod, rplColor);
        _Tp* const rgb12=getPixelColorWithOverflow(src, xi, yi+1, ofMethod, rplColor);
        _Tp* const rgb22=getPixelColorWithOverflow(src, xi+1, yi+1, ofMethod, rplColor);
        //calculate linear interpolation
		for (int i=0; i<cn; i++)
			color[i] = (_Tp)(a*rgb11[i]+b*rgb21[i]+c*rgb12[i]+d*rgb22[i]);
        return color;
      }//if
    }//default
    case IM_BICUBIC: 
    case IM_BICUBIC2:
    case IM_BSPLINE:
	case IM_BOX:
	case IM_HERMITE:
	case IM_HAMMING:
	case IM_SINC:
	case IM_BLACKMAN:
	case IM_BESSEL:
	case IM_GAUSSIAN:
	case IM_QUADRATIC:
	case IM_MITCHELL:
	case IM_CATROM:
		{
					//bicubic interpolation(s)
				  if (((xi+2)<0) || ((xi-1)>=src.cols()) || ((yi+2)<0) || ((yi-1)>=src.rows()))
				  { //all points are outside bounds?:
					switch (ofMethod)
					{
					  case OM_COLOR: case OM_TRANSPARENT: case OM_BACKGROUND:
						  {
							//we don't need to interpolate anything with all points outside in this case
							_Tp* const pixel = getPixelColorWithOverflow (src, -999, -999, ofMethod, rplColor);
							for (int i=0; i<cn; i++)
								color[i] = pixel[i];
							break;
						  }
					  default:
						//recalculate coordinates and use faster method later on
						overflowCoordinates (src, x,y,ofMethod);
						xi=(int)(x); if (x<0) xi--;   //x and/or y have changed ... recalculate xi and yi
						yi=(int)(y); if (y<0) yi--;
					}//switch
				  }//if

				  //calculate multiplication factors for all pixels
				  int i;
				  switch (inMethod) 
				  {
					case IM_BICUBIC:
					  for (i=0; i<4; i++) 
					  {
						kernelx[i]=KernelCubic((float)(xi+i-1-x));
						kernely[i]=KernelCubic((float)(yi+i-1-y));
					  }//for i
					  break;
					case IM_BICUBIC2:
					  for (i=0; i<4; i++) 
					  {
						kernelx[i]=KernelGeneralizedCubic((float)(xi+i-1-x), -0.5);
						kernely[i]=KernelGeneralizedCubic((float)(yi+i-1-y), -0.5);
					  }//for i
					  break;
					case IM_BSPLINE:
					  for (i=0; i<4; i++) 
					  {
						kernelx[i]=KernelBSpline((float)(xi+i-1-x));
						kernely[i]=KernelBSpline((float)(yi+i-1-y));
					  }//for i
					  break;
					case IM_BOX:
					  for (i=0; i<4; i++) 
					  {
						kernelx[i]=KernelBox((float)(xi+i-1-x));
						kernely[i]=KernelBox((float)(yi+i-1-y));
					  }//for i
					  break;
					case IM_HERMITE:
					  for (i=0; i<4; i++) 
					  {
						kernelx[i]=KernelHermite((float)(xi+i-1-x));
						kernely[i]=KernelHermite((float)(yi+i-1-y));
					  }//for i
					  break;
					case IM_HAMMING:
					  for (i=0; i<4; i++) 
					  {
						kernelx[i]=KernelHamming((float)(xi+i-1-x));
						kernely[i]=KernelHamming((float)(yi+i-1-y));
					  }//for i
					  break;
					case IM_SINC:
					  for (i=0; i<4; i++) 
					  {
						kernelx[i]=KernelSinc((float)(xi+i-1-x));
						kernely[i]=KernelSinc((float)(yi+i-1-y));
					  }//for i
					  break;
					case IM_BLACKMAN:
					  for (i=0; i<4; i++) 
					  {
						kernelx[i]=KernelBlackman((float)(xi+i-1-x));
						kernely[i]=KernelBlackman((float)(yi+i-1-y));
					  }//for i
					  break;
					case IM_BESSEL:
					  for (i=0; i<4; i++) {
						kernelx[i]=KernelBessel((float)(xi+i-1-x));
						kernely[i]=KernelBessel((float)(yi+i-1-y));
					  }//for i
					  break;
					case IM_GAUSSIAN:
					  for (i=0; i<4; i++) {
						kernelx[i]=KernelGaussian((float)(xi+i-1-x));
						kernely[i]=KernelGaussian((float)(yi+i-1-y));
					  }//for i
					  break;
					case IM_QUADRATIC:
					  for (i=0; i<4; i++) {
						kernelx[i]=KernelQuadratic((float)(xi+i-1-x));
						kernely[i]=KernelQuadratic((float)(yi+i-1-y));
					  }//for i
					  break;
					case IM_MITCHELL:
					  for (i=0; i<4; i++) {
						kernelx[i]=KernelMitchell((float)(xi+i-1-x));
						kernely[i]=KernelMitchell((float)(yi+i-1-y));
					  }//for i
					  break;
					case IM_CATROM:
					  for (i=0; i<4; i++) {
						kernelx[i]=KernelCatrom((float)(xi+i-1-x));
						kernely[i]=KernelCatrom((float)(yi+i-1-y));
					  }//for i
					  break;
                          default:
                          assert(false);
				  }//switch
				  memset (rr, 0, sizeof(rr));
				  if (((xi+2)<src.cols()) && xi>=1 && ((yi+2)<src.rows()) && (yi>=1))
				  {
					//optimized interpolation (faster pixel reads) for RGB24 images with all pixels inside bounds
			//        uchar *pxptr, *pxptra;
					for (yii=yi-1; yii<yi+3; yii++)
					{
			//          pxptr=(uchar *)BlindGetPixelPointer(xi-1, yii);    //calculate pointer to first byte in row
					  kernelyc=kernely[yii-(yi-1)];
					  //alpha not supported or valid (optimized bicubic int. for no alpha channel)
						_Tp* pxptr = (_Tp*)&src.data.ptr[yii][(xi-1)*cn];
					  {
						kernel=kernelyc*kernelx[0];
						for (int icn=0; icn<cn; icn++) rr[icn] += kernel*(float)(*pxptr++);
						kernel=kernelyc*kernelx[1];
						for (int icn=0; icn<cn; icn++) rr[icn] += kernel*(float)(*pxptr++);
						kernel=kernelyc*kernelx[2];
						for (int icn=0; icn<cn; icn++) rr[icn] += kernel*(float)(*pxptr++);
						kernel=kernelyc*kernelx[3];
						for (int icn=0; icn<cn; icn++) rr[icn] += kernel*(float)(*pxptr++);
					  }
					}//yii
				  }
				  else
				  {
					//slower more flexible interpolation for border pixels and paletted images
					for (yii=yi-1; yii<yi+3; yii++)
					{
					  kernelyc=kernely[yii-(yi-1)];
					  for (xii=xi-1; xii<xi+3; xii++)
					  {
						kernel=kernelyc*kernelx[xii-(xi-1)];
						_Tp* const rgbs=getPixelColorWithOverflow (src, xii, yii, ofMethod, rplColor);
						for (int icn=0; icn<cn; icn++)
							rr[icn] += kernel*(float)rgbs[icn];
					  }//xii
					}//yii
				  }//if
				  //for all colors, clip to 0..255 and assign to COLOR
				  for (i=0; i<cn; i++)
				  {
					if (rr[i]>255) rr[i]=255; if (rr[i]<0) rr[i]=0;
				  }
				  return Scalar_<_Tp>((_Tp)rr[0],(_Tp)rr[1],(_Tp)rr[2],(_Tp)rr[3]);
		}
    case IM_LANCZOS:
      //lanczos window (16*16) sinc interpolation
      if (((xi+6)<0) || ((xi-5)>=src.cols()) || ((yi+6)<0) || ((yi-5)>=src.rows()))
	  {
        //all points are outside bounds
        switch (ofMethod)
		{
          case OM_COLOR: case OM_TRANSPARENT: case OM_BACKGROUND:
			  {
            //we don't need to interpolate anything with all points outside in this case
			  _Tp* const rgbs=getPixelColorWithOverflow (src, -999, -999, ofMethod, rplColor);
			  for (int icn=0; icn<cn; icn++)
				  color[icn] = rgbs[icn];
            return color;
			  }
            break;
          default:
            //recalculate coordinates and use faster method later on
            overflowCoordinates (src, x,y,ofMethod);
            xi=(int)(x); if (x<0) xi--;   //x and/or y have changed ... recalculate xi and yi
            yi=(int)(y); if (y<0) yi--;
        }//switch
      }//if

      for (xii=xi-5; xii<xi+7; xii++) 
		  kernelx[xii-(xi-5)]=KernelLanczosSinc((float)(xii-x), 6.0f);
	  memset (rr, sizeof(rr), 0);

      if (((xi+6)<src.cols()) && ((xi-5)>=0) && ((yi+6)<src.rows()) && ((yi-5)>=0))
	  {
        //optimized interpolation (faster pixel reads) for RGB24 images with all pixels inside bounds
        for (yii=yi-5; yii<yi+7; yii++)
		{
			_Tp *pxptr = &((_Tp*)src.data.ptr[yii])[(xi-5)*cn];
	        kernelyc=KernelLanczosSinc((float)(yii-y),6.0f);
			for (xii=0; xii<12; xii++) 
			{
			  kernel=kernelyc*kernelx[xii];
			  for (int icn=0; icn<cn; icn++)
				  rr[icn]+=kernel*(float)(*pxptr++);
			}//for xii
        }//yii
      }
	  else
	  {
        //slower more flexible interpolation for border pixels and paletted images
        for (yii=yi-5; yii<yi+7; yii++)
		{
          kernelyc=KernelLanczosSinc((float)(yii-y),6.0f);
          for (xii=xi-5; xii<xi+7; xii++)
		  {
            kernel=kernelyc*kernelx[xii-(xi-5)];
            _Tp* const rgbs=getPixelColorWithOverflow(src, xii, yii, ofMethod, rplColor);
			for (int icn=0; icn<cn; icn++)
				rr[icn]+=kernel*(float)rgbs[icn];
          }//xii
        }//yii
      }//if
      //for all colors, clip to 0..255 and assign to COLOR
	  for (int icn=0; icn<cn; icn++)
	  {
		if (rr[icn]>255) rr[icn]=255; if (rr[icn]<0) rr[icn]=0;
		color[icn]=(_Tp)rr[icn];
	  }
      return color;
  }//switch
}

////////////////////////////////////////////////////////////////////////////////
/**
 * Helper function for getAreaColorInterpolated.
 * Adds 'surf' portion of image pixel with color 'color' to (rr,gg,bb,aa).
 */
template <typename _Tp>
void ipTransformation<_Tp>::addAveragingCont(_Tp const *color, float const surf, float* rr, int cn)
{
	for (int i=0; i<cn; i++)
		rr[i]+=(float)color[i]*surf;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * This method is similar to getPixelColorInterpolated, but this method also properly handles 
 * subsampling.
 * If you need to sample original image with interval of more than 1 pixel (as when shrinking an image), 
 * you should use this method instead of getPixelColorInterpolated or aliasing will occur.
 * When area width and height are both less than pixel, this method gets pixel color by interpolating
 * color of frame center with selected (inMethod) interpolation by calling getPixelColorInterpolated. 
 * If width and height are more than 1, method calculates color by averaging color of pixels within area.
 * Interpolation method is not used in this case. Pixel color is interpolated by averaging instead.
 * If only one of both is more than 1, method uses combination of interpolation and averaging.
 * Chosen interpolation method is used, but since it is averaged later on, there is little difference
 * between IM_BILINEAR (perhaps best for this case) and better methods. IM_NEAREST_NEIGHBOUR again
 * leads to aliasing artifacts.
 * This method is a bit slower than getPixelColorInterpolated and when aliasing is not a problem, you should
 * simply use the later. 
 *
 * \param  xc, yc - center of (rectangular) area
 * \param  w, h - width and height of area
 * \param  inMethod - interpolation method that is used, when interpolation is used (see above)
 * \param  ofMethod - overflow method used when retrieving individual pixel colors
 * \param  rplColor - replacement colour to use, in OM_COLOR
 *
 * \author ***bd*** 2.2004
 */
template <typename _Tp>
Scalar_<_Tp> ipTransformation<_Tp>::getAreaColorInterpolated(const Mat& src,
  float const xc, float const yc, float const w, float const h, 
  InterpolationMethod const inMethod, 
  OverflowMethod const ofMethod, 
  _Tp* const rplColor)
{
	Scalar_<_Tp> color;      //calculated colour
	int cn = src.channels();
	if (h<=1 && w<=1)
	{
		//both width and height are less than one... we will use interpolation of center point
		return getPixelColorInterpolated (src, xc, yc, inMethod, ofMethod, rplColor);
	}
	else 
	{
		//area is wider and/or taller than one pixel:
		Rect_<float> area(xc-w/2.0f, yc-h/2.0f, w, h);   //area
		int xi1=(int)(area.x+0.49999999f);                //low x
		int yi1=(int)(area.y+0.49999999f);                //low y
		
		
		int xi2=(int)(area.limx()+0.5f);                      //top x
		int yi2=(int)(area.limy()+0.5f);                      //top y (for loops)
		
		float rr[4]={0};                                        //red, green, blue and alpha components
		int x,y;                                                  //loop counters
		float s=0;                                                //surface of all pixels
		float cps;                                                //surface of current crosssection
		if (h>1 && w>1) 
		{
			//width and height of area are greater than one pixel, so we can employ "ordinary" averaging
			Rect_<float> intBL, intTR;     //bottom left and top right intersection
			intBL=area.intersectRect(Rect_<float>(((float)xi1)-0.5f, ((float)yi1)-0.5f, 1.0f, 1.0f));
			intTR=area.intersectRect(Rect_<float>(((float)xi2)-0.5f, ((float)yi2)-0.5f, 1.0f, 1.0f));
			float wBL, wTR, hBL, hTR;
			wBL=intBL.width;            //width of bottom left pixel-area intersection
			hBL=intBL.height;           //height of bottom left...
			wTR=intTR.width;            //width of top right...
			hTR=intTR.height;           //height of top right...
			
			if (cn == 1)
			{
				rr[0] += (float)(*getPixelColorWithOverflow(src, xi1,yi1,ofMethod,rplColor))*wBL*hBL;    //bottom left pixel
				rr[0] += (float)(*getPixelColorWithOverflow(src, xi2,yi1,ofMethod,rplColor))*wTR*hBL;    //bottom right pixel
				rr[0] += (float)(*getPixelColorWithOverflow(src, xi1,yi2,ofMethod,rplColor))*wBL*hTR;    //top left pixel
				rr[0] += (float)(*getPixelColorWithOverflow(src, xi2,yi2,ofMethod,rplColor))*wTR*hTR;    //top right pixel
				//bottom and top row
				for (x=xi1+1; x<xi2; x++)
				{
					rr[0] += (float)(*getPixelColorWithOverflow(src, x,yi1,ofMethod,rplColor))*hBL;    //bottom row
					rr[0] += (float)(*getPixelColorWithOverflow(src, x,yi2,ofMethod,rplColor))*hTR;    //top row
				}
				//leftmost and rightmost column
				for (y=yi1+1; y<yi2; y++)
				{
					rr[0] += (float)(*getPixelColorWithOverflow(src, xi1,y,ofMethod,rplColor))*wBL;    //left column
					rr[0] += (float)(*getPixelColorWithOverflow(src, xi2,y,ofMethod,rplColor))*wTR;    //right column
				}
				for (y=yi1+1; y<yi2; y++)
				{
					for (x=xi1+1; x<xi2; x++) 
					{ 
						_Tp* const pixel = getPixelColorWithOverflow(src, x,y,ofMethod,rplColor);
						rr[0] += (float)*pixel;
					}//for x
				}//for y
			}
			else
			{
				addAveragingCont(getPixelColorWithOverflow(src, xi1,yi1,ofMethod,rplColor), wBL*hBL, rr, cn);    //bottom left pixel
				addAveragingCont(getPixelColorWithOverflow(src, xi2,yi1,ofMethod,rplColor), wTR*hBL, rr, cn);    //bottom right pixel
				addAveragingCont(getPixelColorWithOverflow(src, xi1,yi2,ofMethod,rplColor), wBL*hTR, rr, cn);    //top left pixel
				addAveragingCont(getPixelColorWithOverflow(src, xi2,yi2,ofMethod,rplColor), wTR*hTR, rr, cn);    //top right pixel
				//bottom and top row
				for (x=xi1+1; x<xi2; x++)
				{
					addAveragingCont(getPixelColorWithOverflow(src, x,yi1,ofMethod,rplColor), hBL, rr, cn);    //bottom row
					addAveragingCont(getPixelColorWithOverflow(src, x,yi2,ofMethod,rplColor), hTR, rr, cn);    //top row
				}
				//leftmost and rightmost column
				for (y=yi1+1; y<yi2; y++)
				{
					addAveragingCont(getPixelColorWithOverflow(src, xi1,y,ofMethod,rplColor), wBL, rr, cn);    //left column
					addAveragingCont(getPixelColorWithOverflow(src, xi2,y,ofMethod,rplColor), wTR, rr, cn);    //right column
				}
				for (y=yi1+1; y<yi2; y++)
				{
					for (x=xi1+1; x<xi2; x++) 
					{ 
						_Tp* const pixel = getPixelColorWithOverflow(src, x,y,ofMethod,rplColor);
						for (int icn=0; icn<cn; icn++)
							rr[icn] += (float)pixel[icn];
					}//for x
				}//for y
			}
		} 
		else 
		{
			//width or height greater than one:
			Rect_<float> intersect;                                          //intersection with current pixel
			Point2f center;
			for (y=yi1; y<=yi2; y++) 
			{
				for (x=xi1; x<=xi2; x++) 
				{
					intersect=area.intersectRect(Rect_<float>(((float)x)-0.5f, ((float)y)-0.5f, 1.0f, 1.0f));
					center=intersect.center();
					color=getPixelColorInterpolated(src, center.x, center.y, inMethod, ofMethod, rplColor);
					cps=intersect.area();
					for (int icn=0; icn<cn; icn++)
						rr[icn] += (float)color[icn]*cps;
				}//for x
			}//for y      
		}//if
		
		s=area.area();
		if (cn==1)
		{
			rr[0]/=s;
			if (rr[0]>255) rr[0]=255; if (rr[0]<0) rr[0]=0;
			color[0] = (_Tp)rr[0];
		}
		else
		{
			for (int icn=0; icn<cn; icn++)
			{
				rr[icn]/=s;
				if (rr[icn]>255) rr[icn]=255; if (rr[icn]<0) rr[icn]=0;
				color[icn] = (_Tp)rr[icn];
			}
		}
	}//if
	return color;
}
/**
* This is (a bit) faster version of getPixelColor. 
* It tests bounds only in debug mode (_DEBUG defined).
* 
* It is an error to request out-of-borders pixel with this method. 
* In DEBUG mode an exception will be thrown, and data will be violated in non-DEBUG mode. 
* \author ***bd*** 2.2004
  */
template <typename _Tp>
_Tp* ipTransformation<_Tp>::BlindGetPixelColor(const Mat& src, const int x,const int y)
{
#ifdef _DEBUG
  if (!src.isInside(x,y)) throw 0;
#endif
  return &((_Tp*)src.data.ptr[y])[x*src.channels()];
}
/*
uchar ipTransformation::BlindGetPixelColor(Mat* pSrc, const int x,const int y)
{
	return pSrc->data.ptr[y][x];
}*/

template <typename _Tp>
float ipTransformation<_Tp>::KernelBSpline(const float x)
{
	if (x>2.0f) return 0.0f;
	// thanks to Kristian Kratzenstein
	float a, b, c, d;
	float xm1 = x - 1.0f; // Was calculatet anyway cause the "if((x-1.0f) < 0)"
	float xp1 = x + 1.0f;
	float xp2 = x + 2.0f;

	if ((xp2) <= 0.0f) a = 0.0f; else a = xp2*xp2*xp2; // Only float, not float -> double -> float
	if ((xp1) <= 0.0f) b = 0.0f; else b = xp1*xp1*xp1;
	if (x <= 0) c = 0.0f; else c = x*x*x;  
	if ((xm1) <= 0.0f) d = 0.0f; else d = xm1*xm1*xm1;

	return (0.16666666666666666667f * (a - (4.0f * b) + (6.0f * c) - (4.0f * d)));

	/* equivalent <Vladim?r Kloucek>
	if (x < -2.0)
		return(0.0f);
	if (x < -1.0)
		return((2.0f+x)*(2.0f+x)*(2.0f+x)*0.16666666666666666667f);
	if (x < 0.0)
		return((4.0f+x*x*(-6.0f-3.0f*x))*0.16666666666666666667f);
	if (x < 1.0)
		return((4.0f+x*x*(-6.0f+3.0f*x))*0.16666666666666666667f);
	if (x < 2.0)
		return((2.0f-x)*(2.0f-x)*(2.0f-x)*0.16666666666666666667f);
	return(0.0f);
	*/
}

////////////////////////////////////////////////////////////////////////////////
/**
 * Bilinear interpolation kernel:
  \verbatim
          /
         | 1-t           , if  0 <= t <= 1
  h(t) = | t+1           , if -1 <= t <  0
         | 0             , otherwise
          \
  \endverbatim
 * ***bd*** 2.2004
 */
template <typename _Tp>
float ipTransformation<_Tp>::KernelLinear(const float t)
{
//  if (0<=t && t<=1) return 1-t;
//  if (-1<=t && t<0) return 1+t;
//  return 0;
	
	//<Vladim?r Kloucek>
	if (t < -1.0f)
		return 0.0f;
	if (t < 0.0f)
		return 1.0f+t;
	if (t < 1.0f)
		return 1.0f-t;
	return 0.0f;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * Bicubic interpolation kernel (a=-1):
  \verbatim
          /
         | 1-2|t|**2+|t|**3          , if |t| < 1
  h(t) = | 4-8|t|+5|t|**2-|t|**3     , if 1<=|t|<2
         | 0                         , otherwise
          \
  \endverbatim
 * ***bd*** 2.2004
 */
template <typename _Tp>
float ipTransformation<_Tp>::KernelCubic(const float t)
{
  float abs_t = (float)fabs(t);
  float abs_t_sq = abs_t * abs_t;
  if (abs_t<1) return 1-2*abs_t_sq+abs_t_sq*abs_t;
  if (abs_t<2) return 4 - 8*abs_t +5*abs_t_sq - abs_t_sq*abs_t;
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * Bicubic kernel (for a=-1 it is the same as BicubicKernel):
  \verbatim
          /
         | (a+2)|t|**3 - (a+3)|t|**2 + 1     , |t| <= 1
  h(t) = | a|t|**3 - 5a|t|**2 + 8a|t| - 4a   , 1 < |t| <= 2
         | 0                                 , otherwise
          \
  \endverbatim
 * Often used values for a are -1 and -1/2.
 */
template <typename _Tp>
float ipTransformation<_Tp>::KernelGeneralizedCubic(const float t, const float a)
{
  float abs_t = (float)fabs(t);
  float abs_t_sq = abs_t * abs_t;
  if (abs_t<1) return (a+2)*abs_t_sq*abs_t - (a+3)*abs_t_sq + 1;
  if (abs_t<2) return a*abs_t_sq*abs_t - 5*a*abs_t_sq + 8*a*abs_t - 4*a;
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * Lanczos windowed sinc interpolation kernel with radius r.
  \verbatim
          /
  h(t) = | sinc(t)*sinc(t/r)       , if |t|<r
         | 0                       , otherwise
          \
  \endverbatim
 * ***bd*** 2.2004
 */
template <typename _Tp>
float ipTransformation<_Tp>::KernelLanczosSinc(const float t, const float r)
{
  if (fabs(t) > r) return 0;
  if (t==0) return 1;
  float pit=(float)(CVLIB_PI*t);
  float pitd=pit/r;
  return (float)((sin(pit)/pit) * (sin(pitd)/pitd));
}

////////////////////////////////////////////////////////////////////////////////
template <typename _Tp>
float ipTransformation<_Tp>::KernelBox(const float x)
{
	if (x < -0.5f)
		return 0.0f;
	if (x < 0.5f)
		return 1.0f;
	return 0.0f;
}
////////////////////////////////////////////////////////////////////////////////
template <typename _Tp>
float ipTransformation<_Tp>::KernelHermite(const float x)
{
	if (x < -1.0f)
		return 0.0f;
	if (x < 0.0f)
		return (-2.0f*x-3.0f)*x*x+1.0f;
	if (x < 1.0f)
		return (2.0f*x-3.0f)*x*x+1.0f;
	return 0.0f;
//	if (fabs(x)>1) return 0.0f;
//	return(0.5f+0.5f*(float)cos(CVLIB_PI*x));
}
////////////////////////////////////////////////////////////////////////////////
template <typename _Tp>
float ipTransformation<_Tp>::KernelHamming(const float x)
{
	if (x < -1.0f)
		return 0.0f;
	if (x < 0.0f)
		return 0.92f*(-2.0f*x-3.0f)*x*x+1.0f;
	if (x < 1.0f)
		return 0.92f*(2.0f*x-3.0f)*x*x+1.0f;
	return 0.0f;
//	if (fabs(x)>1) return 0.0f;
//	return(0.54f+0.46f*(float)cos(CVLIB_PI*x));
}
////////////////////////////////////////////////////////////////////////////////
template <typename _Tp>
float ipTransformation<_Tp>::KernelSinc(const float x)
{
	if (x == 0.0)
		return(1.0);
	return((float)sin( (float)(CVLIB_PI*x) )/(float)(CVLIB_PI*x));
}
////////////////////////////////////////////////////////////////////////////////
template <typename _Tp>
float ipTransformation<_Tp>::KernelBlackman(const float x)
{
	//if (fabs(x)>1) return 0.0f;
	return (0.42f+0.5f*(float)cos(CVLIB_PI*x)+0.08f*(float)cos(2.0f*CVLIB_PI*x));
}
////////////////////////////////////////////////////////////////////////////////
template <typename _Tp>
float ipTransformation<_Tp>::KernelBessel_J1(const float x)
{
	double p, q;
	
	static const double
	Pone[] =
	{
		0.581199354001606143928050809e+21,
		-0.6672106568924916298020941484e+20,
		0.2316433580634002297931815435e+19,
		-0.3588817569910106050743641413e+17,
		0.2908795263834775409737601689e+15,
		-0.1322983480332126453125473247e+13,
		0.3413234182301700539091292655e+10,
		-0.4695753530642995859767162166e+7,
		0.270112271089232341485679099e+4
	},
	Qone[] =
	{
		0.11623987080032122878585294e+22,
		0.1185770712190320999837113348e+20,
		0.6092061398917521746105196863e+17,
		0.2081661221307607351240184229e+15,
		0.5243710262167649715406728642e+12,
		0.1013863514358673989967045588e+10,
		0.1501793594998585505921097578e+7,
		0.1606931573481487801970916749e+4,
		0.1e+1
	};
		
	p = Pone[8];
	q = Qone[8];
	for (int i=7; i >= 0; i--)
	{
		p = p*x*x+Pone[i];
		q = q*x*x+Qone[i];
	}
	return (float)(p/q);
}
////////////////////////////////////////////////////////////////////////////////
template <typename _Tp>
float ipTransformation<_Tp>::KernelBessel_P1(const float x)
{
	double p, q;
	
	int i;
	
	static const double
	Pone[] =
	{
		0.352246649133679798341724373e+5,
		0.62758845247161281269005675e+5,
		0.313539631109159574238669888e+5,
		0.49854832060594338434500455e+4,
		0.2111529182853962382105718e+3,
		0.12571716929145341558495e+1
	},
	Qone[] =
	{
		0.352246649133679798068390431e+5,
		0.626943469593560511888833731e+5,
		0.312404063819041039923015703e+5,
		0.4930396490181088979386097e+4,
		0.2030775189134759322293574e+3,
		0.1e+1
	};
		
	p = Pone[5];
	q = Qone[5];
	for (i=4; i >= 0; i--)
	{
		p = p*(8.0/x)*(8.0/x)+Pone[i];
		q = q*(8.0/x)*(8.0/x)+Qone[i];
	}
	return (float)(p/q);
}
////////////////////////////////////////////////////////////////////////////////
template <typename _Tp>
float ipTransformation<_Tp>::KernelBessel_Q1(const float x)
{
	double p, q;
	
	static const double
	Pone[] =
	{
		0.3511751914303552822533318e+3,
		0.7210391804904475039280863e+3,
		0.4259873011654442389886993e+3,
		0.831898957673850827325226e+2,
		0.45681716295512267064405e+1,
		0.3532840052740123642735e-1
	},
	Qone[] =
	{
		0.74917374171809127714519505e+4,
		0.154141773392650970499848051e+5,
		0.91522317015169922705904727e+4,
		0.18111867005523513506724158e+4,
		0.1038187585462133728776636e+3,
		0.1e+1
	};
		
	p = Pone[5];
	q = Qone[5];
	for (int i=4; i >= 0; i--)
	{
		p = p*(8.0/x)*(8.0/x)+Pone[i];
		q = q*(8.0/x)*(8.0/x)+Qone[i];
	}
	return (float)(p/q);
}
////////////////////////////////////////////////////////////////////////////////
template <typename _Tp>
float ipTransformation<_Tp>::KernelBessel_Order1(float x)
{
	float p, q;
	
	if (x == 0.0)
		return (0.0f);
	p = x;
	if (x < 0.0)
		x=(-x);
	if (x < 8.0)
		return(p*KernelBessel_J1(x));
	q = (float)sqrt(2.0f/(CVLIB_PI*x))*(float)(KernelBessel_P1(x)*(1.0f/sqrt(2.0f)*(sin(x)-cos(x)))-8.0f/x*KernelBessel_Q1(x)*
		(-1.0f/sqrt(2.0f)*(sin(x)+cos(x))));
	if (p < 0.0f)
		q = (-q);
	return (q);
}
////////////////////////////////////////////////////////////////////////////////
template <typename _Tp>
float ipTransformation<_Tp>::KernelBessel(const float x)
{
	if (x == 0.0f)
		return (float)(CVLIB_PI/4.0f);
	return(KernelBessel_Order1((float)(CVLIB_PI*x))/(2.0f*x));
}
////////////////////////////////////////////////////////////////////////////////
template <typename _Tp>
float ipTransformation<_Tp>::KernelGaussian(const float x)
{
	return (float)(exp(-2.0f*x*x)*0.79788456080287f/*sqrt(2.0f/CVLIB_PI)*/);
}
////////////////////////////////////////////////////////////////////////////////
template <typename _Tp>
float ipTransformation<_Tp>::KernelQuadratic(const float x)
{
	if (x < -1.5f)
		return(0.0f);
	if (x < -0.5f)
		return(0.5f*(x+1.5f)*(x+1.5f));
	if (x < 0.5f)
		return(0.75f-x*x);
	if (x < 1.5f)
		return(0.5f*(x-1.5f)*(x-1.5f));
	return(0.0f);
}
////////////////////////////////////////////////////////////////////////////////
template <typename _Tp>
float ipTransformation<_Tp>::KernelMitchell(const float x)
{
#define KM_B (1.0f/3.0f)
#define KM_C (1.0f/3.0f)
#define KM_P0 ((  6.0f - 2.0f * KM_B ) / 6.0f)
#define KM_P2 ((-18.0f + 12.0f * KM_B + 6.0f * KM_C) / 6.0f)
#define KM_P3 (( 12.0f - 9.0f  * KM_B - 6.0f * KM_C) / 6.0f)
#define KM_Q0 ((  8.0f * KM_B + 24.0f * KM_C) / 6.0f)
#define KM_Q1 ((-12.0f * KM_B - 48.0f * KM_C) / 6.0f)
#define KM_Q2 ((  6.0f * KM_B + 30.0f * KM_C) / 6.0f)
#define KM_Q3 (( -1.0f * KM_B -  6.0f * KM_C) / 6.0f)
	
	if (x < -2.0)
		return(0.0f);
	if (x < -1.0)
		return(KM_Q0-x*(KM_Q1-x*(KM_Q2-x*KM_Q3)));
	if (x < 0.0f)
		return(KM_P0+x*x*(KM_P2-x*KM_P3));
	if (x < 1.0f)
		return(KM_P0+x*x*(KM_P2+x*KM_P3));
	if (x < 2.0f)
		return(KM_Q0+x*(KM_Q1+x*(KM_Q2+x*KM_Q3)));
	return(0.0f);
}
////////////////////////////////////////////////////////////////////////////////
template <typename _Tp>
float ipTransformation<_Tp>::KernelCatrom(const float x)
{
	if (x < -2.0)
		return(0.0f);
	if (x < -1.0)
		return(0.5f*(4.0f+x*(8.0f+x*(5.0f+x))));
	if (x < 0.0)
		return(0.5f*(2.0f+x*x*(-5.0f-3.0f*x)));
	if (x < 1.0)
		return(0.5f*(2.0f+x*x*(-5.0f+3.0f*x)));
	if (x < 2.0)
		return(0.5f*(4.0f+x*(-8.0f+x*(5.0f-x))));
	return(0.0f);
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//ipRotate


template <typename _Tp>
ipRotate<_Tp>::ipRotate(const ipRotate& from)
{
	m_type = from.m_type;
	m_rAngle = from.m_rAngle;
	m_inMethod = from.m_inMethod;
	m_ofMethod = from.m_ofMethod;
	m_pRepColor = from.m_pRepColor;
	m_fOptimizeRightAngles = from.m_fOptimizeRightAngles;
	m_fKeepOriginalSize = from.m_fKeepOriginalSize;
	m_nAngle = from.m_nAngle;
	m_repColor = from.m_repColor;
}

template <typename _Tp>
ipRotate<_Tp>::ipRotate(RotateType nType)
{
	m_type = nType;
	m_nAngle = 0;
}

template <typename _Tp>
ipRotate<_Tp>::ipRotate(float rAngle)
{
	m_nAngle = 1;
	m_rAngle = rAngle;
}

template <typename _Tp>
ipRotate<_Tp>::ipRotate(float rAngle, InterpolationMethod inMethod/*=IM_BILINEAR*/,
		 OverflowMethod ofMethod/*=OM_BACKGROUND*/, COLOR *replColor/*=0*/,
		 bool const optimizeRightAngles/*=true*/, bool const bKeepOriginalSize/*=false*/)
{
	m_nAngle = 2;
	m_rAngle = rAngle;
	m_inMethod = inMethod;
	m_ofMethod = ofMethod;
	if (replColor)
	{
		m_pRepColor = &m_repColor;
		m_repColor = *replColor;
	}
	else
		m_pRepColor = NULL;
	m_fOptimizeRightAngles = optimizeRightAngles;
	m_fKeepOriginalSize = bKeepOriginalSize;
}

template <typename _Tp>
bool ipRotate<_Tp>::process (Mat* pmSrc, Mat* pmDst /*= NULL*/)
{
	if (m_nAngle == 0)
	{
		switch (m_type) {
		case RLeft:
		{
			Mat tempMat(pmSrc->cols(), pmSrc->rows(), pmSrc->type1());
			uchar **ppbSrc, **ppbDst;
			ppbSrc = pmSrc->data.ptr;
			ppbDst = tempMat.data.ptr;
			if (pmSrc->channels() == 1)
			{
				for (int y = 0; y < pmSrc->rows(); y++)
				{
					for (int x = 0; x < pmSrc->cols(); x++)
					{
						ppbDst[pmSrc->cols() - x - 1][y] = ppbSrc[y][x];
					}
				}
			}
			else {
				int cn = pmSrc->channels();
				int dstH = pmSrc->cols();
				for (int y = 0, ycn = 0; y < pmSrc->rows(); y++, ycn += cn)
				{
					uchar* psrc = ppbSrc[y];
					for (int x = 0, xcn = 0; x < dstH; x++)
					{
						for (int k = 0; k < cn; k++, xcn++)
							ppbDst[dstH - 1 - x][ycn + k] = psrc[xcn];
					}
				}
			}
			if (pmDst)
				*pmDst = tempMat;
			else
				*pmSrc = tempMat;
		}
		break;
		case RRight:
		{
			Mat tempMat(pmSrc->cols(), pmSrc->rows(), pmSrc->type1());
			uchar **ppbSrc, **ppbDst;
			ppbSrc = pmSrc->data.ptr;
			ppbDst = tempMat.data.ptr;
			if (pmSrc->channels() == 1)
			{
				for (int y = 0; y < pmSrc->rows(); y++)for (int x = 0; x < pmSrc->cols(); x++)
					ppbDst[x][pmSrc->rows() - y - 1] = ppbSrc[y][x];
			}
			else
			{
				int cn = pmSrc->channels();
				int rows = pmSrc->rows();
				int ycn = (rows - 1)*cn;
				for (int y = 0; y < rows; y++, ycn -= cn)
				{
					uchar* psrc = ppbSrc[y];
					for (int x = 0, xcn = 0; x < pmSrc->cols(); x++)
					{
						for (int k = 0; k < cn; k++, xcn++)
							ppbDst[x][ycn + k] = psrc[xcn];
					}

				}
			}
			if (pmDst)
				*pmDst = tempMat;
			else
				*pmSrc = tempMat;
		}
		break;
		case R180:
		{
			Mat tempMat(pmSrc->rows(), pmSrc->cols(), pmSrc->type1());
			uchar **ppbSrc, **ppbDst;
			ppbSrc = pmSrc->data.ptr;
			ppbDst = tempMat.data.ptr;

			if (pmSrc->channels() == 1)
			{
				for (int y = 0; y < pmSrc->rows(); y++)
				{
					for (int x = 0; x < pmSrc->cols(); x++)
					{
						ppbDst[pmSrc->rows() - y - 1][pmSrc->cols() - x - 1] = ppbSrc[y][x];
					}
				}
			}
			else {
				int cn = pmSrc->channels();
				for (int y = 0; y < pmSrc->rows(); y++)for (int x = 0; x < pmSrc->cols(); x++)
				{
					for (int k = 0; k < cn; k++)
						ppbDst[pmSrc->rows() - y - 1][(pmSrc->cols() - x - 1)*cn + k] = ppbSrc[y][x*cn + k];
				}
			}
			if (pmDst)
				*pmDst = tempMat;
			else
				*pmSrc = tempMat;
		}
		break;
		}
	}
	else if (m_nAngle == 1)
	{
		double ang = m_rAngle*acos((float)0)/90;
		int newWidth, newHeight;
		int nWidth = pmSrc->cols();
		int nHeight= pmSrc->rows();
		double cos_angle = cos(ang);
		double sin_angle = sin(ang);
		
		// Calculate the size of the new bitmap
		Point2i p1(0,0);
		Point2i p2(nWidth,0);
		Point2i p3(0,nHeight);
		Point2i p4(nWidth-1,nHeight);
		Point2i newP1,newP2,newP3,newP4, leftTop, rightTop, leftBottom, rightBottom;
		
		newP1.x = p1.x;
		newP1.y = p1.y;
		newP2.x = (int)cvutil::round(p2.x*cos_angle - p2.y*sin_angle);
		newP2.y = (int)cvutil::round(p2.x*sin_angle + p2.y*cos_angle);
		newP3.x = (int)cvutil::round(p3.x*cos_angle - p3.y*sin_angle);
		newP3.y = (int)cvutil::round(p3.x*sin_angle + p3.y*cos_angle);
		newP4.x = (int)cvutil::round(p4.x*cos_angle - p4.y*sin_angle);
		newP4.y = (int)cvutil::round(p4.x*sin_angle + p4.y*cos_angle);
		
		leftTop.x = MIN(MIN(newP1.x,newP2.x),MIN(newP3.x,newP4.x));
		leftTop.y = MIN(MIN(newP1.y,newP2.y),MIN(newP3.y,newP4.y));
		rightBottom.x = MAX(MAX(newP1.x,newP2.x),MAX(newP3.x,newP4.x));
		rightBottom.y = MAX(MAX(newP1.y,newP2.y),MAX(newP3.y,newP4.y));
		leftBottom.x = leftTop.x;
		leftBottom.y = 2+rightBottom.y;
		rightTop.x = 2+rightBottom.x;
		rightTop.y = leftTop.y;
		
		newWidth = rightTop.x - leftTop.x;
		newHeight= leftBottom.y - leftTop.y;

		Mat tempMat(newHeight, newWidth, pmSrc->type());
		
		int x,y,newX,newY,oldX,oldY;

		uchar **ppbSrc, **ppbDst;
		ppbSrc = pmSrc->data.ptr;
		ppbDst = tempMat.data.ptr;
		
		if (pmSrc->channels()==1)
		{
			for (y = leftTop.y, newY = 0; y<leftBottom.y; y++,newY++)
			{
				for (x = leftTop.x, newX = 0; x<rightTop.x; x++,newX++)
				{
					oldX = cvutil::round(x*cos_angle + y*sin_angle);
					oldY = cvutil::round(y*cos_angle - x*sin_angle);
					oldX = oldX < 0 ? 0 : oldX < pmSrc->cols() ? oldX : pmSrc->cols()-1;
					oldY = oldY < 0 ? 0 : oldY < pmSrc->rows() ? oldY : pmSrc->rows()-1;

					ppbDst[newY][newX] = ppbSrc[oldY][oldX];
				}
			}
		}
		else
		{
			int cn=pmSrc->channels();
			for (y = leftTop.y, newY = 0; y<leftBottom.y; y++,newY++)
			{
				for (x = leftTop.x, newX = 0; x<rightTop.x; x++,newX++)
				{
					oldX = cvutil::round(x*cos_angle + y*sin_angle);
					oldY = cvutil::round(y*cos_angle - x*sin_angle);
					oldX = oldX < 0 ? 0 : oldX < pmSrc->cols() ? oldX : pmSrc->cols()-1;
					oldY = oldY < 0 ? 0 : oldY < pmSrc->rows() ? oldY : pmSrc->rows()-1;

					memcpy(&ppbDst[newY][newX*cn], &ppbSrc[oldY][oldX*cn], cn);
				}
			}
		}
		//select the destination
		if (pmDst) 
		{
			pmDst->release();
			pmDst->create (tempMat, true);
		}
		else
		{
			pmSrc->release();
			pmSrc->create (tempMat, true);
		}
	}
	else if (m_nAngle == 2)
	{
		double ang = -m_rAngle*acos(0.0f)/90.0f;		//convert angle to radians and invert (positive angle performs clockwise rotation)
		float cos_angle = (float) cos(ang);			//these two are needed later (to rotate)
		float sin_angle = (float) sin(ang);
		
		float cx = (float)(pmSrc->cols() / 2);
		float cy = (float)(pmSrc->rows() / 2);

		int i;

		Point2f p[4];
		p[0] = Point2f(-cx, cy);
		p[1] = Point2f(cx, cy);
		p[2] = Point2f(cx, -cy);
		p[3] = Point2f(-cx, -cy);

		Point2f newp[4];								//rotated positions of corners
		for (i = 0; i < 4; i++) 
		{
			newp[i].x = (p[i].x*cos_angle - p[i].y*sin_angle);
			newp[i].y = (p[i].x*sin_angle + p[i].y*cos_angle);
		}

		if (m_rAngle >= 0)
		{
			newp[0].x = newp[0].x / newp[0].y * cy;
			newp[0].y = cy;
			
			newp[1].y = newp[1].y / newp[1].x * cx;
			newp[1].x = cx;
			
			newp[2].x = -newp[2].x / newp[2].y * cy;
			newp[2].y = -cy;
			
			newp[3].y = -newp[3].y / newp[3].x * cx;
			newp[3].x = -cx;

		}
		else
		{
			newp[0].y = -newp[0].y / newp[0].x * cx;
			newp[0].x = -cx;

			newp[1].x = newp[1].x / newp[1].y * cy;
			newp[1].y = cy;

			newp[2].y = newp[2].y / newp[2].x * cx;
			newp[2].x = cx;

			newp[3].x = -newp[3].x / newp[3].y * cy;
			newp[3].y = -cy;

		}

		for (i = 0; i < 4; i++) 
		{
			p[i].x = (newp[i].x*cos_angle + newp[i].y*sin_angle);
			p[i].y = (-newp[i].x*sin_angle + newp[i].y*cos_angle);
		}

		float startx = (float)MAX(p[0].x, p[3].x);
		float starty = (float)MIN(p[0].y, p[1].y);

		float endx = (float)MIN(p[1].x, p[2].x);
		float endy = (float)MAX(p[2].y, p[3].y);

		int newWidth, newHeight;
		newWidth = (int)floor(endx - startx);
		newHeight = (int)floor(starty - endy);
		Mat matTemp(newHeight, newWidth, pmSrc->type1());

		int y, x;
		int yy, xx;
		int yy1, xx1;
		if (pmSrc->channels() == 1)
		{
			for(y = 0; y < newHeight; y++)
			{
				for(x = 0; x < newWidth; x++)
				{
					xx = (int)(x + startx);
					yy = (int)(starty - y);
				
					xx1 = (int)floor(cos_angle * xx - sin_angle * yy);
					yy1 = (int)floor(sin_angle * xx + cos_angle * yy);
				
					xx = (int)(xx1 + cx);
					yy = (int)(-yy1 + cy);
				
					matTemp.data.ptr[y][x] = pmSrc->data.ptr[yy][xx];
				}
			}
		}
		else
		{
			int cn=pmSrc->channels();
			for(y = 0; y < newHeight; y++)
			{
				for(x = 0; x < newWidth; x++)
				{
					xx = (int)(x + startx);
					yy = (int)(starty - y);
				
					xx1 = (int)floor(cos_angle * xx - sin_angle * yy);
					yy1 = (int)floor(sin_angle * xx + cos_angle * yy);
				
					xx = (int)(xx1 + cx);
					yy = (int)(-yy1 + cy);
				
					memcpy (&matTemp.data.ptr[y][x*cn], &pmSrc->data.ptr[yy][xx*cn], cn);
				}
			}
		}

		if (pmDst) 
		{
			pmDst->release();
			pmDst->create (matTemp, true);
		}
		else
		{
			pmSrc->release();
			pmSrc->create (matTemp, true);
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
//ipResample
template <typename _Tp>
ipResample<_Tp>::ipResample(int newx, int newy, int mode /*= 1*/)
{
	m_newx = newx;
	m_newy = newy;
	m_nMode = mode;
	m_nMethod = 1;
}

template <typename _Tp>
ipResample<_Tp>::ipResample(int newx, int newy, InterpolationMethod const inMethod/*=IM_BICUBIC2*/,
		   OverflowMethod const ofMethod/*=OM_REPEAT*/, bool const disableAveraging/*=false*/)
{
	m_newx = newx;
	m_newy = newy;
	m_inMethod = inMethod;
	m_ofMethod = ofMethod;
	m_fDisableAveraging = disableAveraging;
	m_nMethod = 2;
}

template <typename _Tp>
ipResample<_Tp>::ipResample(const ipResample& from)
{
	m_newx = from.m_newx;
	m_newy = from.m_newy;
	m_inMethod = from.m_inMethod;
	m_ofMethod = from.m_ofMethod;
	m_fDisableAveraging = from.m_fDisableAveraging;
	m_nMethod = from.m_nMethod;
}

template <typename _Tp>
bool ipResample<_Tp>::process (Mat* pSrc, Mat* pDst /*= NULL*/)
{
	if ( pDst && !ipBaseCore::equalType(pSrc, pDst) )
	{
		pDst->release();
		pDst->create(*pSrc);
	}

	int src_width = pSrc->cols();
	int src_height = pSrc->rows();
	int cn=  pSrc->channels();

	if (m_nMethod == 1)
	{
		if (m_newx==0 || m_newy==0) return false;

		if (pSrc->cols()==m_newx && pSrc->rows()==m_newy)
		{
			if (pDst)
			{
				pDst->release();
				pDst->create (*pSrc, true);
			}
			return true;
		}

		float xScale, yScale, fX, fY;
		xScale = (float)pSrc->cols()  / (float)m_newx;
		yScale = (float)pSrc->rows() / (float)m_newy;
		Mat newImage(m_newy, m_newx, pSrc->type1());
		if (!newImage.isValid()) 
			return false;

		switch (m_nMode) 
		{
		case 1: // nearest pixel
		{ 
			fY = 0;
			for(int y=0; y<m_newy; y++, fY+=yScale)
			{
				fX = 0;
				uchar* dst = newImage.data.ptr[y];
				uchar* src = pSrc->data.ptr[(int)MIN(fY,src_height-1)];
				int idx=0;
				for(int x=0; x<m_newx; x++, fX+=xScale)
				{
					for (int k=0; k<cn; k++, idx++)
						dst [idx] = src[((int)fX)*cn+k];
				}
			}
			break;
		}
		case 2: // bicubic interpolation by Blake L. Carlson <blake-carlson(at)uiowa(dot)edu
		{
			float f_x, f_y, a, b, rr[4], r1, r2;
			int   i_x, i_y, xx, yy;

			_Tp** ppsrc = (_Tp**)pSrc->data.ptr;
			_Tp** ppdst = (_Tp**)newImage.data.ptr;

			for(int y=0; y<m_newy; y++)
			{
				f_y = (float) y * yScale - 0.5f;
				i_y = (int) floor(f_y);
				a   = f_y - (float)floor(f_y);
				for(int x=0; x<m_newx; x++)
				{
					f_x = (float) x * xScale - 0.5f;
					i_x = (int) floor(f_x);
					b   = f_x - (float)floor(f_x);

					for (int icn=0; icn<cn; icn++)
						rr[icn]=0;
					for(int m=-1; m<3; m++) 
					{
						r1 = this->KernelBSpline((float) m - a);
						yy = i_y+m;
						if (yy<0) 
							yy=0;
						if (yy>=src_height) 
							yy = src_height-1;
						for(int n=-1; n<3; n++) 
						{
							r2 = r1 * this->KernelBSpline(b - (float)n);
							xx = i_x+n;
							if (xx<0) xx=0;
							if (xx>=src_width) 
								xx=src_width-1;
							for (int icn=0, idx=xx*cn; icn<cn; icn++, idx++)
								rr[icn] += (float)(ppsrc[yy][idx] * r2);
						}
					}
					for (int icn=0, idx=x*cn; icn<cn; icn++, idx++)
						ppdst[y][idx] = (_Tp)rr[icn];
				}
			}
			break;
		}
		default: // bilinear interpolation
//			if (!(src_width>m_newx && src_height>m_newy)) 
			{
				//?1999 Steve McMahon (steve@dogma.demon.co.uk)
				int ifX, ifY, ifX1, ifY1, xmax, ymax;
				float ir1, ir2, dx, dy;
				xmax = src_width-1;
				ymax = src_height-1;
				_Tp** ppsrc = (_Tp**)pSrc->data.ptr;
				_Tp** ppdst = (_Tp**)newImage.data.ptr;
				fY = 0;
				for(int y=0; y<m_newy; y++, fY+=yScale)
				{
					ifY = (int)fY;
					ifY1 = MIN(ymax, ifY+1);
					dy = fY - ifY;
					_Tp* ppsrc_ifY = ppsrc[ifY];
					_Tp* ppsrc_ifY1 = ppsrc[ifY1];
					_Tp* ppdst_y=ppdst[y];
					fX = 0;
					for(int x=0, xcn=0; x<m_newx; x++, fX+=xScale)
					{
						ifX = (int)fX;
						ifX1 = MIN(xmax, ifX+1);
						dx = fX - ifX;
						// interpolate using the four nearest pixels in the source
						for (int icn=0; icn<cn; icn++) {
							// Interplate in x direction:
							ir1 = (float)(ppsrc_ifY[ifX*cn+icn]  * (1 - dy) + ppsrc_ifY1[ifX*cn+icn] * dy);
							ir2 = (float)(ppsrc_ifY[ifX1*cn+icn]  * (1 - dy) + ppsrc_ifY1[ifX1*cn+icn] * dy);
							// interpolate in y:
							ppdst_y[xcn++] = (_Tp)(ir1 * (1 - dx) + ir2 * dx);
						}
					}
				} 
			}
//			else 
			{
// 				//high resolution shrink, thanks to Henrik Stellmann <henrik.stellmann@volleynet.de>
// 				const int ACCURACY = 1000;
// 				int i,j; // index for faValue
// 				int x,y; // coordinates in  source image
// 				uchar* pSource;
// 				uchar* pDest = newImage.m_Info.pImage;
// 				int* naAccu  = new int[3 * m_newx + 3];
// 				int* naCarry = new int[3 * m_newx + 3];
// 				int* naTemp;
// 				int  nWeightX,nWeightY;
// 				float fEndX;
// 				int nScale = (int)(ACCURACY * xScale * yScale);
// 
// 				memset(naAccu,  0, sizeof(int) * 3 * m_newx);
// 				memset(naCarry, 0, sizeof(int) * 3 * m_newx);
// 
// 				int u, v = 0; // coordinates in dest image
// 				float fEndY = yScale - 1.0f;
// 				for (y = 0; y < pSrc->height(); y++)
// 				{
// 					pSource = info.pImage + y * info.dwEffWidth;
// 					u = i = 0;
// 					fEndX = xScale - 1.0f;
// 					if ((float)y < fEndY) 
// 					{       // complete source row goes into dest row
// 						for (x = 0; x < pSrc->width(); x++)
// 						{
// 							if ((float)x < fEndX)
// 							{       // complete source pixel goes into dest pixel
// 								for (j = 0; j < 3; j++)	
// 									naAccu[i + j] += (*pSource++) * ACCURACY;
// 							} 
// 							else 
// 							{       // source pixel is splitted for 2 dest pixels
// 								nWeightX = (int)(((float)x - fEndX) * ACCURACY);
// 								for (j = 0; j < 3; j++)
// 								{
// 									naAccu[i] += (ACCURACY - nWeightX) * (*pSource);
// 									naAccu[3 + i++] += nWeightX * (*pSource++);
// 								}
// 								fEndX += xScale;
// 								u++;
// 							}
// 						}
// 					}
// 					else 
// 					{       // source row is splitted for 2 dest rows       
// 						nWeightY = (int)(((float)y - fEndY) * ACCURACY);
// 						for (x = 0; x < pSrc->width(); x++)
// 						{
// 							if ((float)x < fEndX)
// 							{       // complete source pixel goes into 2 pixel
// 								for (j = 0; j < 3; j++)
// 								{
// 									naAccu[i + j] += ((ACCURACY - nWeightY) * (*pSource));
// 									naCarry[i + j] += nWeightY * (*pSource++);
// 								}
// 							} 
// 							else 
// 							{       // source pixel is splitted for 4 dest pixels
// 								nWeightX = (int)(((float)x - fEndX) * ACCURACY);
// 								for (j = 0; j < 3; j++) 
// 								{
// 									naAccu[i] += ((ACCURACY - nWeightY) * (ACCURACY - nWeightX)) * (*pSource) / ACCURACY;
// 									*pDest++ = (uchar)(naAccu[i] / nScale);
// 									naCarry[i] += (nWeightY * (ACCURACY - nWeightX) * (*pSource)) / ACCURACY;
// 									naAccu[i + 3] += ((ACCURACY - nWeightY) * nWeightX * (*pSource)) / ACCURACY;
// 									naCarry[i + 3] = (nWeightY * nWeightX * (*pSource)) / ACCURACY;
// 									i++;
// 									pSource++;
// 								}
// 								fEndX += xScale;
// 								u++;
// 							}
// 						}
// 						if (u < m_newx)
// 						{ // possibly not completed due to rounding errors
// 							for (j = 0; j < 3; j++) *pDest++ = (uchar)(naAccu[i++] / nScale);
// 						}
// 						naTemp = naCarry;
// 						naCarry = naAccu;
// 						naAccu = naTemp;
// 						memset(naCarry, 0, sizeof(int) * 3);    // need only to set first pixel zero
// 						pDest = newImage.info.pImage + (++v * newImage.info.dwEffWidth);
// 						fEndY += yScale;
// 					}
// 				}
// 				if (v < m_newy)
// 				{	// possibly not completed due to rounding errors
// 					for (i = 0; i < 3 * m_newx; i++) *pDest++ = (uchar)(naAccu[i] / nScale);
// 				}
// 				delete [] naAccu;
// 				delete [] naCarry;
			}
		}

		//select the destination
		if (pDst)
		{
			pDst->release();
			pDst->create(newImage, true);
		}
		else 
		{
			pSrc->release();
			pSrc->create (newImage, true);
		}

		return true;
	}
	else if (m_nMethod == 2)
	{
		if (m_newx<=0 || m_newy<=0 )
			return false;
		
		if (src_width==m_newx && src_height==m_newy) 
		{
			//image already correct size (just copy and return)
			if (pDst)
			{
				pDst->release();
				pDst->create (*pSrc, true);
			}
			return true;
		}//if
		
		//calculate scale of new image (less than 1 for enlarge)
		float xScale, yScale;
		xScale = (float)src_width  / (float)m_newx;    
		yScale = (float)src_height / (float)m_newy;
		
		//create temporary destination image
		Mat newImage(m_newy,m_newx,pSrc->type1());
		if (!newImage.isValid()) return false;
		
		float sX, sY;         //source location
		int dX,dY;           //destination pixel (int value)
		if ((xScale<=1 && yScale<=1) || m_fDisableAveraging) 
		{
			//image is being enlarged (or interpolation on demand)
			//RGB24 image (optimized version with direct writes)
			sY = 0.5f*yScale-0.5f;
			for(dY=0; dY<m_newy; dY++, sY += yScale)
			{
				_Tp* pdst = (_Tp*)newImage.data.ptr[dY];
				sX = 0.5f*xScale-0.5f;
				for(dX=0; dX<m_newx; dX++, sX += xScale)
				{
					Scalar_<_Tp> q=this->getPixelColorInterpolated(*pSrc, sX,sY,m_inMethod,m_ofMethod,0);
					for (int icn=0; icn<cn; icn++)
						pdst[dX*cn+icn] = q[icn];
				}//for dX
			}//for dY
		} 
		else 
		{
			//image size is being reduced (averaging enabled)
			sY = 0.5f*yScale-0.5f;
			for(dY=0; dY<m_newy; dY++, sY+=yScale)
			{
				sX = 0.5f*xScale-0.5f;
				for(dX=0; dX<m_newx; dX++, sX+=xScale)
				{
					Scalar_<_Tp> t=this->getAreaColorInterpolated(*pSrc, sX, sY, xScale, yScale, m_inMethod, m_ofMethod,0);
					setElementOfMat (newImage, dX, dY, &t);
				}//for x
			}//for y
		}//if
		
		//copy new image to the destination
		if (pDst) 
		{
			pDst->release();
			pDst->create (newImage, true);
		}
		else 
		{
			pSrc->release();
			pSrc->create(newImage, true);
		}
		return true;
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////
//ipCrop

template <typename _Tp>
ipCrop<_Tp>::ipCrop(const ipCrop& from)
{
	m_rect=from.m_rect;
	m_nMethod=from.m_nMethod;
	m_rAngle=from.m_rAngle;
	m_fAngle=from.m_fAngle;
}

template <typename _Tp>
ipCrop<_Tp>::ipCrop(int left, int top, int right, int bottom)
{
	m_rect.x = left;
	m_rect.y = top;
	m_rect.width = right - left;
	m_rect.height = bottom - top;
	m_nMethod = 1;
}

template <typename _Tp>
ipCrop<_Tp>::ipCrop(const Rect& rect)
{
	m_rect = rect;
	m_nMethod = 1;
}

template <typename _Tp>
ipCrop<_Tp>::ipCrop(int topx, int topy, int width, int height, float angle)
{
	m_rect.x = topx;
	m_rect.y = topy;
	m_rect.width = width;
	m_rect.height = height;
	m_rAngle = angle;
	m_nMethod = 2;
}

template <typename _Tp>
bool ipCrop<_Tp>::process (Mat* pmSrc, Mat* pmDst /*= NULL*/)
{
	if (m_nMethod == 1)
	{
		int i;
		int nRows = m_rect.height;
		int nCols = m_rect.width;
		int topx = m_rect.x;
		int topy = m_rect.y;
		
		if ( pmSrc->isInside( topx, topy )==false)
			return false;		
		if ( pmSrc->isInside( topx + nCols-1, topy )==false)
			return false;		
		if ( pmSrc->isInside( topx, topy + nRows-1 )==false)
			return false;		
		if ( pmSrc->isInside( topx + nCols-1, topy + nRows-1 )==false)
			return false;		

		if (nRows <= 0 || nCols <= 0)
			return false;

		Mat destImage(nRows, nCols, pmSrc->type1());
		int cn=pmSrc->channels()*pmSrc->step();
		
		for (i = 0; i < nRows; i ++)
		{
			uchar* pbuf=pmSrc->data.ptr[i+topy];
			memcpy (destImage.data.ptr[i], pbuf + topx*cn, cn*nCols);
		}	
		if (pmDst) 
		{
			pmDst->release();
			pmDst->create (destImage, true);
		}
		else 
		{
			pmSrc->release();
			pmSrc->create(destImage, true);
		}
	}
	else
	{
		m_rAngle = m_rAngle * (float)(acos(float(0)) / 90);

		double cos_angle = cos(m_rAngle/*/57.295779513082320877*/);
		double sin_angle = sin(m_rAngle/*/57.295779513082320877*/);
		
		// if there is nothing special, call the original Crop():
		if ( fabs(m_rAngle)<0.0002 )
		{
			ipCrop crop(m_rect);
			return crop.process(pmSrc);
		}

		int cx = m_rect.x + m_rect.width / 2;
		int cy = m_rect.y + m_rect.height / 2;
		
		Point2i p1(-m_rect.width / 2, m_rect.height / 2);
		Point2i p2(m_rect.width / 2, m_rect.height / 2);
		Point2i p3(m_rect.width / 2, -m_rect.height / 2);
		Point2i p4(-m_rect.width / 2, -m_rect.height / 2);
	
		Point2i np1, np2, np3, np4;
		np1.x = (int)floor(cos_angle * p1.x - sin_angle * p1.y);
		np1.y = (int)floor(sin_angle * p1.x + cos_angle * p1.y);
		
		np1.x += cx;
		np1.y = -np1.y + cy;
		
		// check: corners of the rectangle must be inside
		if ( pmSrc->isInside( np1.x, np1.y )==false)
			return false;		

		np2.x = (int)floor(cos_angle * p2.x - sin_angle * p2.y);
		np2.y = (int)floor(sin_angle * p2.x + cos_angle * p2.y);

		np2.x += cx;
		np2.y = -np2.y + cy;
		
		// check: corners of the rectangle must be inside
		if ( pmSrc->isInside( np2.x, np2.y )==false)
			return false;		

		np3.x = (int)floor(cos_angle * p3.x - sin_angle * p3.y);
		np3.y = (int)floor(sin_angle * p3.x + cos_angle * p3.y);

		np3.x += cx;
		np3.y = -np3.y + cy;
		
		// check: corners of the rectangle must be inside
		if ( pmSrc->isInside( np3.x, np3.y )==false)
			return false;		

		np4.x = (int)floor(cos_angle * p4.x - sin_angle * p4.y);
		np4.y = (int)floor(sin_angle * p4.x + cos_angle * p4.y);

		np4.x += cx;
		np4.y = -np4.y + cy;
		
		// check: corners of the rectangle must be inside
		if ( pmSrc->isInside( np4.x, np4.y )==false)
			return false;		

		Mat destImage(m_rect.height, m_rect.width , pmSrc->type());

		int y, x;
		int xx, yy;
		int xx1, yy1;
		for(y = 0; y < m_rect.height; y++)
			for(x = 0; x < m_rect.width; x++)
			{
				xx = x + m_rect.x - cx;
				yy = cy - y - m_rect.y;

				xx1 = (int)floor(cos_angle * xx - sin_angle * yy);
				yy1 = (int)floor(sin_angle * xx + cos_angle * yy);

				xx = xx1 + cx;
				yy = -yy1 + cy;

				destImage.data.ptr[y][x] = pmSrc->data.ptr[yy][xx];
			}
		m_rAngle = m_rAngle *  (float)(90 / acos(float(0)));
		if ( m_rAngle  > 45 && m_rAngle < 135 )
		{
			ipRotate<_Tp> rot(90, (cvlib::InterpolationMethod)1, (cvlib::OverflowMethod)2, NULL, true, false);
			rot.process(&destImage);
		}

		if (pmDst) 
		{
			pmDst->release();
			pmDst->create (destImage, true);
		}
		else 
		{
			pmSrc->release();
			pmSrc->create(destImage, true);
		}

/*	
		startx = MIN(m_rect.x, m_rect.x - (int)(sin_angle*(double)m_rect.height));
		endx   = m_rect.x + (int)(cos_angle*(double)m_rect.width);
		endy   = m_rect.y + (int)(cos_angle*(double)m_rect.height + sin_angle*(double)m_rect.width);
		// check: corners of the rectangle must be inside
 		if ( pSrc->isInside( startx, m_rect.y )==false || pSrc->isInside( endx, endy ) == false )
 			return false;
		
		// first crop to bounding rectangle
		CoImage tmp;
		ipCrop crop(startx, m_rect.y, endx, endy);
		if (false == crop.process (pSrc, &tmp))
			return false;
		
		// the midpoint of the image now became the same as the midpoint of the rectangle
		// rotate new image with minus angle amount
		ipRotate rot((float)(-m_rAngle*57.295779513082320877));
		if ( false == rot.process (&tmp) )
			return false;

		// crop rotated image to the original selection rectangle
		endx   = (tmp.width() + pSrc->width())/2;
		startx = (tmp.width() - pSrc->width())/2;
		starty = (tmp.height() + pSrc->height())/2;
		endy   = (tmp.height() - pSrc->height())/2;
		ipCrop crop1(startx, starty, endx, endy);
		if ( false == crop1.process( &tmp ) )
			return false;
		
		if (pDst) 
		{
			pDst->release();
			pDst->create (tmp, true);
		}
		else 
		{
			pSrc->release();
			pSrc->create(tmp, true);
		}
*/		
		return true;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
//ipSkew

template <typename _Tp>
ipSkew<_Tp>::ipSkew(const ipSkew& from)
{
	m_rxgain = from.m_rxgain;
	m_rygain = from.m_rygain;
	m_nxpivot=from.m_nxpivot;
	m_nypivot=from.m_nypivot;
	m_fEnableInterpolation = from.m_fEnableInterpolation;
}

template <typename _Tp>
ipSkew<_Tp>::ipSkew(float xgain, float ygain, int xpivot/*=0*/, int ypivot/*=0*/, bool bEnableInterpolation /*= false*/)
{
	m_rxgain = xgain;
	m_rygain = ygain;
	m_nxpivot = xpivot;
	m_nypivot = ypivot;
	m_fEnableInterpolation = bEnableInterpolation;
}

template <typename _Tp>
bool ipSkew<_Tp>::process (Mat* pSrc, Mat* pDst /*= NULL*/)
{
	if ( pDst && !ipBaseCore::equalTypeSize(pSrc, pDst) )
	{
		pDst->release();
		pDst->create(*pSrc);
	}
	float nx,ny;
	
	Mat tmp = *pSrc;
	if (!tmp.isValid())
		return false;
	
	int xmin,xmax,ymin,ymax;
	xmin = ymin = 0;
	xmax = pSrc->cols(); ymax=pSrc->rows();
	for(int y=ymin; y<ymax; y++)
	{
		for(int x=xmin; x<xmax; x++)
		{
			nx = x + (m_rxgain*(y - m_nypivot));
			ny = y + (m_rygain*(x - m_nxpivot));
			if (m_fEnableInterpolation)
			{
				Scalar_<_Tp> tcolor=this->getPixelColorInterpolated(*pSrc, nx, ny, IM_BILINEAR, OM_BACKGROUND);
				setElementOfMat (tmp, x, y, &tcolor);
			}
			else
			{
				setElementOfMat (*pSrc, tmp, (int)nx, (int)ny, x, y);
			}
		}
	}
	//copy new image to the destination
	if (pDst) 
	{
		pDst->release();
		pDst->create (tmp, true);
	}
	else 
	{
		pSrc->release();
		pSrc->create(tmp, true);
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////
//ipExpand

template <typename _Tp>
ipExpand<_Tp>::ipExpand(const ipExpand& from)
{
	m_nLeft=from.m_nLeft;
	m_nTop=from.m_nTop;
	m_nRight=from.m_nRight;
	m_nBottom=from.m_nBottom;
	m_color=from.m_newy;
}

template <typename _Tp>
ipExpand<_Tp>::ipExpand(int left, int top, int right, int bottom, COLOR canvascolor)
{
	m_nLeft=left;
	m_nTop=top;
	m_nRight=right;
	m_nBottom=bottom;
	m_color=canvascolor;
}

template <typename _Tp>
bool ipExpand<_Tp>::process(Mat* pmSrc, Mat* pmDst /*= NULL*/)
{
	if (pmDst && !ipBaseCore::equalType(pmSrc, pmDst))
		return false;
	pmDst->release();
	int newx = pmSrc->cols() + m_nLeft + m_nRight;
	int newy = pmSrc->rows() + m_nTop + m_nBottom;
	pmDst->create(newy, newx, pmSrc->type1());
	*pmDst = Scalar(m_color.x, m_color.y, m_color.z);
	pmDst->drawMat(*pmSrc, Point(m_nLeft, m_nTop));
	return true;
}

//////////////////////////////////////////////////////////////////////////
//ipThumbnail

template <typename _Tp>
ipThumbnail<_Tp>::ipThumbnail(const ipThumbnail& from)
{
	m_newx=from.m_newx;
	m_newy=from.m_newy;
}

template <typename _Tp>
ipThumbnail<_Tp>::ipThumbnail(int newx, int newy, COLOR canvascolor)
{
	m_newx=newx;
	m_newy=newy;
	m_canvasColor=canvascolor;
}

template <typename _Tp>
bool ipThumbnail<_Tp>::process (Mat* pSrc, Mat* pDst /*= NULL*/)
{
    if ( pDst && !ipBaseCore::equalTypeSize(pSrc, pDst) )
		return false;
	if (!pSrc->isValid()) return false;

    if ((m_newx <= 0) || (m_newy <= 0)) return false;

	Mat tmp = *pSrc;
	if (!tmp.isValid()) return false;

    // determine whether we need to shrink the image
	if ((pSrc->cols() > m_newx) || (pSrc->rows() > m_newy)) {
        float fScale;
        float fAspect = (float) m_newx / (float) m_newy;
        if (fAspect * pSrc->rows() > pSrc->cols()) {
            fScale = (float) m_newy / pSrc->rows();
        } else {
            fScale = (float) m_newx / pSrc->cols();
        }
		ipResample<_Tp> resampler((int) (fScale * pSrc->cols()), (int) (fScale * pSrc->rows()), 0);
		resampler.process (&tmp);
    }

    // expand the frame
	if (pDst)
		ip::expand (tmp, *pDst, m_newx, m_newy, m_canvasColor);
	else
		ip::expand (tmp, *pSrc, m_newx, m_newy, m_canvasColor);
	return true;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//ipTransGeo

namespace ip
{
void rotate(const Mat& src, Mat& dst, RotateType rottype)
{
	TYPE type = src.type();
	if (type == MAT_Tuchar) {
		ipRotate<uchar> t(rottype); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tshort) {
		ipRotate<short> t(rottype); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tint) {
		ipRotate<int> t(rottype); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tfloat) {
		ipRotate<float> t(rottype); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tdouble) {
		ipRotate<double> t(rottype); t.process ((Mat*)&src, &dst);
	}

}
void rotate(const Mat& src, Mat& dst, float rAngle)
{
	TYPE type = src.type();
	if (type == MAT_Tuchar) {
		ipRotate<uchar> t(rAngle); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tshort) {
		ipRotate<short> t(rAngle); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tint) {
		ipRotate<int> t(rAngle); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tfloat) {
		ipRotate<float> t(rAngle); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tdouble) {
		ipRotate<double> t(rAngle); t.process ((Mat*)&src, &dst);
	}
}
void rotate(const Mat& src, Mat& dst, float rAngle, InterpolationMethod inMethod,
		   OverflowMethod ofMethod, COLOR *replColor,
		   bool const optimizeRightAngles, bool const bKeepOriginalSize)
{
	TYPE type = src.type();
	if (type == MAT_Tuchar) {
		ipRotate<uchar> t(rAngle, inMethod, ofMethod, replColor, optimizeRightAngles, bKeepOriginalSize); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tshort) {
		ipRotate<short> t(rAngle, inMethod, ofMethod, replColor, optimizeRightAngles, bKeepOriginalSize); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tint) {
		ipRotate<int> t(rAngle, inMethod, ofMethod, replColor, optimizeRightAngles, bKeepOriginalSize); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tfloat) {
		ipRotate<float> t(rAngle, inMethod, ofMethod, replColor, optimizeRightAngles, bKeepOriginalSize); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tdouble) {
		ipRotate<double> t(rAngle, inMethod, ofMethod, replColor, optimizeRightAngles, bKeepOriginalSize); t.process ((Mat*)&src, &dst);
	}
}
void resample(const Mat& src, Mat& dst, int newx, int newy, int mode)
{
	TYPE type = src.type();
	if (type == MAT_Tuchar) {
		ipResample<uchar> t(newx, newy, mode); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tshort) {
		ipResample<short> t(newx, newy, mode); t.process ((Mat*)&src, &dst);
	} else if (type ==MAT_Tint) {
		ipResample<int> t(newx, newy, mode); t.process ((Mat*)&src, &dst);
	} else if (type ==MAT_Tfloat) {
		ipResample<float> t(newx, newy, mode); t.process ((Mat*)&src, &dst);
	} else if (type ==MAT_Tdouble) {
		ipResample<double> t(newx, newy, mode); t.process ((Mat*)&src, &dst);
	}
}
void resample(const Mat& src, Mat& dst, int newx, int newy, InterpolationMethod const inMethod,
			 OverflowMethod const ofMethod, bool const disableAveraging)
{
	TYPE type = src.type();
	if (type == MAT_Tuchar) {
		ipResample<uchar> t(newx, newy, inMethod, ofMethod, disableAveraging); t.process ((Mat*)&src, &dst);
	} else if (type==MAT_Tshort) {
		ipResample<short> t(newx, newy, inMethod, ofMethod, disableAveraging); t.process ((Mat*)&src, &dst);
	} else if (type==MAT_Tint) {
		ipResample<int> t(newx, newy, inMethod, ofMethod, disableAveraging); t.process ((Mat*)&src, &dst);
	} else if (type==MAT_Tfloat) {
		ipResample<float> t(newx, newy, inMethod, ofMethod, disableAveraging); t.process ((Mat*)&src, &dst);
	} else if (type==MAT_Tdouble) {
		ipResample<double> t(newx, newy, inMethod, ofMethod, disableAveraging); t.process ((Mat*)&src, &dst);
	}
}
void crop(const Mat& src, Mat& dst, int left, int top, int right, int bottom)
{
	TYPE type = src.type();
	if (type == MAT_Tuchar) {
		ipCrop<uchar> t(left, top, right, bottom); t.process ((Mat*)&src, &dst);
	} else if (type==MAT_Tshort) {
		ipCrop<short> t(left, top, right, bottom); t.process ((Mat*)&src, &dst);
	} else if (type==MAT_Tint) {
		ipCrop<int> t(left, top, right, bottom); t.process ((Mat*)&src, &dst);
	} else if (type==MAT_Tfloat) {
		ipCrop<float> t(left, top, right, bottom); t.process ((Mat*)&src, &dst);
	} else if (type==MAT_Tdouble) {
		ipCrop<double> t(left, top, right, bottom); t.process ((Mat*)&src, &dst);
	}
}
void crop(const Mat& src, Mat& dst, const Rect& rect)
{
	TYPE type = src.type();
	if (type == MAT_Tuchar) {
		ipCrop<uchar> t(rect); t.process ((Mat*)&src, &dst);
	} else if (type==MAT_Tshort) {
		ipCrop<short> t(rect); t.process ((Mat*)&src, &dst);
	} else if (type==MAT_Tint) {
		ipCrop<int> t(rect); t.process ((Mat*)&src, &dst);
	} else if (type==MAT_Tfloat) {
		ipCrop<float> t(rect); t.process ((Mat*)&src, &dst);
	} else if (type==MAT_Tdouble) {
		ipCrop<double> t(rect); t.process ((Mat*)&src, &dst);
	}
}
void crop(const Mat& src, Mat& dst, int topx, int topy, int width, int height, float angle)
{
	TYPE type = src.type();
	if (type == MAT_Tuchar) {
		ipCrop<uchar> t(topx, topy, width, height, angle); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tshort) {
		ipCrop<short> t(topx, topy, width, height, angle); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tint) {
		ipCrop<int> t(topx, topy, width, height, angle); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tfloat) {
		ipCrop<float> t(topx, topy, width, height, angle); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tdouble) {
		ipCrop<double> t(topx, topy, width, height, angle); t.process ((Mat*)&src, &dst);
	}
}
void skew(const Mat& src, Mat& dst, float xgain, float ygain, int xpivot, int ypivot, bool bEnableInterpolation)
{
	TYPE type = src.type();
	if (type == MAT_Tuchar) {
		ipSkew<uchar> t(xgain, ygain, xpivot, ypivot, bEnableInterpolation); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tshort) {
		ipSkew<short> t(xgain, ygain, xpivot, ypivot, bEnableInterpolation); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tint) {
		ipSkew<int> t(xgain, ygain, xpivot, ypivot, bEnableInterpolation); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tfloat) {
		ipSkew<float> t(xgain, ygain, xpivot, ypivot, bEnableInterpolation); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tdouble) {
		ipSkew<double> t(xgain, ygain, xpivot, ypivot, bEnableInterpolation); t.process ((Mat*)&src, &dst);
	}
}
void expand(const Mat& src, Mat& dst, int left, int top, int right, int bottom, COLOR canvascolor)
{
	TYPE type = src.type();
	if (type == MAT_Tuchar) {
		ipExpand<uchar> t(left, top, right, bottom, canvascolor); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tshort) {
		ipExpand<short> t(left, top, right, bottom, canvascolor); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tint) {
		ipExpand<int> t(left, top, right, bottom, canvascolor); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tfloat) {
		ipExpand<float> t(left, top, right, bottom, canvascolor); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tdouble) {
		ipExpand<double> t(left, top, right, bottom, canvascolor); t.process ((Mat*)&src, &dst);
	}
}
void expand(const Mat& src, Mat& dst, int newx, int newy, COLOR canvascolor)
{
	expand (src, dst, 0,0,newx-src.cols(),newy-src.rows(),canvascolor);
}
void thumbnail(const Mat& src, Mat& dst, int newx, int newy, COLOR canvascolor)
{
	TYPE type = src.type();
	if (type == MAT_Tuchar) {
		ipThumbnail<uchar> t(newx, newy, canvascolor); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tshort) {
		ipThumbnail<short> t(newx, newy, canvascolor); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tint) {
		ipThumbnail<int> t(newx, newy, canvascolor); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tfloat) {
		ipThumbnail<float> t(newx, newy, canvascolor); t.process ((Mat*)&src, &dst);
	} else if (type == MAT_Tdouble) {
		ipThumbnail<double> t(newx, newy, canvascolor); t.process ((Mat*)&src, &dst);
	}
}
void cropEx(const Mat& src, Mat& dst, const Rect& rect, const Scalar& value)
{
	if (rect.width <=0 || rect.height<=0)
		return;
	int borderx=MAX3(-rect.x, rect.limx()-src.cols(), 0);
	int bordery=MAX3(-rect.y, rect.limy()-src.rows(), 0);
	if (borderx==0 && bordery==0)
	{
		src.subMat (rect, dst);
	}
	else
	{
		Mat canvas(src.rows()+bordery*2, src.cols()+borderx*2, src.type1(), value);
		canvas.drawMat (src, Point2i(borderx, bordery));
		Rect region=rect;
		region.x += borderx;
		region.y += bordery;
		canvas.subMat(region, dst);
	}
}


}

}
