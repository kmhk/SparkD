#pragma once

#include "cvlibbase/Inc/Mat.h"
#include "cvlibbase/Inc/XFile.h"
#include "full_strip_detection.h"
#include "strip_predictor_impl.h"

namespace cvlib
{

	class CVLIB_DECLSPEC strip_predictor
	{
	public:

		strip_predictor();

		strip_predictor(
			const Vecf& initial_shape,
			const std::vector<std::vector<impl::regression_tree> >& forests,
			const std::vector<std::vector<float> >& pixel_coordinates
		);

		int num_parts() const;

		int num_features() const;

		full_strip_detection detect(const Vecf& vsignal, const Range& rect) const;

		bool fromFile(XFile* pfile);
		bool toFile(XFile* pfile) const;
		bool fromFile(const char* szfilename);
		bool toFile(const char* szfilename) const;

		Vecf getTemplate() const;
	protected:
		Vecf initial_shape;
		std::vector<std::vector<impl::regression_tree> > forests;
		std::vector<std::vector<int> > anchor_idx;
		std::vector<std::vector<float> > deltas;
	};

}