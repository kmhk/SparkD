#include "ObjectBoostDetectorABC.h"
#include "ScaleXY.h"
#include "cvlibbase/Inc/MatOperation.h"
#include "cvlibbase/Inc/PtrArray.h"
#include "cvlibbase/Inc/CVlibUtil.h"
#include "cvlibbase/Inc/ColorSpace.h"
#include "objdetect_utils.h"

namespace cvlib
{

	static const uchar glikelihood[] = { 255, 128, 64, 32, 16, 8, 4, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	ObjectBoostDetectorABC::Param::Param()
	{
		rStepX = 0.05f;
		rStepY = 0.05f;
		rScale = 1.1f;
		nMinSize = 24;
		nMaxSize = 2048;
		nAutoSave = 0;
		nOverlap = 1;
		nExportSamFeature = 0;
		nOnlyOne = 0;
		szClassifierPath[0] = 0;
	}

	void ObjectBoostDetectorABC::Param::copy(ObjectBoostDetectorABC::Param* pParam)
	{
		ObjectBoostDetectorABC::Param* pOther = (ObjectBoostDetectorABC::Param*)pParam;
		rStepX = pOther->rStepX;
		rStepY = pOther->rStepY;
		rScale = pOther->rScale;
		nMinSize = pOther->nMinSize;
		nMaxSize = pOther->nMaxSize;
		nAutoSave = pOther->nAutoSave;
		nOverlap = pOther->nOverlap;
		nOnlyOne = pOther->nOnlyOne;
		nExportSamFeature = pOther->nExportSamFeature;
		strcpy(szClassifierPath, pOther->szClassifierPath);
	}

	//////////////////////////////////////////////////////////////////////////
	/*int ObjectBoostDetectorABC::compareRegion(const void* pResult1, const void* pResult2)
	{
		RectVal* p1 = *(RectVal**)pResult1;
		RectVal* p2 = *(RectVal**)pResult2;
		if (p1->rval > p2->rval)
			return 1;
		return -1;
	}*/

	ObjectBoostDetectorABC::ObjectBoostDetectorABC()
	{
		m_pCascadeClassifier = NULL;
		m_fCreate = 0;

		m_pTrainingData = NULL;
		m_nTrainIdx = 0;
		m_szFilename = NULL;
	}

	ObjectBoostDetectorABC::~ObjectBoostDetectorABC()
	{
		release();
	}

	int		ObjectBoostDetectorABC::create(ObjectBoostDetectorABC::Param* pBParam)
	{
		if (flagCreate())
			release();
		if (pBParam)
			m_Param.copy(pBParam);
		m_pCascadeClassifier = newCascadeClassifierFromFolder(m_Param.szClassifierPath);
		m_fCreate = 0;
		if (m_pCascadeClassifier == NULL || m_pCascadeClassifier->getSize() == 0)
			return 0;
		m_fCreate = 1;
		return 1;
	}

	int		ObjectBoostDetectorABC::createFromOneFile(const char* szDataPath)
	{
		if (flagCreate())
			release();
		m_pCascadeClassifier = newCascadeClassifierFromOneFile(szDataPath);
		m_fCreate = 0;
		if (m_pCascadeClassifier == NULL || m_pCascadeClassifier->getSize() == 0)
			return 0;
		m_fCreate = 1;
		return 1;
	}

	int ObjectBoostDetectorABC::create(CascadeClassifierObjectDetectABC* pCascadeClassifier)
	{
		if (flagCreate())
			release();
		m_pCascadeClassifier = pCascadeClassifier;
		m_fCreate = 2;
		return 1;
	}

	void	ObjectBoostDetectorABC::release()
	{
		if (m_fCreate == 1 && m_pCascadeClassifier)
			delete m_pCascadeClassifier;
		m_pCascadeClassifier = NULL;

		m_pyramidOfImage.release();
		m_pyramidOfMask.release();
	}

	CascadeClassifierObjectDetectABC* ObjectBoostDetectorABC::newCascadeClassifierFromOneFile(const char* szFilename)
	{
		XFileDisk file;
		bool fFlag = file.open(szFilename, "rb");
		if (!fFlag)
			return 0;
		CascadeClassifierObjectDetectABC* newClassifier = newCascadeClassifierFromOneFile(&file);
		file.close();
		return newClassifier;
	}

	bool ObjectBoostDetectorABC::isIncludeRect(const RectVal& r, int nWidth, int nHeight)
	{
		if ((r.x >= 0) && ((r.x + r.width) <= nWidth)
			&& (r.y >= 0) && ((r.y + r.height) <= nHeight))
			return true;
		else
			return false;
	}

	void ObjectBoostDetectorABC::removeOverlappedRegion(PtrArray* pDetectedRegion)
	{
		RectVal *preRegion, *nextRegion;
		int nRectCount = pDetectedRegion->getSize();

		for (int i = 0; i < nRectCount; i++)
		{
			for (int j = i + 1; j < nRectCount; j++)
			{
				int	nSx, nFx, nSy, nFy;
				int	nOverLap = 0;
				preRegion = (RectVal*)pDetectedRegion->getAt(i);
				nextRegion = (RectVal*)pDetectedRegion->getAt(j);
				nSx = MAX(preRegion->x, nextRegion->x);
				nSy = MAX(preRegion->y, nextRegion->y);
				nFx = MIN(preRegion->x + preRegion->width, nextRegion->x + nextRegion->width);
				nFy = MIN(preRegion->y + preRegion->height, nextRegion->y + nextRegion->height);
				if (nSx < nFx && nSy < nFy)
				{
					nOverLap = (nFx - nSx) * (nFy - nSy);

					nSx = preRegion->width * preRegion->height;
					nFx = nextRegion->width * nextRegion->height;

					if (nOverLap * 100 > MIN(nSx, nFx) * 49)
					{
						delete (Rect*)pDetectedRegion->getAt(j);
						pDetectedRegion->removeAt(j);
						j--;
						nRectCount--;
					}
				}
			}
		}
	}

	void ObjectBoostDetectorABC::markOverlappedRegion(PtrArray* pDetectedRegion)
	{
		PtrArray tempArray;
		tempArray.copy(*pDetectedRegion);
		Rect *preRegion, *nextRegion;
		int nRectCount = pDetectedRegion->getSize();

		for (int i = 0; i < nRectCount; i++)
		{
			for (int j = i + 1; j < nRectCount; j++)
			{
				int	nSx, nFx, nSy, nFy;
				int	nOverLap = 0;
				preRegion = ((RectVal*)tempArray.getAt(i));
				nextRegion = ((RectVal*)tempArray.getAt(j));
				nSx = MAX(preRegion->x, nextRegion->x);
				nSy = MAX(preRegion->y, nextRegion->y);
				nFx = MIN(preRegion->x + preRegion->width, nextRegion->x + nextRegion->width);
				nFy = MIN(preRegion->y + preRegion->height, nextRegion->y + nextRegion->height);
				if (nSx < nFx && nSy < nFy)
				{
					nOverLap = (nFx - nSx) * (nFy - nSy);

					nSx = preRegion->width * preRegion->height;
					nFx = nextRegion->width * nextRegion->height;

					if (nOverLap * 100 > MIN(nSx, nFx) * 49)
					{
						((RectVal*)tempArray.getAt(j))->nFlag = 1;
						tempArray.removeAt(j);
						j--;
						nRectCount--;
					}
				}
			}
		}
	}

	void ObjectBoostDetectorABC::saveNonobject(const Mat& image, const Rect& rect, int& nFirst, int& nTotalNum)
	{
		static char szSuffix[CVLIB_PATH_MAX] = "";
		if (nFirst)
		{
			time_t	ltime;
			time(&ltime);
			sprintf(szSuffix, "%d", (int)ltime);
			nFirst = 0;
		}
		char szFilename[CVLIB_PATH_MAX];
		sprintf(szFilename, "%sbootstrap/nonobject_%s_%d.png", m_Param.szClassifierPath, szSuffix, nTotalNum);
		cvutil::mkDir(szFilename);
		Mat outMat;
		image.subMat(rect, outMat);
		if (!outMat.toImage(szFilename, CVLIB_IMG_FORMAT_PNG))
			assert(false);
		nTotalNum++;
	}

	void ObjectBoostDetectorABC::freePtrList(PtrList& rectlist)
	{
		if (rectlist.flagCreate())
		{
			while (rectlist.count())
			{
				rectlist.setPosition(PtrList::POS_BEGIN, 0);
				RectVal* pRect = (RectVal*)rectlist.data();
				delete pRect;
				rectlist.remove();
			}
			rectlist.release();
		}
	}

	void ObjectBoostDetectorABC::sortRegions(PtrArray& rectList) const
	{
		//qsort(rectList.getData(), rectList.getSize(), sizeof(RectVal*), compareRegion);
		int nDetectedRegionCount = (int)rectList.getSize();

		for (int i = 0; i < nDetectedRegionCount - 1; i++)
		{
			RectVal* pRegion1 = (RectVal*)rectList[i];
			for (int j = i + 1; j < nDetectedRegionCount; j++)
			{
				RectVal* pRegion2 = (RectVal*)rectList[j];
				if (pRegion1->rval < pRegion2->rval)
				{
					RectVal xTemp = *pRegion1;
					*pRegion1 = *pRegion2;
					*pRegion2 = xTemp;
				}
			}
		}

	}

	int	ObjectBoostDetectorABC::extract(const Mat& image, PtrList& blobList)
	{
		Mat gray;
		ColorSpace::RGBtoGray(image, gray);
		Mat*	pmatImages;
		int		nCountImage;
		int		ii;

		m_nObjectHeight = m_pCascadeClassifier->m_nObjectHeight;
		m_nObjectWidth = m_pCascadeClassifier->m_nObjectWidth;

		int		nStepX = cvutil::round(m_Param.rStepX * m_nObjectWidth);
		int		nStepY = cvutil::round(m_Param.rStepY * m_nObjectHeight);

		PtrArray rectList;
		int nFirst = 1;

		// COLOR image to gray image
		objutils::pyramidOfImage(gray, &pmatImages, &nCountImage, m_Param.rScale, m_Param.nMinSize, MAX(m_nObjectHeight, m_nObjectWidth), m_Param.nMaxSize);
		// matching architecture
		int nTotalNum = 0;
		m_nScanedNum = 0;
		for (ii = 0; ii < nCountImage; ii++)
		{
			const Mat& mat = pmatImages[ii];

			if (!mat.isValid())
				continue;

			float rRate = (float)gray.rows() / (float)mat.rows();
			Rect	rect(0, 0, m_nObjectWidth, m_nObjectHeight);
			if (MAX(m_nObjectWidth, m_nObjectHeight)*rRate > m_Param.nMaxSize)
				continue;

			extractLayerFeature(mat);

			while (rect.y + m_nObjectHeight <= mat.rows())
			{
				rect.x = 0;
				while (rect.x + m_nObjectWidth <= mat.cols())
				{
					m_nScanedNum++;
					float rConf;
					int rEval = m_pCascadeClassifier->eval(updateFeature(rect.x, rect.y, mat), rect.x, rect.y, &rConf);
					if (rEval == 1)
					{
						if (m_Param.nAutoSave == 1)
							saveNonobject(mat, rect, nFirst, nTotalNum);
						if (m_Param.nExportSamFeature == 1)
						{
							if (!exportSampleFeature(rect.x, rect.y))
							{
								if (rectList.getSize())
								{
									blobList.create();
									for (ii = 0; ii < rectList.getSize(); ii++)
										blobList.add(rectList[ii]);
								}
								rectList.removeAll();

								// free memory
								for (ii = nCountImage - 1; ii >= 0; ii--)
									pmatImages[ii].release();
								delete[] pmatImages;
								return 0;
							}
						}
						RectVal* pResultElem = new RectVal();
						pResultElem->x = cvutil::round(rect.x * rRate);
						pResultElem->y = cvutil::round(rect.y * rRate);
						pResultElem->width = cvutil::round(rect.width * rRate);
						pResultElem->height = cvutil::round(rect.height * rRate);
						pResultElem->rval = rConf;
						pResultElem->nFlag = 0;
						if (isIncludeRect(*pResultElem, gray.cols(), gray.rows()))
							rectList.add(pResultElem);
						else
							delete pResultElem;
					}
					rect.x += nStepX;
				}
				rect.y += nStepY;
			}
		}

		sortRegions(rectList);

		if (m_Param.nOverlap)
			removeOverlappedRegion(&rectList);
		else
			markOverlappedRegion(&rectList);

		if (rectList.getSize())
		{
			if (m_Param.nOnlyOne == 1)
			{
				int nMaxWidth = 0, nIdx = 0;
				for (ii = 0; ii < rectList.getSize(); ii++)
				{
					RectVal* pResult = (RectVal*)rectList[ii];
					if (nMaxWidth < pResult->width && pResult->nFlag == 0)
					{
						nMaxWidth = pResult->width;
						nIdx = ii;
					}
				}
				for (ii = 0; ii < rectList.getSize(); ii++)
				{
					if (ii != nIdx)
					{
						delete (RectVal*)rectList[ii];
						rectList[ii] = NULL;
					}
				}
			}
			if (!blobList.flagCreate())
				blobList.create();
			for (ii = 0; ii < rectList.getSize(); ii++)
			{
				if (rectList[ii])
					blobList.add(rectList[ii]);
			}
		}
		rectList.removeAll();

		// free memory
		if (pmatImages)
		{
			for (ii = nCountImage - 1; ii >= 0; ii--)
				pmatImages[ii].release();
			delete[] pmatImages;
		}

		return 1;
	}

	static inline int max_value(int n1, int n2)
	{
		return n1 > n2 ? n1 : n2;
	}
	static inline float max_value(float n1, float n2)
	{
		return n1 > n2 ? n1 : n2;
	}

	int	ObjectBoostDetectorABC::extract(const Mat& image, const Mat& mask, PtrList& blobList, Mat* heatmap)
	{
		Mat gray;
		ColorSpace::RGBtoGray(image, gray);
		Mat*	pmatImages;
		Mat*	pmask_images = 0;
		int		nCountImage;
		int		ii;

		m_nObjectHeight = m_pCascadeClassifier->m_nObjectHeight;
		m_nObjectWidth = m_pCascadeClassifier->m_nObjectWidth;

		int		nStepX = cvutil::round(m_Param.rStepX * m_nObjectWidth);
		int		nStepY = cvutil::round(m_Param.rStepY * m_nObjectHeight);

		PtrArray rectList;
		int nFirst = 1;

		int total_layer = m_pCascadeClassifier->getActiveSize();

		// COLOR image to gray image
		objutils::pyramidOfImage(gray, &pmatImages, &nCountImage, m_Param.rScale, m_Param.nMinSize, MAX(m_nObjectHeight, m_nObjectWidth), m_Param.nMaxSize);
		if (mask.isValid())
			objutils::pyramidOfImage(mask, &pmask_images, &nCountImage, m_Param.rScale, m_Param.nMinSize, MAX(m_nObjectHeight, m_nObjectWidth), m_Param.nMaxSize);

		if (heatmap) {
			heatmap->create(image.size(), MAT_Tuchar);
			heatmap->zero();
		}
		Mat stretched_heatmap, subheatmap;

		// matching architecture
		int nTotalNum = 0;
		m_nScanedNum = 0;
		for (ii = 0; ii < nCountImage; ii++)
		{
			const Mat& mat = pmatImages[ii];
			if (!mat.isValid())
				continue;

			float rRate = (float)gray.rows() / (float)mat.rows();
			Rect	rect(0, 0, m_nObjectWidth, m_nObjectHeight);
			if (MAX(m_nObjectWidth, m_nObjectHeight)*rRate > m_Param.nMaxSize)
				continue;

			extractLayerFeature(mat);

			if (heatmap) {
				subheatmap.create(mat.size(), MAT_Tuchar);
				subheatmap.zero();
			}

			if (!mask.isValid())
			{
				while (rect.y + m_nObjectHeight <= mat.rows())
				{
					rect.x = 0;
					while (rect.x + m_nObjectWidth <= mat.cols())
					{
						m_nScanedNum++;
						int passed_layers = 0;
						float rConf;
						int rEval = m_pCascadeClassifier->eval(updateFeature(rect.x, rect.y, mat), rect.x, rect.y, &rConf, &passed_layers);
						if (rEval == 1)
						{
							if (m_Param.nAutoSave == 1)
								saveNonobject(mat, rect, nFirst, nTotalNum);
							if (m_Param.nExportSamFeature == 1)
							{
								if (!exportSampleFeature(rect.x, rect.y))
								{
									if (rectList.getSize())
									{
										blobList.create();
										for (ii = 0; ii < rectList.getSize(); ii++)
											blobList.add(rectList[ii]);
									}
									rectList.removeAll();

									// free memory
									for (ii = nCountImage - 1; ii >= 0; ii--)
										pmatImages[ii].release();
									delete[] pmatImages;
									return 0;
								}
							}
							RectVal* pResultElem = new RectVal();
							pResultElem->x = cvutil::round(rect.x * rRate);
							pResultElem->y = cvutil::round(rect.y * rRate);
							pResultElem->width = cvutil::round(rect.width * rRate);
							pResultElem->height = cvutil::round(rect.height * rRate);
							pResultElem->rval = rConf;
							pResultElem->nFlag = 0;
							if (isIncludeRect(*pResultElem, gray.cols(), gray.rows()))
								rectList.add(pResultElem);
							else
								delete pResultElem;
						}
						if (heatmap) {
							subheatmap.data.ptr[rect.y + m_nObjectHeight / 2][rect.x + m_nObjectWidth / 2] = glikelihood[total_layer - passed_layers];
						}
						rect.x += nStepX;
					}
					rect.y += nStepY;
				}
			}
			else
			{
				const Mat& mat_mask = pmask_images[ii];
				while (rect.y + m_nObjectHeight <= mat.rows())
				{
					rect.x = 0;
					while (rect.x + m_nObjectWidth <= mat.cols())
					{
						if (mat_mask.data.ptr[rect.y + m_nObjectHeight / 2][rect.x + m_nObjectWidth / 2] > 50)
						{
							m_nScanedNum++;
							float rConf;
							int passed_layers = 0;
							int rEval = m_pCascadeClassifier->eval(updateFeature(rect.x, rect.y, mat), rect.x, rect.y, &rConf, &passed_layers);
							if (rEval == 1)
							{
								if (m_Param.nAutoSave == 1)
									saveNonobject(mat, rect, nFirst, nTotalNum);
								if (m_Param.nExportSamFeature == 1)
								{
									if (!exportSampleFeature(rect.x, rect.y))
									{
										if (rectList.getSize())
										{
											blobList.create();
											for (ii = 0; ii < rectList.getSize(); ii++)
												blobList.add(rectList[ii]);
										}
										rectList.removeAll();

										// free memory
										for (ii = nCountImage - 1; ii >= 0; ii--)
											pmatImages[ii].release();
										delete[] pmatImages;
										return 0;
									}
								}
								RectVal* pResultElem = new RectVal();
								pResultElem->x = cvutil::round(rect.x * rRate);
								pResultElem->y = cvutil::round(rect.y * rRate);
								pResultElem->width = cvutil::round(rect.width * rRate);
								pResultElem->height = cvutil::round(rect.height * rRate);
								pResultElem->rval = rConf;
								pResultElem->nFlag = 0;
								if (isIncludeRect(*pResultElem, gray.cols(), gray.rows()))
									rectList.add(pResultElem);
								else
									delete pResultElem;
							}
							if (heatmap) {
								subheatmap.data.ptr[rect.y + m_nObjectHeight / 2][rect.x + m_nObjectHeight / 2] = glikelihood[total_layer - passed_layers];
							}
						}
						rect.x += nStepX;
					}
					rect.y += nStepY;
				}
			}
			if (heatmap)
			{
				ip::resize(subheatmap, stretched_heatmap, image.cols(), image.rows(), ip::INTER_LINEAR);
				int elemcount = image.rows()*image.cols();
				uchar* heatmap_ptr = heatmap->data.ptr[0];
				const uchar* stretched_heatmap_ptr = stretched_heatmap.data.ptr[0];
				for (int k = 0; k < elemcount; k++, heatmap_ptr++, stretched_heatmap_ptr++) {
					*heatmap_ptr = max_value(*heatmap_ptr, *stretched_heatmap_ptr);
				}
			}
		}
		
		sortRegions(rectList);

		if (m_Param.nOverlap)
			removeOverlappedRegion(&rectList);
		else
			markOverlappedRegion(&rectList);

		if (rectList.getSize())
		{
			if (m_Param.nOnlyOne == 1)
			{
				int nMaxWidth = 0, nIdx = 0;
				for (ii = 0; ii < rectList.getSize(); ii++)
				{
					RectVal* pResult = (RectVal*)rectList[ii];
					if (nMaxWidth < pResult->width && pResult->nFlag == 0)
					{
						nMaxWidth = pResult->width;
						nIdx = ii;
					}
				}
				for (ii = 0; ii < rectList.getSize(); ii++)
				{
					if (ii != nIdx)
					{
						delete (RectVal*)rectList[ii];
						rectList[ii] = NULL;
					}
				}
			}
			if (!blobList.flagCreate())
				blobList.create();
			for (ii = 0; ii < rectList.getSize(); ii++)
			{
				if (rectList[ii])
					blobList.add(rectList[ii]);
			}
		}
		rectList.removeAll();

		// free memory
		if (pmatImages)
		{
			for (ii = nCountImage - 1; ii >= 0; ii--)
				pmatImages[ii].release();
			delete[] pmatImages;
		}
		if (pmask_images)
		{
			for (ii = nCountImage - 1; ii >= 0; ii--)
				pmask_images[ii].release();
			delete[] pmask_images;
		}
		return 1;
	}
	int	ObjectBoostDetectorABC::extractHeatmap(const Mat& image, Mat* heatmap, Mat* sizeheatmap)
	{
		Mat gray;
		ColorSpace::RGBtoGray(image, gray);
		Mat*	pmatImages;
		int		nCountImage;
		int		ii;

		m_nObjectHeight = m_pCascadeClassifier->m_nObjectHeight;
		m_nObjectWidth = m_pCascadeClassifier->m_nObjectWidth;
		int		nStepX = cvutil::round(m_Param.rStepX * m_nObjectWidth);
		int		nStepY = cvutil::round(m_Param.rStepY * m_nObjectHeight);

		int nFirst = 1;

		int total_layer = m_pCascadeClassifier->getActiveSize();

		// COLOR image to gray image
		objutils::pyramidOfImage(gray, &pmatImages, &nCountImage, m_Param.rScale, m_Param.nMinSize, MAX(m_nObjectHeight, m_nObjectWidth));

		if (heatmap) {
			heatmap->create(image.size(), MAT_Tuchar);
			heatmap->zero();
		}
		if (sizeheatmap) {
			sizeheatmap->create(image.size(), MAT_Tfloat);
			sizeheatmap->zero();
		}
		Mat stretched_heatmap, subheatmap;
		Mat stretched_sizeheatmap, sub_sizeheatmap;

		int max_obj_size = MAX(image.cols(), image.rows());

		// matching architecture
		int nTotalNum = 0;
		m_nScanedNum = 0;
		for (ii = 0; ii < nCountImage; ii++)
		{
			const Mat& mat = pmatImages[ii];

			float rRate = (float)gray.rows() / (float)mat.rows();
			Rect	rect(0, 0, m_nObjectWidth, m_nObjectHeight);
			if (MAX(m_nObjectWidth, m_nObjectHeight)*rRate > m_Param.nMaxSize)
				continue;

			extractLayerFeature(mat);

			if (heatmap) {
				subheatmap.create(mat.size(), MAT_Tuchar);
				subheatmap.zero();
			}
			if (sizeheatmap) {
				sub_sizeheatmap.create(mat.size(), MAT_Tfloat);
				sub_sizeheatmap.zero();
			}

			//if (!mask.isValid())
			{
				while (rect.y + m_nObjectHeight <= mat.rows())
				{
					rect.x = 0;
					while (rect.x + m_nObjectWidth <= mat.cols())
					{
						m_nScanedNum++;
						int passed_layers = 0;
						float rConf;
						int rEval = m_pCascadeClassifier->eval(updateFeature(rect.x, rect.y, mat), rect.x, rect.y, &rConf, &passed_layers);
						if (rEval == 1)
						{
							if (m_Param.nAutoSave == 1)
								saveNonobject(mat, rect, nFirst, nTotalNum);
							if (m_Param.nExportSamFeature == 1)
							{
								if (!exportSampleFeature(rect.x, rect.y))
								{
									// free memory
									for (ii = nCountImage - 1; ii >= 0; ii--)
										pmatImages[ii].release();
									delete[] pmatImages;
									return 0;
								}
							}
						}
						if (heatmap) {
							subheatmap.data.ptr[rect.y + m_nObjectHeight / 2][rect.x + m_nObjectWidth / 2] = glikelihood[total_layer - passed_layers];
						}
						if (sizeheatmap) {
							sub_sizeheatmap.data.fl[rect.y + m_nObjectHeight / 2][rect.x + m_nObjectWidth / 2] = m_nObjectWidth;
						}
						rect.x += nStepX;
					}
					rect.y += nStepY;
				}
			}

			if (heatmap)
			{
				ip::resize(subheatmap, stretched_heatmap, image.cols(), image.rows(), ip::INTER_LINEAR);
				int elemcount = image.rows()*image.cols();
				uchar* heatmap_ptr = heatmap->data.ptr[0];
				const uchar* stretched_heatmap_ptr = stretched_heatmap.data.ptr[0];
				for (int k = 0; k < elemcount; k++, heatmap_ptr++, stretched_heatmap_ptr++) {
					*heatmap_ptr = max_value(*heatmap_ptr, *stretched_heatmap_ptr);
				}
			}
			if (sizeheatmap)
			{
				ip::resize(sub_sizeheatmap, stretched_sizeheatmap, image.cols(), image.rows(), ip::INTER_LINEAR);
				int elemcount = image.rows()*image.cols();
				float* sizeheatmap_ptr = sizeheatmap->data.fl[0];
				const float* stretched_sizeheatmap_ptr = stretched_sizeheatmap.data.fl[0];
				for (int k = 0; k < elemcount; k++, sizeheatmap_ptr++, stretched_sizeheatmap_ptr++) {
					*sizeheatmap_ptr = max_value(*sizeheatmap_ptr, *stretched_sizeheatmap_ptr);
				}
			}
		}

		// free memory
		if (pmatImages)
		{
			for (ii = nCountImage - 1; ii >= 0; ii--)
				pmatImages[ii].release();
			delete[] pmatImages;
		}

		return 1;
	}
	int	ObjectBoostDetectorABC::extractCameraMode(const Mat& image, const Mat& mask, PtrList& blobList, Mat* heatmap)
	{
		Mat gray;
		ColorSpace::RGBtoGray(image, gray);

		int		ii;

		m_nObjectHeight = m_pCascadeClassifier->m_nObjectHeight;
		m_nObjectWidth = m_pCascadeClassifier->m_nObjectWidth;
		int		nStepX = cvutil::round(m_Param.rStepX * m_nObjectWidth);
		int		nStepY = cvutil::round(m_Param.rStepY * m_nObjectHeight);

		PtrArray rectList;
		int nFirst = 1;

		int total_layer = m_pCascadeClassifier->getActiveSize();

		// COLOR image to gray image
		objutils::pyramidOfImage(gray, m_pyramidOfImage, m_Param.rScale, m_Param.nMinSize, MAX(m_nObjectHeight, m_nObjectWidth), m_Param.nMaxSize);
		if (mask.isValid())
			objutils::pyramidOfImage(mask, m_pyramidOfMask, m_Param.rScale, m_Param.nMinSize, MAX(m_nObjectHeight, m_nObjectWidth), m_Param.nMaxSize);

		if (heatmap) {
			heatmap->create(image.size(), MAT_Tuchar);
			heatmap->zero();
		}
		Mat stretched_heatmap, subheatmap;

		// matching architecture
		int nTotalNum = 0;
		m_nScanedNum = 0;
		for (ii = 0; ii < m_pyramidOfImage.m_countImages; ii++)
		{
			const Mat& mat = m_pyramidOfImage.m_psubimages[ii];
			if (!mat.isValid())
				continue;

			float rRate = (float)gray.rows() / (float)mat.rows();
			Rect	rect(0, 0, m_nObjectWidth, m_nObjectHeight);
			if (MAX(m_nObjectWidth, m_nObjectHeight)*rRate > m_Param.nMaxSize)
				continue;

			extractLayerFeature(mat);

			if (heatmap) {
				subheatmap.create(mat.size(), MAT_Tuchar);
				subheatmap.zero();
			}

			if (!mask.isValid())
			{
				while (rect.y + m_nObjectHeight <= mat.rows())
				{
					rect.x = 0;
					while (rect.x + m_nObjectWidth <= mat.cols())
					{
						m_nScanedNum++;
						int passed_layers = 0;
						float rConf;
						int rEval = m_pCascadeClassifier->eval(updateFeature(rect.x, rect.y, mat), rect.x, rect.y, &rConf, &passed_layers);
						if (rEval == 1)
						{
							/*if (m_Param.nAutoSave == 1)
								saveNonobject(mat, rect, nFirst, nTotalNum);
							if (m_Param.nExportSamFeature == 1)
							{
								if (!exportSampleFeature(rect.x, rect.y))
								{
									if (rectList.getSize())
									{
										blobList.create();
										for (ii = 0; ii < rectList.getSize(); ii++)
											blobList.add(rectList[ii]);
									}
									rectList.removeAll();
									return 0;
								}
							}*/
							RectVal* pResultElem = new RectVal();
							pResultElem->x = cvutil::round(rect.x * rRate);
							pResultElem->y = cvutil::round(rect.y * rRate);
							pResultElem->width = cvutil::round(rect.width * rRate);
							pResultElem->height = cvutil::round(rect.height * rRate);
							pResultElem->rval = rConf;
							pResultElem->nFlag = 0;
							if (isIncludeRect(*pResultElem, gray.cols(), gray.rows()))
								rectList.add(pResultElem);
							else
								delete pResultElem;
						}
						if (heatmap) {
							subheatmap.data.ptr[rect.y + m_nObjectHeight / 2][rect.x + m_nObjectHeight / 2] = glikelihood[total_layer - passed_layers];
						}
						rect.x += nStepX;
					}
					rect.y += nStepY;
				}
			}
			else
			{
				const Mat& mat_mask = m_pyramidOfMask.m_psubimages[ii];
				while (rect.y + m_nObjectHeight <= mat.rows())
				{
					rect.x = 0;
					while (rect.x + m_nObjectWidth <= mat.cols())
					{
						if (mat_mask.data.ptr[rect.y + m_nObjectHeight / 2][rect.x + m_nObjectWidth / 2] > 50)
						{
							m_nScanedNum++;
							float rConf;
							int passed_layers = 0;
							int rEval = m_pCascadeClassifier->eval(updateFeature(rect.x, rect.y, mat), rect.x, rect.y, &rConf, &passed_layers);
							if (rEval == 1)
							{
								if (m_Param.nAutoSave == 1)
									saveNonobject(mat, rect, nFirst, nTotalNum);
								if (m_Param.nExportSamFeature == 1)
								{
									if (!exportSampleFeature(rect.x, rect.y))
									{
										if (rectList.getSize())
										{
											blobList.create();
											for (ii = 0; ii < rectList.getSize(); ii++)
												blobList.add(rectList[ii]);
										}
										rectList.removeAll();
										return 0;
									}
								}
								RectVal* pResultElem = new RectVal();
								pResultElem->x = cvutil::round(rect.x * rRate);
								pResultElem->y = cvutil::round(rect.y * rRate);
								pResultElem->width = cvutil::round(rect.width * rRate);
								pResultElem->height = cvutil::round(rect.height * rRate);
								pResultElem->rval = rConf;
								pResultElem->nFlag = 0;
								if (isIncludeRect(*pResultElem, gray.cols(), gray.rows()))
									rectList.add(pResultElem);
								else
									delete pResultElem;
							}
							if (heatmap) {
								subheatmap.data.ptr[rect.y + m_nObjectHeight / 2][rect.x + m_nObjectHeight / 2] = glikelihood[total_layer - passed_layers];
							}
						}
						rect.x += nStepX;
					}
					rect.y += nStepY;
				}
			}
			if (heatmap)
			{
				ip::resize(subheatmap, stretched_heatmap, image.cols(), image.rows(), ip::INTER_LINEAR);
				int elemcount = image.rows()*image.cols();
				uchar* heatmap_ptr = heatmap->data.ptr[0];
				const uchar* stretched_heatmap_ptr = stretched_heatmap.data.ptr[0];
				for (int k = 0; k < elemcount; k++, heatmap_ptr++, stretched_heatmap_ptr++) {
					*heatmap_ptr = max_value(*heatmap_ptr, *stretched_heatmap_ptr);
				}
			}
		}

		sortRegions(rectList);

		if (m_Param.nOverlap)
			removeOverlappedRegion(&rectList);
		else
			markOverlappedRegion(&rectList);

		if (rectList.getSize())
		{
			if (m_Param.nOnlyOne == 1)
			{
				int nMaxWidth = 0, nIdx = 0;
				for (ii = 0; ii < rectList.getSize(); ii++)
				{
					RectVal* pResult = (RectVal*)rectList[ii];
					if (nMaxWidth < pResult->width && pResult->nFlag == 0)
					{
						nMaxWidth = pResult->width;
						nIdx = ii;
					}
				}
				for (ii = 0; ii < rectList.getSize(); ii++)
				{
					if (ii != nIdx)
					{
						delete (RectVal*)rectList[ii];
						rectList[ii] = NULL;
					}
				}
			}
			if (!blobList.flagCreate())
				blobList.create();
			for (ii = 0; ii < rectList.getSize(); ii++)
			{
				if (rectList[ii])
					blobList.add(rectList[ii]);
			}
		}
		rectList.removeAll();

		return 1;
	}

	int	ObjectBoostDetectorABC::extract(const Mat& image, Vector<Rect>& regions)
	{
		PtrList bloblist;
		bloblist.create();
		int ret = extract(image, bloblist);
		if (bloblist.flagCreate())
		{
			regions.setSize(bloblist.count());
			for (int i = 0; i < bloblist.count(); i++)
			{
				bloblist.setPosition(PtrList::POS_BEGIN, i);
				RectVal* pRect = (RectVal*)bloblist.data();
				regions[i] = *pRect;
			}
		}
		freePtrList(bloblist);
		return ret;
	}

	void ObjectBoostDetectorABC::evaluate(const Mat& _image, int& pass_count, int& total_count)
	{
		m_nObjectHeight = m_pCascadeClassifier->m_nObjectHeight;
		m_nObjectWidth = m_pCascadeClassifier->m_nObjectWidth;
		Mat image;
		ip::resize(_image, image, m_nObjectWidth, m_nObjectHeight, ip::INTER_LINEAR);
		extractLayerFeature(image);
		float rconf;
		pass_count = 0;
		total_count = m_pCascadeClassifier->getActiveSize();
		int rEval = m_pCascadeClassifier->eval(updateFeature(0, 0, image), 0, 0, &rconf, &pass_count);
	}

}