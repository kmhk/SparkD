//  EngineLib.h
//  EngineLib
//
//  Created by WOLF on 10/24/15.
//  Copyright (c) 2015 wolf. All rights reserved.
//

#import <Foundation/Foundation.h>
#include "UIImage+cvlib.h"

bool _MainProcess(cvlib::Mat matSrc, int mode, cvlib::Mat& mdraw, cvlib::Rect* testT_rect, int* testT_color, cvlib::Rect* controlC_rect, int* controlC_color, cvlib::Rect* betweenCT_rect, int* betweenCT_color, bool bAutoBalance=false);
//

double Solver3(double Tgrey, double Cgrey, double CTgrey, bool MLmodel);
double Solver4(int Trgb, int Crgb, int CTrgb);
