#include "DrawText.h"
#include <vector>
#include "SelectionPolygon.h"
#include "cvlibutil.h"

namespace cvlib
{

static void PolyLine( Mat& img, const Point* v, int count, bool is_closed, Scalar color, int thickness, int line_type, int shift );
static void ThickLine( Mat& img, Point p0, Point p1, Scalar color, int thickness, int line_type, int flags, int shift );

extern const char* g_HersheyGlyphs[];

#define CV_AA 16

enum { XY_SHIFT = 16, XY_ONE = 1 << XY_SHIFT, DRAWING_STORAGE_BLOCK = (1<<12) - 256 };
	/*
void cvInitFont( CvFont *font, int font_face, double hscale, double vscale,
            double shear, int thickness, int line_type )
{
    assert( font != 0 && hscale > 0 && vscale > 0 && thickness >= 0 );

    font->ascii = cv::getFontData(font_face);
    font->font_face = font_face;
    font->hscale = (float)hscale;
    font->vscale = (float)vscale;
    font->thickness = thickness;
    font->shear = (float)shear;
    font->greek = font->cyrillic = 0;
    font->line_type = line_type;
}
*/

enum { FONT_SIZE_SHIFT=8, FONT_ITALIC_ALPHA=(1 << 8),
       FONT_ITALIC_DIGIT=(2 << 8), FONT_ITALIC_PUNCT=(4 << 8),
       FONT_ITALIC_BRACES=(8 << 8), FONT_HAVE_GREEK=(16 << 8),
       FONT_HAVE_CYRILLIC=(32 << 8) };

static const int HersheyPlain[] = {
(5 + 4*16) + FONT_HAVE_GREEK,
199, 214, 217, 233, 219, 197, 234, 216, 221, 222, 228, 225, 211, 224, 210, 220,
200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 212, 213, 191, 226, 192,
215, 190, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 193, 84,
194, 85, 86, 87, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126,
195, 223, 196, 88 };

static const int HersheyPlainItalic[] = {
(5 + 4*16) + FONT_ITALIC_ALPHA + FONT_HAVE_GREEK,
199, 214, 217, 233, 219, 197, 234, 216, 221, 222, 228, 225, 211, 224, 210, 220,
200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 212, 213, 191, 226, 192,
215, 190, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 193, 84,
194, 85, 86, 87, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161,
162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176,
195, 223, 196, 88 };

static const int HersheyComplexSmall[] = {
(6 + 7*16) + FONT_HAVE_GREEK,
1199, 1214, 1217, 1275, 1274, 1271, 1272, 1216, 1221, 1222, 1219, 1232, 1211, 1231, 1210, 1220,
1200, 1201, 1202, 1203, 1204, 1205, 1206, 1207, 1208, 1209, 1212, 2213, 1241, 1238, 1242,
1215, 1273, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013,
1014, 1015, 1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023, 1024, 1025, 1026, 1223, 1084,
1224, 1247, 586, 1249, 1101, 1102, 1103, 1104, 1105, 1106, 1107, 1108, 1109, 1110, 1111,
1112, 1113, 1114, 1115, 1116, 1117, 1118, 1119, 1120, 1121, 1122, 1123, 1124, 1125, 1126,
1225, 1229, 1226, 1246 };

static const int HersheyComplexSmallItalic[] = {
(6 + 7*16) + FONT_ITALIC_ALPHA + FONT_HAVE_GREEK,
1199, 1214, 1217, 1275, 1274, 1271, 1272, 1216, 1221, 1222, 1219, 1232, 1211, 1231, 1210, 1220,
1200, 1201, 1202, 1203, 1204, 1205, 1206, 1207, 1208, 1209, 1212, 1213, 1241, 1238, 1242,
1215, 1273, 1051, 1052, 1053, 1054, 1055, 1056, 1057, 1058, 1059, 1060, 1061, 1062, 1063,
1064, 1065, 1066, 1067, 1068, 1069, 1070, 1071, 1072, 1073, 1074, 1075, 1076, 1223, 1084,
1224, 1247, 586, 1249, 1151, 1152, 1153, 1154, 1155, 1156, 1157, 1158, 1159, 1160, 1161,
1162, 1163, 1164, 1165, 1166, 1167, 1168, 1169, 1170, 1171, 1172, 1173, 1174, 1175, 1176,
1225, 1229, 1226, 1246 };

static const int HersheySimplex[] = {
(9 + 12*16) + FONT_HAVE_GREEK,
2199, 714, 717, 733, 719, 697, 734, 716, 721, 722, 728, 725, 711, 724, 710, 720,
700, 701, 702, 703, 704, 705, 706, 707, 708, 709, 712, 713, 691, 726, 692,
715, 690, 501, 502, 503, 504, 505, 506, 507, 508, 509, 510, 511, 512, 513,
514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 525, 526, 693, 584,
694, 2247, 586, 2249, 601, 602, 603, 604, 605, 606, 607, 608, 609, 610, 611,
612, 613, 614, 615, 616, 617, 618, 619, 620, 621, 622, 623, 624, 625, 626,
695, 723, 696, 2246 };

static const int HersheyDuplex[] = {
(9 + 12*16) + FONT_HAVE_GREEK,
2199, 2714, 2728, 2732, 2719, 2733, 2718, 2727, 2721, 2722, 2723, 2725, 2711, 2724, 2710, 2720,
2700, 2701, 2702, 2703, 2704, 2705, 2706, 2707, 2708, 2709, 2712, 2713, 2730, 2726, 2731,
2715, 2734, 2501, 2502, 2503, 2504, 2505, 2506, 2507, 2508, 2509, 2510, 2511, 2512, 2513,
2514, 2515, 2516, 2517, 2518, 2519, 2520, 2521, 2522, 2523, 2524, 2525, 2526, 2223, 2084,
2224, 2247, 587, 2249, 2601, 2602, 2603, 2604, 2605, 2606, 2607, 2608, 2609, 2610, 2611,
2612, 2613, 2614, 2615, 2616, 2617, 2618, 2619, 2620, 2621, 2622, 2623, 2624, 2625, 2626,
2225, 2229, 2226, 2246 };

static const int HersheyComplex[] = {
(9 + 12*16) + FONT_HAVE_GREEK + FONT_HAVE_CYRILLIC,
2199, 2214, 2217, 2275, 2274, 2271, 2272, 2216, 2221, 2222, 2219, 2232, 2211, 2231, 2210, 2220,
2200, 2201, 2202, 2203, 2204, 2205, 2206, 2207, 2208, 2209, 2212, 2213, 2241, 2238, 2242,
2215, 2273, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013,
2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025, 2026, 2223, 2084,
2224, 2247, 587, 2249, 2101, 2102, 2103, 2104, 2105, 2106, 2107, 2108, 2109, 2110, 2111,
2112, 2113, 2114, 2115, 2116, 2117, 2118, 2119, 2120, 2121, 2122, 2123, 2124, 2125, 2126,
2225, 2229, 2226, 2246 };

static const int HersheyComplexItalic[] = {
(9 + 12*16) + FONT_ITALIC_ALPHA + FONT_ITALIC_DIGIT + FONT_ITALIC_PUNCT +
FONT_HAVE_GREEK + FONT_HAVE_CYRILLIC,
2199, 2764, 2778, 2782, 2769, 2783, 2768, 2777, 2771, 2772, 2219, 2232, 2211, 2231, 2210, 2220,
2750, 2751, 2752, 2753, 2754, 2755, 2756, 2757, 2758, 2759, 2212, 2213, 2241, 2238, 2242,
2765, 2273, 2051, 2052, 2053, 2054, 2055, 2056, 2057, 2058, 2059, 2060, 2061, 2062, 2063,
2064, 2065, 2066, 2067, 2068, 2069, 2070, 2071, 2072, 2073, 2074, 2075, 2076, 2223, 2084,
2224, 2247, 587, 2249, 2151, 2152, 2153, 2154, 2155, 2156, 2157, 2158, 2159, 2160, 2161,
2162, 2163, 2164, 2165, 2166, 2167, 2168, 2169, 2170, 2171, 2172, 2173, 2174, 2175, 2176,
2225, 2229, 2226, 2246 };

static const int HersheyTriplex[] = {
(9 + 12*16) + FONT_HAVE_GREEK,
2199, 3214, 3228, 3232, 3219, 3233, 3218, 3227, 3221, 3222, 3223, 3225, 3211, 3224, 3210, 3220,
3200, 3201, 3202, 3203, 3204, 3205, 3206, 3207, 3208, 3209, 3212, 3213, 3230, 3226, 3231,
3215, 3234, 3001, 3002, 3003, 3004, 3005, 3006, 3007, 3008, 3009, 3010, 3011, 3012, 3013,
2014, 3015, 3016, 3017, 3018, 3019, 3020, 3021, 3022, 3023, 3024, 3025, 3026, 2223, 2084,
2224, 2247, 587, 2249, 3101, 3102, 3103, 3104, 3105, 3106, 3107, 3108, 3109, 3110, 3111,
3112, 3113, 3114, 3115, 3116, 3117, 3118, 3119, 3120, 3121, 3122, 3123, 3124, 3125, 3126,
2225, 2229, 2226, 2246 };

static const int HersheyTriplexItalic[] = {
(9 + 12*16) + FONT_ITALIC_ALPHA + FONT_ITALIC_DIGIT +
FONT_ITALIC_PUNCT + FONT_HAVE_GREEK,
2199, 3264, 3278, 3282, 3269, 3233, 3268, 3277, 3271, 3272, 3223, 3225, 3261, 3224, 3260, 3270,
3250, 3251, 3252, 3253, 3254, 3255, 3256, 3257, 3258, 3259, 3262, 3263, 3230, 3226, 3231,
3265, 3234, 3051, 3052, 3053, 3054, 3055, 3056, 3057, 3058, 3059, 3060, 3061, 3062, 3063,
2064, 3065, 3066, 3067, 3068, 3069, 3070, 3071, 3072, 3073, 3074, 3075, 3076, 2223, 2084,
2224, 2247, 587, 2249, 3151, 3152, 3153, 3154, 3155, 3156, 3157, 3158, 3159, 3160, 3161,
3162, 3163, 3164, 3165, 3166, 3167, 3168, 3169, 3170, 3171, 3172, 3173, 3174, 3175, 3176,
2225, 2229, 2226, 2246 };

static const int HersheyScriptSimplex[] = {
(9 + 12*16) + FONT_ITALIC_ALPHA + FONT_HAVE_GREEK,
2199, 714, 717, 733, 719, 697, 734, 716, 721, 722, 728, 725, 711, 724, 710, 720,
700, 701, 702, 703, 704, 705, 706, 707, 708, 709, 712, 713, 691, 726, 692,
715, 690, 551, 552, 553, 554, 555, 556, 557, 558, 559, 560, 561, 562, 563,
564, 565, 566, 567, 568, 569, 570, 571, 572, 573, 574, 575, 576, 693, 584,
694, 2247, 586, 2249, 651, 652, 653, 654, 655, 656, 657, 658, 659, 660, 661,
662, 663, 664, 665, 666, 667, 668, 669, 670, 671, 672, 673, 674, 675, 676,
695, 723, 696, 2246 };

static const int HersheyScriptComplex[] = {
(9 + 12*16) + FONT_ITALIC_ALPHA + FONT_ITALIC_DIGIT + FONT_ITALIC_PUNCT + FONT_HAVE_GREEK,
2199, 2764, 2778, 2782, 2769, 2783, 2768, 2777, 2771, 2772, 2219, 2232, 2211, 2231, 2210, 2220,
2750, 2751, 2752, 2753, 2754, 2755, 2756, 2757, 2758, 2759, 2212, 2213, 2241, 2238, 2242,
2215, 2273, 2551, 2552, 2553, 2554, 2555, 2556, 2557, 2558, 2559, 2560, 2561, 2562, 2563,
2564, 2565, 2566, 2567, 2568, 2569, 2570, 2571, 2572, 2573, 2574, 2575, 2576, 2223, 2084,
2224, 2247, 586, 2249, 2651, 2652, 2653, 2654, 2655, 2656, 2657, 2658, 2659, 2660, 2661,
2662, 2663, 2664, 2665, 2666, 2667, 2668, 2669, 2670, 2671, 2672, 2673, 2674, 2675, 2676,
2225, 2229, 2226, 2246 };


static const int* getFontData(int fontFace)
{
    bool isItalic = (fontFace & FONT_ITALIC) != 0;
    const int* ascii = 0;

    switch( fontFace & 15 )
    {
    case FONT_HERSHEY_SIMPLEX:
        ascii = HersheySimplex;
        break;
    case FONT_HERSHEY_PLAIN:
        ascii = !isItalic ? HersheyPlain : HersheyPlainItalic;
        break;
    case FONT_HERSHEY_DUPLEX:
        ascii = HersheyDuplex;
        break;
    case FONT_HERSHEY_COMPLEX:
        ascii = !isItalic ? HersheyComplex : HersheyComplexItalic;
        break;
    case FONT_HERSHEY_TRIPLEX:
        ascii = !isItalic ? HersheyTriplex : HersheyTriplexItalic;
        break;
    case FONT_HERSHEY_COMPLEX_SMALL:
        ascii = !isItalic ? HersheyComplexSmall : HersheyComplexSmallItalic;
        break;
    case FONT_HERSHEY_SCRIPT_SIMPLEX:
        ascii = HersheyScriptSimplex;
        break;
    case FONT_HERSHEY_SCRIPT_COMPLEX:
        ascii = HersheyScriptComplex;
        break;
    default:
        assert (false);
    }
    return ascii;
}


void putText( Mat& img, const String& text, const Point2i& org,
              int fontFace, double fontScale, Scalar color,
              int thickness=1, int line_type=8, bool bottomLeftOrigin=false )

{
    const int* ascii = getFontData(fontFace);

///    double buf[4];
 //   scalarToRawData(color, buf, img.type1());

    int base_line = -(ascii[0] & 15);
    int hscale = cvutil::round(fontScale*XY_ONE), vscale = hscale;

    if( line_type == CV_AA && img.type() != MAT_Tbyte )
        line_type = 8;

    if( bottomLeftOrigin )
        vscale = -vscale;

    int view_x = org.x << XY_SHIFT;
    int view_y = (org.y << XY_SHIFT) + base_line*vscale;
    std::vector<Point> pts;
    pts.reserve(1 << 10);
    const char **faces = g_HersheyGlyphs;

    for( int i = 0; text[i] != '\0'; i++ )
    {
        int c = (uchar)text[i];
        Point p;

        if( c >= 127 || c < ' ' )
            c = '?';

        const char* ptr = faces[ascii[(c-' ')+1]];
        p.x = (uchar)ptr[0] - 'R';
        p.y = (uchar)ptr[1] - 'R';
        int dx = p.y*hscale;
        view_x -= p.x*hscale;
        pts.resize(0);

        for( ptr += 2;; )
        {
            if( *ptr == ' ' || !*ptr )
            {
                if( pts.size() > 1 )
                    PolyLine( img, &pts[0], (int)pts.size(), false, color, thickness, line_type, XY_SHIFT );
                if( !*ptr++ )
                    break;
                pts.resize(0);
            }
            else
            {
                p.x = (uchar)ptr[0] - 'R';
                p.y = (uchar)ptr[1] - 'R';
                ptr += 2;
                pts.push_back(Point(p.x*hscale + view_x, p.y*vscale + view_y));
            }
        }
        view_x += dx;
    }
}

Size getTextSize( const String& text, int fontFace, double fontScale, int thickness, int* _base_line)
{
    Size size;
    double view_x = 0;
    const char **faces = g_HersheyGlyphs;
    const int* ascii = getFontData(fontFace);

    int base_line = (ascii[0] & 15);
    int cap_line = (ascii[0] >> 4) & 15;
    size.height = cvutil::round((cap_line + base_line)*fontScale + (thickness+1)/2);

    for( int i = 0; text[i] != '\0'; i++ )
    {
        int c = (uchar)text[i];
        Point p;

        if( c >= 127 || c < ' ' )
            c = '?';

        const char* ptr = faces[ascii[(c-' ')+1]];
        p.x = (uchar)ptr[0] - 'R';
        p.y = (uchar)ptr[1] - 'R';
        view_x += (p.y - p.x)*fontScale;
    }

    size.width = cvutil::round(view_x + thickness);
    if( _base_line )
        *_base_line = cvutil::round(base_line*fontScale + thickness*0.5);
    return size;
}




static void ThickLine( Mat& img, Point p0, Point p1, Scalar color,
           int thickness, int line_type, int flags, int shift )
{
    static const double INV_XY_ONE = 1./XY_ONE;

    p0.x <<= XY_SHIFT - shift;
    p0.y <<= XY_SHIFT - shift;
    p1.x <<= XY_SHIFT - shift;
    p1.y <<= XY_SHIFT - shift;

    if( thickness <= 1 )
    {
		COLOR c((uchar)color.vec_array[2],(uchar)color.vec_array[1],(uchar)color.vec_array[0]);
        p0.x = (p0.x + (XY_ONE>>1)) >> XY_SHIFT;
        p0.y = (p0.y + (XY_ONE>>1)) >> XY_SHIFT;
        p1.x = (p1.x + (XY_ONE>>1)) >> XY_SHIFT;
        p1.y = (p1.y + (XY_ONE>>1)) >> XY_SHIFT;
		img.drawLine (p0, p1, c);
       /* if( line_type < CV_AA )
        {
            if( line_type == 1 || line_type == 4 || shift == 0 )
            {
                p0.x = (p0.x + (XY_ONE>>1)) >> XY_SHIFT;
                p0.y = (p0.y + (XY_ONE>>1)) >> XY_SHIFT;
                p1.x = (p1.x + (XY_ONE>>1)) >> XY_SHIFT;
                p1.y = (p1.y + (XY_ONE>>1)) >> XY_SHIFT;
                Line( img, p0, p1, color, line_type );
            }
            else
                Line2( img, p0, p1, color );
        }
        else
            LineAA( img, p0, p1, color );*/
    }
    else
    {
        Point pt[4], dp = Point(0,0);
        double dx = (p0.x - p1.x)*INV_XY_ONE, dy = (p1.y - p0.y)*INV_XY_ONE;
        double r = dx * dx + dy * dy;
        int i, oddThickness = thickness & 1;
        thickness <<= XY_SHIFT - 1;

        if( fabs(r) > DBL_EPSILON )
        {
            r = (thickness + oddThickness*XY_ONE*0.5)/sqrt(r);
            dp.x = cvutil::round( dy * r );
            dp.y = cvutil::round( dx * r );

            pt[0].x = p0.x + dp.x;
            pt[0].y = p0.y + dp.y;
            pt[1].x = p0.x - dp.x;
            pt[1].y = p0.y - dp.y;
            pt[2].x = p1.x - dp.x;
            pt[2].y = p1.y - dp.y;
            pt[3].x = p1.x + dp.x;
            pt[3].y = p1.y + dp.y;
			ifillConvexPoly(img, pt, 4, color);
        }

        for( i = 0; i < 2; i++ )
        {
            if( flags & (i+1) )
            {
                if( line_type < CV_AA )
                {
                    Point center;
                    center.x = (p0.x + (XY_ONE>>1)) >> XY_SHIFT;
                    center.y = (p0.y + (XY_ONE>>1)) >> XY_SHIFT;
					int radius = (thickness + (XY_ONE>>1)) >> XY_SHIFT;
					COLOR colorref((uchar)color.vec_array[2],(uchar)color.vec_array[1],(uchar)color.vec_array[0]);
					img.drawEllipse (Rect(center.x-radius,center.y-radius,radius*2+1,radius*2+1), colorref);
                }
                else
                {
                    //EllipseEx( img, p0, Size(thickness, thickness),0, 0, 360, color, -1, line_type );
                }
            }
            p0 = p1;
        }
    }
}
static void PolyLine( Mat& img, const Point* v, int count, bool is_closed,
          Scalar color, int thickness, int line_type, int shift )
{
    if( !v || count <= 0 )
        return;

    int i = is_closed ? count - 1 : 0;
    int flags = 2 + !is_closed;
    Point p0;
    assert( 0 <= shift && shift <= XY_SHIFT && thickness >= 0 );

    p0 = v[i];
    for( i = !is_closed; i < count; i++ )
    {
        Point p = v[i];
        ThickLine( img, p0, p, color, thickness, line_type, flags, shift );
        p0 = p;
        flags = 2;
    }
}

}