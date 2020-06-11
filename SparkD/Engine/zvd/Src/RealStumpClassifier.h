
/*!
 * \file    RealStumpClassifier.
 * \ingroup	boosting
 * \brief	RealBoosting.
 * \author	
 */

#pragma once

#include "StumpClassifier.h"

namespace cvlib 
{

	/**
	 * @brief
	 */
	class CVLIB_DECLSPEC RealStumpClassifier : public StumpClassifierABC
	{
		friend class RealStumpTrainer;
	public:
		float  m_rZ;
	public:
		RealStumpClassifier();
		virtual ~RealStumpClassifier();

		virtual ClassifierABC* New() const { return new RealStumpClassifier; }

		virtual	void	create(int nNum);
		virtual void	release();

		virtual int		fromFile(XFile* fp);
		virtual int		toFile(XFile* fp) const;

		virtual float	eval(Vec* pvFea);
		virtual float	eval(float r);

		int whichBin(float rVal) const;

	public:
		float*	m_prValue;
		int		m_nBinnum;
		float	m_rStep;
		float	m_rMin;
		float	m_rMax;
	};

}