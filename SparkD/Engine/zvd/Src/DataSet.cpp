#include "DataSet.h"
#include "cvlibbase/Inc/XFileDisk.h"

namespace cvlib {

DataSet::DataSet()
{
	init();
}

DataSet::DataSet (int nDim, int nCount, int nMode/*= DS_ALL*/)
{
	init ();
	create (nDim, nCount, nMode);
}

DataSet::DataSet (const DataSet& other)
{
	copyFrom (other);
}

DataSet::DataSet (const char* szFilename)
{
	init();

	XFileDisk xfile;
	if (!xfile.open(szFilename, "rb"))
		return;
	fromFile (&xfile);
}

DataSet::~DataSet()
{
	release();
}

void	DataSet::init ()
{
	m_nFlagCreate = 0;

	m_nMaxCount = 0;
	m_nCount = 0;
	m_nDim = 0;
	m_prCls = NULL;
	m_pprData = NULL;
	m_prWeights = NULL;
	m_nGrowBy = 0;
	m_nMode = DS_ALL;
}

int	DataSet::fromFile (XFile* pFile)
{
	if (pFile->read(&m_nDim, sizeof(m_nDim), 1) != 1)
		return 0;
	if (pFile->read(&m_nCount, sizeof(m_nCount), 1) != 1)
		return 0;
	m_nMaxCount = m_nCount;
	m_nMode = DS_ALL;
	
	create (m_nDim, m_nCount);
	if (pFile->read(m_prCls, sizeof(double), m_nCount) != m_nCount)
		return 0;
	for (int i = 0; i < m_nMaxCount; i ++)
		pFile->read(m_pprData[i], sizeof(double), m_nDim);
	
	return 1;
}
int	DataSet::toFile (XFile* pFile) const
{
	if (pFile->write(&m_nDim, sizeof(m_nDim), 1) != 1)
		return 0;
	if (pFile->write (&m_nCount, sizeof(m_nCount), 1) != 1)
		return 0;
	if (pFile->write(m_prCls, sizeof(double), m_nCount) != m_nCount)
		return 0;
	for (int i = 0; i < m_nMaxCount; i ++)
	{
		pFile->write(m_pprData[i], sizeof(double), m_nDim);
	}
	return 1;
}

int	DataSet::create (int nDim, int nCount, int nMode/*= DS_ALL*/)
{
	m_nMaxCount = nCount;
	m_nCount = m_nMaxCount;
	m_nDim = nDim;
	m_nMode = nMode;
	m_prCls = new double[m_nMaxCount]; memset (m_prCls,0,sizeof(double)*m_nMaxCount);
	m_prWeights = new double[m_nMaxCount]; memset (m_prWeights,0,sizeof(double)*m_nMaxCount);
	m_pprData = new double*[m_nMaxCount]; memset (m_pprData,0,sizeof(double*)*m_nMaxCount);
	if (m_nMode == DS_ALL)
	{
		for (int i = 0; i < m_nMaxCount; i ++)
		{
			m_pprData[i] = new double[m_nDim]; 
			memset (m_pprData[i],0,sizeof(double)*m_nDim);
		}
	}
	m_nFlagCreate = 1;
	return 1;
}

int	DataSet::copyFrom (const DataSet& other)
{
	if (!create (other.m_nDim, other.m_nCount, DS_ALL))
		return 0;
	memcpy (m_prCls, other.m_prCls, sizeof(double)*m_nCount);
	memcpy (m_prWeights, other.m_prWeights, sizeof(double)*m_nCount);
 	for (int i = 0; i < m_nCount; i ++)
 		memcpy (m_pprData[i], other.m_pprData[i], sizeof(double)*m_nDim);
	return 1;
}

void DataSet::release ()
{
	if (m_nMaxCount == 0 && m_nDim == 0)
		return;

	if (m_nMode == DS_ALL)
	{
		for (int i = m_nMaxCount - 1; i >= 0; i --)
			delete []m_pprData[i];
	}
	delete [](m_pprData);
	delete [](m_prCls);
	delete [](m_prWeights);

	m_nFlagCreate = 0;
	init ();
}

int	DataSet::getSampleNum (int nClass)
{
	int nCount = 0;
	for (int i = 0; i < m_nCount; i ++)
	{
		if (fabs(m_prCls[i] - nClass) < 1e-7)
			nCount ++;
	}
	return nCount;
}

int DataSet::getClassNum()
{
	int		nMaxClassNum = m_nCount;
	int nClassNum = 0;
	int* pnLabel = new int[nMaxClassNum];
	int i;
	for (i = 0; i < m_nCount; i ++)
	{
		int nThisLabel = (int)m_prCls[i];
		int j;
		for (j = 0; j < nClassNum; j ++)
			if (nThisLabel == pnLabel[j])
				break;
		if (j == nClassNum)
		{
			pnLabel[nClassNum] = nThisLabel;
			nClassNum ++;
		}
	}
	delete [] (pnLabel);
	return nClassNum;
}

int DataSet::statistics(double* prMin, double* prMax, double* prMean)
{
	double* prTMin=new double[m_nDim];
	double* prTMax=new double[m_nDim];
	double* prTMean=new double[m_nDim];
	for (int id=0; id<m_nDim; id++)
	{
		prTMin[id]=1e10;
		prTMax[id]=-1e10;
		prTMean[id]=0;
		for (int i=0; i<m_nCount; i++)
		{
			prTMean[id] += m_pprData[i][id];
			prTMin[id] = MIN(prTMin[id], m_pprData[i][id]);
			prTMax[id] = MAX(prTMax[id], m_pprData[i][id]);
		}
		prTMean[id] /= m_nCount;
	}
	if (prMin)
		memcpy (prMin, prTMin, sizeof(double)*m_nDim);
	if (prMax)
		memcpy (prMax, prTMax, sizeof(double)*m_nDim);
	if (prMean)
		memcpy (prMean, prTMean, sizeof(double)*m_nDim);
	delete []prTMin;
	delete []prTMax;
	delete []prTMean;
	return 1;
}

int	DataSet::add (double* prData, double rCls, double rWei/*=0.0*/)
{
	if (m_nCount == m_nMaxCount)
	{
		if (m_nMode == DS_ALL)
		{
			DataSet newData(*this);
			release();
			if (m_nGrowBy==0)
			{
				m_nGrowBy = m_nCount/8;
				m_nGrowBy = (m_nGrowBy < 4) ? 4 : ((m_nGrowBy > 1024) ? 1024 : m_nGrowBy);
			}
			int nNewMaxCount = newData.getMaxCount() + m_nGrowBy;

			create (newData.m_nDim, nNewMaxCount, DS_ALL);
			memcpy (m_prCls, newData.m_prCls, sizeof(m_prCls[0])*newData.count());
			memcpy (m_prWeights, newData.m_prWeights, sizeof(m_prWeights[0])*newData.count());
			for (int i=0; i<newData.count(); i++)
				memcpy (m_pprData[i], newData.m_pprData[i], sizeof(double)*m_nDim);
			m_nCount=newData.count();
			newData.release();
		}
		else if (m_nMode == DS_ADDRESS)
		{
			int nDim = m_nDim;
			int nCount=m_nCount;
			double** pprTempData = new double*[m_nCount];
			double* prTempCls = new double[m_nCount];
			double* prTempWei = new double[m_nCount];
			memcpy (prTempCls, m_prCls, sizeof(m_prCls[0])*m_nCount);
			memcpy (prTempWei, m_prWeights, sizeof(m_prWeights[0])*m_nCount);
			memcpy (pprTempData, m_pprData, sizeof(double*)*m_nCount);

			release();
			if (m_nGrowBy==0)
			{
				int m_nGrowBy = (nCount+1)/8;
				m_nGrowBy = (nCount < 4) ? 4 : ((m_nGrowBy > 1024) ? 1024 : m_nGrowBy);
			}
			int nNewMaxCount = nCount + m_nGrowBy;
			create (nDim, nNewMaxCount, DS_ALL);
			
			memcpy (m_prCls, prTempCls, sizeof(m_prCls[0])*nCount);
			memcpy (m_prWeights, prTempWei, sizeof(m_prWeights[0])*nCount);
			for (int i=0; i<nCount; i++)
				m_pprData[i]=pprTempData[i];
			m_nCount=nCount;
		}
	}
	m_prCls[m_nCount] = rCls;
	m_prWeights[m_nCount] = rWei;
	if (m_nMode == DS_ALL)
		memcpy (m_pprData[m_nCount], prData, sizeof(double)*m_nDim);
	else if (m_nMode == DS_ADDRESS)
		m_pprData[m_nCount] = prData;
	m_nCount ++;
	return m_nCount;
}

void DataSet::setData (int nIdx, const double* prData, double rCls, double rWei/*=0.0*/)
{
	memcpy (m_pprData[nIdx], prData, sizeof(double)*m_nDim);
	m_prCls[nIdx] = rCls;
	m_prWeights[nIdx] = rWei;
}

Matd* DataSet::toMat()
{
	if (m_nMode == DS_ALL)
		return new Matd(m_pprData, m_nCount, m_nDim);
	else
		return 0;
}

void DataSet::fromMat(Matd* pM)
{
	for (int i=0; i<m_nCount; i++)
	{
		for (int ic=0; ic<m_nDim; ic++)
			m_pprData[i][ic] = pM->data.db[i][ic];
	}
}

void DataSet::sortClsIdx()
{
	DataSet temp(*this);
}

int	DataSet::fromFile (const char* szFilename)
{
	return Object::fromFile(szFilename);
}
int	DataSet::toFile (const char* szFilename) const
{
 	return Object::toFile(szFilename);
}
void DataSet::fromMat(Mat& mdata, Vec& vclass)
{
	assert (mdata.rows() == vclass.length());
	release();
	create (mdata.cols(), mdata.rows());
	for (int i=0; i<m_nCount; i++)
	{
		for (int ic=0; ic<m_nDim; ic++)
			m_pprData[i][ic] = mdata.data.fl[i][ic];
		m_prCls[i]=vclass.data.fl[i];
	}
}
DataSet* createDataSetCol (const DataSet& src)
{
	int dims = src.dims();
	int count = src.count();
	DataSet* dst = new DataSet (dims-1, count, DataSet::DS_ALL);
	for (int i=0; i<count; i++)
	{
		memcpy(dst->m_pprData[i], src.m_pprData[i], sizeof(double)*(dims-1));
		dst->m_prCls[i] = src.m_pprData[i][dims-1];
	}
	return dst;
}
DataSet* createDataSetForSubSamples (const DataSet& src, const int* idxs, int count)
{
	int dims = src.dims();
	DataSet* dst = new DataSet (dims, count, DataSet::DS_ALL);
	for (int i=0; i<count; i++)
	{
		memcpy(dst->m_pprData[i], src.m_pprData[idxs[i]], sizeof(double)*dims);
		dst->m_prCls[i] = src.m_prCls[i];
	}
	return dst;
}

}