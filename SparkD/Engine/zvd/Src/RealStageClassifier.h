
/*!
 * \file    RealStageClassfiler.h
 * \ingroup	boosting
 * \brief	RealBoosting
 * \author
 */

#pragma once

#include "StumpClassifier.h"

namespace cvlib
{

	class CVLIB_DECLSPEC RealStageClassifier : public ClassifierABC
	{
	public:
		int		m_nCount;
		float	m_rThreshold;
		StumpClassifierABC**	m_ppStumpClassifier;
	public:
		RealStageClassifier();
		virtual ~RealStageClassifier();
		virtual ClassifierABC* New() const { return new RealStageClassifier; }

		void	virtual create(int nNum);
		void	virtual release();
		int		virtual fromFile(XFile* pFile);
		int		virtual toFile(XFile* pFile) const;
		float	eval(Vec* pvFea);
	};

}