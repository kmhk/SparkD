#pragma once

#include "_cvlibbase.h"

namespace cvlib {
	namespace ip {
		//! various border interpolation methods
		enum {
			BORDER_CONSTANT = 0, //!< `iiiiii|abcdefgh|iiiiiii`  with some specified `i`
			BORDER_REPLICATE = 1, //!< `aaaaaa|abcdefgh|hhhhhhh`
			BORDER_REFLECT = 2, //!< `fedcba|abcdefgh|hgfedcb`
			BORDER_WRAP = 3, //!< `cdefgh|abcdefgh|abcdefg`
			BORDER_REFLECT_101 = 4, //!< `gfedcb|abcdefgh|gfedcba`
			BORDER_TRANSPARENT = 5, //!< `uvwxyz|absdefgh|ijklmno`

			BORDER_REFLECT101 = BORDER_REFLECT_101, //!< same as BORDER_REFLECT_101
			BORDER_DEFAULT = BORDER_REFLECT_101, //!< same as BORDER_REFLECT_101
			BORDER_ISOLATED = 16 //!< do not look outside of ROI
		};

		//! 1D interpolation function: returns coordinate of the "donor" pixel for the specified location p.
		CVLIB_DECLSPEC int borderInterpolate(int p, int len, int borderType);

	}
}
