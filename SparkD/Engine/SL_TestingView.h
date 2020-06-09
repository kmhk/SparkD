//
//  SL_TestingView.h
//  SparkLabs
//
//  Created by Spark Diagnostics on 09/05/16.
//  Copyright (c) 2016 Spark Diagnostics. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>
#define COLUMN_NUM 11
#define MAX_ROWS 6

//import "VIPhotoView.h"
//#import "Reachability.h"
//#import <SystemConfiguration/SystemConfiguration.h>

@interface SL_TestingView : UIViewController<UIImagePickerControllerDelegate, UINavigationControllerDelegate, AVCaptureVideoDataOutputSampleBufferDelegate,AVCaptureAudioDataOutputSampleBufferDelegate,UIScrollViewDelegate>
{
 // BOOL isZooming;
  //  VIPhotoView *photoView;
}


-(void) MainProcess:(UIImage *)imgSrc :(bool) bFlip :(int) imageCount;
@end
