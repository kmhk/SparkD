
#pragma once

#include "_cvlibbase.h"

int VResizeLinearVec_32f_avx(const cvlib::uchar** _src, cvlib::uchar* _dst, const cvlib::uchar* _beta, int width );

int VResizeCubicVec_32f_avx(const cvlib::uchar** _src, cvlib::uchar* _dst, const cvlib::uchar* _beta, int width );
