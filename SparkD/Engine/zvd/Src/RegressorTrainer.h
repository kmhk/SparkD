
/*!
 * \file
 * \brief
 * \author
 */

#pragma once

#include "cvlibbase/Inc/_cvlibbase.h"
#include "ClassifierABC.h"
#include "DataSet.h"

namespace cvlib
{

	class CVLIB_DECLSPEC RegressorTrainerABC : public mlTrainerABC
	{
	public:
		RegressorTrainerABC() {};
		virtual ~RegressorTrainerABC() {};

		virtual void setData(const Mat* pmExamples, const Vec* pvObj) = 0;
		virtual int	train(ClassifierABC* pMachine) = 0;
		virtual void setData(const DataSet* pData);
		virtual void release() = 0;
	};

	class CVLIB_DECLSPEC LinearRegressorTrainer : public RegressorTrainerABC
	{
	public:
		LinearRegressorTrainer() {};
		virtual ~LinearRegressorTrainer();

		virtual void setData(const Mat* pmExamples, const Vec* pvObj);
		virtual int	train(ClassifierABC* pMachine);
		virtual MachineABC* train(DataSet* pData);
		virtual int trainAccuracy(DataSet* pData, Vec& vcoeff);
		virtual void release();
	protected:
		int	itrainAccuracy(Vec& vcoeff);
		Mat m_Data;
		Vec m_vObj;
	};

	class CVLIB_DECLSPEC QuadraticRegressorTrainer : public LinearRegressorTrainer
	{
	public:
		virtual void setData(const Mat* pmExamples, const Vec* pvObj);
		virtual MachineABC* train(DataSet* pData);
	};

	class CVLIB_DECLSPEC LeastSquareTrainer : public IILog
	{
	public:
		LeastSquareTrainer() {}
		virtual ~LeastSquareTrainer() {}
		virtual MachineABC* train(DataSet* pData);
	};

}