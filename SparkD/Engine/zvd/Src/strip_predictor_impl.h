#pragma once

#include "cvlibbase/Inc/Mat.h"
#include "cvlibbase/Inc/XFile.h"
#include <vector>
#include "strip_transforms.h"

#include "regression_tree.h"

namespace cvlib
{
	namespace impl
    {

	    float location (const Vecf& shape,int idx);

		void create_shape_relative_encoding (
						const Vecf& shape,
						const std::vector<float>& pixel_coordinates,
						std::vector<int>& anchor_idx, 
						std::vector<float>& deltas
		);

		strip_transform_affine find_tform_between_shapes (const Vecf& from_shape, const Vecf& to_shape);

	    strip_transform_affine normalizing_tform (const Range& rect);

	    strip_transform_affine unnormalizing_tform (const Range& rect);

		void extract_feature_strip_values (
			const Vecf& vsignal,
			const Range& rect,
			const Vecf& current_shape,
			const Vecf& reference_shape,
			const std::vector<int>& reference_pixel_anchor_idx,
			const std::vector<float>& reference_pixel_deltas,
			std::vector<float>& feature_pixel_values
		);

    } // end namespace impl

}