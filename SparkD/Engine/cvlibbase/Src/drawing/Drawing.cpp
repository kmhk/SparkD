#include "Drawing.h"
#include "cvlibutil.h"

namespace cvlib
{

	static void PolyLine(Mat& img, const Point* v, int count, bool is_closed, Scalar color, int thickness, int line_type, int shift);
	static void ThickLine(Mat& img, Point p0, Point p1, Scalar color, int thickness, int line_type, int flags, int shift);

	extern const char* g_HersheyGlyphs[];

#define CV_AA 16

	enum { XY_SHIFT = 16, XY_ONE = 1 << XY_SHIFT, DRAWING_STORAGE_BLOCK = (1 << 12) - 256 };
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

	enum {
		FONT_SIZE_SHIFT = 8, FONT_ITALIC_ALPHA = (1 << 8),
		FONT_ITALIC_DIGIT = (2 << 8), FONT_ITALIC_PUNCT = (4 << 8),
		FONT_ITALIC_BRACES = (8 << 8), FONT_HAVE_GREEK = (16 << 8),
		FONT_HAVE_CYRILLIC = (32 << 8)
	};

	static const int HersheyPlain[] = {
		(5 + 4 * 16) + FONT_HAVE_GREEK,
		199, 214, 217, 233, 219, 197, 234, 216, 221, 222, 228, 225, 211, 224, 210, 220,
		200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 212, 213, 191, 226, 192,
		215, 190, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
		14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 193, 84,
		194, 85, 86, 87, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
		112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126,
		195, 223, 196, 88 };

	static const int HersheyPlainItalic[] = {
		(5 + 4 * 16) + FONT_ITALIC_ALPHA + FONT_HAVE_GREEK,
		199, 214, 217, 233, 219, 197, 234, 216, 221, 222, 228, 225, 211, 224, 210, 220,
		200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 212, 213, 191, 226, 192,
		215, 190, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
		64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 193, 84,
		194, 85, 86, 87, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161,
		162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176,
		195, 223, 196, 88 };

	static const int HersheyComplexSmall[] = {
		(6 + 7 * 16) + FONT_HAVE_GREEK,
		1199, 1214, 1217, 1275, 1274, 1271, 1272, 1216, 1221, 1222, 1219, 1232, 1211, 1231, 1210, 1220,
		1200, 1201, 1202, 1203, 1204, 1205, 1206, 1207, 1208, 1209, 1212, 2213, 1241, 1238, 1242,
		1215, 1273, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013,
		1014, 1015, 1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023, 1024, 1025, 1026, 1223, 1084,
		1224, 1247, 586, 1249, 1101, 1102, 1103, 1104, 1105, 1106, 1107, 1108, 1109, 1110, 1111,
		1112, 1113, 1114, 1115, 1116, 1117, 1118, 1119, 1120, 1121, 1122, 1123, 1124, 1125, 1126,
		1225, 1229, 1226, 1246 };

	static const int HersheyComplexSmallItalic[] = {
		(6 + 7 * 16) + FONT_ITALIC_ALPHA + FONT_HAVE_GREEK,
		1199, 1214, 1217, 1275, 1274, 1271, 1272, 1216, 1221, 1222, 1219, 1232, 1211, 1231, 1210, 1220,
		1200, 1201, 1202, 1203, 1204, 1205, 1206, 1207, 1208, 1209, 1212, 1213, 1241, 1238, 1242,
		1215, 1273, 1051, 1052, 1053, 1054, 1055, 1056, 1057, 1058, 1059, 1060, 1061, 1062, 1063,
		1064, 1065, 1066, 1067, 1068, 1069, 1070, 1071, 1072, 1073, 1074, 1075, 1076, 1223, 1084,
		1224, 1247, 586, 1249, 1151, 1152, 1153, 1154, 1155, 1156, 1157, 1158, 1159, 1160, 1161,
		1162, 1163, 1164, 1165, 1166, 1167, 1168, 1169, 1170, 1171, 1172, 1173, 1174, 1175, 1176,
		1225, 1229, 1226, 1246 };

	static const int HersheySimplex[] = {
		(9 + 12 * 16) + FONT_HAVE_GREEK,
		2199, 714, 717, 733, 719, 697, 734, 716, 721, 722, 728, 725, 711, 724, 710, 720,
		700, 701, 702, 703, 704, 705, 706, 707, 708, 709, 712, 713, 691, 726, 692,
		715, 690, 501, 502, 503, 504, 505, 506, 507, 508, 509, 510, 511, 512, 513,
		514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 525, 526, 693, 584,
		694, 2247, 586, 2249, 601, 602, 603, 604, 605, 606, 607, 608, 609, 610, 611,
		612, 613, 614, 615, 616, 617, 618, 619, 620, 621, 622, 623, 624, 625, 626,
		695, 723, 696, 2246 };

	static const int HersheyDuplex[] = {
		(9 + 12 * 16) + FONT_HAVE_GREEK,
		2199, 2714, 2728, 2732, 2719, 2733, 2718, 2727, 2721, 2722, 2723, 2725, 2711, 2724, 2710, 2720,
		2700, 2701, 2702, 2703, 2704, 2705, 2706, 2707, 2708, 2709, 2712, 2713, 2730, 2726, 2731,
		2715, 2734, 2501, 2502, 2503, 2504, 2505, 2506, 2507, 2508, 2509, 2510, 2511, 2512, 2513,
		2514, 2515, 2516, 2517, 2518, 2519, 2520, 2521, 2522, 2523, 2524, 2525, 2526, 2223, 2084,
		2224, 2247, 587, 2249, 2601, 2602, 2603, 2604, 2605, 2606, 2607, 2608, 2609, 2610, 2611,
		2612, 2613, 2614, 2615, 2616, 2617, 2618, 2619, 2620, 2621, 2622, 2623, 2624, 2625, 2626,
		2225, 2229, 2226, 2246 };

	static const int HersheyComplex[] = {
		(9 + 12 * 16) + FONT_HAVE_GREEK + FONT_HAVE_CYRILLIC,
		2199, 2214, 2217, 2275, 2274, 2271, 2272, 2216, 2221, 2222, 2219, 2232, 2211, 2231, 2210, 2220,
		2200, 2201, 2202, 2203, 2204, 2205, 2206, 2207, 2208, 2209, 2212, 2213, 2241, 2238, 2242,
		2215, 2273, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013,
		2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025, 2026, 2223, 2084,
		2224, 2247, 587, 2249, 2101, 2102, 2103, 2104, 2105, 2106, 2107, 2108, 2109, 2110, 2111,
		2112, 2113, 2114, 2115, 2116, 2117, 2118, 2119, 2120, 2121, 2122, 2123, 2124, 2125, 2126,
		2225, 2229, 2226, 2246 };

	static const int HersheyComplexItalic[] = {
		(9 + 12 * 16) + FONT_ITALIC_ALPHA + FONT_ITALIC_DIGIT + FONT_ITALIC_PUNCT +
		FONT_HAVE_GREEK + FONT_HAVE_CYRILLIC,
		2199, 2764, 2778, 2782, 2769, 2783, 2768, 2777, 2771, 2772, 2219, 2232, 2211, 2231, 2210, 2220,
		2750, 2751, 2752, 2753, 2754, 2755, 2756, 2757, 2758, 2759, 2212, 2213, 2241, 2238, 2242,
		2765, 2273, 2051, 2052, 2053, 2054, 2055, 2056, 2057, 2058, 2059, 2060, 2061, 2062, 2063,
		2064, 2065, 2066, 2067, 2068, 2069, 2070, 2071, 2072, 2073, 2074, 2075, 2076, 2223, 2084,
		2224, 2247, 587, 2249, 2151, 2152, 2153, 2154, 2155, 2156, 2157, 2158, 2159, 2160, 2161,
		2162, 2163, 2164, 2165, 2166, 2167, 2168, 2169, 2170, 2171, 2172, 2173, 2174, 2175, 2176,
		2225, 2229, 2226, 2246 };

	static const int HersheyTriplex[] = {
		(9 + 12 * 16) + FONT_HAVE_GREEK,
		2199, 3214, 3228, 3232, 3219, 3233, 3218, 3227, 3221, 3222, 3223, 3225, 3211, 3224, 3210, 3220,
		3200, 3201, 3202, 3203, 3204, 3205, 3206, 3207, 3208, 3209, 3212, 3213, 3230, 3226, 3231,
		3215, 3234, 3001, 3002, 3003, 3004, 3005, 3006, 3007, 3008, 3009, 3010, 3011, 3012, 3013,
		2014, 3015, 3016, 3017, 3018, 3019, 3020, 3021, 3022, 3023, 3024, 3025, 3026, 2223, 2084,
		2224, 2247, 587, 2249, 3101, 3102, 3103, 3104, 3105, 3106, 3107, 3108, 3109, 3110, 3111,
		3112, 3113, 3114, 3115, 3116, 3117, 3118, 3119, 3120, 3121, 3122, 3123, 3124, 3125, 3126,
		2225, 2229, 2226, 2246 };

	static const int HersheyTriplexItalic[] = {
		(9 + 12 * 16) + FONT_ITALIC_ALPHA + FONT_ITALIC_DIGIT +
		FONT_ITALIC_PUNCT + FONT_HAVE_GREEK,
		2199, 3264, 3278, 3282, 3269, 3233, 3268, 3277, 3271, 3272, 3223, 3225, 3261, 3224, 3260, 3270,
		3250, 3251, 3252, 3253, 3254, 3255, 3256, 3257, 3258, 3259, 3262, 3263, 3230, 3226, 3231,
		3265, 3234, 3051, 3052, 3053, 3054, 3055, 3056, 3057, 3058, 3059, 3060, 3061, 3062, 3063,
		2064, 3065, 3066, 3067, 3068, 3069, 3070, 3071, 3072, 3073, 3074, 3075, 3076, 2223, 2084,
		2224, 2247, 587, 2249, 3151, 3152, 3153, 3154, 3155, 3156, 3157, 3158, 3159, 3160, 3161,
		3162, 3163, 3164, 3165, 3166, 3167, 3168, 3169, 3170, 3171, 3172, 3173, 3174, 3175, 3176,
		2225, 2229, 2226, 2246 };

	static const int HersheyScriptSimplex[] = {
		(9 + 12 * 16) + FONT_ITALIC_ALPHA + FONT_HAVE_GREEK,
		2199, 714, 717, 733, 719, 697, 734, 716, 721, 722, 728, 725, 711, 724, 710, 720,
		700, 701, 702, 703, 704, 705, 706, 707, 708, 709, 712, 713, 691, 726, 692,
		715, 690, 551, 552, 553, 554, 555, 556, 557, 558, 559, 560, 561, 562, 563,
		564, 565, 566, 567, 568, 569, 570, 571, 572, 573, 574, 575, 576, 693, 584,
		694, 2247, 586, 2249, 651, 652, 653, 654, 655, 656, 657, 658, 659, 660, 661,
		662, 663, 664, 665, 666, 667, 668, 669, 670, 671, 672, 673, 674, 675, 676,
		695, 723, 696, 2246 };

	static const int HersheyScriptComplex[] = {
		(9 + 12 * 16) + FONT_ITALIC_ALPHA + FONT_ITALIC_DIGIT + FONT_ITALIC_PUNCT + FONT_HAVE_GREEK,
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

		switch (fontFace & 15)
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
			assert(false);
		}
		return ascii;
	}


	void putText(Mat& img, const String& text, const Point2i& org,
		int fontFace, double fontScale, Scalar color,
		int thickness = 1, int line_type = 8, bool bottomLeftOrigin = false)

	{
		const int* ascii = getFontData(fontFace);

		///    double buf[4];
		//   scalarToRawData(color, buf, img.type1());

		int base_line = -(ascii[0] & 15);
		int hscale = cvutil::round(fontScale*XY_ONE), vscale = hscale;

		if (line_type == CV_AA && img.type() != MAT_Tbyte)
			line_type = 8;

		if (bottomLeftOrigin)
			vscale = -vscale;

		int view_x = org.x << XY_SHIFT;
		int view_y = (org.y << XY_SHIFT) + base_line*vscale;
		std::vector<Point> pts;
		pts.reserve(1 << 10);
		const char **faces = g_HersheyGlyphs;

		for (int i = 0; text[i] != '\0'; i++)
		{
			int c = (uchar)text[i];
			Point p;

			if (c >= 127 || c < ' ')
				c = '?';

			const char* ptr = faces[ascii[(c - ' ') + 1]];
			p.x = (uchar)ptr[0] - 'R';
			p.y = (uchar)ptr[1] - 'R';
			int dx = p.y*hscale;
			view_x -= p.x*hscale;
			pts.resize(0);

			for (ptr += 2;; )
			{
				if (*ptr == ' ' || !*ptr)
				{
					if (pts.size() > 1)
						PolyLine(img, &pts[0], (int)pts.size(), false, color, thickness, line_type, XY_SHIFT);
					if (!*ptr++)
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

	Size getTextSize(const String& text, int fontFace, double fontScale, int thickness, int* _base_line)
	{
		Size size;
		double view_x = 0;
		const char **faces = g_HersheyGlyphs;
		const int* ascii = getFontData(fontFace);

		int base_line = (ascii[0] & 15);
		int cap_line = (ascii[0] >> 4) & 15;
		size.height = cvutil::round((cap_line + base_line)*fontScale + (thickness + 1) / 2);

		for (int i = 0; text[i] != '\0'; i++)
		{
			int c = (uchar)text[i];
			Point p;

			if (c >= 127 || c < ' ')
				c = '?';

			const char* ptr = faces[ascii[(c - ' ') + 1]];
			p.x = (uchar)ptr[0] - 'R';
			p.y = (uchar)ptr[1] - 'R';
			view_x += (p.y - p.x)*fontScale;
		}

		size.width = cvutil::round(view_x + thickness);
		if (_base_line)
			*_base_line = cvutil::round(base_line*fontScale + thickness*0.5);
		return size;
	}




	static void ThickLine(Mat& img, Point p0, Point p1, Scalar color,
		int thickness, int line_type, int flags, int shift)
	{
		static const double INV_XY_ONE = 1. / XY_ONE;

		p0.x <<= XY_SHIFT - shift;
		p0.y <<= XY_SHIFT - shift;
		p1.x <<= XY_SHIFT - shift;
		p1.y <<= XY_SHIFT - shift;

		if (thickness <= 1)
		{
			COLOR c((uchar)color.vec_array[2], (uchar)color.vec_array[1], (uchar)color.vec_array[0]);
			p0.x = (p0.x + (XY_ONE >> 1)) >> XY_SHIFT;
			p0.y = (p0.y + (XY_ONE >> 1)) >> XY_SHIFT;
			p1.x = (p1.x + (XY_ONE >> 1)) >> XY_SHIFT;
			p1.y = (p1.y + (XY_ONE >> 1)) >> XY_SHIFT;
			img.drawLine(p0, p1, c);
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
			Point pt[4], dp = Point(0, 0);
			double dx = (p0.x - p1.x)*INV_XY_ONE, dy = (p1.y - p0.y)*INV_XY_ONE;
			double r = dx * dx + dy * dy;
			int i, oddThickness = thickness & 1;
			thickness <<= XY_SHIFT - 1;

			if (fabs(r) > DBL_EPSILON)
			{
				r = (thickness + oddThickness*XY_ONE*0.5) / sqrt(r);
				dp.x = cvutil::round(dy * r);
				dp.y = cvutil::round(dx * r);

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

			for (i = 0; i < 2; i++)
			{
				if (flags & (i + 1))
				{
					if (line_type < CV_AA)
					{
						Point center;
						center.x = (p0.x + (XY_ONE >> 1)) >> XY_SHIFT;
						center.y = (p0.y + (XY_ONE >> 1)) >> XY_SHIFT;
						int radius = (thickness + (XY_ONE >> 1)) >> XY_SHIFT;
						COLOR colorref((uchar)color.vec_array[2], (uchar)color.vec_array[1], (uchar)color.vec_array[0]);
						img.drawEllipse(Rect(center.x - radius, center.y - radius, radius * 2 + 1, radius * 2 + 1), colorref);
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
	static void PolyLine(Mat& img, const Point* v, int count, bool is_closed,
		Scalar color, int thickness, int line_type, int shift)
	{
		if (!v || count <= 0)
			return;

		int i = is_closed ? count - 1 : 0;
		int flags = 2 + !is_closed;
		Point p0;
		assert(0 <= shift && shift <= XY_SHIFT && thickness >= 0);

		p0 = v[i];
		for (i = !is_closed; i < count; i++)
		{
			Point p = v[i];
			ThickLine(img, p0, p, color, thickness, line_type, flags, shift);
			p0 = p;
			flags = 2;
		}
	}

	bool selectionPolygonFill(const Mat& image, Mat& selection, Rect& boundingRegion, const vec2i* points, int npoints, int level)
	{
		if (points == NULL || npoints < 3)
			return false;

		if (!selection.isValid() || selection.size() != image.size() || selection.type1() != MAT_Tuchar)
		{
			selection.create(image.size(), MAT_Tuchar);
			if (level == 0)
				selection = 255;
			else
				selection = 0;
		}
		
		Size imgsize = image.size();

		uchar* plocal = (uchar*)calloc(imgsize.width*imgsize.height, 1);
		struct RECT { int left, top, right, bottom; };
		RECT localbox = { imgsize.width,0,0,imgsize.height };

		int x, y, i = 0;
		const vec2i *current;
		const vec2i *next = NULL;
		const vec2i *start = NULL;
		//trace contour
		while (i < npoints) {
			current = &points[i];
			if (current->x != -1)
			{
				if (i == 0 || (i>0 && points[i - 1].x == -1))
					start = &points[i];

				if ((i + 1) == npoints || points[i + 1].x == -1)
					next = start;
				else
					next = &points[i + 1];

				float beta;
				if (current->x != next->x) 
				{
					beta = (float)(next->y - current->y) / (float)(next->x - current->x);
					if (current->x < next->x) 
					{
						for (x = current->x; x <= next->x; x++) 
						{
							y = (int)(current->y + (x - current->x) * beta);
							if (image.isInside(x, y)) 
								plocal[x + y * imgsize.width] = 255;
						}
					}
					else 
					{
						for (x = current->x; x >= next->x; x--)
						{
							y = (int)(current->y + (x - current->x) * beta);
							if (image.isInside(x, y)) 
								plocal[x + y * imgsize.width] = 255;
						}
					}
				}
				if (current->y != next->y)
				{
					beta = (float)(next->x - current->x) / (float)(next->y - current->y);
					if (current->y < next->y) 
					{
						for (y = current->y; y <= next->y; y++)
						{
							x = (int)(current->x + (y - current->y) * beta);
							if (image.isInside(x, y)) 
								plocal[x + y * imgsize.width] = 255;
						}
					}
					else
					{
						for (y = current->y; y >= next->y; y--)
						{
							x = (int)(current->x + (y - current->y) * beta);
							if (image.isInside(x, y)) 
								plocal[x + y * imgsize.width] = 255;
						}
					}
				}
			}
			else {
				i++;
				continue;
			}

			RECT r2;
			if (current->x < next->x) { r2.left = current->x; r2.right = next->x; }
			else { r2.left = next->x; r2.right = current->x; }
			if (current->y < next->y) { r2.bottom = current->y; r2.top = next->y; }
			else { r2.bottom = next->y; r2.top = current->y; }
			if (localbox.top < r2.top) localbox.top = MAX(0L, MIN(imgsize.height - 1, r2.top + 1));
			if (localbox.left > r2.left) localbox.left = MAX(0L, MIN(imgsize.width - 1, r2.left - 1));
			if (localbox.right < r2.right) localbox.right = MAX(0L, MIN(imgsize.width - 1, r2.right + 1));
			if (localbox.bottom > r2.bottom) localbox.bottom = MAX(0L, MIN(imgsize.height - 1, r2.bottom - 1));

			i++;
		}

		//fill the outer region
		int npix = (localbox.right - localbox.left)*(localbox.top - localbox.bottom);
		if (npix <= 0) {
			free(plocal);
			return false;
		}

		vec2i* pix = (vec2i*)calloc(npix, sizeof(vec2i));
		uchar back = 0, mark = 1;
		int fx, fy, fxx, fyy, first, last;
		int xmin = 0;
		int xmax = 0;
		int ymin = 0;
		int ymax = 0;

		for (int side = 0; side < 4; side++) {
			switch (side) {
			case 0:
				xmin = localbox.left; xmax = localbox.right + 1; ymin = localbox.bottom; ymax = localbox.bottom + 1;
				break;
			case 1:
				xmin = localbox.right; xmax = localbox.right + 1; ymin = localbox.bottom; ymax = localbox.top + 1;
				break;
			case 2:
				xmin = localbox.left; xmax = localbox.right + 1; ymin = localbox.top; ymax = localbox.top + 1;
				break;
			case 3:
				xmin = localbox.left; xmax = localbox.left + 1; ymin = localbox.bottom; ymax = localbox.top + 1;
				break;
			}
			//fill from the border points
			for (y = ymin; y < ymax; y++) {
				for (x = xmin; x < xmax; x++) {
					if (plocal[x + y*imgsize.width] == 0) {
						// Subject: FLOOD FILL ROUTINE              Date: 12-23-97 (00:57)       
						// Author:  Petter Holmberg                 Code: QB, QBasic, PDS        
						// Origin:  petter.holmberg@usa.net         Packet: GRAPHICS.ABC
						first = 0;
						last = 1;
						while (first != last) {
							fx = pix[first].x;
							fy = pix[first].y;
							fxx = fx + x;
							fyy = fy + y;
							for (;;)
							{
								if (fxx >= localbox.left && fxx <= localbox.right && fyy >= localbox.bottom && fyy <= localbox.top &&
									(plocal[fxx + fyy*imgsize.width] == back))
								{
									plocal[fxx + fyy*imgsize.width] = mark;
									if (fyy > 0 && plocal[fxx + (fyy - 1)*imgsize.width] == back) {
										pix[last].x = fx;
										pix[last].y = fy - 1;
										last++;
										if (last == npix) last = 0;
									}
									if ((fyy + 1) < imgsize.height && plocal[fxx + (fyy + 1)*imgsize.width] == back) {
										pix[last].x = fx;
										pix[last].y = fy + 1;
										last++;
										if (last == npix) last = 0;
									}
								}
								else {
									break;
								}
								fx++;
								fxx++;
							};

							fx = pix[first].x - 1;
							fy = pix[first].y;
							fxx = fx + x;
							fyy = fy + y;

							for (;; )
							{
								if (fxx >= localbox.left && fxx <= localbox.right && fyy >= localbox.bottom && fyy <= localbox.top &&
									(plocal[fxx + fyy*imgsize.width] == back))
								{
									plocal[fxx + (y + fy)*imgsize.width] = mark;
									if (fyy > 0 && plocal[fxx + (fyy - 1)*imgsize.width] == back) {
										pix[last].x = fx;
										pix[last].y = fy - 1;
										last++;
										if (last == npix) last = 0;
									}
									if ((fyy + 1) < imgsize.height && plocal[fxx + (fyy + 1)*imgsize.width] == back) {
										pix[last].x = fx;
										pix[last].y = fy + 1;
										last++;
										if (last == npix) last = 0;
									}
								}
								else {
									break;
								}
								fx--;
								fxx--;
							}

							first++;
							if (first == npix) first = 0;
						}
					}
				}
			}
		}

		//transfer the region

		uchar* pSelection = selection.data.ptr[0];
		int yoffset;
		for (y = localbox.bottom; y <= localbox.top; y++) {
			yoffset = y * imgsize.width;
			for (x = localbox.left; x <= localbox.right; x++)
				if (plocal[x + yoffset] != 1) pSelection[x + yoffset] = (uchar)level;
		}
		RECT rSelectionBox = { imgsize.width,imgsize.height,0,0 };
		if (rSelectionBox.top <= localbox.top) rSelectionBox.top = MIN(imgsize.height, localbox.top + 1);
		if (rSelectionBox.left > localbox.left) rSelectionBox.left = MIN(imgsize.width, localbox.left);
		if (rSelectionBox.right <= localbox.right) rSelectionBox.right = MIN(imgsize.width, localbox.right + 1);
		if (rSelectionBox.bottom > localbox.bottom) rSelectionBox.bottom = MIN(imgsize.height, localbox.bottom);

		boundingRegion.x = localbox.left;
		boundingRegion.y = localbox.bottom;
		boundingRegion.width = localbox.right - localbox.left;
		boundingRegion.height = localbox.top - localbox.bottom;

		free(plocal);
		free(pix);

		return true;
	}

	bool selectionPolygonLine(const Mat& image, Mat& selection, Rect& boundingRegion, const vec2i* points, int npoints, int level, int thickness)
	{
		float rthick = thickness*0.5f;
		for (int i = 0; i < npoints - 1; i++)
		{
			vec2i pt1 = points[i];
			vec2i pt2 = points[i + 1];
			vec2f dir;
			{
				dir.x = (float)(pt2.x - pt1.x), dir.y = (float)(pt2.y - pt1.y);
				float r = 1.0f / dir.norm();
				dir.x *= r, dir.y *= r;
			}
			vec2f dir2(dir.y, -dir.x);
			vec2i subpoints[4];
			subpoints[0].x = cvutil::round(pt1.x + rthick*dir2.x), subpoints[0].y = cvutil::round(pt1.y + rthick*dir2.y);
			subpoints[1].x = cvutil::round(pt2.x + rthick*dir2.x), subpoints[1].y = cvutil::round(pt2.y + rthick*dir2.y);
			subpoints[2].x = cvutil::round(pt2.x - rthick*dir2.x), subpoints[2].y = cvutil::round(pt2.y - rthick*dir2.y);
			subpoints[3].x = cvutil::round(pt1.x - rthick*dir2.x), subpoints[3].y = cvutil::round(pt1.y - rthick*dir2.y);

			Rect subRegion;
			if (!selectionPolygonFill(image, selection, subRegion, subpoints, 4, level))
				return false;

			if (i == 0)
			{
				boundingRegion = subRegion;
			}
			else
			{
				boundingRegion = boundingRegion.unionRect(subRegion);
			}
		}
		for (int i = 0; i < npoints; i++)
		{
			Vector<vec2i> subpoints;
			Rect rect(points[i].x - thickness / 2, points[i].y - thickness / 2, thickness, thickness);
			circlePoints(rect, subpoints);
			if (subpoints.getSize() > 0)
			{
				Rect subRegion;
				if (!selectionPolygonFill(image, selection, subRegion, &subpoints[0], subpoints.getSize(), 255))
					return false;
				boundingRegion = boundingRegion.unionRect(subRegion);
			}
		}
		return true;
	}

	void ifillConvexPoly(Mat& image, const Point* points, int npoints, Scalar color, float rOpacity)
	{
		assert(image.type() == MAT_Tuchar);
		Mat mask;
		Rect boundingRegion;
		selectionPolygonFill(image, mask, boundingRegion, points, npoints, 255);
		double colorbuf[4];
		scalarToRawData(color, colorbuf, (int)image.type1());

		const float rNopacity = ABS(rOpacity), rCopacity = 1 - MAX(rOpacity, 0.0f);
		if (ABS(rCopacity) < 1E-3)
		{
			int step = image.channels()*image.step();
			for (int y = boundingRegion.y; y < boundingRegion.limy(); y++)
			{
				const uchar* pmask = mask.data.ptr[y];
				uchar* dst = image.data.ptr[y];
				for (int x = boundingRegion.x; x < boundingRegion.limx(); x++)
				{
					if (pmask[x] == 255)
						memcpy(&dst[x*step], colorbuf, step);
				}
			}
		}
		else
		{
			const uchar* pcolor = (const uchar*)colorbuf;
			int step = image.channels()*image.step();
			for (int y = boundingRegion.y; y < boundingRegion.limy(); y++)
			{
				const uchar* pmask = mask.data.ptr[y];
				uchar* dst = image.data.ptr[y];
				for (int x = boundingRegion.x; x < boundingRegion.limx(); x++)
				{
					if (pmask[x] == 255)
					{
						for (int k = 0; k < step; k++)
						{
							dst[x*step+k] = (uchar)(rCopacity*dst[x*step + k] + rNopacity*pcolor[k]);
						}
					}
				}
			}
		}
	}

	static void SWAP_func(int &nA, int &nB)
	{
		int nTmpswap;
		nTmpswap = nA; nA = nB; nB = nTmpswap;
	}

	void linePoints(int nX0, int nY0, int nX1, int nY1,
		int* &pnX, int* &pnY, //ouput points
		int &nNum)//number of the points
	{
		int i;
		int steep = 1;
		int sx, sy;  /* step positive or negative (1 or -1) */
		int dx, dy;  /* delta (difference in X and Y between points) */
		int e;
		int alloc_num = (int)(sqrt((double)((nX1 - nX0)*(nX1 - nX0) + (nY1 - nY0)*(nY1 - nY0))) + 3);
		pnX = new int[alloc_num];
		pnY = new int[alloc_num];
		nNum = 0;
		/*
		* optimize for vertical and horizontal lines here
		*/
		dx = ABS(nX1 - nX0);
		sx = ((nX1 - nX0) > 0) ? 1 : -1;
		dy = ABS(nY1 - nY0);
		sy = ((nY1 - nY0) > 0) ? 1 : -1;
		if (dy > dx)
		{
			steep = 0;
			SWAP_func(nX0, nY0);
			SWAP_func(dx, dy);
			SWAP_func(sx, sy);
		}
		e = (dy << 1) - dx;
		for (i = 0; i < dx; i++)
		{
			if (steep)
			{
				pnX[nNum] = nX0; pnY[nNum] = nY0;
				nNum++;
			}
			else
			{
				pnX[nNum] = nY0; pnY[nNum] = nX0;
				nNum++;
			}
			while (e >= 0)
			{
				nY0 += sy;
				e -= (dx << 1);
			}
			nX0 += sx;
			e += (dy << 1);
		}
	}
	void	linePoints(const vec2i& pt1, const vec2i& pt2, Vector<vec2i>& points)
	{
		int nX0, nY0, nX1, nY1;
		int i;
		int steep = 1;
		int sx, sy;  /* step positive or negative (1 or -1) */
		int dx, dy;  /* delta (difference in X and Y between points) */
		int e;

		nX0 = pt1.x;
		nY0 = pt1.y;
		nX1 = pt2.x;
		nY1 = pt2.y;

		/*
		* optimize for vertical and horizontal lines here
		*/
		dx = ABS(nX1 - nX0);
		sx = ((nX1 - nX0) > 0) ? 1 : -1;
		dy = ABS(nY1 - nY0);
		sy = ((nY1 - nY0) > 0) ? 1 : -1;
		if (dy > dx)
		{
			steep = 0;
			SWAP_func(nX0, nY0);
			SWAP_func(dx, dy);
			SWAP_func(sx, sy);
		}
		e = (dy << 1) - dx;

		points.resize(dx);

		for (i = 0; i < dx; i++)
		{
			if (steep)
			{
				//				points.add(vec2i(nX0, nY0));
				points[i] = vec2i(nX0, nY0);
			}
			else
			{
				points[i] = vec2i(nY0, nX0);
			}
			while (e >= 0)
			{
				nY0 += sy;
				e -= (dx << 1);
			}
			nX0 += sx;
			e += (dy << 1);
		}
	}
	static const float SinTable[] =
	{ 0.0000000f, 0.0174524f, 0.0348995f, 0.0523360f, 0.0697565f, 0.0871557f,
		0.1045285f, 0.1218693f, 0.1391731f, 0.1564345f, 0.1736482f, 0.1908090f,
		0.2079117f, 0.2249511f, 0.2419219f, 0.2588190f, 0.2756374f, 0.2923717f,
		0.3090170f, 0.3255682f, 0.3420201f, 0.3583679f, 0.3746066f, 0.3907311f,
		0.4067366f, 0.4226183f, 0.4383711f, 0.4539905f, 0.4694716f, 0.4848096f,
		0.5000000f, 0.5150381f, 0.5299193f, 0.5446390f, 0.5591929f, 0.5735764f,
		0.5877853f, 0.6018150f, 0.6156615f, 0.6293204f, 0.6427876f, 0.6560590f,
		0.6691306f, 0.6819984f, 0.6946584f, 0.7071068f, 0.7193398f, 0.7313537f,
		0.7431448f, 0.7547096f, 0.7660444f, 0.7771460f, 0.7880108f, 0.7986355f,
		0.8090170f, 0.8191520f, 0.8290376f, 0.8386706f, 0.8480481f, 0.8571673f,
		0.8660254f, 0.8746197f, 0.8829476f, 0.8910065f, 0.8987940f, 0.9063078f,
		0.9135455f, 0.9205049f, 0.9271839f, 0.9335804f, 0.9396926f, 0.9455186f,
		0.9510565f, 0.9563048f, 0.9612617f, 0.9659258f, 0.9702957f, 0.9743701f,
		0.9781476f, 0.9816272f, 0.9848078f, 0.9876883f, 0.9902681f, 0.9925462f,
		0.9945219f, 0.9961947f, 0.9975641f, 0.9986295f, 0.9993908f, 0.9998477f,
		1.0000000f, 0.9998477f, 0.9993908f, 0.9986295f, 0.9975641f, 0.9961947f,
		0.9945219f, 0.9925462f, 0.9902681f, 0.9876883f, 0.9848078f, 0.9816272f,
		0.9781476f, 0.9743701f, 0.9702957f, 0.9659258f, 0.9612617f, 0.9563048f,
		0.9510565f, 0.9455186f, 0.9396926f, 0.9335804f, 0.9271839f, 0.9205049f,
		0.9135455f, 0.9063078f, 0.8987940f, 0.8910065f, 0.8829476f, 0.8746197f,
		0.8660254f, 0.8571673f, 0.8480481f, 0.8386706f, 0.8290376f, 0.8191520f,
		0.8090170f, 0.7986355f, 0.7880108f, 0.7771460f, 0.7660444f, 0.7547096f,
		0.7431448f, 0.7313537f, 0.7193398f, 0.7071068f, 0.6946584f, 0.6819984f,
		0.6691306f, 0.6560590f, 0.6427876f, 0.6293204f, 0.6156615f, 0.6018150f,
		0.5877853f, 0.5735764f, 0.5591929f, 0.5446390f, 0.5299193f, 0.5150381f,
		0.5000000f, 0.4848096f, 0.4694716f, 0.4539905f, 0.4383711f, 0.4226183f,
		0.4067366f, 0.3907311f, 0.3746066f, 0.3583679f, 0.3420201f, 0.3255682f,
		0.3090170f, 0.2923717f, 0.2756374f, 0.2588190f, 0.2419219f, 0.2249511f,
		0.2079117f, 0.1908090f, 0.1736482f, 0.1564345f, 0.1391731f, 0.1218693f,
		0.1045285f, 0.0871557f, 0.0697565f, 0.0523360f, 0.0348995f, 0.0174524f,
		0.0000000f, -0.0174524f, -0.0348995f, -0.0523360f, -0.0697565f, -0.0871557f,
		-0.1045285f, -0.1218693f, -0.1391731f, -0.1564345f, -0.1736482f, -0.1908090f,
		-0.2079117f, -0.2249511f, -0.2419219f, -0.2588190f, -0.2756374f, -0.2923717f,
		-0.3090170f, -0.3255682f, -0.3420201f, -0.3583679f, -0.3746066f, -0.3907311f,
		-0.4067366f, -0.4226183f, -0.4383711f, -0.4539905f, -0.4694716f, -0.4848096f,
		-0.5000000f, -0.5150381f, -0.5299193f, -0.5446390f, -0.5591929f, -0.5735764f,
		-0.5877853f, -0.6018150f, -0.6156615f, -0.6293204f, -0.6427876f, -0.6560590f,
		-0.6691306f, -0.6819984f, -0.6946584f, -0.7071068f, -0.7193398f, -0.7313537f,
		-0.7431448f, -0.7547096f, -0.7660444f, -0.7771460f, -0.7880108f, -0.7986355f,
		-0.8090170f, -0.8191520f, -0.8290376f, -0.8386706f, -0.8480481f, -0.8571673f,
		-0.8660254f, -0.8746197f, -0.8829476f, -0.8910065f, -0.8987940f, -0.9063078f,
		-0.9135455f, -0.9205049f, -0.9271839f, -0.9335804f, -0.9396926f, -0.9455186f,
		-0.9510565f, -0.9563048f, -0.9612617f, -0.9659258f, -0.9702957f, -0.9743701f,
		-0.9781476f, -0.9816272f, -0.9848078f, -0.9876883f, -0.9902681f, -0.9925462f,
		-0.9945219f, -0.9961947f, -0.9975641f, -0.9986295f, -0.9993908f, -0.9998477f,
		-1.0000000f, -0.9998477f, -0.9993908f, -0.9986295f, -0.9975641f, -0.9961947f,
		-0.9945219f, -0.9925462f, -0.9902681f, -0.9876883f, -0.9848078f, -0.9816272f,
		-0.9781476f, -0.9743701f, -0.9702957f, -0.9659258f, -0.9612617f, -0.9563048f,
		-0.9510565f, -0.9455186f, -0.9396926f, -0.9335804f, -0.9271839f, -0.9205049f,
		-0.9135455f, -0.9063078f, -0.8987940f, -0.8910065f, -0.8829476f, -0.8746197f,
		-0.8660254f, -0.8571673f, -0.8480481f, -0.8386706f, -0.8290376f, -0.8191520f,
		-0.8090170f, -0.7986355f, -0.7880108f, -0.7771460f, -0.7660444f, -0.7547096f,
		-0.7431448f, -0.7313537f, -0.7193398f, -0.7071068f, -0.6946584f, -0.6819984f,
		-0.6691306f, -0.6560590f, -0.6427876f, -0.6293204f, -0.6156615f, -0.6018150f,
		-0.5877853f, -0.5735764f, -0.5591929f, -0.5446390f, -0.5299193f, -0.5150381f,
		-0.5000000f, -0.4848096f, -0.4694716f, -0.4539905f, -0.4383711f, -0.4226183f,
		-0.4067366f, -0.3907311f, -0.3746066f, -0.3583679f, -0.3420201f, -0.3255682f,
		-0.3090170f, -0.2923717f, -0.2756374f, -0.2588190f, -0.2419219f, -0.2249511f,
		-0.2079117f, -0.1908090f, -0.1736482f, -0.1564345f, -0.1391731f, -0.1218693f,
		-0.1045285f, -0.0871557f, -0.0697565f, -0.0523360f, -0.0348995f, -0.0174524f,
		-0.0000000f, 0.0174524f, 0.0348995f, 0.0523360f, 0.0697565f, 0.0871557f,
		0.1045285f, 0.1218693f, 0.1391731f, 0.1564345f, 0.1736482f, 0.1908090f,
		0.2079117f, 0.2249511f, 0.2419219f, 0.2588190f, 0.2756374f, 0.2923717f,
		0.3090170f, 0.3255682f, 0.3420201f, 0.3583679f, 0.3746066f, 0.3907311f,
		0.4067366f, 0.4226183f, 0.4383711f, 0.4539905f, 0.4694716f, 0.4848096f,
		0.5000000f, 0.5150381f, 0.5299193f, 0.5446390f, 0.5591929f, 0.5735764f,
		0.5877853f, 0.6018150f, 0.6156615f, 0.6293204f, 0.6427876f, 0.6560590f,
		0.6691306f, 0.6819984f, 0.6946584f, 0.7071068f, 0.7193398f, 0.7313537f,
		0.7431448f, 0.7547096f, 0.7660444f, 0.7771460f, 0.7880108f, 0.7986355f,
		0.8090170f, 0.8191520f, 0.8290376f, 0.8386706f, 0.8480481f, 0.8571673f,
		0.8660254f, 0.8746197f, 0.8829476f, 0.8910065f, 0.8987940f, 0.9063078f,
		0.9135455f, 0.9205049f, 0.9271839f, 0.9335804f, 0.9396926f, 0.9455186f,
		0.9510565f, 0.9563048f, 0.9612617f, 0.9659258f, 0.9702957f, 0.9743701f,
		0.9781476f, 0.9816272f, 0.9848078f, 0.9876883f, 0.9902681f, 0.9925462f,
		0.9945219f, 0.9961947f, 0.9975641f, 0.9986295f, 0.9993908f, 0.9998477f,
		1.0000000f
	};


	static void
		sincos(int angle, float& cosval, float& sinval)
	{
		angle += (angle < 0 ? 360 : 0);
		sinval = SinTable[angle];
		cosval = SinTable[450 - angle];
	}
	/*
	constructs polygon that represents elliptic arc.
	*/
	void ellipse2Poly(const vec2i& center, Size axes, int angle, int arc_start, int arc_end, int delta, Vector<vec2i>& pts)
	{
		float alpha, beta;
		double size_a = axes.width, size_b = axes.height;
		double cx = center.x, cy = center.y;
		Point prevPt(INT_MIN, INT_MIN);
		int i;

		while (angle < 0)
			angle += 360;
		while (angle > 360)
			angle -= 360;

		if (arc_start > arc_end)
		{
			i = arc_start;
			arc_start = arc_end;
			arc_end = i;
		}
		while (arc_start < 0)
		{
			arc_start += 360;
			arc_end += 360;
		}
		while (arc_end > 360)
		{
			arc_end -= 360;
			arc_start -= 360;
		}
		if (arc_end - arc_start > 360)
		{
			arc_start = 0;
			arc_end = 360;
		}
		sincos(angle, alpha, beta);
		pts.removeAll();

		for (i = arc_start; i < arc_end + delta; i += delta)
		{
			double x, y;
			angle = i;
			if (angle > arc_end)
				angle = arc_end;
			if (angle < 0)
				angle += 360;

			x = size_a * SinTable[450 - angle];
			y = size_b * SinTable[angle];
			Point pt;
			pt.x = cvutil::round(cx + x * alpha - y * beta);
			pt.y = cvutil::round(cy + x * beta + y * alpha);
			if (pt != prevPt) {
				pts.add(pt);
				prevPt = pt;
			}
		}
	}

	void	circlePoints(const Rect& rect, Vector<vec2i>& points)
	{
		int nXrad = rect.width / 2;
		int nYrad = rect.height / 2;
		int nXradSq = nXrad * nXrad;
		int nYradSq = nYrad * nYrad;
		int nXcen = rect.x + nXrad;
		int nYcen = rect.y + nYrad;
		int nXradBkPt = (int)(nXradSq / sqrt((double)nXradSq + nYradSq));
		int nYradBkPt = (int)(nYradSq / sqrt((double)nXradSq + nYradSq));
		int nX, nY;			// nX & nY are in image coordinates
		if (nXrad <= 0 || nYrad <= 0)
			return;

		// count # of iterations in loops below
		int nPerimLength = 4 * nXradBkPt + 4 * nYradBkPt + 4;
		int* pnTemp = (int*)malloc(sizeof(int) * nPerimLength * 2);
		int* pnPerim = pnTemp;
		*(pnPerim)++ = nXcen;  // perimeter starts at top and proceeds clockwise
		*(pnPerim)++ = nYcen - nYrad;

		// from 0 to 45 degrees, measured cw from top
		for (nX = 1; nX <= nXradBkPt; nX++)
		{
			nY = (int)(-nYrad * sqrt(1 - ((double)nX / nXrad)*((double)nX / nXrad)));
			*(pnPerim)++ = nXcen + nX;
			*(pnPerim)++ = nYcen + nY;
		}

		// from 45 to 135 degrees (including right axis)
		for (nY = -nYradBkPt; nY <= nYradBkPt; nY++) {
			nX = (int)(nXrad * sqrt(1 - ((double)nY / nYrad)*((double)nY / nYrad)));
			*(pnPerim)++ = nXcen + nX;
			*(pnPerim)++ = nYcen + nY;
		}
		// from 135 to 225 degrees (including down axis)
		for (nX = nXradBkPt; nX >= -nXradBkPt; nX--) {
			nY = (int)(nYrad * sqrt(1 - ((double)nX / nXrad)*((double)nX / nXrad)));
			*(pnPerim)++ = nXcen + nX;
			*(pnPerim)++ = nYcen + nY;
		}
		// from 225 to 315 degrees (including left axis)
		for (nY = nYradBkPt; nY >= -nYradBkPt; nY--) {
			nX = (int)(-nXrad * sqrt(1 - ((double)nY / nYrad)*((double)nY / nYrad)));
			*(pnPerim)++ = nXcen + nX;
			*(pnPerim)++ = nYcen + nY;
		}
		// from 315 to 360 degrees
		for (nX = -nXradBkPt; nX < 0; nX++) {
			nY = (int)(-nYrad * sqrt(1 - ((double)nX / nXrad)*((double)nX / nXrad)));
			*(pnPerim)++ = nXcen + nX;
			*(pnPerim)++ = nYcen + nY;
		}
		points.resize(nPerimLength);
		memcpy(&points[0], pnTemp, sizeof(int)*nPerimLength * 2);
		free(pnTemp);
	}

	bool clipLine(const Size& img_size, vec2i& pt1, vec2i& pt2)
	{
		int64 x1, y1, x2, y2;
		int c1, c2;
		int64 right = img_size.width - 1, bottom = img_size.height - 1;

		if (img_size.width <= 0 || img_size.height <= 0)
			return false;

		x1 = pt1.x; y1 = pt1.y; x2 = pt2.x; y2 = pt2.y;
		c1 = (x1 < 0) + (x1 > right) * 2 + (y1 < 0) * 4 + (y1 > bottom) * 8;
		c2 = (x2 < 0) + (x2 > right) * 2 + (y2 < 0) * 4 + (y2 > bottom) * 8;

		if ((c1 & c2) == 0 && (c1 | c2) != 0)
		{
			int64 a;
			if (c1 & 12)
			{
				a = c1 < 8 ? 0 : bottom;
				x1 += (a - y1) * (x2 - x1) / (y2 - y1);
				y1 = a;
				c1 = (x1 < 0) + (x1 > right) * 2;
			}
			if (c2 & 12)
			{
				a = c2 < 8 ? 0 : bottom;
				x2 += (a - y2) * (x2 - x1) / (y2 - y1);
				y2 = a;
				c2 = (x2 < 0) + (x2 > right) * 2;
			}
			if ((c1 & c2) == 0 && (c1 | c2) != 0)
			{
				if (c1)
				{
					a = c1 == 1 ? 0 : right;
					y1 += (a - x1) * (y2 - y1) / (x2 - x1);
					x1 = a;
					c1 = 0;
				}
				if (c2)
				{
					a = c2 == 1 ? 0 : right;
					y2 += (a - x2) * (y2 - y1) / (x2 - x1);
					x2 = a;
					c2 = 0;
				}
			}

			assert((c1 & c2) != 0 || (x1 | y1 | x2 | y2) >= 0);

			pt1.x = (int)x1;
			pt1.y = (int)y1;
			pt2.x = (int)x2;
			pt2.y = (int)y2;
		}

		return (c1 | c2) == 0;
	}

	bool clipLine(const Rect& img_rect, vec2i& pt1, vec2i& pt2)
	{
		vec2i tl = img_rect.tl();
		pt1 -= tl; pt2 -= tl;
		bool inside = clipLine(img_rect.size(), pt1, pt2);
		pt1 += tl; pt2 += tl;

		return inside;
	}
}