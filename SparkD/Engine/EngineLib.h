//  EngineLib.h
//  EngineLib
//
//  Created by WOLF on 10/24/15.
//  Copyright (c) 2015 wolf. All rights reserved.
//

#import <Foundation/Foundation.h>
#include "opencv2/opencv.hpp"
//#import "SparkDiagnostics-Swift.h"

bool _MainProcess(cv::Mat matSrc, int mode, cv::Mat& mdraw, cv::Rect* testT_rect, int* testT_color, cv::Rect* controlC_rect, int* controlC_color, cv::Rect* betweenCT_rect, int* betweenCT_color, bool bAutoBalance=false);

double Solver3(double Tgrey, double Cgrey, double CTgrey, bool MLmodel);
double Solver4(int Trgb, int Crgb, int CTrgb);
