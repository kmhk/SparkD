#include "CascadeClassifierObjectABC.h"
#include "cvlibbase/Inc/cvlibutil.h"
#include "cvlibbase/Inc/XFileDisk.h"

namespace cvlib
{

#define CVLIB_EPSILON 0.000001f

	//////////////////////////////////////////////////////////////////////////
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////

	StumpClassifierObjectDetectABC::StumpClassifierObjectDetectABC()
	{
		strcpy(m_szVersion, "");
		m_pFeaElem = NULL;
	}

	StumpClassifierObjectDetectABC::~StumpClassifierObjectDetectABC()
	{
		release();
	}

	void StumpClassifierObjectDetectABC::release()
	{
		RealStumpClassifier::release();
		if (m_pFeaElem)
		{
			delete m_pFeaElem;
			m_pFeaElem = NULL;
		}
	}

	int StumpClassifierObjectDetectABC::fromFile(XFile* pFile)
	{
		RealStumpClassifier::fromFile(pFile);
		createFeaElem();
		m_pFeaElem->fromFile(pFile);
		return 1;
	}

	int StumpClassifierObjectDetectABC::toFile(XFile* pFile) const
	{
		RealStumpClassifier::toFile(pFile);
		assert(m_pFeaElem);
		m_pFeaElem->toFile(pFile);
		return 1;
	}

	float	StumpClassifierObjectDetectABC::eval(float rVal)
	{
		return m_prValue[whichBin(rVal)];
	}

	float	StumpClassifierObjectDetectABC::eval(const void* arg) const
	{
		return m_prValue[whichBin(m_pFeaElem->eval(arg))];
	}

	float	StumpClassifierObjectDetectABC::eval(TrainingData* pTrainingData, int nIdx) const
	{
		return m_prValue[whichBin(m_pFeaElem->eval(pTrainingData, nIdx))];
	}

	//////////////////////////////////////////////////////////////////////////
	StageClassifierObjectDetectABC::StageClassifierObjectDetectABC()
	{
		strcpy(m_szVersion, "frstage");
		m_rQ = 0.0f;
		m_nObjectWidth = m_nObjectHeight = 0;
	}

	int	StageClassifierObjectDetectABC::fromFile(XFile* pFile)
	{
		int ii;
		ClassifierABC::fromFile(pFile);

		pFile->read(&m_nObjectWidth, sizeof(m_nObjectWidth), 1);
		pFile->read(&m_nObjectHeight, sizeof(m_nObjectHeight), 1);
		pFile->read(&m_nCount, sizeof(m_nCount), 1);
		pFile->read(&m_rThreshold, sizeof(m_rThreshold), 1);
		pFile->read(&m_rQ, sizeof(m_rQ), 1);

		create(m_nCount);

		for (ii = 0; ii < m_nCount; ii++)
		{
			StumpClassifierObjectDetectABC* pClassifier = newStumpClassifier();
			pClassifier->fromFile(pFile);
			m_ppStumpClassifier[ii] = pClassifier;
		}
		return 1;
	}

	int	StageClassifierObjectDetectABC::toFile(XFile* pFile) const
	{
		int ii;

		ClassifierABC::toFile(pFile);

		pFile->write(&m_nObjectWidth, sizeof(m_nObjectWidth), 1);
		pFile->write(&m_nObjectHeight, sizeof(m_nObjectHeight), 1);
		pFile->write(&m_nCount, sizeof(m_nCount), 1);
		pFile->write(&m_rThreshold, sizeof(m_rThreshold), 1);
		pFile->write(&m_rQ, sizeof(m_rQ), 1);

		for (ii = 0; ii < m_nCount; ii++)
		{
			StumpClassifierObjectDetectABC* pClassifier = (StumpClassifierObjectDetectABC*)m_ppStumpClassifier[ii];
			pClassifier->toFile(pFile);
		}
		return 1;
	}

	float StageClassifierObjectDetectABC::eval(Vec* pFea)
	{
		float rEval = 0.0f;
		for (int ii = 0; ii < m_nCount; ii++)
		{
			StumpClassifierObjectDetectABC* pClassifier = (StumpClassifierObjectDetectABC*)m_ppStumpClassifier[ii];
			rEval += pClassifier->eval(pFea);
		}
		return rEval;
	}

	float StageClassifierObjectDetectABC::eval(float rVal)
	{
		float rEval = 0.0f;
		for (int i = 0; i < m_nCount; i++)
		{
			StumpClassifierObjectDetectABC* pClassifier = (StumpClassifierObjectDetectABC*)m_ppStumpClassifier[i];
			rEval += pClassifier->eval(rVal);
		}
		return rEval;
	}

	float StageClassifierObjectDetectABC::eval(const void* arg)
	{
		float rEval = 0.0f;
		for (int i = 0; i < m_nCount; i++)
		{
			StumpClassifierObjectDetectABC* pClassifier = (StumpClassifierObjectDetectABC*)m_ppStumpClassifier[i];
			rEval += pClassifier->eval(arg);
		}
		return rEval;
	}

	float StageClassifierObjectDetectABC::eval(TrainingData* pTrainingData, int nIdx)
	{
		float rEval = 0.0f;
		for (int i = 0; i < m_nCount; i++)
		{
			StumpClassifierObjectDetectABC* pClassifier = (StumpClassifierObjectDetectABC*)m_ppStumpClassifier[i];
			rEval += pClassifier->eval(pTrainingData, nIdx);
		}
		return rEval;
	}

	//////////////////////////////////////////////////////////////////////////
	CascadeClassifierObjectDetectABC::CascadeClassifierObjectDetectABC()
	{
		m_nFeaNum = 0;
	}

	CascadeClassifierObjectDetectABC::~CascadeClassifierObjectDetectABC()
	{
		release();
		m_nFeaNum = 0;
	}

	int		CascadeClassifierObjectDetectABC::loadFromFolder(const char* szFilename)
	{
		int i;
		char szCascaderoot[500];
		char* pchSuffix;
		XFileDisk xfile;
		int nNum;

		m_nFeaNum = 0;
		strcpy(szCascaderoot, szFilename);
		pchSuffix = szCascaderoot + strlen(szCascaderoot);
		for (i = 0; ; i++)
		{
			sprintf(pchSuffix, "%d/%s", i, "stageclassifier.txt");
			if (!xfile.open(szCascaderoot, "rb")) {
				break;
			}
			xfile.close();
		}
		nNum = i;
		if (nNum < 1)
			return 1;
		for (i = 0; i < nNum; i++)
		{
			sprintf(pchSuffix, "%d/%s", i, "stageclassifier.txt");
			if (xfile.open(szCascaderoot, "rb"))
			{
				StageClassifierObjectDetectABC* pStage = newStageClassifier();
				pStage->fromFile(&xfile);
				m_nFeaNum += pStage->m_nCount;
				xfile.close();
				add(pStage);
				m_nObjectWidth = pStage->m_nObjectWidth;
				m_nObjectHeight = pStage->m_nObjectHeight;
			}
		}
		m_nNum = nNum;
		return 1;
	}

	int		CascadeClassifierObjectDetectABC::saveToFolder(const char* szFilename) const
	{
		int i;
		char szCascaderoot[500];
		char* pchSuffix;
		FILE* pFile;

		strcpy(szCascaderoot, szFilename);
		pchSuffix = szCascaderoot + strlen(szCascaderoot);

		for (i = 0; i < getActiveSize(); i++)
		{
			sprintf(pchSuffix, "%d/%s", i, "stageclassifier.txt");
			cvutil::mkDir(szCascaderoot);
			pFile = fopen(szCascaderoot, "wb");
			if (pFile)
			{
				XFileDisk file(pFile);
				getAt(i)->toFile(&file);
				fclose(pFile);
			}
		}
		return 1;
	}

	float	CascadeClassifierObjectDetectABC::eval(const void* arg) const
	{
		int i;

		float rRet = 0.0f, rNestingStumpVal = 0.0f;
		for (i = 0; i < getActiveSize(); i++)
		{
			StageClassifierObjectDetectABC* pStageClassifier = (StageClassifierObjectDetectABC*)getAt(i);
			float rStgCal = pStageClassifier->eval(arg);
			rNestingStumpVal += rStgCal;
			if (rNestingStumpVal < pStageClassifier->m_rThreshold - CVLIB_EPSILON)
				break;
		}
		if (i != getActiveSize())
			return 0.0f;
		return 1.0f;
	}
	float	CascadeClassifierObjectDetectABC::eval(TrainingData* pTrainingData, int nIdx) const
	{
		int i;

		float rRet = 0.0f, rNestingStumpVal = 0.0f;
		for (i = 0; i < getActiveSize(); i++)
		{
			StageClassifierObjectDetectABC* pStageClassifier = (StageClassifierObjectDetectABC*)getAt(i);
			float rStgCal = pStageClassifier->eval(pTrainingData, nIdx);
			rNestingStumpVal += rStgCal;
			if (rNestingStumpVal < pStageClassifier->m_rThreshold - CVLIB_EPSILON)
				break;
		}
		if (i != getActiveSize())
			return 0.0f;
		return 1.0f;
	}

	int		CascadeClassifierObjectDetectABC::eval(const void* arg, int nX, int nY, float* prConf/*=NULL*/, int* pnLayer/*=NULL*/)
	{
		int i;
		float rRet = 0.0f, rNestingStumpVal = 0.0f;
		for (i = 0; i < getActiveSize(); i++)
		{
			StageClassifierObjectDetectABC* pStageClassifier = (StageClassifierObjectDetectABC*)getAt(i);
			float rStgCal = pStageClassifier->eval(arg, nX, nY);
			rNestingStumpVal += rStgCal;
			if (rNestingStumpVal < pStageClassifier->m_rThreshold - CVLIB_EPSILON)
				break;
		}
		if (prConf)
			*prConf = rNestingStumpVal;

		if (pnLayer)
			*pnLayer = i;

		if (i != getActiveSize())
			return 0;
		return 1;
	}
	int		CascadeClassifierObjectDetectABC::eval(TrainingData* pTrainingData, int nIdx, int nX, int nY, float* prConf/*=NULL*/, int* pnLayer/*=NULL*/)
	{
		int i;
		float rRet = 0.0f, rNestingStumpVal = 0.0f;
		for (i = 0; i < getActiveSize(); i++)
		{
			StageClassifierObjectDetectABC* pStageClassifier = (StageClassifierObjectDetectABC*)getAt(i);
			float rStgCal = pStageClassifier->eval(pTrainingData, nIdx, nX, nY);
			rNestingStumpVal += rStgCal;
			if (rNestingStumpVal < pStageClassifier->m_rThreshold - CVLIB_EPSILON)
				break;
		}
		if (prConf)
			*prConf = rNestingStumpVal;

		if (pnLayer)
			*pnLayer = i;

		if (i != getActiveSize())
			return 0;
		return 1;
	}

	Vec*	CascadeClassifierObjectDetectABC::extract(const void* arg)
	{
		return 0;
	}

	void CascadeClassifierObjectDetectABC::fastProcessing()
	{
		for (int i = 0; i < getActiveSize(); i++)
		{
			StageClassifierObjectDetectABC* pStageClassifier = (StageClassifierObjectDetectABC*)getAt(i);
			pStageClassifier->fastProcessing();
		}
	}

	float CascadeClassifierObjectDetectABC::getThreshold(int i)
	{
		StageClassifierObjectDetectABC* pStageClassifier = (StageClassifierObjectDetectABC*)getAt(i);
		return pStageClassifier->m_rThreshold;
	}

	//////////////////////////////////////////////////////////////////////////
	int	CascadeClassifierObjectDetectABC::fromFile(const char* szFilename)
	{
		XFileDisk file;
		bool fFlag = file.open(szFilename, "rb");
		if (!fFlag)
			return 0;
		return fromFile(&file);
	}

	int	CascadeClassifierObjectDetectABC::toFile(const char* szFilename) const
	{
		XFileDisk file;
		bool fFlag = file.open(szFilename, "wb");
		if (!fFlag)
			return 0;
		return toFile(&file);
	}

	int	CascadeClassifierObjectDetectABC::fromFile(XFile* pFile)
	{
		int nNum;
		pFile->read(&m_nObjectWidth, sizeof(m_nObjectWidth), 1);
		pFile->read(&m_nObjectHeight, sizeof(m_nObjectHeight), 1);
		pFile->read(&nNum, sizeof(nNum), 1);

		for (int i = 0; i < nNum; i++)
		{
			StageClassifierObjectDetectABC* pStage = newStageClassifier();
			pStage->fromFile(pFile);
			m_nFeaNum += pStage->m_nCount;
			add(pStage);
		}
		return 1;
	}

	int	CascadeClassifierObjectDetectABC::toFile(XFile* pFile) const
	{
		pFile->write(&m_nObjectWidth, sizeof(m_nObjectWidth), 1);
		pFile->write(&m_nObjectHeight, sizeof(m_nObjectHeight), 1);
		pFile->write(&m_nNum, sizeof(m_nNum), 1);
		for (int i = 0; i < m_nNum; i++)
			getAt(i)->toFile(pFile);
		return 1;
	}
	void	CascadeClassifierObjectDetectABC::getMinMaxValues(Mat& minmaxs)
	{
		int stump_count = 0;
		for (int i = 0; i < getActiveSize(); i++)
		{
			StageClassifierObjectDetectABC* pStageClassifier = (StageClassifierObjectDetectABC*)getAt(i);
			stump_count += pStageClassifier->m_nCount;
		}
		minmaxs.create(stump_count, 2, MAT_Tfloat);
		int idx = 0;
		for (int i = 0; i < getActiveSize(); i++)
		{
			StageClassifierObjectDetectABC* pStageClassifier = (StageClassifierObjectDetectABC*)getAt(i);
			for (int k = 0; k < pStageClassifier->m_nCount; k++)
			{
				RealStumpClassifier* pstump = (RealStumpClassifier*)pStageClassifier->m_ppStumpClassifier[k];
				minmaxs.data.fl[idx][0] = pstump->m_rMin;
				minmaxs.data.fl[idx][1] = pstump->m_rMax;
				idx++;
			}
		}
	}


}
