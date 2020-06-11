/*!
 * \file
 * \brief		
 * \author		
 */
#include "StumpClassifier.h"

namespace cvlib 
{

static char mszVersion[] = "0304-StumpBoost";

int	StumpClassifierABC::fromFile (XFile* fp)
{
	ClassifierABC::fromFile (fp);
	strcpy (mszVersion, m_szVersion);
	fp->read (&m_nCompidx, sizeof(m_nCompidx), 1);
	return 1;
}
int	StumpClassifierABC::toFile (XFile* fp) const
{
	ClassifierABC::toFile (fp);
	fp->write (&m_nCompidx, sizeof(m_nCompidx), 1);
	return 1;
}


StumpClassifier::StumpClassifier()
{
	setSelfID (CLASSIFIER_STUMP);
	setVersion (CLASSIFIER_STUMP);
	strcpy (m_szVersion, mszVersion);
	m_nCompidx = -1;
	m_rAlpha = 0.0f;
	m_rThreshold = 0.0f;
}
StumpClassifier::~StumpClassifier()
{
}
int		StumpClassifier::fromFile (XFile* fp)
{
	StumpClassifierABC::fromFile(fp);
	fp->read (&m_rAlpha, sizeof(m_rAlpha), 1);
	fp->read (&m_rThreshold, sizeof(m_rThreshold), 1);
	return 1;
}
int		StumpClassifier::toFile (XFile* fp) const
{
	StumpClassifierABC::toFile(fp);
	fp->write (&m_rAlpha, sizeof(m_rAlpha), 1);
	fp->write (&m_rThreshold, sizeof(m_rThreshold), 1);
	return 1;
}
float	StumpClassifier::eval(Vec* pvFea)
{
	if (pvFea->data.fl[m_nCompidx] > m_rThreshold)
		return 1.0f;
	else
		return 0.0f;
}
float	StumpClassifier::eval(float r)
{
	if (r > m_rThreshold)
		return 1.0f;
	else
		return 0.0f;
}

}
