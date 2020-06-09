
#include "EngineLib.h"
#include "zvd.h"
#include "ColorSpace.h"
//#include "global.h"
#include <assert.h>
#include "UIImage+cvlib.h"


// Functions to be added by Pritesh:
    // calc_coefficients
    // Solver1

//Additional Functions to be added by Jimmy

    // _MainProcess1 for 10 Big Square Patterns RGB recognition - no small patterns  (see images containing 10 big patterns)
    // _MainProcess2 for 5 Big Square Patterns RGB recognition - no small patterns  (see shared images containing 5 big patterns)

static cv::Rect convertTiltedRectToRect (const cvlib::TiltedRect& r){
    int nminx = r.m_pts[0].x;
    int nminy = r.m_pts[0].y;
    int nmaxx = r.m_pts[0].x;
    int nmaxy = r.m_pts[0].y;
    for (int i=1; i < 4; i++)
    {
        nminx = MIN(r.m_pts[i].x, nminx);
        nminy = MIN(r.m_pts[i].y, nminy);
        nmaxx = MAX(r.m_pts[i].x, nmaxx);
        nmaxy = MAX(r.m_pts[i].y, nmaxy);
    }
    return cv::Rect(nminx, nminy, nmaxx-nminx+1, nmaxy-nminy+1);
}
static void convertPoints(const cvlib::TiltedRect& r, cv::Point* pts, bool flag)
{
    for (int i=0; i<4; i++)
    {
        pts[i].x = r.m_pts[i].x;
        pts[i].y = r.m_pts[i].y;
    }
    pts[4].x = r.m_pts[0].x;
    pts[4].y = r.m_pts[0].y;
    if (flag)
    {
        for (int i=0; i<5; i++){
            int t=pts[i].x;
            pts[i].x = pts[i].y;
            pts[i].y = t;
        }
    }
}


//bool _MainProcess(cv::Mat matSrc, int mode, cv::Mat& mdraw, cv::Rect* prtResult_middle, int* pnColor_middle,	cv::Rect* prtResult_right, int* pnColor_right, int* pnCnt_left, bool bAutoBalance)

bool _MainProcess(cv::Mat matSrc, int mode, cv::Mat& mdraw, cv::Rect* testT_rect, int* testT_color, cv::Rect* controlC_rect, int* controlC_color, cv::Rect* betweenCT_rect, int* betweenCT_color, bool bAutoBalance)
{
    mdraw = matSrc.clone();
    int cn = matSrc.channels();
    cvlib::Mat image;
    bool frot = false;
    if (cn==3)
    {
        image.create(matSrc.rows, matSrc.cols, cvlib::MAT_Tuchar3);
        for (int i=0; i<matSrc.rows; i++)
            memcpy(image.data.ptr[i], matSrc.data+matSrc.step*i, matSrc.cols*3);
    }
    else {
        image.create(matSrc.rows, matSrc.cols, cvlib::MAT_Tuchar4);
        for (int i=0; i<matSrc.rows; i++)
            memcpy(image.data.ptr[i], matSrc.data+matSrc.step*i, matSrc.cols*4);
        cvlib::Mat t;
        cvlib::ColorSpace::toRGB(image, t, cvlib::CT_RGBA);//CT_BGRA
        image = t;
    }
    
    //check the code for image frot (not in original zvd)
 /*   if (image.rows() > image.cols())
    {
        image = image.transposed();
        frot = true;
    }
  */
    
    cvlib::ZVD zvd;
    cvlib::ZVDRegion vdregion;
    
    if (zvd.detect(image))
    {
        zvd.getVDRegion(vdregion);
        int i;
        for (i=0; i < (int)cvlib::VD_max; i++)
        {
            cvlib::TiltedRect rr = vdregion.getRect((cvlib::VDElemType)i);
            cv::Rect r = convertTiltedRectToRect(rr);
            if (i == 1) {
               // testT_rect = &r;
                testT_rect[0] = cv::Rect(r.y,r.x,r.height,r.width);
            } else if (i==2) {
               // controlC_rect = &r;
                controlC_rect[0] = cv::Rect(r.y,r.x,r.height,r.width);
            } else {
               // betweenCT_rect = &r;
                betweenCT_rect[0] = cv::Rect(r.y,r.x,r.height,r.width);
            }
            cv::Point pts[5];
            convertPoints(rr, pts, frot);
            for (int k=0; k<4; k++){
                cv::line(mdraw, pts[k], pts[k+1], cv::Scalar(255,0,0),8);
            }
        }
        cvlib::Mat img;
        zvd.getTestTImage(img);
        cvlib::COLOR color  = zvd.getDominantColor(img);
        printf("dominant color of testT : %d %d %d\n", color.r, color.g, color.b);
        testT_color[0] = (int)rgb_pixels(color.r,color.g,color.b);
        printf("dominant color of between CTLine: %d \n", testT_color[0]);

        zvd.getControlCImage(img);
        color  = zvd.getDominantColor(img);
        printf("dominant color of controlC : %d %d %d\n", color.r, color.g, color.b);
        controlC_color[0] = (int)rgb_pixels(color.r,color.g,color.b);
        printf("dominant color of between CTLine: %d \n", controlC_color[0]);

        zvd.getBetweenCTlineImage(img);
        color  = zvd.getDominantColor(img);
        printf("dominant color of between CTLine : %d %d %d\n", color.r, color.g, color.b);
        betweenCT_color[0] = (int)rgb_pixels(color.r,color.g,color.b);
        printf("dominant color of between CTLine: %d \n", betweenCT_color[0]);

        return true;
    }
    else
    {
        return false;
    }
}


//function by Pritesh


double Solver3(double Tgrey, double Cgrey, double CTgrey, bool MLmodel) {

    double vitDValue1 = 0.0;
   vitDValue1 = (((double)arc4random()/0x100000000)*100);

    if (MLmodel) {
                 
        return vitDValue1;
    
    } else {

        return vitDValue1;
    }
}

double Solver4(int Trgb, int Crgb, int CTrgb) {

    double vitDValue1 = 0.0;

    vitDValue1 = (((double)arc4random()/0x100000000)*100);

    return vitDValue1;

}





