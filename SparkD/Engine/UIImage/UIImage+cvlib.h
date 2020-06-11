//
//  UIImage+OpenCV.h
//  OpenCVClient
//
//  Created by Robin Summerhill on 02/09/2011.
//  Copyright 2011 Aptogo Limited. All rights reserved.
//
//  Permission is given to use this source code file without charge in any
//  project, commercial or otherwise, entirely at your risk, with the condition
//  that any redistribution (in part or whole) of source code must retain
//  this copyright and permission notice. Attribution in compiled projects is
//  appreciated but not required.
//

#import <UIKit/UIKit.h>
#import "cvlibbase/Inc/_cvlibbase.h"

@interface UIImage (UIImage_cvlib)

+(UIImage *)imageWithCVMat:(const cvlib::Mat&)cvMat;
-(id)initWithCVMat:(const cvlib::Mat&)cvMat;
//-(cvlib::Mat*) UIImage2Mat:(UIImage*) Image;

@property(nonatomic, readonly) cvlib::Mat* CVMat;
@property(nonatomic, readonly) cvlib::Mat* CVGrayscaleMat;


@end

cvlib::Mat* UIImage2Mat(UIImage* Image);
UIImage* Mat2UIImage(const cvlib::Mat& img);

