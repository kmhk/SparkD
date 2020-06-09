/*!
 * \file	statistics.cpp
 * \ingroup math
 * \brief
 * \author
 */

#include "Vec.h"
#include "Statistics.h"
#include "MatOperation.h"

namespace cvlib
{

	void meanVector(const Vec* pvecExamples, int nNum, Vec* pvecMean, const Vec* pvecWeight /*= NULL*/)
	{
		assert(pvecMean->type() == MAT_Tfloat);
		assert(pvecMean->length() == pvecExamples->length());

		int i, j, nDim = pvecExamples->length();
		const Vec* pTemp = pvecExamples;
		const float* prWeights = pvecWeight ? pvecWeight->data.fl : NULL;

		Vec		vMean(nDim, MAT_Tdouble);
		TYPE type = pvecExamples->type();

		double	rSum = 0.0;
		pvecMean->zero();
		vMean.zero();
		if (prWeights == NULL)
		{
			switch (type)
			{
			case MAT_Tfloat:
				for (i = 0; i < nNum; i++, pTemp++)
				{
					for (j = 0; j < nDim; j++)
						vMean.data.db[j] += pTemp->data.fl[j];
				}
				break;
			case MAT_Tbyte:
				for (i = 0; i < nNum; i++, pTemp++)
				{
					for (j = 0; j < nDim; j++)
						vMean.data.db[j] += pTemp->data.ptr[j];
				}
				break;
			default:
				assert(false);
			}
			for (j = 0; j < nDim; j++)
			{
				vMean.data.db[j] /= nNum;
				pvecMean->data.fl[j] = (float)vMean.data.db[j];
			}
		}
		else
		{
			switch (type)
			{
			case MAT_Tfloat:
				for (i = 0; i < nNum; i++, pTemp++)
				{
					for (j = 0; j < nDim; j++)
						vMean.data.db[j] += (double)pTemp->data.fl[j] * (double)prWeights[i];
					rSum += prWeights[i];
				}
				break;
			case MAT_Tbyte:
				for (i = 0; i < nNum; i++, pTemp++)
				{
					for (j = 0; j < nDim; j++)
						vMean.data.db[j] += (double)pTemp->data.ptr[j] * (double)prWeights[i];
					rSum += prWeights[i];
				}
				break;
			default:
				assert(false);
			}
			for (j = 0; j < nDim; j++)
			{
				vMean.data.db[j] /= rSum;
				pvecMean->data.fl[j] = (float)vMean.data.db[j];
			}
		}
		vMean.release();
	}

	void covariance(const Vec* pvecExamples, int nNum, Mat* pmatCov, Vec* pvecMean, const Vec* pvecWeight/* = NULL*/)
	{
		int		nDim = pvecExamples->length();
		Vec*	pMean;
		if (pvecMean == NULL)
		{
			pMean = new Vec(nDim, MAT_Tfloat);
			meanVector(pvecExamples, nNum, pMean, pvecWeight);
		}
		else
		{
			pMean = pvecMean;
		}

		variation(pvecExamples, nNum, pmatCov, pMean, pvecWeight);

		float rSum = 0.0f;
		if (pvecWeight)
		{
			for (int i = 0; i < nNum; i++)
				rSum += pvecWeight->data.fl[i];
		}
		else
			rSum = (float)nNum;
		MatOp::mul(pmatCov, 1.0f / (float)rSum, pmatCov);

		if (pvecMean == NULL)
		{
			pMean->release();
			delete pMean;
		}
	}

	void variation(const Vec* pvecExamples, int nNum, Mat* pmatVar, Vec* pvecMean, const Vec* pvecWeight/* = NULL*/)
	{
		int		nDim = pvecExamples->length();
		Vec*	pMean;
		if (pvecMean == NULL)
		{
			pMean = new Vec(nDim, MAT_Tfloat);
			meanVector(pvecExamples, nNum, pMean, pvecWeight);
		}
		else
		{
			pMean = pvecMean;
		}
		int		i, j;
		const Vec*	pTemp;
		Vec		vecExample;
		//	Mat		matM;
		TYPE	type = pvecExamples->type();
		vecExample.create(nDim, MAT_Tfloat);
		float* prExample = vecExample.data.fl;
		const float* prWeights = pvecWeight ? pvecWeight->data.fl : NULL;
		//	matM.create (nDim, nDim, MAT_Tfloat);
		pmatVar->zero();
		for (i = 0; i < nNum; i++)
		{
			pTemp = pvecExamples + i;
			switch (type)
			{
			case MAT_Tfloat:
				for (j = 0; j < nDim; j++)
					prExample[j] = pTemp->data.fl[j] - pMean->data.fl[j];
				break;
			case MAT_Tbyte:
				for (j = 0; j < nDim; j++)
					prExample[j] = pTemp->data.ptr[j] - pMean->data.fl[j];
				break;
			default:
				assert(false);
			}

			//		matM.zero();
			switch (pmatVar->type())
			{
			case MAT_Tfloat:
			{
				float** pprVar = pmatVar->data.fl;
				if (prWeights)
				{
					for (int k = 0; k < nDim; k++)
					{
						for (j = 0; j < nDim; j++)
						{
							pprVar[k][j] += prExample[k] * prExample[j] * prWeights[i];
						}
					}
				}
				else
				{
					for (int k = 0; k < nDim; k++)
					{
						for (j = 0; j < nDim; j++)
						{
							pprVar[k][j] += prExample[k] * prExample[j];
						}
					}
				}
			}
			break;
			case MAT_Tdouble:
			{
				double** pprVar = pmatVar->data.db;
				if (prWeights)
				{
					for (int k = 0; k < nDim; k++)
					{
						for (j = 0; j < nDim; j++)
						{
							pprVar[k][j] += prExample[k] * prExample[j] * prWeights[i];
						}
					}
				}
				else
				{
					for (int k = 0; k < nDim; k++)
					{
						for (j = 0; j < nDim; j++)
						{
							pprVar[k][j] += prExample[k] * prExample[j];
						}
					}
				}
			}
			break;
			default:
			{
				assert(false);
			}
			}
			//		VecMulTrVec (&vecExample, &vecExample, &matM);
			//		if (pvecWeight)
			//			RealMulMat (pvecWeight->data.fl[i], &matM, &matM);
			//		MatOp::add (pmatVar, &matM, pmatVar);
		}

		//	matM.release();
		vecExample.release();

		if (pvecMean == NULL)
			delete pMean;
	}

}