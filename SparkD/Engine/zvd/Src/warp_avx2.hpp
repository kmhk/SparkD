
#pragma once

#include "cvlibbase/Inc/_cvlibbase.h"

int VResizeLinearVec_32s8u_avx2(const cvlib::uchar** _src, cvlib::uchar* dst, const cvlib::uchar* _beta, int width );

template<int shiftval>
int VResizeLinearVec_32f16_avx2(const cvlib::uchar** _src, cvlib::uchar* _dst, const cvlib::uchar* _beta, int width );

int VResizeCubicVec_32s8u_avx2(const cvlib::uchar** _src, cvlib::uchar* dst, const cvlib::uchar* _beta, int width );

template<int shiftval>
int VResizeCubicVec_32f16_avx2(const cvlib::uchar** _src, cvlib::uchar* _dst, const cvlib::uchar* _beta, int width );

