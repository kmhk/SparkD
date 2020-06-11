#include "cvlibipbase.h"

namespace cvlib
{
	namespace ip {
		/*
		Various border types, image boundaries are denoted with '|'

		* BORDER_REPLICATE:     aaaaaa|abcdefgh|hhhhhhh
		* BORDER_REFLECT:       fedcba|abcdefgh|hgfedcb
		* BORDER_REFLECT_101:   gfedcb|abcdefgh|gfedcba
		* BORDER_WRAP:          cdefgh|abcdefgh|abcdefg
		* BORDER_CONSTANT:      iiiiii|abcdefgh|iiiiiii  with some specified 'i'
		*/
		int borderInterpolate(int p, int len, int borderType)
		{
			if ((unsigned)p < (unsigned)len)
				;
			else if (borderType == BORDER_REPLICATE)
				p = p < 0 ? 0 : len - 1;
			else if (borderType == BORDER_REFLECT || borderType == BORDER_REFLECT_101)
			{
				int delta = borderType == BORDER_REFLECT_101;
				if (len == 1)
					return 0;
				do
				{
					if (p < 0)
						p = -p - 1 + delta;
					else
						p = len - 1 - (p - len) - delta;
				} while ((unsigned)p >= (unsigned)len);
			}
			else if (borderType == BORDER_WRAP)
			{
				if (p < 0)
					p -= ((p - len + 1) / len)*len;
				if (p >= len)
					p %= len;
			}
			else if (borderType == BORDER_CONSTANT)
				p = -1;
			else
				assert(false);// "Unknown/unsupported border type"
			return p;
		}
	}

}
