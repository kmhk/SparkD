/*
 *  cvlib - Computer Vision Library
 *  All rights reserved.
 */

/*!
 * \file	FeatureVec.h
 * \brief	
 * \author	
 */
#pragma once

#include "FeatureABC.h"
#include "cvlibbase/Inc/Mat.h"

namespace cvlib
{

#define FEATURE_VEC "gray"

	class CVLIB_DECLSPEC FeatureVec : public FeatureABC, public Vec
	{
	public:
		FeatureVec();
		virtual ~FeatureVec();

		virtual FeatureABC* clone() const;
		virtual int read(XFile* pFile);
		virtual int write(XFile* pFile)const;
		virtual void release();

		virtual float dist(const FeatureABC* pFeature) const;
		virtual int dimension() const;
		virtual void getData(Vec* pv1);

		static FeatureVec* createFromID(const char* szID);

		void calcIntegralImage(bool fSquare = 0, bool fTilted = 0);

		Size m_size;
		Mati* m_pmSum;
		Mati* m_pmTilted;
		Matd* m_pmSquare;
	};

}