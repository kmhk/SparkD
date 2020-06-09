#include "FeatureVec.h"
#include "Distance.h"
#include "Mat.h"
#include "IntegralImage.h"

namespace cvlib
{

	FeatureVec::FeatureVec()
	{
		strcpy(m_szID, FEATURE_VEC);
		assert(strlen(m_szID) < CVLIB_MAX_IDLEN);
		m_size.height = m_size.width = 0;
		m_pmSum = NULL;
		m_pmSquare = NULL;
		m_pmTilted = NULL;
	}

	FeatureVec::~FeatureVec()
	{
		release();
	}

	int FeatureVec::read(XFile* pFile)
	{
		char szID[CVLIB_MAX_IDLEN];
		if (pFile->read(szID, sizeof(szID), 1) != 1)
			return 0;
		if (pFile->read(&m_size, sizeof(m_size), 1) != 1)
			return 0;
		if (strcmp(szID, FEATURE_VEC))
			return 0;
		if (fromFile(pFile))
			return 1;
		return 0;
	}

	int FeatureVec::write(XFile* pFile) const
	{
		if (pFile->write(m_szID, sizeof(m_szID), 1) != 1)
			return 0;
		if (pFile->write(&m_size, sizeof(m_size), 1) != 1)
			return 0;
		toFile(pFile);
		return 1;
	}

	void FeatureVec::release()
	{
		Vec::release();
		if (m_pmSum)
			delete m_pmSum;
		if (m_pmSquare)
			delete m_pmSquare;
		if (m_pmTilted)
			delete m_pmTilted;
		m_pmSum = NULL;
		m_pmSquare = NULL;
		m_pmTilted = NULL;
		m_size.height = m_size.width = 0;
	}

	float FeatureVec::dist(const FeatureABC* pFeature) const
	{
		const float* prFea1 = data.fl;
		const float* prFea2 = ((const FeatureVec*)pFeature)->data.fl;
		int nLen = length();
		return Distance::distanceCorrelation(prFea1, prFea2, nLen);
	}

	int FeatureVec::dimension() const
	{
		return length();
	}

	void FeatureVec::getData(Vec* pv1)
	{
		pv1->create(*(Vec*)this, true);
	}

	FeatureVec* FeatureVec::createFromID(const char* szID)
	{
		if (strcmp(szID, FEATURE_VEC))
			return NULL;
		return new FeatureVec;
	}

	FeatureABC* FeatureVec::clone() const
	{
		FeatureVec* pFea = new FeatureVec;
		pFea->create(*(Vec*)this, true);
		pFea->m_size = m_size;
		return pFea;
	}

	void FeatureVec::calcIntegralImage(bool fSquare/*=false*/, bool fTilted/*=false*/)
	{
		Matb mTemp(m_size.height, m_size.width);
		int k = 0;
		for (int i = 0; i < m_size.height; i++)
			for (int j = 0; j < m_size.width; j++, k++)
				mTemp.data.ptr[i][j] = (uchar)data.fl[k];

		if (!fSquare && !fTilted)
		{
			m_pmSum = new Mati(m_size.height + 1, m_size.width + 1);
			ip::integralImage(mTemp, *m_pmSum);
		}
		else if (fSquare && !fTilted)
		{
			m_pmSum = new Mati(m_size.height + 1, m_size.width + 1);
			m_pmSquare = new Matd(m_size.height + 1, m_size.width + 1);
			ip::integralImage(mTemp, *m_pmSum, *m_pmSquare);
		}
		else if (fSquare && fTilted)
		{
			m_pmSum = new Mati(m_size.height + 1, m_size.width + 1);
			m_pmSquare = new Matd(m_size.height + 1, m_size.width + 1);
			m_pmTilted = new Mati(m_size.height + 1, m_size.width + 1);
			ip::integralImage(mTemp, *m_pmSum, *m_pmTilted, *m_pmSquare);
		}
		else
		{
			m_pmSum = new Mati(m_size.height + 1, m_size.width + 1);
			m_pmTilted = new Mati(m_size.height + 1, m_size.width + 1);
			ip::integralTiltedImage(mTemp, *m_pmSum, *m_pmTilted);
		}

		mTemp.release();
	}

}
