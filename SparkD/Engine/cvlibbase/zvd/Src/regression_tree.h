#pragma once

#include "XFile.h"
#include "Mat.h"
#include <vector>

namespace cvlib
{
	namespace impl
    {
        struct CVLIB_DECLSPEC split_feature
        {
            int idx1;
            int idx2;
            float thresh;
        };

        inline int left_child (int idx) { return 2*idx + 1; }

		inline int right_child (int idx) { return 2*idx + 2; }

        struct regression_tree
        {
            std::vector<split_feature> splits;
            std::vector<Vecf> leaf_values;

            int num_leaves() const { return (int)leaf_values.size(); }

            inline const Vecf& operator()(const std::vector<float>& feature_pixel_values, int& i) const
            {
                i = 0;
                while (i < (int)splits.size())
                {
                    if (feature_pixel_values[splits[i].idx1] - feature_pixel_values[splits[i].idx2] > splits[i].thresh)
                        i = left_child(i);
                    else
                        i = right_child(i);
                }
                i = i - (int)splits.size();
                return leaf_values[i];
            }

            bool toFile(XFile* pfile) const;
            bool fromFile(XFile* pfile);
        };

    } // end namespace impl

}
