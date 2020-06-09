
/*!
 * \file    TrainingData.h
 * \ingroup	base
 */

#pragma once

#include "Mat.h"

namespace cvlib {

	class CVLIB_DECLSPEC TrainingData : public Object
	{
	public:
		TrainingData();
		virtual ~TrainingData();
	public:

		bool	virtual create(int nMaxnumsamples, int nFeaturelen, int nMaxMemory, TYPE type = MAT_Tfloat);
		void	virtual release();
		void	releaseCache();

		void	setSampleFeature(int nIdx, Vec* pvFea);

		void	setNumSamples(int nNum);

		void	setWeightsAndClasses(
			int nNum1, float rWeight1, float rCls1,
			int nNum2, float rWeight2, float rCls2);

		void	getSample(int idx, Vec* pvSample);

		virtual int	fromFile(const char* szFilename);
		virtual int	fromFile(XFile* pFile);
		virtual int	toFile(const char* szFilename) const;
		virtual int	toFile(XFile* pFile) const;
	public:
		int		m_nDim;
		int		m_nPosCount, m_nNegCount;
		int		m_nMaxnum;

		Mat		m_mFeature;
		Vec		m_vClass;
		Vec		m_vWeights;

		Mat*	m_pvalcache;
		Mat*	m_pidxcache;
	};

}
