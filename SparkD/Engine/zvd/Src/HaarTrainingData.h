#pragma once

#include "TrainingData.h"

namespace cvlib {

	class CVLIB_DECLSPEC HaarTrainingData : public TrainingData
	{
	public:
		HaarTrainingData();
		~HaarTrainingData();

		bool	virtual create(int nMaxnumsamples, int nFeaturelen, int nMaxMemory, TYPE type = MAT_Tfloat);
		void	virtual release();

		void	GetTiltedFeature(int nIdx, Vec* pvData);

	public:
		Size	m_winsize;
		Mat		m_mTilted;
		Vec		m_vNormfactor;
	};

}