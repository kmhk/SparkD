#include "TrainingData.h"
#include "cvlibbase/Inc/_cvlibbase.h"
#include <assert.h>

namespace cvlib
{

	TrainingData::TrainingData()
	{
		m_pvalcache = NULL;
		m_pidxcache = NULL;
		m_nMaxnum = 0;
		m_nPosCount = m_nNegCount = 0;
		m_nDim = 0;
	}

	TrainingData::~TrainingData()
	{
		release();
	}

	bool TrainingData::create(int nMaxnumsamples, int nFeaturelen, int nMaxMemory, TYPE type/*= MAT_Tfloat*/)
	{
		int		nSizeelem = 1;
		switch (type)
		{
		case MAT_Tfloat:
		case MAT_Tint:
			nSizeelem = 4;
			break;
		case MAT_Tuchar:
			nSizeelem = 1;
			break;
		}
		int nNum = nMaxMemory * 1024 * 1024 / (nSizeelem * nMaxnumsamples);
		if (nFeaturelen < nNum)
			nNum = nFeaturelen;

		m_nDim = nNum;
		if (m_mFeature.create(nMaxnumsamples, nFeaturelen, type) == 0)
			return false;

		m_nMaxnum = nMaxnumsamples;

		if (m_vClass.create(nMaxnumsamples, MAT_Tfloat) == 0)
			return false;
		if (m_vWeights.create(nMaxnumsamples, MAT_Tdouble) == 0)
			return false;

		m_pvalcache = NULL;
		m_pidxcache = NULL;

		return true;
	}

	void	TrainingData::releaseCache()
	{
		if (m_pvalcache != NULL)
		{
			m_pvalcache->release();
			delete m_pvalcache;
			m_pvalcache = NULL;
		}
		if (m_pidxcache != NULL)
		{
			m_pidxcache->release();
			delete m_pidxcache;
			m_pidxcache = NULL;
		}
	}

	void	TrainingData::release()
	{
		m_mFeature.release();
		m_vClass.release();
		m_vWeights.release();

		releaseCache();
	}

	void	TrainingData::setNumSamples(int nNum)
	{
		m_mFeature.resizeRows(nNum);
		m_vClass.resize(nNum);
		m_vWeights.resize(nNum);
	}

	void	TrainingData::setWeightsAndClasses(
		int nNum1, float rWeight1, float rCls1,
		int nNum2, float rWeight2, float rCls2)
	{
		int j;

		assert(nNum1 + nNum2 <= m_nMaxnum);

		for (j = 0; j < nNum1; j++)
		{
			m_vWeights.data.db[j] = rWeight1;
			m_vClass.data.fl[j] = rCls1;
		}
		for (j = nNum1; j < nNum1 + nNum2; j++)
		{
			m_vWeights.data.db[j] = rWeight2;
			m_vClass.data.fl[j] = rCls2;
		}
	}

	void	TrainingData::setSampleFeature(int nIdx, Vec* pvFea)
	{
		assert(m_nMaxnum >= nIdx);
		uchar* pbDest = m_mFeature.data.ptr[nIdx];
		uchar* pbSrc = pvFea->data.ptr;
		memcpy(pbDest, pbSrc, m_mFeature.step() * m_mFeature.cols());
	}

	void	TrainingData::getSample(int idx, Vec* pvSample)
	{
		pvSample->create(m_mFeature.data.ptr[idx], m_nDim, m_mFeature.type());
	}

	int	TrainingData::fromFile(const char* szFilename)
	{
		XFileDisk xfile;
		if (!xfile.open(szFilename, "rb"))
			return 0;
		return fromFile(&xfile);
	}

	int	TrainingData::toFile(const char* szFilename) const
	{
		XFileDisk xfile;
		if (!xfile.open(szFilename, "wb"))
			return 0;
		return toFile(&xfile);
	}

	int	TrainingData::fromFile(XFile* pFile)
	{
		try
		{
			pFile->read(&m_nDim, sizeof(m_nDim), 1);
			pFile->read(&m_nPosCount, sizeof(m_nPosCount), 1);
			pFile->read(&m_nNegCount, sizeof(m_nNegCount), 1);
			pFile->read(&m_nMaxnum, sizeof(m_nMaxnum), 1);
			m_mFeature.fromFile(pFile);
			m_vClass.fromFile(pFile);
			m_vWeights.fromFile(pFile);
		}
		catch (Exception e)
		{
			return 0;
		}
		return 1;
	}

	int	TrainingData::toFile(XFile* pFile) const
	{
		try
		{
			pFile->write(&m_nDim, sizeof(m_nDim), 1);
			pFile->write(&m_nPosCount, sizeof(m_nPosCount), 1);
			pFile->write(&m_nNegCount, sizeof(m_nNegCount), 1);
			pFile->write(&m_nMaxnum, sizeof(m_nMaxnum), 1);
			m_mFeature.toFile(pFile);
			m_vClass.toFile(pFile);
			m_vWeights.toFile(pFile);
		}
		catch (Exception e)
		{
			return 0;
		}
		return 1;
	}

}
