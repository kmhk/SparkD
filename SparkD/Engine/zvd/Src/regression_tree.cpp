#include "regression_tree.h"

namespace cvlib {

	namespace impl {

		bool regression_tree::toFile(XFile* pfile) const
		{
			int count2 = (int)splits.size();
			pfile->write(&count2, sizeof(count2), 1);
			for (int n = 0; n < count2; n++)
			{
				const impl::split_feature& feature = splits[n];
				pfile->write(&feature, sizeof(feature), 1);
				//regressor.splits.push_back(feature);
			}
			int leafcount = (int)leaf_values.size();
			pfile->write(&leafcount, sizeof(leafcount), 1);
			for (int n = 0; n < leafcount; n++)
			{
				const Vecf& v = leaf_values[n];
				v.toFile(pfile);
			}
			return true;
		}
		bool regression_tree::fromFile(XFile* pfile)
		{
			int count2;
			pfile->read(&count2, sizeof(count2), 1);
			splits.resize(count2);
			for (int n = 0; n < count2; n++)
			{
				impl::split_feature& feature = splits[n];
				pfile->read(&feature, sizeof(feature), 1);
				//regressor.splits.push_back(feature);
			}
			int leafcount;
			pfile->read(&leafcount, sizeof(leafcount), 1);
			leaf_values.resize(leafcount);
			for (int n = 0; n < leafcount; n++)
			{
				Vecf& v = leaf_values[n];
				v.fromFile(pfile);
			}
			return true;
		}
		// ------------------------------------------------------------------------------------

	}

}