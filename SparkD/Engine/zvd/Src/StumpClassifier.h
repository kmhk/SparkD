
/*!
 * \file    StumpClassifer.h
 * \ingroup	boosting
 * \brief	Stump
 * \author	
 */

#pragma once

#include "ClassifierABC.h"

namespace cvlib
{

	class CVLIB_DECLSPEC StumpClassifierABC : public ClassifierABC
	{
	public:
		int		m_nCompidx;

		StumpClassifierABC() { m_nCompidx = -1; }
		virtual ~StumpClassifierABC() {}
		virtual ClassifierABC* New() const { return 0; }
		virtual int		fromFile(XFile* fp);
		virtual int		toFile(XFile* fp) const;
		virtual float	eval(Vec* pvFea) = 0;
		virtual float	eval(float r) = 0;
	};

	/**
	 * @brief  Stump
	 */
	class CVLIB_DECLSPEC StumpClassifier : public StumpClassifierABC
	{
	public:
		StumpClassifier();
		virtual ~StumpClassifier();
		virtual ClassifierABC* New() const { return new StumpClassifier; }

		virtual int		fromFile(XFile* fp);
		virtual int		toFile(XFile* fp) const;

		virtual float	eval(Vec* pvFea);
		virtual float	eval(float r);

	public:
		float	m_rAlpha;
		float	m_rThreshold;
	};

}