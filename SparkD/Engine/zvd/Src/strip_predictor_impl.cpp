#include "strip_predictor_impl.h"
#include <limits>

namespace cvlib {

	namespace impl {


		// ------------------------------------------------------------------------------------

		float location(const Vecf& shape, int idx)
		{
			return shape[idx];
		}

		// ------------------------------------------------------------------------------------
		static int nearest_shape_point(const Vecf& shape, float pt)
		{
			// find the nearest part of the shape to this pixel
			float best_dist = std::numeric_limits<float>::infinity();
			const int num_shape_parts = shape.length() / 2;
			int best_idx = 0;
			for (int j = 0; j < num_shape_parts; ++j)
			{
				float p = location(shape, j) - pt;
				const float dist = fabsf(p);
				if (dist < best_dist)
				{
					best_dist = dist;
					best_idx = j;
				}
			}
			return best_idx;
		}

		// ------------------------------------------------------------------------------------

		void create_shape_relative_encoding(
			const Vecf& shape,
			const std::vector<float>& strip_coordinates,
			std::vector<int>& anchor_idx,
			std::vector<float>& deltas
			)
			/*!
				requires
					- shape.size()%2 == 0
					- shape.size() > 0
				ensures
					- #anchor_idx.size() == pixel_coordinates.size()
					- #deltas.size()     == pixel_coordinates.size()
					- for all valid i:
						- pixel_coordinates[i] == location(shape,#anchor_idx[i]) + #deltas[i]
			!*/
		{
			anchor_idx.resize(strip_coordinates.size());
			deltas.resize(strip_coordinates.size());


			for (unsigned int i = 0; i < strip_coordinates.size(); ++i)
			{
				anchor_idx[i] = nearest_shape_point(shape, strip_coordinates[i]);
				deltas[i] = strip_coordinates[i] - location(shape, anchor_idx[i]);
			}
		}

		// ------------------------------------------------------------------------------------

		strip_transform_affine find_tform_between_shapes(const Vecf& from_shape, const Vecf& to_shape)
		{
			assert(from_shape.length() == to_shape.length() && (from_shape.length() % 2) == 0 && from_shape.length() > 0);
			std::vector<float> from_points, to_points;
			const int num = from_shape.length();
			from_points.reserve(num);
			to_points.reserve(num);
			if (num == 1)
			{
				// Just use an identity transform if there is only one landmark.
				return strip_transform_affine();
			}

			for (int i = 0; i < num; ++i)
			{
				from_points.push_back(location(from_shape, i));
				to_points.push_back(location(to_shape, i));
			}
			return find_similarity_transform(from_points, to_points);
		}

		// ------------------------------------------------------------------------------------

		strip_transform_affine normalizing_tform(const Range& rect)
			/*!
				ensures
					- returns a transform that maps rect.tl_corner() to (0,0) and rect.br_corner()
						to (1,1).
			!*/
		{
			std::vector<float> from_points, to_points;
			from_points.push_back(rect.start); to_points.push_back(0.0f);
			from_points.push_back(rect.end); to_points.push_back(1.0f);
			return find_strip_affine_transform(from_points, to_points);
		}

		// ------------------------------------------------------------------------------------

		strip_transform_affine unnormalizing_tform(const Range& rect)
			/*!
				ensures
					- returns a transform that maps (0,0) to rect.tl_corner() and (1,1) to
						rect.br_corner().
			!*/
		{
			std::vector<float> from_points, to_points;
			to_points.push_back(rect.start); from_points.push_back(0.0f);
			to_points.push_back(rect.end); from_points.push_back(1.0f);
			return find_strip_affine_transform(from_points, to_points);
		}

		// ------------------------------------------------------------------------------------

		void extract_feature_strip_values(
			const Vecf& vsignal,
			const Range& rect,
			const Vecf& current_shape,
			const Vecf& reference_shape,
			const std::vector<int>& reference_pixel_anchor_idx,
			const std::vector<float >& reference_pixel_deltas,
			std::vector<float>& feature_pixel_values
			)
			/*!
				requires
					- image_type == an image object that implements the interface defined in
						dlib/image_processing/generic_image.h
					- reference_pixel_anchor_idx.size() == reference_pixel_deltas.size()
					- current_shape.size() == reference_shape.size()
					- reference_shape.size()%2 == 0
					- max(mat(reference_pixel_anchor_idx)) < reference_shape.size()/2
				ensures
					- #feature_pixel_values.size() == reference_pixel_deltas.size()
					- for all valid i:
						- #feature_pixel_values[i] == the value of the pixel in img_ that
							corresponds to the pixel identified by reference_pixel_anchor_idx[i]
							and reference_pixel_deltas[i] when the pixel is located relative to
							current_shape rather than reference_shape.
			!*/
		{
			//assert(img.channels() == 1);
			float tform = find_tform_between_shapes(reference_shape, current_shape).get_m();
			const strip_transform_affine tform_to_img = unnormalizing_tform(rect);

			feature_pixel_values.resize(reference_pixel_deltas.size());
			for (unsigned int i = 0; i < feature_pixel_values.size(); ++i)
			{
				// Compute the Point2i in the current shape corresponding to the i-th pixel and
				// then map it from the normalized shape space into pixel space.
				float pt = tform*reference_pixel_deltas[i] + location(current_shape, reference_pixel_anchor_idx[i]);
				float p = tform_to_img(pt);
				if (p > 0 && p < vsignal.length() - 1)
					feature_pixel_values[i] = (float)vsignal[(int)p];
				else
					feature_pixel_values[i] = 0;
			}
		}

	}

}