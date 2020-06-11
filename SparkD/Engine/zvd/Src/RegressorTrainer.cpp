/*!
 * \file
 * \brief	
 * \author	
 */

#include "RegressorTrainer.h"
#include "Regressor.h"
#include "cvlibbase/Inc/Matoperation.h"
#include "cvlibbase/Inc/LUDecomposition.h"

namespace cvlib
{

//////////////////////////////////////////////////////////////////////////
void RegressorTrainerABC::setData(const DataSet* pData)
{
	Mat mExample(pData->count(), pData->m_nDim, MAT_Tdouble);
	Vec vOvj(pData->count(), MAT_Tdouble);
	for (int i=0; i<pData->count(); i++)
	{
		for (int j=0; j<pData->m_nDim; j++)
			mExample.data.db[i][j] = pData->m_pprData[i][j];
		vOvj.data.db[i]=pData->m_prCls[i];
	}
	setData (&mExample, &vOvj);
}

//////////////////////////////////////////////////////////////////////////
LinearRegressorTrainer::~LinearRegressorTrainer()
{
	release();
}

void LinearRegressorTrainer::setData (const Mat* pmExamples, const Vec* pvObj)
{
	int sample_count = pmExamples->rows();
	int dims = pmExamples->cols();
	m_Data.create(sample_count, dims + 1, MAT_Tdouble);
	for(int i = 0; i < sample_count; i ++)
	{
		int j;
		for (j = 0; j < dims; j ++)
			m_Data.data.db[i][j] = pmExamples->data.db[i][j];
		m_Data.data.db[i][j] = 1;
	}
	m_vObj = *pvObj;
}

int	LinearRegressorTrainer::train (ClassifierABC* pMachine)
{
	LinearRegressor* pRegressor = (LinearRegressor*)pMachine;

	int i;

	Mat mB(m_Data.cols(), m_Data.cols(), MAT_Tdouble);
	MatOp::trAA(&mB, &m_Data);

	LUDecomposition lu(&mB);
	if (!lu.isNonSingular())
		return 0;

	Mat mI(m_Data.cols(), m_Data.cols(), MAT_Tdouble);
	mI.zero();
	for (i = 0; i < mI.rows(); i ++)
		mI.data.db[i][i] = 1.0;
	Mat* pmInverse = lu.solve (&mI);
	if (pmInverse==0)
		return 0;
	
	Mat mTemp(m_Data.cols(), m_Data.rows(), MAT_Tdouble);
	Mat mTr = m_Data.transposed();
	MatOp::mul(&mTemp, pmInverse, &mTr);

	pRegressor->m_vCoeff.create (m_Data.cols(), MAT_Tfloat);
	Vec vRet(m_Data.cols(), MAT_Tdouble);
	MatOp::mul(&vRet, &mTemp, &m_vObj);
	for (i = 0; i < vRet.length(); i ++)
		pRegressor->m_vCoeff.data.fl[i] = (float)vRet.data.db[i];
	
	delete pmInverse;

	return 1;
}
int	LinearRegressorTrainer::itrainAccuracy (Vec& vcoeff)
{
	int i;

	Mat mB(m_Data.cols(), m_Data.cols(), MAT_Tdouble);
	MatOp::trAA(&mB, &m_Data);
	LUDecomposition lu(&mB);
	
	Mat mI(m_Data.cols(), m_Data.cols(), MAT_Tdouble);
	mI.zero();
	for (i = 0; i < mI.rows(); i ++)
		mI.data.db[i][i] = 1.0;
	Mat* pmInverse = lu.solve (&mI);
	if (pmInverse==0)
		return 0;
	
	Mat mTemp(m_Data.cols(), m_Data.rows(), MAT_Tdouble);
	Mat mTr = m_Data.transposed();
	MatOp::mul(&mTemp, pmInverse, &mTr);

	vcoeff.create (m_Data.cols(), MAT_Tdouble);
	MatOp::mul(&vcoeff, &mTemp, &m_vObj);

	delete pmInverse;

	return 1;
}

MachineABC* LinearRegressorTrainer::train(DataSet* pData)
{
	RegressorTrainerABC::setData (pData);
	LinearRegressor* pRegressor=new LinearRegressor;
	if (!train (pRegressor)){
		delete pRegressor;
		pRegressor=0;
	}
	return pRegressor;
}
int LinearRegressorTrainer::trainAccuracy(DataSet* pData, Vec& vcoeff)
{
	RegressorTrainerABC::setData (pData);
	return itrainAccuracy (vcoeff);
}

void LinearRegressorTrainer::release ()
{
	m_Data.release();
	m_vObj.release();
}

//////////////////////////////////////////////////////////////////////////
void QuadraticRegressorTrainer::setData (const Mat* pmExamples, const Vec* pvObj)
{
	int nInNum = pmExamples->cols();
	int nConNum = nInNum * 2 + nInNum*(nInNum-1) / 2 + 1;
	
	m_Data.create(pmExamples->rows(), nConNum, MAT_Tdouble);
	int i, j;
	for (i = 0; i < pmExamples->rows(); i ++)
	{
		Vec vTemp(pmExamples->data.db[i], nInNum, MAT_Tdouble);
		const Vec* pvConverted = QuadraticRegressor::iConvert(&vTemp);
		for (j = 0; j < nConNum-1; j ++)
			m_Data.data.db[i][j] = pvConverted->data.db[j];
		m_Data.data.db[i][j] = 1;
	}
	m_vObj = *pvObj;
}

MachineABC* QuadraticRegressorTrainer::train(DataSet* pData)
{
	RegressorTrainerABC::setData (pData);
	QuadraticRegressor* pRegressor=new QuadraticRegressor;
	if (!LinearRegressorTrainer::train (pRegressor))
	{
		delete pRegressor; pRegressor=0;
	}
	return pRegressor;
}
//////////////////////////////////////////////////////////////////////////

MachineABC* LeastSquareTrainer::train(DataSet* pData)
{
	LeastSquareMachine* pMachine=new LeastSquareMachine;

	Matd* m_pmData=new Matd((double**)pData->m_pprData, pData->m_nCount, pData->m_nDim);
	Vecd* m_pvObj=new Vecd((double*)pData->m_prCls, pData->m_nCount);
	
	Mat mB(m_pmData->cols(), m_pmData->cols(), MAT_Tdouble);
	MatOp::trAA(&mB, m_pmData);
	mB.invert();
		
	Mat mTemp(m_pmData->cols(), m_pmData->rows(), MAT_Tdouble);
	Mat mTr = m_pmData->transposed();
	MatOp::mul(&mTemp, &mB, &mTr);
	
	pMachine->m_vCoeff.create (m_pmData->cols(), MAT_Tfloat);
	Vec vRet(m_pmData->cols(), MAT_Tdouble);
	MatOp::mul(&vRet, &mTemp, m_pvObj);
	for (int i = 0; i < vRet.length(); i ++)
		pMachine->m_vCoeff.data.fl[i] = (float)vRet.data.db[i];
	delete m_pmData;
	delete m_pvObj;
	return pMachine;
}

}