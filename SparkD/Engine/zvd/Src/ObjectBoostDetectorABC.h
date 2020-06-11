/*
 *  Computer Vision Library
 *  Copyright 2006-2011 by  Information Center
 *  All rights reserved.
 */

 /*!
 * \file
 * \brief	
 * \author 
 */ 

#pragma once

#include "cvlibbase/Inc/_cvlibbase.h"
#include "TrainingData.h"
#include "CascadeClassifierObjectABC.h"
#include "objdetect_utils.h"

namespace cvlib
{

	class CVLIB_DECLSPEC RectVal : public Rect
	{
	public:
		float rval;
		int nFlag;
		RectVal() { rval = 0.0f; nFlag = 0; }
		RectVal(const RectVal& t) : Rect(t) { rval = t.rval; nFlag = t.nFlag; }
		RectVal& operator=(const RectVal& t) { 
			x = t.x; y = t.y; width = t.width; height = t.height;
			rval = t.rval; nFlag = t.nFlag; return *this; 
		}
	};

	class CVLIB_DECLSPEC ObjectBoostDetectorABC
	{
		friend class ObjectBoostTrainerABC;
	public:
		class CVLIB_DECLSPEC Param
		{
		public:
			Param();
			virtual ~Param() {}
			virtual void copy(ObjectBoostDetectorABC::Param* pParam);

			float	rStepX;
			float	rStepY;
			float	rScale;
			int		nMinSize;
			int		nMaxSize;
			int     nAutoSave;
			int		nOverlap; // 1 : remove overlap, 0 : mark overlapped regions 2 : clustering
			int		nExportSamFeature;
			int		nOnlyOne;
			char	szClassifierPath[CVLIB_PATH_MAX];
		};
		Param		m_Param;
		int			m_nScanedNum;
		int			m_nTrainIdx;

		ObjectBoostDetectorABC();
		~ObjectBoostDetectorABC();

		virtual int create(ObjectBoostDetectorABC::Param* pBParam);
		virtual int create(CascadeClassifierObjectDetectABC* pCascadeClassifier);
		virtual int	createFromOneFile(const char* szDataPath);
		virtual void release();
		virtual int flagCreate() { return m_fCreate; }

		virtual int	extract(const Mat& image, PtrList& blobList);
		int	extract(const Mat& image, const Mat& mask, PtrList& blobList, Mat* heatmap = 0);
		int	extractHeatmap(const Mat& image, Mat* heatmap = 0, Mat* heatmapSize = 0);
		int	extractCameraMode(const Mat& image, const Mat& mask, PtrList& blobList, Mat* heatmap = 0);
		int extract(const Mat& image, Vector<Rect>& regions);
		void evaluate(const Mat& image, int& pass_count, int& total_count);

		virtual void freePtrList(PtrList& blobList);
		CascadeClassifierObjectDetectABC*	getCascadeClassifier() { return m_pCascadeClassifier; }

		void setExportParam(TrainingData* pTrainingData, int nTrainIdx, int nLimIdx) { m_pTrainingData = pTrainingData, m_nTrainIdx = nTrainIdx, m_nLimIdx = nLimIdx; }
		virtual const void* extractLayerFeature(const Mat& image) = 0;

		//static int compareRegion(const void* pResult1, const void* pResult2);
	protected:
		virtual CascadeClassifierObjectDetectABC* newCascadeClassifierFromFolder(const char* szFilename) = 0;
		virtual CascadeClassifierObjectDetectABC* newCascadeClassifierFromOneFile(const char* szFilename);
		virtual CascadeClassifierObjectDetectABC* newCascadeClassifierFromOneFile(XFile* szFilename) = 0;
		virtual const void* updateFeature(int nX, int nY, const Mat& image) = 0;
		virtual int exportSampleFeature(int nX, int nY) { return 0; }
		virtual void exportFeature() {}

	protected:
		void	saveNonobject(const Mat& image, const Rect& rect, int& nFirst, int& nTotalNum);
		bool	isIncludeRect(const RectVal& r, int nWidth, int nHeight);
		void	removeOverlappedRegion(PtrArray* pDetectedRegion);
		void	markOverlappedRegion(PtrArray* pDetectedRegion);
		void	sortRegions(PtrArray& regions) const;


	protected:
		CascadeClassifierObjectDetectABC*	m_pCascadeClassifier;
		int m_fCreate;
		int	m_nObjectHeight;
		int	m_nObjectWidth;
		const char* m_szFilename;

		//! data for training
		TrainingData*		m_pTrainingData;
		int					m_nLimIdx;

	private:
		// variables on camera mode
		objutils::ImagePyramid		m_pyramidOfImage;
		objutils::ImagePyramid		m_pyramidOfMask;
	};

}
