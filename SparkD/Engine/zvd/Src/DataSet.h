
/*!
 * \file    DataSet.h
 * \ingroup	base
 * \brief	
 * \author	
 */

#pragma once

#include "cvlibbase/Inc/Mat.h"
#include "cvlibbase/Inc/XFile.h"
#include "cvlibbase/Inc/IILog.h"
#include <stdio.h>
#include "ClassifierABC.h"

namespace cvlib
{

	class DataSet;

	class CVLIB_DECLSPEC mlTrainerABC : public IILog
	{
	public:
		virtual MachineABC* train(DataSet*) { return 0; }
	};

	class CVLIB_DECLSPEC DataSet : public Object
	{
	public:
		enum
		{
			DS_ALL,
			DS_ADDRESS
		};
	public:
		int		m_nCount;
		int		m_nDim;
		double*	m_prCls;
		double**	m_pprData;
		double*	m_prWeights;

		DataSet();
		DataSet(int nDim, int nCount, int nMode = DS_ALL);
		DataSet(const DataSet& other);
		DataSet(const char* szFilename);
		~DataSet();

		virtual int		create(int nDim, int nCount, int nMode = DS_ALL);
		int			flagCreate() const { return m_nFlagCreate; }

		virtual void	release();

		virtual	int		fromFile(const char* szFilename);
		virtual int		fromFile(XFile* pFile);
		virtual	int		toFile(const char* szFilename) const;
		virtual int		toFile(XFile* pFile) const;

		Matd*			toMat();
		void			fromMat(Matd* pM);
		void			fromMat(Mat& mdata, Vec& vclass);

		virtual int		getSampleNum(int nClass);
		virtual int		getClassNum();
		virtual int		statistics(double* prMin, double* prMax, double* prMean);

		inline int		dims() const { return m_nDim; }
		inline int		count() const { return m_nCount; }
		inline int		getMaxCount() const { return m_nMaxCount; }
		int				copyFrom(const DataSet& other);
		int				add(double* prData, double rCls, double rWei = 0.0);

		void			setData(int nIdx, const double* prData, double rCls, double rWei = 0.0);
		void			sortClsIdx();
	protected:
		int		m_nMode;
		int		m_nFlagCreate;
		int		m_nMaxCount;
		int		m_nGrowBy;

		void	init();
	};

	CVLIB_DECLSPEC DataSet* createDataSetCol(const DataSet& src);
	CVLIB_DECLSPEC DataSet* createDataSetForSubSamples(const DataSet& src, const int* idxs, int count);

}