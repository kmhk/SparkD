
#include "EngineLib.h"
#include "zvd.h"
#include "ColorSpace.h"
#include <assert.h>
#include "UIImage+cvlib.h"


// Functions to be added by Pritesh:
    // calc_coefficients
    // Solver1

//Additional Functions to be added by Jimmy

    // _MainProcess1 for 10 Big Square Patterns RGB recognition - no small patterns  (see images containing 10 big patterns)
    // _MainProcess2 for 5 Big Square Patterns RGB recognition - no small patterns  (see shared images containing 5 big patterns)

static void convertPoints(const cvlib::TiltedRect& r, cvlib::Point* pts, bool flag)
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
bool _MainProcess(cvlib::Mat matSrc, int mode, cvlib::Mat& mdraw, cvlib::Rect* testT_rect, int* testT_color, cvlib::Rect* controlC_rect, int* controlC_color, cvlib::Rect* betweenCT_rect, int* betweenCT_color, bool bAutoBalance)
{
    mdraw = cvlib::Mat(matSrc);
    bool frot = false;

    cvlib::ZVD zvd;
    cvlib::ZVDRegion vdregion;
    
    if (zvd.detect(matSrc))
    {
        zvd.getVDRegion(vdregion);
        int i;
        for (i=0; i < (int)cvlib::VD_max; i++)
        {
            cvlib::TiltedRect rr = vdregion.getRect((cvlib::VDElemType)i);
    //        cvlib::Rect r = convertTiltedRectToRect(rr);
            cvlib::Point pts[5];
            convertPoints(rr, pts, frot);
            for (int k=0; k<4; k++){
                mdraw.drawLine(pts[k], pts[k+1], cvlib::COLOR(255,0,0), 8);
            }
        }
        cvlib::Mat img;
        zvd.getTestTImage(img);
        cvlib::COLOR color  = zvd.getDominantColor(img);
        printf("dominant color of testT : %d %d %d\n", color.r, color.g, color.b);
        testT_color[0] = (int)rgb_pixels(color.r,color.g,color.b);

        zvd.getControlCImage(img);
        color  = zvd.getDominantColor(img);
        printf("dominant color of controlC : %d %d %d\n", color.r, color.g, color.b);
        controlC_color[0] = (int)rgb_pixels(color.r,color.g,color.b);

        zvd.getBetweenCTlineImage(img);
        color  = zvd.getDominantColor(img);
        printf("dominant color of between CTLine : %d %d %d\n", color.r, color.g, color.b);
        betweenCT_color[0] = (int)rgb_pixels(color.r,color.g,color.b);

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





