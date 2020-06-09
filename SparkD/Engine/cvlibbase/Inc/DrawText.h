#pragma once

#include "Mat.h"

namespace cvlib
{

//! returns bounding box of the text string
CVLIB_DECLSPEC Size getTextSize(const String& text, int fontFace,
                            double fontScale, int thickness,
                            int* baseLine);

}