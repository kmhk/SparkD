/*! 
 * \file    cvliboperation.h
 * \ingroup base
 * \brief   cvlibbase.
 * \author  
 */
#pragma once

#include "cvlibmacros.h"

namespace cvlib
{

template<typename _Tp> static inline _Tp saturate_cast(uchar v) { return _Tp(v); }
template<typename _Tp> static inline _Tp saturate_cast(short v) { return _Tp(v); }
template<typename _Tp> static inline _Tp saturate_cast(int v) { return _Tp(v); }
template<typename _Tp> static inline _Tp saturate_cast(float v) { return _Tp(v); }
template<typename _Tp> static inline _Tp saturate_cast(double v) { return _Tp(v); }

//////////////////////////////////////////////////////////////////////////////////////////////////
#define CVLIB_WHOLE_SEQ_END_INDEX 0x3fffffff
#define CVLIB_WHOLE_SEQ  Slice(0, CVLIB_WHOLE_SEQ_END_INDEX)

inline Range::Range() : start(0), end(0) {}
inline Range::Range(int _start, int _end) : start(_start), end(_end) {}
inline Range::Range(const Slice& slice) : start(slice.start_index), end(slice.end_index)
{
    if( start == 0 && end == CVLIB_WHOLE_SEQ_END_INDEX )
        *this = Range::all();
}

inline int Range::size() const { return end - start; }
inline bool Range::empty() const { return start == end; }
inline Range Range::all() { return Range(INT_MIN, INT_MAX); }

static inline bool operator == (const Range& r1, const Range& r2)
{ return r1.start == r2.start && r1.end == r2.end; }

static inline bool operator != (const Range& r1, const Range& r2)
{ return !(r1 == r2); }

static inline bool operator !(const Range& r)
{ return r.start == r.end; }

static inline Range operator & (const Range& r1, const Range& r2)
{
    Range r(MAX(r1.start, r2.start), MIN(r1.end, r2.end));
    r.end = MAX(r.end, r.start);
    return r;
}

static inline Range& operator &= (Range& r1, const Range& r2)
{
    r1 = r1 & r2;
    return r1;
}

static inline Range operator + (const Range& r1, int delta)
{
    return Range(r1.start + delta, r1.end + delta);
}

static inline Range operator + (int delta, const Range& r1)
{
    return Range(r1.start + delta, r1.end + delta);
}

static inline Range operator - (const Range& r1, int delta)
{
    return r1 + (-delta);
}

inline Range::operator Slice() const
{ return *this != Range::all() ? Slice(start, end) : CVLIB_WHOLE_SEQ; }

}