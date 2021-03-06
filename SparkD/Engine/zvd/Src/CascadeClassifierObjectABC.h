/*
 *  cvlib - Computer Vision Library
 *  All rights reserved.
 */

/*!
 * \file	CascadeClassifierObjectABC.h
 * \brief	
 * \author	
 */
#pragma once
#include "RealStumpClassifier.h"
#include "RealStageClassifier.h"
#include "TrainingData.h"

namespace cvlib
{

	//////////////////////////////////////////////////////////////////////////
	class CVLIB_DECLSPEC FeaElementABC
	{
	public:
		FeaElementABC() {}
		virtual ~FeaElementABC() {}

		virtual int fromFile(XFile* pFile) = 0;
		virtual int toFile(XFile* pFile) const = 0;

		virtual FeaElementABC* clone() const = 0;

		// to be used for object evaluation
		virtual float eval(const void* arg) const = 0;
		virtual float eval(TrainingData* pTrainingData, int nIdx) const = 0;

		// to be used comparison between two objects
		virtual float dist(const void* arg1, const void* arg2) const = 0;
		virtual float extract(const void* arg) = 0;
	};


	//////////////////////////////////////////////////////////////////////////
	class CVLIB_DECLSPEC StumpClassifierObjectDetectABC : public RealStumpClassifier
	{
	public:
		StumpClassifierObjectDetectABC();
		virtual ~StumpClassifierObjectDetectABC();

		virtual void	release();

		virtual int		fromFile(XFile* pFile);
		virtual int		toFile(XFile* pFile) const;

		float			eval(Vec* pFea) { return RealStumpClassifier::eval(pFea); }
		float			eval(float rVal);
		virtual float	eval(const void* arg) const;
		virtual float	eval(TrainingData* pTrainingData, int nIdx) const;

		virtual void	setFeaElement(FeaElementABC* pFea) { m_pFeaElem = pFea->clone(); }
		virtual float	extract(const void* arg) { return m_pFeaElem->extract(arg); }

		virtual void	createFeaElem() = 0;
		FeaElementABC*	getFeaElement() const { return m_pFeaElem; }
	protected:
		FeaElementABC* m_pFeaElem;
	};

	class CVLIB_DECLSPEC StageClassifierObjectDetectABC : public RealStageClassifier
	{
	public:
		float	m_rQ;
		int		m_nObjectWidth;
		int		m_nObjectHeight;

		StageClassifierObjectDetectABC();

		virtual int		fromFile(XFile* pFile);
		virtual int		toFile(XFile* pFile) const;

		virtual float	eval(Vec* pFea);
		float			eval(float rVal);
		virtual float	eval(const void* arg);
		virtual float	eval(TrainingData* pTrainingData, int nIdx);
		virtual float	eval(TrainingData* pTrainingData, int nIdx, int nX, int nY) = 0;
		virtual float	eval(const void* arg, int nX, int nY) = 0;
		virtual	void	fastProcessing() {}

		virtual StumpClassifierObjectDetectABC* newStumpClassifier() const = 0;
	};

	class CVLIB_DECLSPEC CascadeClassifierObjectDetectABC : public ClassifierSet
	{
	public:
		CascadeClassifierObjectDetectABC();
		virtual ~CascadeClassifierObjectDetectABC();

		virtual int		fromFile(const char* szFilename);
		virtual int		toFile(const char* szFilename) const;
		virtual int		fromFile(XFile* pFile);
		virtual int		toFile(XFile* pFile) const;

		virtual int		loadFromFolder(const char* szFilename);
		virtual int		saveToFolder(const char* szFilename) const;

		virtual int		toC(const char* szFilename) const { return 0; }

		virtual int		eval(const void* arg, int nX, int nY, float* prConf = NULL, int* pnLayer = NULL);
		virtual float	eval(const void* arg) const;
		virtual int		eval(TrainingData* pTrainingData, int nIdx, int nX, int nY, float* prConf = NULL, int* pnLayer = NULL);
		virtual float	eval(TrainingData* pTrainingData, int nIdx) const;
		virtual Vec*	extract(const void* arg);
		virtual void	getMinMaxValues(Mat& minmaxs);
		virtual void	fastProcessing();

		float getThreshold(int i);

		virtual StageClassifierObjectDetectABC* newStageClassifier() const = 0;
		int		m_nObjectWidth;
		int		m_nObjectHeight;
	protected:
		int				m_nFeaNum;
		char		m_szStageName[64];
	};


}
