/*
 *  cvlib - Computer Vision Library
 *  Copyright 2006-2017 by  Information Center
 *  All rights reserved.

 */

/*!
 * \file ObjectCascadeClassifierHaar.h
 * \brief Haar, boost
 * \author 
 */

#pragma once

#include "CascadeClassifierObjectABC.h"
#include "HaarFeature.h"

namespace cvlib
{

	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////

	class CVLIB_DECLSPEC HaarElem : public FeaElementABC
	{
	public:
		struct SHaarBasisData
		{
			int* pnSum;
			int* pnTilted;
			float rNormFactor;
		};
	public:
		STHaarFeature		m_feature;
		SFastHaarFeature	m_fastfeature;

		HaarElem();
		HaarElem(const HaarElem& other);
		HaarElem(Rect& rect, int& nCode, int& nRadius);
		virtual ~HaarElem();

		virtual int fromFile(XFile* pFile);
		virtual int toFile(XFile* pFile) const;

		virtual FeaElementABC* clone() const;

		virtual float eval(const void* arg) const;
		virtual float eval(TrainingData* pTrainingData, int nIdx) const;
		virtual float dist(const void* arg1, const void* arg2) const;
		virtual float extract(const void* arg);
	};

	class CVLIB_DECLSPEC CascadeClassifierObjectHaar : public CascadeClassifierObjectDetectABC
	{
	public:
		class CVLIB_DECLSPEC Stump : public StumpClassifierObjectDetectABC
		{
		public:
			Stump() { strcpy(m_szVersion, "ObjectHaarStump"); }
			virtual void	createFeaElem() { m_pFeaElem = (FeaElementABC*)(new HaarElem); }
		};

		class CVLIB_DECLSPEC Stage : public StageClassifierObjectDetectABC
		{
		public:
			Stage() { strcpy(m_szVersion, "ObjectHaarStage"); }
			virtual float	eval(const void* arg, int nX, int nY);
			virtual float	eval(TrainingData* pTrainingData, int nIdx, int nX, int nY);
			virtual void	fastProcessing();
			virtual void	release();
			virtual StumpClassifierObjectDetectABC* newStumpClassifier() const { return new Stump; }

			void	setStep(int nStep);
		public:
			float**		m_pprValues;
		protected:
			int		m_nStep;

			int		m_prevx, m_prevy;
			//SFastHaarFeature	m_prevFastFeature;
		};
	public:
		virtual int		toC(const char* szFilename) const;
		virtual StageClassifierObjectDetectABC* newStageClassifier() const { return new Stage; }
		void	setStep(int nStep);
	};

}