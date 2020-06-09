#include "strip_predictor.h"
#include "ColorSpace.h"
#include "strip_transforms.h"

namespace cvlib
{

	strip_predictor::strip_predictor() {}
	strip_predictor::strip_predictor(
		const Vecf& initial_shape_,
		const std::vector<std::vector<impl::regression_tree> >& forests_,
		const std::vector<std::vector<float > >& pixel_coordinates
		) : initial_shape(initial_shape_), forests(forests_)
	{
		anchor_idx.resize(pixel_coordinates.size());
		deltas.resize(pixel_coordinates.size());
		// Each cascade uses a different set of pixels for its features.  We compute
		// their representations relative to the initial shape now and save it.
		for (unsigned int i = 0; i < pixel_coordinates.size(); ++i)
			impl::create_shape_relative_encoding(initial_shape, pixel_coordinates[i], anchor_idx[i], deltas[i]);
	}

	bool strip_predictor::toFile(XFile* pfile) const
	{
		initial_shape.toFile(pfile);

		int cascade_depth = (int)forests.size();
		pfile->write(&cascade_depth, sizeof(cascade_depth), 1);
		int num_trees_per_cascade_level = (int)forests[0].size();
		pfile->write(&num_trees_per_cascade_level, sizeof(num_trees_per_cascade_level), 1);
		int feature_pool_size = (int)anchor_idx[0].size();
		pfile->write(&feature_pool_size, sizeof(feature_pool_size), 1);

		// write forests
		for (int i = 0; i < cascade_depth; i++)
		{
			const std::vector<impl::regression_tree>& tree = forests[i];
			for (int k = 0; k < num_trees_per_cascade_level; k++)
			{
				const impl::regression_tree& regressor = tree[k];
				regressor.toFile(pfile);
			}
		}
		// write anchor_idx;
		for (int i = 0; i < cascade_depth; i++)
		{
			const std::vector<int>& anchors = anchor_idx[i];
			for (int k = 0; k < feature_pool_size; k++)
			{
				const int& idx = anchors[k];
				pfile->write(&idx, sizeof(idx), 1);
			}
		}
		// write deltas
		for (int i = 0; i < cascade_depth; i++)
		{
			const std::vector<float>& points = deltas[i];
			for (int k = 0; k < feature_pool_size; k++)
			{
				const float& p = points[k];
				pfile->write(&p, sizeof(p), 1);
			}
		}
		return true;
	}
	bool strip_predictor::fromFile(XFile* pfile)
	{
		initial_shape.fromFile(pfile);

		int cascade_depth;
		pfile->read(&cascade_depth, sizeof(cascade_depth), 1);
		int num_trees_per_cascade_level;
		pfile->read(&num_trees_per_cascade_level, sizeof(num_trees_per_cascade_level), 1);
		int feature_pool_size;
		pfile->read(&feature_pool_size, sizeof(feature_pool_size), 1);

		// read forests
		forests.resize(cascade_depth);
		for (int i = 0; i < cascade_depth; i++)
		{
			std::vector<impl::regression_tree>& tree = forests[i];
			tree.resize(num_trees_per_cascade_level);
			for (int k = 0; k < num_trees_per_cascade_level; k++)
			{
				impl::regression_tree& regressor = tree[k];
				regressor.fromFile(pfile);
			}
		}
		// read anchor_idx;
		anchor_idx.resize(cascade_depth);
		for (int i = 0; i < cascade_depth; i++)
		{
			std::vector<int>& anchors = anchor_idx[i];
			anchors.resize(feature_pool_size);
			for (int k = 0; k < feature_pool_size; k++)
			{
				int& idx = anchors[k];
				pfile->read(&idx, sizeof(idx), 1);
			}
		}
		// read deltas
		deltas.resize(cascade_depth);
		for (int i = 0; i < cascade_depth; i++)
		{
			std::vector<float>& points = deltas[i];
			points.resize(feature_pool_size);
			for (int k = 0; k < feature_pool_size; k++)
			{
				float& p = points[k];
				pfile->read(&p, sizeof(p), 1);
			}
		}
		return true;
	}
	bool strip_predictor::fromFile(const char* szfilename)
	{
		XFileDisk xfile;
		if (!xfile.open(szfilename, "rb"))
			return false;
		return fromFile(&xfile);
	}
	bool strip_predictor::toFile(const char* szfilename) const
	{
		XFileDisk xfile;
		if (!xfile.open(szfilename, "wb"))
			return false;
		return toFile(&xfile);
	}

	int strip_predictor::num_parts() const
	{
		return initial_shape.length();
	}

	int strip_predictor::num_features() const
	{
		int num = 0;
		for (unsigned int iter = 0; iter < forests.size(); ++iter)
			for (unsigned int i = 0; i < forests[iter].size(); ++i)
				num += forests[iter][i].num_leaves();
		return num;
	}
	Vecf strip_predictor::getTemplate() const
	{
		return initial_shape;
	}
	full_strip_detection strip_predictor::detect(const Vecf& vsignal, const Range& rect) const
	{
		using namespace impl;
		Vecf current_shape = initial_shape;
		std::vector<float> feature_pixel_values;
		for (unsigned int iter = 0; iter < forests.size(); ++iter)
		{
			extract_feature_strip_values(vsignal, rect, current_shape, initial_shape,
				anchor_idx[iter], deltas[iter], feature_pixel_values);
			int leaf_idx;
			// evaluate all the trees at this level of the cascade.
			for (unsigned int i = 0; i < forests[iter].size(); ++i)
			{
				current_shape += forests[iter][i](feature_pixel_values, leaf_idx);
			}
		}

		// convert the current_shape into a full_object_detection
		const strip_transform_affine tform_to_img = unnormalizing_tform(rect);
		std::vector<int> parts(current_shape.length());
		for (unsigned int i = 0; i < parts.size(); ++i)
		{
			float pt = location(current_shape, i);
			float pt2 = tform_to_img(pt);
			parts[i] = (int)pt2;
		}
		return full_strip_detection(rect, parts);
	}

}
