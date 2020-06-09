#include "SelectionPolygon.h"
#include "cvlibutil.h"

namespace cvlib
{

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
							dst[x*step+k] = (uchar)(rOpacity*dst[x*step + k] + rNopacity*pcolor[k]);
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
