#include "full_strip_detection.h"

namespace cvlib
{

void full_strip_detection::drawTo(Mat& image, COLOR color, int thickness) const
{
/*	COLOR red(255,0,0);
	COLOR green(0,255,0);
	int count = num_parts();

	for (int i=0; i<count; i++)
	{
		Point2i pt = part(i);
		image.drawCross(pt, 2, color);
	}

	image.drawPolygon(&parts[0], count, color, thickness, true);*/

//	image.drawRect(this->rect, color, thickness);
//	image.drawPolygon(&parts[0], 17, green, thickness, true);

/*	image.drawPolygon(&parts[27], 4, green, thickness, true);

	image.drawPolygon(&parts[17], 5, green, thickness, true); // left eye brow

	image.drawPolygon(&parts[22], 5, green, thickness, true); 

	image.drawPolygon(&parts[30], 6, green, thickness, false);*/ // right eye brow

//	image.drawPolygon(&parts[36], 6, green, thickness, false); // left eye

//	image.drawPolygon(&parts[42], 6, green, thickness, false); // right eye

/*	image.drawPolygon(&parts[48], 11, green, thickness, false);

	image.drawPolygon(&parts[60], 7, green, thickness, false);*/
}

void full_strip_detection::toFile (XFile* pfile)
{
}
void full_strip_detection::fromFile (XFile* pfile)
{
}

}