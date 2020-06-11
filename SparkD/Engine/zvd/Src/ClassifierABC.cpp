/*!
 * \file
 * \brief	
 * \author	
 */
#include "ClassifierABC.h"
#include "cvlibbase/Inc/IniFile.h"

namespace cvlib 
{

/************************************************************************/
/*                                                                      */
/************************************************************************/
MachineABC::MachineABC()
{
	memset (m_szVersion, 0, sizeof(char) * VERSION_LEN);
}

MachineABC::~MachineABC()
{
	release();
}

void MachineABC::setVersion (const char* szVersion)
{
	memcpy (m_szVersion, szVersion, VERSION_LEN);
}

void MachineABC::getVersion (char* szVersion) const
{
	memcpy (szVersion, m_szVersion, VERSION_LEN);
}

void MachineABC::release ()
{
}

int	MachineABC::fromFile (XFile* pFile) 
{
	pFile->read(m_szVersion, 1, VERSION_LEN);
	return 1;
}
int	MachineABC::toFile (XFile* pFile) const
{
	pFile->write(m_szVersion, 1, VERSION_LEN);
	return 1;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
MachineSet::MachineSet()
{
	m_nNum = m_nActiveNum = 0;
	m_pClassifierArray = NULL;
}

MachineSet::~MachineSet()
{
	release();
}

int MachineSet::add (MachineABC* pClassifier)
{
	MachineABC** pClassifiers = new MachineABC*[m_nNum + 1];
	if (m_nNum != 0)
	{
		memcpy(pClassifiers, m_pClassifierArray, sizeof(MachineABC*) * m_nNum);
	}
	pClassifiers[m_nNum] = pClassifier;

	if (m_pClassifierArray)
		delete []m_pClassifierArray;
	m_pClassifierArray = pClassifiers;

	m_nNum ++;
	m_nActiveNum = m_nNum;
	return m_nNum-1;
}

void MachineSet::removeAt (int nIdx)
{
	if (nIdx < 0 || nIdx >= m_nNum)
		return;
	if (getSize() == 0)
		return;

	m_pClassifierArray[nIdx]->release();
	delete m_pClassifierArray[nIdx];

	for (int i = nIdx; i < m_nNum - 1; i ++)
		m_pClassifierArray[i] = m_pClassifierArray[i+1];
	m_nNum --;
	m_nActiveNum = m_nNum;
}

void MachineSet::detachAt (int nIdx)
{
	if (nIdx < 0 || nIdx >= m_nNum)
		return;
	if (getSize() == 0)
		return;
	
	for (int i = nIdx; i < m_nNum - 1; i ++)
		m_pClassifierArray[i] = m_pClassifierArray[i+1];
	m_nNum --;
	m_nActiveNum = m_nNum;
}

void MachineSet::release ()
{
	if (m_pClassifierArray)
	{
		for (int i = 0; i < getSize(); i ++)
		{
			m_pClassifierArray[i]->release();
			delete m_pClassifierArray[i];
		}
		delete []m_pClassifierArray;
		m_pClassifierArray = NULL;
		m_nNum = 0;
		m_nActiveNum = m_nNum;
	}
}

void MachineSet::detachAll()
{
	if (m_pClassifierArray)
	{
		delete []m_pClassifierArray;
		m_pClassifierArray = NULL;
		m_nNum = 0;
		m_nActiveNum = m_nNum;
	}
}

int MachineSet::fromFile (XFile* pFile)
{
	MachineABC::fromFile (pFile);

	if (strcmp(m_szVersion, CLASSIFIER_SET))
		return 0;
	pFile->read (&m_nNum, sizeof(m_nNum), 1);
	m_pClassifierArray = new MachineABC*[m_nNum];
	for (int i = 0; i < m_nNum; i ++)
	{
		//m_pClassifierArray[i] = (MachineABC*)loadMachine (pFile);
	}
	return 1;
}

int MachineSet::toFile (XFile* pFile) const
{
	MachineABC::toFile (pFile);

	pFile->write (&m_nNum, sizeof(m_nNum), 1);
	for (int i = 0; i < m_nNum; i ++)
		m_pClassifierArray[i]->toFile(pFile);
	return 1;
}

int	MachineSet::fromIniFile(const char* szFilename)
{
	IniFile ini(szFilename);
	if (!ini.readFile())
		return 0;
	if (ini.findKey(String("MachineSet")) == IniFile::noID)
		return 0;
	int num = ini.getValueI(String("MachineSet"), String("num"));
	for (int i = 0; i < num; i++)
	{
		char szvalue[260];
		sprintf(szvalue, "machine%d", i);
		String szPath = ini.getValue(String("MachineSet"), szvalue);
		MachineABC* pmachine = 0;// loadMachine(szPath);
		add(pmachine);
	}
	return 1;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
ClassifierSet::ClassifierSet()
{
	setSelfID (CLASSIFIER_SET);
	strcpy (m_szVersion, CLASSIFIER_SET);
	m_nNum = m_nActiveNum = 0;
	m_pClassifierArray = NULL;
}

ClassifierSet::~ClassifierSet()
{
	release();
}

ClassifierABC* ClassifierSet::getAt (int nIdx) const
{
	assert (nIdx >= 0 && nIdx < m_nNum);
	return (ClassifierABC*)m_pClassifierArray[nIdx];
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
MapperSet::MapperSet()
{
	setSelfID (CLASSIFIER_SET);
	strcpy (m_szVersion, CLASSIFIER_SET);
	m_nNum = m_nActiveNum = 0;
	m_pClassifierArray = NULL;
}

MapperSet::~MapperSet()
{
	release();
}

MapperABC* MapperSet::getAt (int nIdx) const
{
	assert (nIdx >= 0 && nIdx < m_nNum);
	return (MapperABC*)m_pClassifierArray[nIdx];
}

}