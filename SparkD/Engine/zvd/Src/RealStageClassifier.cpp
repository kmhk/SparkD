/*!
 * \file
 * \brief	RealBoosting
 */
#include "RealStageClassifier.h"
#include "RealStumpClassifier.h"

namespace cvlib
{

RealStageClassifier::RealStageClassifier()
{
	setSelfID (CLASSIFIER_RBOOST);
	setVersion (CLASSIFIER_RBOOST);

	m_nCount = 0;
	m_rThreshold = 0.0f;
	m_ppStumpClassifier = NULL;

}

RealStageClassifier::~RealStageClassifier()
{
	release();
}

void	RealStageClassifier::create (int nNum)
{
	m_nCount = nNum;
	m_ppStumpClassifier = (StumpClassifierABC**)new RealStumpClassifier*[m_nCount];
}

void	RealStageClassifier::release ()
{
	int i;
	for (i = 0; i < m_nCount; i ++)
	{
		StumpClassifierABC* pClassifier = m_ppStumpClassifier[i];
		pClassifier->release();
		delete pClassifier;
	}
	if (m_ppStumpClassifier)
		delete []m_ppStumpClassifier;
	m_ppStumpClassifier = 0;
	m_nCount = 0;
}

int		RealStageClassifier::fromFile (XFile* pFile)
{
	int i;
	ClassifierABC::fromFile(pFile);

	pFile->read (&m_nCount, sizeof(m_nCount), 1);
	pFile->read (&m_rThreshold, sizeof(m_rThreshold), 1);

	create (m_nCount);

	for (i = 0; i < m_nCount; i ++)
	{
		RealStumpClassifier* pClassifier;
		pClassifier = new RealStumpClassifier();
		pClassifier->fromFile(pFile);
		m_ppStumpClassifier[i] = pClassifier;
	}
	return 1;
}

int		RealStageClassifier::toFile (XFile* pFile) const
{
	int i;

	ClassifierABC::toFile(pFile);

	pFile->write (&m_nCount, sizeof(m_nCount), 1);
	pFile->write (&m_rThreshold, sizeof(m_rThreshold), 1);

	for (i = 0; i < m_nCount; i ++)
	{
		StumpClassifierABC* pClassifier = m_ppStumpClassifier[i];
		pClassifier->toFile(pFile);
	}
	return 1;
}

float	RealStageClassifier::eval(Vec* pvFea)
{
	float rEval = 0.0f;
	for (int i = 0; i < m_nCount; i ++)
	{
		StumpClassifierABC* pClassifier = m_ppStumpClassifier[i];
		rEval += pClassifier->eval(pvFea);
	}
	return rEval;
}

}
