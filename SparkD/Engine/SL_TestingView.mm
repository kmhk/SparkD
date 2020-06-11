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

BOOL bOpen = false;

cvlib::Rect testT_rect[COLUMN_NUM];
int     testT_color[COLUMN_NUM];
cvlib::Rect controlC_rect[COLUMN_NUM];
int controlC_color[COLUMN_NUM];
cvlib::Rect betweenCT_rect[COLUMN_NUM];
int betweenCT_color[COLUMN_NUM];


cvlib::Rect rtResult_middle[COLUMN_NUM];
int     nColor_middle[COLUMN_NUM];
cvlib::Rect rtResult_right[COLUMN_NUM * MAX_ROWS];
int nColor_right[COLUMN_NUM * MAX_ROWS];
int nCnt_right[COLUMN_NUM];
cvlib::Rect rtResult_corner[4];
int nColor_corner[4];
BOOL m_bDetectOk = false;


@implementation SL_TestingView
bool bcameraStatue;
float Screen_W;
float Screen_H;
bool isCameraShow;

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
  //  g_pTestVC = self;

    
#if TARGET_OS_SIMULATOR
    isCameraShow = false;
#else
    isCameraShow = false;
#endif
    // Simulator-specific code

}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}




- (void)captureOutput:(AVCaptureOutput *)captureOutput
didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection *)connection
{
    @autoreleasepool {
        // Create a UIImage from the sample buffer data
        CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
        CVPixelBufferLockBaseAddress(imageBuffer,0);
        uint8_t *baseAddress = (uint8_t *)CVPixelBufferGetBaseAddress(imageBuffer);
        size_t bytesPerRow = CVPixelBufferGetBytesPerRow(imageBuffer);
        size_t width = CVPixelBufferGetWidth(imageBuffer);
        size_t height = CVPixelBufferGetHeight(imageBuffer);
        
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        CGContextRef newContext = CGBitmapContextCreate(baseAddress,
                                                        width, height, 8, bytesPerRow, colorSpace,
                                                        kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst);
        CGImageRef newImage = CGBitmapContextCreateImage(newContext);
        
        CGContextRelease(newContext);
        CGColorSpaceRelease(colorSpace);
        
        
        UIImage *image= [UIImage imageWithCGImage:newImage scale:1 orientation:UIImageOrientationLeftMirrored];
        
        CGImageRelease(newImage);
        
        image = [image fixOrientation];
        CVPixelBufferUnlockBaseAddress(imageBuffer,0);
    }
    
    //[pool release];
}


#define FIXED_SIZE 900

-(UIImage *) MainProcess:(UIImage *)imgSrc :(bool)bFlip :(int) imageCount
{
    if (imageCount == 0) {
        [GlobalHelper fillWithRefPatternLocation:NULL];
    }
    
    cvlib::Mat *pMatSrc;
    pMatSrc = UIImage2Mat(imgSrc);
    if (pMatSrc == NULL)
        return NULL;
    if (bFlip)
        pMatSrc->flipLR();

    memset(testT_rect, 0, sizeof(Rect) * 2);   //  COLUMN_NUM);
    memset(testT_color, 0, sizeof(int) * 2);
    memset(controlC_rect, 0, sizeof(Rect) * 2);
    memset(controlC_color, 0, sizeof(int) * 2);
    memset(betweenCT_rect, 0, sizeof(int) * 2);
    memset(betweenCT_color, 0, sizeof(int) * 2);
    
    NSMutableArray *solver1Values = [NSMutableArray array];
    NSMutableArray *solver2Values = [NSMutableArray array];

    NSLog(@"w:%d, h:%d", pMatSrc->cols(),pMatSrc->rows());


    cvlib::Mat rotateSrc;
    m_bDetectOk = false;
    int g_selector = 4;

     m_bDetectOk = _MainProcess(*pMatSrc, g_selector, rotateSrc, testT_rect, testT_color, controlC_rect, controlC_color, betweenCT_rect, betweenCT_color,true);

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
       
    }

    // TODO: Display Image rotateSrc in main App  [self displayImage: Mat2UIImage(rotateSrc)];
    // TODO: display Alert message somethign like
    UIImage * image = Mat2UIImage(rotateSrc);
    return image;
 /*   if (m_bDetectOk == false)
    {
        [self showMessage:@"Cannot Detect the Image!"
        withTitle:@"Error"];
    }
    else {
        [self showMessage:@"Great Job, Pritesh, EverGlowingComet now do the job!"
        withTitle:@"Success!"];
    }   */
    
    
}

-(void)showMessage:(NSString*)message withTitle:(NSString *)title
{

 UIAlertController * alert=   [UIAlertController
                                  alertControllerWithTitle:title
                                  message:message
                                  preferredStyle:UIAlertControllerStyleAlert];

    UIAlertAction *okAction = [UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:^(UIAlertAction *action){

        //do something when click button
    }];
    [alert addAction:okAction];
    UIViewController *vc = [[[[UIApplication sharedApplication] delegate] window] rootViewController];
    [vc presentViewController:alert animated:YES completion:nil];
}

@end
