/*!
 * \file
 * \brief		RealBoosting
 * \author		
 */
#include "RealStumpClassifier.h"

namespace cvlib 
{

static char mszVersion[] = "RealBoost";

RealStumpClassifier::RealStumpClassifier()
{
	setSelfID (CLASSIFIER_REALBOOST);
	setVersion (CLASSIFIER_REALBOOST);
	m_nCompidx = -1;
	m_rStep = m_rMin = m_rMax = 0.0f;
	m_nBinnum = 0;
	m_prValue = NULL;
	strcpy (m_szVersion, mszVersion);
}

RealStumpClassifier::~RealStumpClassifier()
{
	release();
}

void	RealStumpClassifier::create (int nNum)
{
	m_nBinnum = nNum;
	m_prValue = new float[m_nBinnum];
}

void	RealStumpClassifier::release ()
{
	if (m_prValue)
		delete [] (m_prValue);
	m_prValue = NULL;
}

int	RealStumpClassifier::fromFile (XFile* fp)
{
	StumpClassifierABC::fromFile (fp);
	fp->read (&m_nBinnum, sizeof(m_nBinnum), 1);
	fp->read (&m_rMin, sizeof(m_rMin), 1);
	fp->read (&m_rMax, sizeof(m_rMax), 1);
	fp->read (&m_rStep, sizeof(m_rStep), 1);
	m_prValue = new float[m_nBinnum];
	fp->read (m_prValue, sizeof(m_prValue[0]), m_nBinnum);
	return 1;
}
int		RealStumpClassifier::toFile (XFile* fp) const
{
	StumpClassifierABC::toFile (fp);
	fp->write (&m_nBinnum, sizeof(m_nBinnum), 1);
	fp->write (&m_rMin, sizeof(m_rMin), 1);
	fp->write (&m_rMax, sizeof(m_rMax), 1);
	fp->write (&m_rStep, sizeof(m_rStep), 1);
	fp->write (m_prValue, sizeof(m_prValue[0]), m_nBinnum);
	return 1;
}

float	RealStumpClassifier::eval(Vec* pvFea)
{
	float rVal = pvFea->data.fl[m_nCompidx];
	int i = whichBin (rVal);
	return m_prValue[i];
}
float	RealStumpClassifier::eval(float r)
{
	int i = whichBin (r);
	return m_prValue[i];
}

int RealStumpClassifier::whichBin (float rVal) const
{
	int iBin = (int)((rVal - m_rMin) / m_rStep);
	if (iBin < 0)
		iBin = 0;
	if (iBin >= m_nBinnum)
		iBin = m_nBinnum - 1;
	return iBin;
}

}
