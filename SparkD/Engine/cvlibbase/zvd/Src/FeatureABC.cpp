#include "FeatureABC.h"
#include "FeatureVec.h"
#include <assert.h>

namespace cvlib
{

	FeatureFactory* FeatureFactory::m_pFeatureFactory = NULL;
	bool FeatureABC::write(const char* szfilename) const
	{
		XFileDisk xfile;
		if (!xfile.open(szfilename, "wb"))
			return false;
		write(&xfile);
		return true;
	}

	FeatureSet::FeatureSet()
	{
		m_nNum = 0;
		m_pFeatureArray = NULL;
		strcpy(m_szID, FEATURE_SET);
		assert(strlen(m_szID) < CVLIB_MAX_IDLEN);
	}

	FeatureSet::~FeatureSet()
	{
		release();
	}

	FeatureABC* FeatureSet::clone() const
	{
		FeatureSet* pNew = new FeatureSet;
		for (int i = 0; i < m_nNum; i++)
			pNew->add(m_pFeatureArray[i]->clone());
		return pNew;
	}

	int FeatureSet::add(FeatureABC* pFeature)
	{
		FeatureABC** pFeatures = new FeatureABC*[m_nNum + 1];
		memcpy(pFeatures, m_pFeatureArray, sizeof(FeatureABC*) * m_nNum);
		pFeatures[m_nNum] = pFeature;

		if (m_pFeatureArray)
			delete[]m_pFeatureArray;
		m_pFeatureArray = pFeatures;

		m_nNum++;
		return m_nNum;
	}

	void FeatureSet::removeAt(int nIdx)
	{
		if (nIdx < 0 || nIdx >= m_nNum)
			return;
		if (getSize() == 0)
			return;

		m_pFeatureArray[nIdx]->release();
		delete m_pFeatureArray[nIdx];

		for (int i = nIdx; i < m_nNum - 1; i++)
			m_pFeatureArray[i] = m_pFeatureArray[i + 1];
		m_nNum--;
	}

	const FeatureABC* FeatureSet::getAt(int nIdx) const
	{
		assert(nIdx >= 0 && nIdx < m_nNum);
		return m_pFeatureArray[nIdx];
	}
	FeatureABC* FeatureSet::getAt(int nIdx)
	{
		assert(nIdx >= 0 && nIdx < m_nNum);
		return m_pFeatureArray[nIdx];
	}
	void FeatureSet::setAt(int nIdx, FeatureABC* pfeature)
	{
		assert(nIdx >= 0 && nIdx < m_nNum);
		m_pFeatureArray[nIdx] = pfeature;
	}
	void FeatureSet::release()
	{
		if (m_pFeatureArray)
		{
			for (int i = 0; i < getSize(); i++)
			{
				if (m_pFeatureArray[i])
				{
					m_pFeatureArray[i]->release();
					delete m_pFeatureArray[i];
				}
			}
			delete[]m_pFeatureArray;
			m_pFeatureArray = NULL;
			m_nNum = 0;
		}
	}

	int FeatureSet::read(XFile* pFile)
	{
		char szID[CVLIB_MAX_IDLEN];
		if (pFile->read(szID, sizeof(szID), 1) != 1)
			return 0;
		if (strcmp(szID, FEATURE_SET))
			return 0;
		pFile->read(&m_nNum, sizeof(m_nNum), 1);
		m_pFeatureArray = new FeatureABC*[m_nNum];
		for (int i = 0; i < m_nNum; i++)
			m_pFeatureArray[i] = FeatureFactory::Instance()->readFeature(pFile);
		return 1;
	}

	int FeatureSet::write(XFile* pFile) const
	{
		if (pFile->write(m_szID, sizeof(m_szID), 1) != 1)
			return 0;
		if (pFile->write(&m_nNum, sizeof(m_nNum), 1) != 1)
			return 0;
		for (int i = 0; i < m_nNum; i++)
			m_pFeatureArray[i]->write(pFile);
		return 1;
	}

	int FeatureSet::dimension() const
	{
		int nDim = 0;
		for (int i = 0; i < m_nNum; i++)
			nDim += getAt(i)->dimension();
		return nDim;
	}

	FeatureSet* FeatureSet::createFromID(const char* szID)
	{
		if (strcmp(szID, FEATURE_SET))
			return NULL;
		return new FeatureSet;
	}

	//////////////////////////////////////////////////////////////////////////
	FeatureFactory::FeatureFactory()
	{
		m_pFeatureFactory = NULL;
	}
	FeatureFactory::~FeatureFactory()
	{
		release();
	}
	FeatureFactory* FeatureFactory::Instance()
	{
		if (!m_pFeatureFactory)
		{
			m_pFeatureFactory = new FeatureFactory;
			m_pFeatureFactory->init();
		}
		return m_pFeatureFactory;
	}
	void FeatureFactory::init()
	{
		addFeature(new FeatureVec);
		addFeature(new FeatureSet);
	}

	void FeatureFactory::release()
	{
		for (int i = 0; i < m_Prototypes.getSize(); i++)
		{
			((FeatureABC*)m_Prototypes[i])->release();
			delete ((FeatureABC*)m_Prototypes[i]);
		}
		m_nDefaultNum = 0;
		m_nActiveIdx = -1;
		m_Prototypes.removeAll();
	}

	FeatureABC* FeatureFactory::readFeature(const char* szFilePath)
	{
		XFileDisk xfile;
		if (!xfile.open(szFilePath, "rb"))
			return 0;
		return readFeature(&xfile);
	}

	FeatureABC* FeatureFactory::readFeature(XFile* pFile)
	{
		char szID[CVLIB_MAX_IDLEN];

		int pos = pFile->tell();
		if (pFile->read(szID, sizeof(szID), 1) != 1)
			return 0;
		pFile->seek(pos, SEEK_SET);

		int nIdx = find(szID);
		if (nIdx < 0)
			return NULL;
		FeatureABC* pNew = getPrototype(nIdx)->clone();
		if (!pNew->read(pFile))
		{
			delete pNew;
			pNew = NULL;
		}
		return pNew;
	}

}