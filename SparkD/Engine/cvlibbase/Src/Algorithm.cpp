/*!
 * \file	Algorithm.cpp
 * \ingroup base
 * \brief	
 * \author	
 */

#include "Algorithm.h"

namespace cvlib
{

Algorithm::Algorithm()
{
	m_szID[0] = 0;
	m_szTotalID[0] = 0;
}

Algorithm::~Algorithm()
{
	removeAlgorithms ();
}

const char*  Algorithm::getID ()
{
	if (getAlgorithmCount() == 0)
	{
		if (m_szID[0] == 0)
			strcpy (m_szID, "None");
		return getSelfID();
	}
	else
	{
		m_szTotalID[0] = 0;
		if (m_szID[0] == 0)
			strcpy (m_szID, "Unknown");
		strcpy (m_szTotalID, m_szID);
		for (int i = 0; i < getAlgorithmCount(); i ++)
		{
			Algorithm* pAlgorithm = getAlgorithm(i);
			strcat (m_szTotalID, "[");
			if (pAlgorithm)
				strcat (m_szTotalID, pAlgorithm->getID ());
			else
				strcat (m_szTotalID, "None");
			strcat (m_szTotalID, "]");
		}
	}
	return m_szTotalID;
}

void Algorithm::setSelfID (const char* szID)
{
	assert ( strlen (szID) < CVLIB_IDLEN );
	strcpy (m_szID, szID);
}

const char* Algorithm::getSelfID() const
{
	return m_szID;
}

Algorithm* Algorithm::getAlgorithm (int nIdx) const
{
	return (Algorithm*)m_AlgorithmArray[nIdx];
}

int Algorithm::getAlgorithmCount() const
{
	return m_AlgorithmArray.getSize();
}

int Algorithm::addAlgorithm (Algorithm* pAlgorithm)
{
	return m_AlgorithmArray.add (pAlgorithm);
}

void Algorithm::removeAlgorithms ()
{
	m_AlgorithmArray.removeAll();
}

void Algorithm::delAlgorithm(int nIdx)
{
	m_AlgorithmArray.removeAt(nIdx);
}

void Algorithm::SetAlgorithm (int nIdx, Algorithm* pAlgorithm)
{
	m_AlgorithmArray.setAt(nIdx, pAlgorithm);
}

}
