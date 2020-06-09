//
//  SL_TestingView.m
//  SparkLabs
//
//  Created by Spark Diagnostics on 09/05/16.
//  Copyright (c) 2016 Spark Diagnostics. All rights reserved.
//

#import "SL_TestingView.h"
#import <Foundation/Foundation.h>
#import <Foundation/NSAutoreleasePool.h>
#import <AssetsLibrary/AssetsLibrary.h>
#import "UIImage/UIImage+Utilities.h"
#import "UIImage+Utilities.h"
#import "UIImage/UIImage+FixOrientation.h"
//#import "global.h"
#include "opencv2/highgui/ios.h"
#include "EngineLib.h"
#import "SparkD-Swift.h"

//#import "DBManager.h"
//#import "FMDatabase.h"
//#import "FMResultSet.h"
//#import "NSData+Base64.h"
//#import "SparkD-Swift.h"


@interface SL_TestingView ()


@property(nonatomic,retain)NSString *Base64String;
@property(nonatomic,retain)NSString *Base64String1;


@end
BOOL g_bDetection = false;
BOOL bOpen = false;
cv::Rect testT_rect[COLUMN_NUM];
int	 testT_color[COLUMN_NUM];
cv::Rect controlC_rect[COLUMN_NUM];
int controlC_color[COLUMN_NUM];
cv::Rect betweenCT_rect[COLUMN_NUM];
int betweenCT_color[COLUMN_NUM];

double nHistComp[COLUMN_NUM * MAX_ROWS];
cv::Rect rtResult_corner[4];
int nColor_corner[4];
BOOL m_bDetectOk = false;

@implementation SL_TestingView

// @synthesize c,jsonarray,FMDict,mutablearray,idvalues,dbManager, statusArray;
bool bcameraStatue;
float Screen_W;
float Screen_H;

typedef struct GridRect GridRect;
/* Declare the struct with integer members x, y */
struct GridRect {
    int    x;
    int    y;
    int    height;
    int    width;
};

- (void)viewDidLoad
{
    [super viewDidLoad];
 //   g_pTestView = self;
 //   memset(testT_rect, 0, sizeof(Rect) * COLUMN_NUM);
 //   memset(g_nResultRGB_middle, 0, sizeof(int) * COLUMN_NUM);
 //   memset(nHistComp, 0, sizeof(double) * COLUMN_NUM * MAX_ROWS);
 //   memset(rtResult_right, 0, sizeof(Rect) * COLUMN_NUM * MAX_ROWS);
 //   memset(g_nResultRGB_right, 0, sizeof(int) * COLUMN_NUM * MAX_ROWS);
 //   memset(nCnt_right, 0, sizeof(int) * COLUMN_NUM);
 //   memset(g_dbResult_solver1, 0, 6 * sizeof(double));
    bcameraStatue = false;
    UIInterfaceOrientation orientation = [UIApplication sharedApplication].statusBarOrientation;
    if (( [[[UIDevice currentDevice] systemVersion] floatValue]<8)  && UIInterfaceOrientationIsLandscape(orientation))
    {
        Screen_W = [[UIScreen mainScreen] bounds].size.height;
        Screen_H = [[UIScreen mainScreen] bounds].size.width;
    }
    else
    {
        Screen_W = [[UIScreen mainScreen] bounds].size.width;
        Screen_H = [[UIScreen mainScreen] bounds].size.height;
        
    }
    bcameraStatue = true;
}


- (CGImageRef) CreateRGBAImageWithABGRImage: (CGImageRef) bgraImageRef {
    // the data retrieved from the image ref has 4 bytes per pixel (ABGR).
    CFDataRef abgrData = CGDataProviderCopyData(CGImageGetDataProvider(bgraImageRef));
    UInt8 *pixelData = (UInt8 *) CFDataGetBytePtr(abgrData);
    long length = CFDataGetLength(abgrData);
    
    // abgr to rgba
    // swap the blue and red components for each pixel...
    UInt8 tmpByte = 0;
    for (int index = 0; index < length; index+= 4) {
        tmpByte = pixelData[index];
        pixelData[index] = pixelData[index + 2];
        pixelData[index + 2] = tmpByte;
    }
    
    // grab the bgra image info
    size_t width = CGImageGetWidth(bgraImageRef);
    size_t height = CGImageGetHeight(bgraImageRef);
    size_t bitsPerComponent = CGImageGetBitsPerComponent(bgraImageRef);
    size_t bitsPerPixel = CGImageGetBitsPerPixel(bgraImageRef);
    size_t bytesPerRow = CGImageGetBytesPerRow(bgraImageRef);
    CGColorSpaceRef colorspace = CGImageGetColorSpace(bgraImageRef);
    CGBitmapInfo bitmapInfo = CGImageGetBitmapInfo(bgraImageRef);
    
    // create the argb image
    CFDataRef argbData = CFDataCreate(NULL, pixelData, length);
    CGDataProviderRef provider = CGDataProviderCreateWithCFData(argbData);
    CGImageRef argbImageRef = CGImageCreate(width, height, bitsPerComponent, bitsPerPixel, bytesPerRow,
                                            colorspace, bitmapInfo, provider, NULL, true, kCGRenderingIntentDefault);
    
    // release what we can
    CFRelease(abgrData);
    CFRelease(argbData);
    CGDataProviderRelease(provider);
    
    // return the pretty new image
    return argbImageRef;
}


- (UIImage *)UIImageFromCVMat:(cv::Mat)cvMat {
    NSData *data = [NSData dataWithBytes:cvMat.data length:cvMat.elemSize()*cvMat.total()];
    
    CGColorSpaceRef colorSpace;
    CGBitmapInfo bitmapInfo;
    
    if (cvMat.elemSize() == 1) {
        colorSpace = CGColorSpaceCreateDeviceGray();
        bitmapInfo = kCGImageAlphaNone | kCGBitmapByteOrderDefault;
    } else {
        colorSpace = CGColorSpaceCreateDeviceRGB();
        bitmapInfo = kCGBitmapByteOrder32Little | (
                                                   cvMat.elemSize() == 3? kCGImageAlphaNone : kCGImageAlphaNoneSkipFirst
                                                   );
    }
    
    CGDataProviderRef provider = CGDataProviderCreateWithCFData((__bridge CFDataRef)data);
    
    // Creating CGImage from cv::Mat
    CGImageRef imageRef = CGImageCreate(
                                        cvMat.cols,                 //width
                                        cvMat.rows,                 //height
                                        8,                          //bits per component
                                        8 * cvMat.elemSize(),       //bits per pixel
                                        cvMat.step[0],              //bytesPerRow
                                        colorSpace,                 //colorspace
                                        bitmapInfo,                 // bitmap info
                                        provider,                   //CGDataProviderRef
                                        NULL,                       //decode
                                        false,                      //should interpolate
                                        kCGRenderingIntentDefault   //intent
                                        );
    
    // Getting UIImage from CGImage
    CGImageRef converted = [self CreateRGBAImageWithABGRImage:imageRef];
    UIImage *finalImage = [UIImage imageWithCGImage:converted];
    CGImageRelease(converted);
    CGImageRelease(imageRef);
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(colorSpace);
    
    return finalImage;
}

#define FIXED_SIZE 900

-(void) MainProcess:(UIImage *)imgSrc :(bool) bFlip :(int) imageCount
{
    cv::Mat matSrc;
    UIImageToMat(imgSrc, matSrc);

   /* if (bFlip)
        cv::flip(matSrc, matSrc, 1); */

    memset(testT_rect, 0, sizeof(Rect) * 2);   //  COLUMN_NUM);
    memset(testT_color, 0, sizeof(int) * 2);
    memset(controlC_rect, 0, sizeof(Rect) * 2);
    memset(controlC_color, 0, sizeof(int) * 2);
    memset(betweenCT_rect, 0, sizeof(int) * 2);
    memset(betweenCT_color, 0, sizeof(int) * 2);
    
    NSMutableArray *solver1Values = [NSMutableArray array];
    NSMutableArray *solver2Values = [NSMutableArray array];
    
    NSLog(@"w:%d, h:%d",matSrc.cols,matSrc.rows);

    int g_selector = 4;
    
    cv::Mat rotateSrc;
    m_bDetectOk = false;
    
    m_bDetectOk = _MainProcess(matSrc, g_selector, rotateSrc, testT_rect, testT_color, controlC_rect, controlC_color, betweenCT_rect, betweenCT_color,true);
   
    double Tgrey = 0.0;
    double Cgrey = 0.0;
    double BTgrey = 0.0;

    NSLog(@"R-Test: %d", (testT_color[0] % 256) );
    NSLog(@"G-Test: %d", (testT_color[0] / 256) % 256);
    NSLog(@"B-Teste: %d", (testT_color[0]  / 256) / 256);
    
    NSLog(@"R-CT: %d", (betweenCT_color[0] % 256));
    NSLog(@"G-CT: %d", (betweenCT_color[0] / 256) % 256);
    NSLog(@"B-CT: %d", (betweenCT_color[0]  / 256) / 256);

    Tgrey = (0.3 * double(testT_color[0] % 256) ) + (0.59 * double((testT_color[0] / 256) % 256)) + (0.11 * double((testT_color[0]  / 256) / 256));
    Cgrey = (0.3 * double(controlC_color[0] % 256) ) + (0.59 * double((controlC_color[0] / 256) % 256)) + (0.11 * double((controlC_color[0]  / 256) / 256) );
    BTgrey = (0.3 * double(betweenCT_color[0] % 256) ) + (0.59 * double((betweenCT_color[0] / 256) % 256)) + (0.11 * double((betweenCT_color[0]  / 256) / 256));
    
    if (m_bDetectOk)
    {
        double vitD1 = 0.0;
        double vitD2 = 0.0;
        vitD1 = Solver3(Tgrey, Cgrey, BTgrey, true);
       // vitD2 = Solver4(testT_color[0],controlC_color[0],betweenCT_color[0]);
        vitD2 = Solver3(Tgrey, Cgrey, BTgrey, false);

        
        NSLog(@"Tgrey value Measure: %f", Tgrey);
        NSLog(@"Cgrey value Measure: %f", Cgrey);
        NSLog(@"BTgrey value Measure: %f", BTgrey);
        
        
        [solver1Values addObject:@(vitD1)]; // solver1Value
        [solver1Values addObject:@(vitD2)]; // solver1Value
        [solver2Values addObject:@(vitD1)]; // solver2Value
        [solver2Values addObject:@(vitD2)]; // solver2Value
        NSLog(@"solver3 value: %f", vitD1);
        NSLog(@"solver4 value: %f", vitD2);
        
        [GlobalHelper fillWithSolver1:solver1Values solver2:solver2Values];
        [GlobalHelper fillWithSolver3:vitD1 solver4:vitD2];
        [GlobalHelper fillWithTestT:testT_color[0] controlC:controlC_color[0] betweenCT:betweenCT_color[0]];
     //   [GlobalHelper fillWithRgbPattern:testT_color main:controlC_color histComp:betweenCT_color]
    }
        
    if (rotateSrc.channels() == 3)
            cv::cvtColor(rotateSrc, rotateSrc, CV_BGR2BGRA);//CV_RGB2BGRA);
    
    if (imageCount==3)
    [self displayImage: MatToUIImage(rotateSrc)];
    
    g_bDetection = m_bDetectOk;
    if (g_bDetection == false)
    {
        UIAlertView *alert = [[UIAlertView alloc]initWithTitle:@"Alert" message:@"Detection Failed!" delegate:self cancelButtonTitle:@"OK" otherButtonTitles:nil];
        [alert show];
    }
    else {
      /*  UIAlertView *success = [[UIAlertView alloc]initWithTitle:@"Alert" message:@"Success!" delegate:self cancelButtonTitle:@"OK" otherButtonTitles:nil];
        [success show]; */
    }
}

/*   // this is the original Urine process
 
 
-(void) MainProcess:(UIImage *)imgSrc :(bool) bFlip
{
    cv::Mat matSrc;
    UIImageToMat(imgSrc, matSrc);
    if (bFlip)
        cv::flip(matSrc, matSrc, 1);
    memset(rtResult_middle, 0, sizeof(Rect) * COLUMN_NUM);
    memset(g_nResultRGB_middle, 0, sizeof(int) * COLUMN_NUM);
    memset(rtResult_right, 0, sizeof(Rect) * COLUMN_NUM * MAX_ROWS);
    memset(nHistComp, 0, sizeof(double) * COLUMN_NUM * MAX_ROWS);
    memset(g_nResultRGB_right, 0, sizeof(int) * COLUMN_NUM * MAX_ROWS);
    memset(nCnt_right, 0, sizeof(int) * COLUMN_NUM);
    
  ///  NSLog(@"w:%d, h:%d",matSrc.cols,matSrc.rows);

    int xValuecount;
    
    cv::Mat rotateSrc;
    
    g_selector = (int)[GlobalHelper getCurrentSelectedStripe];
    
    m_bDetectOk = _MainProcess(matSrc, g_selector, rotateSrc, rtResult_middle, g_nResultRGB_middle, rtResult_right, g_nResultRGB_right, nCnt_right, nHistComp, true);
  
    g_nTest = (int)[GlobalHelper getCurrentnTestStrip];
    
    if (m_bDetectOk)    {
        double* pfCoefficient = 0;
        int *pnColor_tmp = 0;
        double *nHist_tmp = 0;

     //   double *xValuesTemp1;
        
     //   int blueC, greenC, redC;
        NSMutableArray *solver1Values = [NSMutableArray array];
        NSMutableArray *solver2Values = [NSMutableArray array];
        NSMutableArray *rgbPattern = [NSMutableArray array];
        NSMutableArray *mainPatternXYWH = [NSMutableArray array];
        NSMutableArray *refPatternXYWH = [NSMutableArray array];
  NSMutableArray *histCompResults = [NSMutableArray array];
        NSMutableArray *rgbMain = [NSMutableArray array];
        NSMutableArray *nPattern = [NSMutableArray array];
        NSMutableArray *xValuesTemp = [NSMutableArray array];
       
        //NSLog(@"nTest %d",g_nTest);
        xValuecount = 0;
        
        for (int ll = 0; ll < g_nTest; ll++)
        {
            NSMutableArray *resultRGBrightLine = [NSMutableArray array];
            NSMutableArray *histogramComp = [NSMutableArray array];
            NSMutableArray *resultMainRect = [NSMutableArray array];
            NSMutableArray *resultRefRect = [NSMutableArray array];

            pnColor_tmp = new int[nCnt_right[ll]]; //Strip nPattern <- nCnt_right
            nHist_tmp = new double[nCnt_right[ll]];
            xValuecount = nCnt_right[ll] + xValuecount;
            // NSLog(@"xValueCount = %d", xValuecount);

            [resultMainRect addObject:@(rtResult_middle[ll].x)];
            [resultMainRect addObject:@(rtResult_middle[ll].y)];
            [resultMainRect addObject:@(rtResult_middle[ll].width)];
            [resultMainRect addObject:@(rtResult_middle[ll].height)];

            for (int i = 0; i < nCnt_right[ll]; i++) //Strip nPattern
            {
                pnColor_tmp[i] = g_nResultRGB_right[ll * 6 + i]; // rgbPattern <- g_nResultRGB_right
                nHist_tmp[i] = nHistComp[ll * 6 + i];
 
                [resultRefRect addObject:@(rtResult_right[ll * 6 +i].x)];
                [resultRefRect addObject:@(rtResult_right[ll * 6 +i].y)];
                [resultRefRect addObject:@(rtResult_right[ll * 6 +i].width)];
                [resultRefRect addObject:@(rtResult_right[ll * 6 +i].height)];
                [resultRGBrightLine addObject:@(pnColor_tmp[i])];
                [histogramComp addObject:@(nHist_tmp[i])];
            }
            
            double *xValues = 0;
            xValues = new double[6];
            
            [xValuesTemp addObjectsFromArray:([GlobalHelper getxValuesStripWithNPos:(ll)])];
            //  NSLog(@"xValues: %@", xValuesTemp);
            
            for (int i = 0; i < nCnt_right[ll]; i++) {
                int j = xValuecount - nCnt_right[ll] + i;
                xValues[i] = [[xValuesTemp objectAtIndex:(j)] doubleValue];
            }
            
            pfCoefficient = _calcCoefficient(pnColor_tmp, nCnt_right[ll], g_nJOrder, xValues); // nOrder <- g_nJOrder
            //  xValues <- g_dbOrder
            //    int blueInput = (g_nResultRGB_middle[ll] / 256 / 256);
            //    int greenInput = (g_nResultRGB_middle[ll]/ 256) % 256;
            //    int redInput = (g_nResultRGB_middle[ll] % 256);

      //      g_dbResult_solver1[ll] = Solver1(g_nResultRGB_middle[ll], xValues, pfCoefficient, g_nJOrder, nCnt_right[ll]);
            
            g_dbResult_solver2[ll] = Solver2(g_nResultRGB_middle[ll], xValues, pnColor_tmp, nCnt_right[ll]);//
            // rgbMain <- g_nResultRGB_middle
            
            [solver1Values addObject:@(g_dbResult_solver1[ll])]; // solver1Value
            [solver2Values addObject:@(g_dbResult_solver2[ll])]; // solver2Value
            [rgbPattern addObject:resultRGBrightLine];
            [histCompResults addObject:histogramComp];
            [rgbMain addObject:@(g_nResultRGB_middle[ll])];
            [nPattern addObject:@(nCnt_right[ll])];
            [mainPatternXYWH addObject:resultMainRect];
            [refPatternXYWH addObject:resultRefRect];

            
            delete[] pfCoefficient;
            delete[] pnColor_tmp;
            delete[] xValues;
            delete[] nHist_tmp;
        }
        
        [GlobalHelper fillWithSolver1:solver1Values solver2:solver2Values];
        [GlobalHelper fillWithRgbPattern:rgbPattern main:rgbMain histComp:histCompResults];
        [GlobalHelper fillWithMainPatternXYWH:mainPatternXYWH refPatternXYWH:refPatternXYWH];
       
    }
    
    if (rotateSrc.channels() == 3) {
        cv::cvtColor(rotateSrc, rotateSrc, CV_BGR2BGRA);//CV_RGB2BGRA);
    }

    UIImage *computedImage = [self UIImageFromCVMat:rotateSrc];
    
    [self displayImage: computedImage];
//    [self displayImage: imgSrc];
    
    
    g_bDetection = m_bDetectOk; //Save not detected image to firebase to seprate place for debug and tuning.
    
    if (g_bDetection == false) {
        [[NSNotificationCenter defaultCenter] postNotificationName:@"recognitionFailure" object:imgSrc];
    }

}
*/


-(void) displayImage:(UIImage *)imageDisplay {
 //   _preview.hidden = true;
 //   photoView.hidden = false;
  //  photoView = [photoView initWithFrame:self.cameraView.bounds andImage:imageDisplay];
  //  photoView.autoresizingMask = (1 << 6) -1;
    _Base64String1 = [UIImageJPEGRepresentation(imageDisplay,0.3) base64EncodedStringWithOptions:0];
    _Base64String1 = [_Base64String1 stringByReplacingOccurrencesOfString:@"+" withString:@"%2B"];
    
    NSLog(@"imgage Displayed");
    [[NSNotificationCenter defaultCenter] postNotificationName:@"imageTaken" object:imageDisplay];
}

@end
