/*!
 * \file
 * \brief	
 * \author	
 */

#include "Regressor.h"

namespace cvlib 
{

static Vec m_vConvert;
LinearRegressor::LinearRegressor()
{
}

LinearRegressor::~LinearRegressor()
{
	release();
}

void	LinearRegressor::release ()
{
	m_vCoeff.release();
}

float	LinearRegressor::eval(Vec* pvFea)
{
	int i;
	float rRet = 0.0f;
	if (pvFea->type() == MAT_Tfloat)
	{
		for (i = 0; i < pvFea->length(); i ++)
			rRet += pvFea->data.fl[i] * m_vCoeff.data.fl[i];
		rRet += m_vCoeff.data.fl[i];
	}
	else if (pvFea->type() == MAT_Tdouble)
	{
		for (i = 0; i < pvFea->length(); i ++)
			rRet += (float)pvFea->data.db[i] * m_vCoeff.data.fl[i];
		rRet += m_vCoeff.data.fl[i];
	}
	return rRet;
}

float LinearRegressor::getConstCoeff()
{
	return m_vCoeff.data.fl[m_vCoeff.length()-1];
}
void LinearRegressor::setConstCoeff(float rConf)
{
	m_vCoeff.data.fl[m_vCoeff.length()-1]=rConf;
}
int	LinearRegressor::fromFile (XFile* pFile)
{
	return m_vCoeff.fromFile(pFile);
}
int	LinearRegressor::toFile (XFile* pFile) const
{
	return m_vCoeff.toFile(pFile);
}


//////////////////////////////////////////////////////////////////////////

void	QuadraticRegressor::release ()
{
	m_vCoeff.release();
	m_vConvert.release();
}

float	QuadraticRegressor::eval(Vec* pvFea)
{
	int i;
	float rRet = 0.0f;
	const Vec* pvConvert = iConvert(pvFea);
	if (pvConvert->type() == MAT_Tfloat)
	{
		for (i = 0; i < pvConvert->length(); i ++)
			rRet += pvConvert->data.fl[i] * m_vCoeff.data.fl[i];
		rRet += m_vCoeff.data.fl[i];
	}
	else if (pvConvert->type() == MAT_Tdouble)
	{
		for (i = 0; i < pvConvert->length(); i ++)
			rRet += (float)pvConvert->data.db[i] * m_vCoeff.data.fl[i];
		rRet += m_vCoeff.data.fl[i];
	}
	return rRet;
}

const Vec* QuadraticRegressor::iConvert (const Vec* pvFea)
{
	int nNum = pvFea->length();

	if (pvFea->type() == MAT_Tdouble)
	{
		if (! (m_vConvert.isValid() && m_vConvert.type() == MAT_Tdouble) )
		{
			m_vConvert.release();
			m_vConvert.create (nNum * 2 + nNum*(nNum-1) / 2, MAT_Tdouble);
		}
		
		const double* prI = pvFea->data.db;
		double* prA = m_vConvert.data.db;
		int i, j;
		int iFea = 0;
		for (i = 0; i < nNum; i ++)
		{
			prA[iFea] = prI[i] * prI[i];
			iFea ++;
		}
		for (i = 0; i < nNum; i ++)
			for (j = i+1; j < nNum; j ++)
			{
				prA[iFea] = prI[i] * prI[j];
				iFea ++;
			}
			for (i = 0; i < nNum; i ++)
			{
				prA[iFea] = prI[i];
				iFea ++;
			}
	}
	else if (pvFea->type() == MAT_Tfloat)
	{
		if (! (m_vConvert.isValid() && m_vConvert.type() == MAT_Tfloat) )
		{
			m_vConvert.release();
			m_vConvert.create (nNum * 2 + nNum*(nNum-1) / 2, MAT_Tfloat);
		}
		
		float* prI = pvFea->data.fl;
		float* prA = m_vConvert.data.fl;
		int i, j;
		int iFea = 0;
		for (i = 0; i < nNum; i ++)
		{
			prA[iFea] = prI[i] * prI[i];
			iFea ++;
		}
		for (i = 0; i < nNum; i ++)
			for (j = i+1; j < nNum; j ++)
			{
				prA[iFea] = prI[i] * prI[j];
				iFea ++;
			}
			for (i = 0; i < nNum; i ++)
			{
				prA[iFea] = prI[i];
				iFea ++;
			}
	}

	return &m_vConvert;
}

float	LeastSquareMachine::eval(Vec* pvFea)
{
	int i;
	float rRet = 0.0f;
	if (pvFea->type() == MAT_Tfloat)
	{
		for (i = 0; i < pvFea->length(); i ++)
			rRet += pvFea->data.fl[i] * m_vCoeff.data.fl[i];
	}
	else if (pvFea->type() == MAT_Tdouble)
	{
		for (i = 0; i < pvFea->length(); i ++)
			rRet += (float)pvFea->data.db[i] * m_vCoeff.data.fl[i];
	}
	return rRet;
}

}