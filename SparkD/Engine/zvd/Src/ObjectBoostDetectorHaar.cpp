#include "ObjectBoostDetectorHaar.h"
#include "IntegralImage.h"
#include "GaussianFilter.h"
#include "HaarTrainingData.h"

namespace cvlib
{

	void ObjectBoostDetectorHaar::release()
	{
		ObjectBoostDetectorABC::release();
		m_mSum.release();
		m_mTilted.release();
		m_mSqSum.release();
	}

	const void* ObjectBoostDetectorHaar::extractLayerFeature(const Mat& image)
	{
		assert(image.channels() == 1);
		//Mat m;
		//ip::gaussian(image, m, 0.5f);
		ip::integralImage(image, m_mSum, m_mTilted, m_mSqSum);

		m_data.pnSum = m_mSum.data.i[0];
		m_data.pnTilted = m_mTilted.data.i[0];
		m_prSqSum = m_mSqSum.data.db[0];

		((CascadeClassifierObjectHaar*)m_pCascadeClassifier)->setStep(image.cols() + 1);

		return &m_data;
	}

	const void* ObjectBoostDetectorHaar::updateFeature(int nX, int nY, const Mat& image)
	{
		Rect normrect = Rect(nX + 1, nY + 1, m_nObjectWidth - 2, m_nObjectHeight - 2);
		int p0, p1, p2, p3;
		double rArea, rValsum;
		double dValsqsum;
		CVLib_SUM_OFFSETS(p0, p1, p2, p3, normrect, image.cols() + 1);
		rArea = (m_nObjectHeight + 1) * (m_nObjectWidth + 1);
		rValsum = m_data.pnSum[p0] - m_data.pnSum[p1] - m_data.pnSum[p2] + m_data.pnSum[p3];
		dValsqsum = m_prSqSum[p0] - m_prSqSum[p1] - m_prSqSum[p2] + m_prSqSum[p3];
		m_data.rNormFactor = (float)sqrt(rArea * dValsqsum - rValsum * rValsum);
		return &m_data;
	}

	int ObjectBoostDetectorHaar::exportSampleFeature(int nX, int nY)
	{
		if (m_nTrainIdx >= m_pTrainingData->m_nMaxnum - 1 || m_nTrainIdx >= m_nLimIdx - 1)
			return 0;
		HaarTrainingData* pHaarTrainData = (HaarTrainingData*)m_pTrainingData;
		Mati mSum((int*)pHaarTrainData->m_mFeature.data.fl[m_nTrainIdx], m_nObjectHeight + 1, m_nObjectWidth + 1);
		Mati mTilted((int*)pHaarTrainData->m_mTilted.data.i[m_nTrainIdx], m_nObjectHeight + 1, m_nObjectWidth + 1);
		pHaarTrainData->m_vNormfactor.data.fl[m_nTrainIdx] = m_data.rNormFactor;
		for (int iH = 0; iH < m_nObjectHeight + 1; iH++)
		{
			for (int iW = 0; iW < m_nObjectWidth + 1; iW++)
			{
				mSum[iH][iW] = m_mSum[iH + nY][iW + nX];
				mTilted[iH][iW] = m_mTilted[iH + nY][iW + nX];
			}
		}
		m_nTrainIdx++;
		return 1;
	}

	CascadeClassifierObjectDetectABC* ObjectBoostDetectorHaar::newCascadeClassifierFromFolder(const char* szFilename)
	{
		CascadeClassifierObjectHaar* pCascadeClassifier = new CascadeClassifierObjectHaar;
		pCascadeClassifier->fromFile(szFilename);
		pCascadeClassifier->fastProcessing();
		return pCascadeClassifier;
	}

	CascadeClassifierObjectDetectABC* ObjectBoostDetectorHaar::newCascadeClassifierFromOneFile(const char* szFilename)
	{
		CascadeClassifierObjectHaar* pCascadeClassifier = new CascadeClassifierObjectHaar;
		pCascadeClassifier->fromFile(szFilename);
		pCascadeClassifier->fastProcessing();
		return pCascadeClassifier;
	}

	CascadeClassifierObjectDetectABC* ObjectBoostDetectorHaar::newCascadeClassifierFromOneFile(XFile* pfile)
	{
		CascadeClassifierObjectHaar* pCascadeClassifier = new CascadeClassifierObjectHaar;
		pCascadeClassifier->fromFile(pfile);
		pCascadeClassifier->fastProcessing();
		return pCascadeClassifier;
	}

}