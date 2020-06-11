#pragma once

#include "cvlibbase/Inc/_cvlibbase.h"

namespace cvlib
{
	enum enPageBinary {
		PB_NIBLACK, PB_SAUVOLA, PB_WOLFJOLION
	};

	void binarization(const Mat& image, Mat& bin, enPageBinary mode, int winx = 0, int winy = 0, float optK = 0.5);
}