#include "HaarTrainingData.h"
#include <assert.h>

namespace cvlib {

	HaarTrainingData::HaarTrainingData()
	{
		m_winsize = Size(0, 0);
	}

	HaarTrainingData::~HaarTrainingData()
	{
	}

	bool HaarTrainingData::create(int nMaxnumsamples, int nFeaturelen, int nMaxMemory, TYPE type /*= MAT_Tfloat*/)
	{
		if (!TrainingData::create(nMaxnumsamples, nFeaturelen, nMaxMemory, type))
			return false;
		if (m_mTilted.create(nMaxnumsamples, nFeaturelen, MAT_Tint) == 0)
			return false;
		if (m_vNormfactor.create(nMaxnumsamples, MAT_Tfloat) == 0)
			return false;
		return true;
	}

	void	HaarTrainingData::release()
	{
		TrainingData::release();
		m_mTilted.release();
		m_vNormfactor.release();
	}

	void	HaarTrainingData::GetTiltedFeature(int nIdx, Vec* pvData)
	{
		pvData->create(m_mTilted.data.i[nIdx], m_nDim, MAT_Tint);
	}

}