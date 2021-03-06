#include "CascadeClassifierObjectHaar.h"
#include "FeatureVec.h"
#include "IntegralImage.h"
#include "HaarTrainingData.h"

namespace cvlib
{

	HaarElem::HaarElem()
	{
		memset(&m_feature, 0, sizeof(m_feature));
		memset(&m_fastfeature, 0, sizeof(m_fastfeature));
	}

	HaarElem::HaarElem(const HaarElem& other)
	{
		m_feature = other.m_feature;
		m_fastfeature = other.m_fastfeature;
	}

	HaarElem::HaarElem(Rect& rect, int& nCode, int& nRadius)
	{
		// 	m_Haar
	}

	HaarElem::~HaarElem()
	{
	}

	int HaarElem::fromFile(XFile* pFile)
	{
		pFile->read(&m_feature, sizeof(STHaarFeature), 1);
		return 1;
	}

	int HaarElem::toFile(XFile* pFile) const
	{
		pFile->write(&m_feature, sizeof(STHaarFeature), 1);
		return 1;
	}

	FeaElementABC* HaarElem::clone() const
	{
		return (FeaElementABC*)(new HaarElem(*this));
	}

	float HaarElem::eval(const void* arg) const
	{
		SHaarBasisData* pData = (SHaarBasisData*)arg;
		float rVal = evalFastHaarFeature(&m_fastfeature, pData->pnSum, pData->pnTilted);
		rVal = (fabs(pData->rNormFactor) < 0.0001) ? 0.0F : (rVal / pData->rNormFactor);
		return rVal;
	}

	float HaarElem::eval(TrainingData* pTrainingData, int nIdx) const
	{
		HaarTrainingData* pData = (HaarTrainingData*)pTrainingData;
		float rVal = evalFastHaarFeature(&m_fastfeature, pData->m_mFeature.data.i[nIdx], pData->m_mTilted.data.i[nIdx]);
		rVal = (fabs(pData->m_vNormfactor.data.fl[nIdx]) < 0.0001) ? 0.0F : (rVal / pData->m_vNormfactor.data.fl[nIdx]);
		return rVal;
	}

	float HaarElem::dist(const void* arg1, const void* arg2) const
	{
		FeatureVec* pFea1 = (FeatureVec*)arg1;
		FeatureVec* pFea2 = (FeatureVec*)arg2;
		float rEval1 = evalFastHaarFeature(&m_fastfeature, pFea1->m_pmSum->data.i[0], pFea1->m_pmTilted->data.i[0]);
		float rEval2 = evalFastHaarFeature(&m_fastfeature, pFea2->m_pmSum->data.i[0], pFea2->m_pmTilted->data.i[0]);
		return (float)fabs(rEval1 - rEval2);
	}

	float HaarElem::extract(const void* arg)
	{
		FeatureVec* pFea = (FeatureVec*)arg;
		assert(pFea->type() == MAT_Tuchar);

		if (!pFea->m_pmSum)
			pFea->calcIntegralImage(false, true);
		float rEval = evalFastHaarFeature(&m_fastfeature, pFea->m_pmSum->data.i[0], pFea->m_pmTilted->data.i[0]);
		return rEval;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void	CascadeClassifierObjectHaar::Stage::fastProcessing()
	{
		for (int i = 0; i < m_nCount; i++)
		{
			Stump* pStumpClassifier = (Stump*)m_ppStumpClassifier[i];
			HaarElem* pElem = (HaarElem*)pStumpClassifier->getFeaElement();
			convertToFastHaarFeature(&pElem->m_feature, &pElem->m_fastfeature, 1, m_nObjectWidth + 1, m_nObjectHeight + 1);
		}
	}

	void	CascadeClassifierObjectHaar::Stage::release()
	{
		StageClassifierObjectDetectABC::release();
	}
	void CascadeClassifierObjectHaar::Stage::setStep(int nStep)
	{
		m_nStep = nStep;
		m_prevx = m_prevy = -1;
		/*for (int i = 0; i < m_nCount; i++)
		{
			Stump* pStumpClassifier = (Stump*)m_ppStumpClassifier[i];
			HaarElem* pHaar = (HaarElem*)pStumpClassifier->getFeaElement();
			//memcpy(&xTemp, &pHaar->m_fastfeature, sizeof(SFastHaarFeature));
			//SFastHaarFeature* pFastHaar = &xTemp;

			convertToFastHaarFeature(&pHaar->m_feature, &pHaar->m_fastfeature, 1, m_nStep);
		}*/
	}

	float CascadeClassifierObjectHaar::Stage::eval(const void* arg, int nX, int nY)
	{
		float rEval = 0.0f;
		HaarElem::SHaarBasisData* pData = (HaarElem::SHaarBasisData*)arg;
		int nOriginStep = m_nObjectWidth + 1;

		for (int i = 0; i < m_nCount; i++)
		{
			Stump* pStumpClassifier = (Stump*)m_ppStumpClassifier[i];
			HaarElem* pHaar = (HaarElem*)pStumpClassifier->getFeaElement();

			
			/*
			memcpy(&m_prevFastFeature, &pHaar->m_feature, sizeof(STHaarFeature));
			SFastHaarFeature* pFastHaar = &pHaar->m_fastfeature;
			if (!pFastHaar->tilted)
			{
				for (int k = 0; k < CVLIB_HAAR_FEATURE_MAX; k++)
				{
					Rect r = m_prevFastFeature.rect[k].r;
					r.x += nX;
					r.y += nY;
					CVLib_SUM_OFFSETS(pFastHaar->rect[k].p0, pFastHaar->rect[k].p1,
					pFastHaar->rect[k].p2, pFastHaar->rect[k].p3,
					r, m_nStep)
				}
			}
			else
			{
				for (int k = 0; k < CVLIB_HAAR_FEATURE_MAX; k++)
				{
					Rect r = m_prevFastFeature.rect[k].r;
					r.x += nX;
					r.y += nY;
					CVLib_TILTED_OFFSETS(pFastHaar->rect[k].p0, pFastHaar->rect[k].p1,
					pFastHaar->rect[k].p2, pFastHaar->rect[k].p3,
					r, m_nStep)
				}
			}*/
			SFastHaarFeature* pFastHaar = &pHaar->m_fastfeature;
			if (m_prevx == -1 || m_prevy == -1)
			{
				//memcpy(pFastHaar, &pHaar->m_fastfeature, sizeof(SFastHaarFeature));
				{
					if (!pFastHaar->tilted)
					{
						for (int k = 0; k < CVLIB_HAAR_FEATURE_MAX; k++)
						{
							SFastHaarFeature::SFastHaarFeature_internal* pr = &pFastHaar->rect[k];
							if (fabs(pr->weight) < 0.0001)
								break;
							STHaarFeature::STHaarFeature_internal* pf = &pHaar->m_feature.rect[k];
							Rect rect = pf->r;
							rect.x += nX;
							rect.y += nY;
							CVLib_SUM_OFFSETS(pr->p0, pr->p1, pr->p2, pr->p3, rect, m_nStep)
						}
					}
					else
					{
						for (int k = 0; k < CVLIB_HAAR_FEATURE_MAX; k++)
						{
							SFastHaarFeature::SFastHaarFeature_internal* pr = &pFastHaar->rect[k];
							if (fabs(pr->weight) < 0.0001)
								break;
							STHaarFeature::STHaarFeature_internal* pf = &pHaar->m_feature.rect[k];
							Rect rect = pf->r;
							rect.x += nX;
							rect.y += nY;
							CVLib_TILTED_OFFSETS(pr->p0, pr->p1, pr->p2, pr->p3, rect, m_nStep)
						}
					}
				}
			}
			else
			{
				int offset = (nX - m_prevx) + (nY - m_prevy)*m_nStep;
				for (int k = 0; k < CVLIB_HAAR_FEATURE_MAX; k++)
				{
					SFastHaarFeature::SFastHaarFeature_internal* pr = &pFastHaar->rect[k];
					pr->p0 += offset;
					pr->p1 += offset;
					pr->p2 += offset;
					pr->p3 += offset;
				}
			}
			float rVal = evalFastHaarFeature(pFastHaar, pData->pnSum, pData->pnTilted);
			rVal = (fabs(pData->rNormFactor) < 0.0001) ? 0.0F : (rVal / pData->rNormFactor);
			rEval += pStumpClassifier->eval(rVal);
		}
		m_prevx = nX;
		m_prevy = nY;
		return rEval;
	}

	float CascadeClassifierObjectHaar::Stage::eval(TrainingData* pTrainingData, int nIdx, int nX, int nY)
	{
		float rEval = 0.0f;
		SFastHaarFeature xTemp;
		int* pnSum = pTrainingData->m_mFeature.data.i[nIdx];
		int* pnTilted = ((HaarTrainingData*)pTrainingData)->m_mTilted.data.i[nIdx];
		float rNormFactor = ((HaarTrainingData*)pTrainingData)->m_vNormfactor.data.fl[nIdx];
		
		int nOriginStep = m_nObjectWidth + 1;
		for (int i = 0; i < m_nCount; i++)
		{
			Stump* pStumpClassifier = (Stump*)m_ppStumpClassifier[i];
			HaarElem* pHaar = (HaarElem*)pStumpClassifier->getFeaElement();
			memcpy(&xTemp, &pHaar->m_fastfeature, sizeof(SFastHaarFeature));
			SFastHaarFeature* pFastHaar = &xTemp;

			{
				if (!pFastHaar->tilted)
				{
					for (int k = 0; k < CVLIB_HAAR_FEATURE_MAX; k++)
					{
						if (fabs(pFastHaar->rect[k].weight) < 0.0001)
							break;
						Rect rect;
						rect.width = pFastHaar->rect[k].p1 - pFastHaar->rect[k].p0;
						rect.height = (pFastHaar->rect[k].p2 - pFastHaar->rect[k].p0) / nOriginStep;
						rect.x = pFastHaar->rect[k].p0 % nOriginStep;
						rect.y = pFastHaar->rect[k].p0 / nOriginStep;
						rect.x += nX;
						rect.y += nY;
						CVLib_SUM_OFFSETS(pFastHaar->rect[k].p0, pFastHaar->rect[k].p1,
							pFastHaar->rect[k].p2, pFastHaar->rect[k].p3,
							rect, m_nStep)
					}
				}
				else
				{
					for (int k = 0; k < CVLIB_HAAR_FEATURE_MAX; k++)
					{
						if (fabs(pFastHaar->rect[k].weight) < 0.0001)
							break;
						Rect rect;
						rect.width = (pFastHaar->rect[k].p2 - pFastHaar->rect[k].p0) / (nOriginStep + 1);
						rect.height = (pFastHaar->rect[k].p1 - pFastHaar->rect[k].p0) / (nOriginStep - 1);
						rect.x = pFastHaar->rect[k].p0 % nOriginStep;
						rect.y = pFastHaar->rect[k].p0 / nOriginStep;
						rect.x += nX;
						rect.y += nY;
						CVLib_TILTED_OFFSETS(pFastHaar->rect[k].p0, pFastHaar->rect[k].p1,
							pFastHaar->rect[k].p2, pFastHaar->rect[k].p3,
							rect, m_nStep)
					}
				}
			}
			float rVal = evalFastHaarFeature(pFastHaar, pnSum, pnTilted);
			rVal = (fabs(rNormFactor) < 0.0001) ? 0.0F : (rVal / rNormFactor);
			rEval += pStumpClassifier->eval(rVal);
		}
		return rEval;
	}

	void CascadeClassifierObjectHaar::setStep(int nStep)
	{
		for (int i = 0; i < getSize(); i++)
		{
			Stage* pStageClassifier = (Stage*)getAt(i);
			pStageClassifier->setStep(nStep);
		}
	}


	int	CascadeClassifierObjectHaar::toC(const char* szFilename) const
	{
		return 1;
	}

}
