/*!
 * \file	haarFeature.h
 * \ingroup VFR
 * \brief	
 * \author	
 */

#include "HaarFeature.h"

namespace cvlib
{

	void HAARLIST::Push_back(STHaarFeature* pTHaarFeature)
	{
		HaarFeatureList.add(pTHaarFeature);
	}
	STHaarFeature* HAARLIST::getAt(int nIdx)
	{
		if (nIdx < 0 || nIdx >= HaarFeatureList.getSize())
			return NULL;

		return (STHaarFeature*)HaarFeatureList.getAt(nIdx);
	}
	void HAARLIST::release()
	{
		for (int i = HaarFeatureList.getSize() - 1; i >= 0; i--)
		{
			delete ((STHaarFeature*)HaarFeatureList.getAt(i));
		}
		HaarFeatureList.removeAll();
	}

	SHaarFeatures*	initPossibleFeatures(Size winsize, int nMode, int nSymmetric)
	{
		SHaarFeatures* features = NULL;
		STHaarFeature* pNewHaarFeature;

		HAARLIST	aTHaarFeatureList;

		int i;
		int mode = nMode, symmetric = nSymmetric;
		int s0 = 36; /* minimum total area size of basic haar feature     */
		int s1 = 12; /* minimum total area size of tilted haar features 2 */
		int s2 = 18; /* minimum total area size of tilted haar features 3 */
		int s3 = 24; /* minimum total area size of tilted haar features 4 */

		int x = 0;
		int y = 0;
		int dx = 0;
		int dy = 0;

		//     factor = ((float) winsize.width) * winsize.height / (15 * 15); //(24 * 24);
#if 0    
		s0 = (int)(s0 * factor);
		s1 = (int)(s1 * factor);
		s2 = (int)(s2 * factor);
		s3 = (int)(s3 * factor);
#else
		s0 = 1;
		s1 = 1;
		s2 = 1;
		s3 = 1;
#endif

		for (x = 0; x < winsize.width; x++)
		{
			for (y = 0; y < winsize.height; y++)
			{
				for (dx = 1; dx <= winsize.width; dx++)
				{
					for (dy = 1; dy <= winsize.height; dy++)
					{
						// haar_x2
						if ((x + dx * 2 <= winsize.width) && (y + dy <= winsize.height)) {
							if (dx * 2 * dy < s0) continue;
							if (!symmetric || (x + x + dx * 2 <= winsize.width)) {
								pNewHaarFeature = haarFeature("haar_x2",
									x, y, dx * 2, dy, -1,
									x + dx, y, dx, dy, +2);
								aTHaarFeatureList.Push_back(pNewHaarFeature);
							}
						}

						// haar_y2
						if ((x + dx * 2 <= winsize.height) && (y + dy <= winsize.width)) {
							if (dx * 2 * dy < s0) continue;
							if (!symmetric || (y + y + dy <= winsize.width)) {
								pNewHaarFeature = haarFeature("haar_y2",
									y, x, dy, dx * 2, -1,
									y, x + dx, dy, dx, +2);
								aTHaarFeatureList.Push_back(pNewHaarFeature);
							}
						}

						// haar_x3
						if ((x + dx * 3 <= winsize.width) && (y + dy <= winsize.height)) {
							if (dx * 3 * dy < s0) continue;
							if (!symmetric || (x + x + dx * 3 <= winsize.width)) {
								pNewHaarFeature = haarFeature("haar_x3",
									x, y, dx * 3, dy, -1,
									x + dx, y, dx, dy, +3);
								aTHaarFeatureList.Push_back(pNewHaarFeature);
							}
						}

						// haar_y3
						if ((x + dx * 3 <= winsize.height) && (y + dy <= winsize.width)) {
							if (dx * 3 * dy < s0) continue;
							if (!symmetric || (y + y + dy <= winsize.width)) {
								pNewHaarFeature = haarFeature("haar_y3",
									y, x, dy, dx * 3, -1,
									y, x + dx, dy, dx, +3);
								aTHaarFeatureList.Push_back(pNewHaarFeature);
							}
						}

						if (mode != 0 /*BASIC*/) {
							// haar_x4
							if ((x + dx * 4 <= winsize.width) && (y + dy <= winsize.height)) {
								if (dx * 4 * dy < s0) continue;
								if (!symmetric || (x + x + dx * 4 <= winsize.width)) {
									pNewHaarFeature = haarFeature("haar_x4",
										x, y, dx * 4, dy, -1,
										x + dx, y, dx * 2, dy, +2);
									aTHaarFeatureList.Push_back(pNewHaarFeature);
								}
							}

							// haar_y4
							if ((x + dx * 4 <= winsize.height) && (y + dy <= winsize.width)) {
								if (dx * 4 * dy < s0) continue;
								if (!symmetric || (y + y + dy <= winsize.width)) {
									pNewHaarFeature = haarFeature("haar_y4",
										y, x, dy, dx * 4, -1,
										y, x + dx, dy, dx * 2, +2);
									aTHaarFeatureList.Push_back(pNewHaarFeature);
								}
							}
						}

						// x2_y2
						if ((x + dx * 2 <= winsize.width) && (y + dy * 2 <= winsize.height)) {
							if (dx * 4 * dy < s0) continue;
							if (!symmetric || (x + x + dx * 2 <= winsize.width)) {
								pNewHaarFeature = haarFeature("haar_x2_y2",
									x, y, dx * 2, dy * 2, -1,
									x, y, dx, dy, +2,
									x + dx, y + dy, dx, dy, +2);
								aTHaarFeatureList.Push_back(pNewHaarFeature);
							}
						}

						if (mode != 0 /*BASIC*/) {
							// point
							if ((x + dx * 3 <= winsize.width) && (y + dy * 3 <= winsize.height)) {
								if (dx * 9 * dy < s0) continue;
								if (!symmetric || (x + x + dx * 3 <= winsize.width)) {
									pNewHaarFeature = haarFeature("haar_point",
										x, y, dx * 3, dy * 3, -1,
										x + dx, y + dy, dx, dy, +9);
									aTHaarFeatureList.Push_back(pNewHaarFeature);
								}
							}
						}

						if (mode == 2 /*ALL*/) {
							// tilted haar_x2                                      (x, y, w, h, b, weight)
							if ((x + 2 * dx <= winsize.width) && (y + 2 * dx + dy <= winsize.height) && (x - dy >= 0)) {
								if (dx * 2 * dy < s1) continue;

								if (!symmetric || (x <= (winsize.width / 2))) {
									pNewHaarFeature = haarFeature("tilted_haar_x2",
										x, y, dx * 2, dy, -1,
										x, y, dx, dy, +2);
									aTHaarFeatureList.Push_back(pNewHaarFeature);
								}
							}

							// tilted haar_y2                                      (x, y, w, h, b, weight)
							if ((x + dx <= winsize.width) && (y + dx + 2 * dy <= winsize.height) && (x - 2 * dy >= 0)) {
								if (dx * 2 * dy < s1) continue;

								if (!symmetric || (x <= (winsize.width / 2))) {
									pNewHaarFeature = haarFeature("tilted_haar_y2",
										x, y, dx, 2 * dy, -1,
										x, y, dx, dy, +2);
									aTHaarFeatureList.Push_back(pNewHaarFeature);
								}
							}

							// tilted haar_x3                                   (x, y, w, h, b, weight)
							if ((x + 3 * dx <= winsize.width) && (y + 3 * dx + dy <= winsize.height) && (x - dy >= 0)) {
								if (dx * 3 * dy < s2) continue;

								if (!symmetric || (x <= (winsize.width / 2))) {
									pNewHaarFeature = haarFeature("tilted_haar_x3",
										x, y, dx * 3, dy, -1,
										x + dx, y + dx, dx, dy, +3);
									aTHaarFeatureList.Push_back(pNewHaarFeature);
								}
							}

							// tilted haar_y3                                      (x, y, w, h, b, weight)
							if ((x + dx <= winsize.width) && (y + dx + 3 * dy <= winsize.height) && (x - 3 * dy >= 0)) {
								if (dx * 3 * dy < s2) continue;

								if (!symmetric || (x <= (winsize.width / 2))) {
									pNewHaarFeature = haarFeature("tilted_haar_y3",
										x, y, dx, 3 * dy, -1,
										x - dy, y + dy, dx, dy, +3);
									aTHaarFeatureList.Push_back(pNewHaarFeature);
								}
							}


							// tilted haar_x4                                   (x, y, w, h, b, weight)
							if ((x + 4 * dx <= winsize.width) && (y + 4 * dx + dy <= winsize.height) && (x - dy >= 0)) {
								if (dx * 4 * dy < s3) continue;

								if (!symmetric || (x <= (winsize.width / 2))) {
									pNewHaarFeature = haarFeature("tilted_haar_x4",
										x, y, dx * 4, dy, -1,
										x + dx, y + dx, dx * 2, dy, +2);
									aTHaarFeatureList.Push_back(pNewHaarFeature);
								}
							}

							// tilted haar_y4                                      (x, y, w, h, b, weight)
							if ((x + dx <= winsize.width) && (y + dx + 4 * dy <= winsize.height) && (x - 4 * dy >= 0)) {
								if (dx * 4 * dy < s3) continue;

								if (!symmetric || (x <= (winsize.width / 2))) {
									pNewHaarFeature = haarFeature("tilted_haar_y4",
										x, y, dx, 4 * dy, -1,
										x - dy, y + dy, dx, 2 * dy, +2);
									aTHaarFeatureList.Push_back(pNewHaarFeature);
								}
							}


							/*

							  // tilted point
							  if ( (x+dx*3 <= winsize.width - 1) && (y+dy*3 <= winsize.height - 1) && (x-3*dy>= 0)) {
							  if (dx*9*dy < 36) continue;
							  if (!symmetric || (x <= (winsize.width / 2) ))  {
								haarFeature = haarFeature( "tilted_haar_point",
									x, y,    dx*3, dy*3, -1,
									x, y+dy, dx  , dy,   +9 );
									aTHaarFeatureList.Push_back(haarFeature);
							  }
							  }
							*/
						}
					}
				}
			}
		}

		features = (SHaarFeatures*)malloc(sizeof(SHaarFeatures) + (sizeof(STHaarFeature) + sizeof(SFastHaarFeature)) * aTHaarFeatureList.getSize());
		features->feature = (STHaarFeature*)(features + 1);
		features->fastfeature = (SFastHaarFeature*)(features->feature + aTHaarFeatureList.getSize());
		features->count = aTHaarFeatureList.getSize();
		features->winsize = winsize;
		for (i = 0; i < aTHaarFeatureList.getSize(); i++)
			memcpy(features->feature + i, aTHaarFeatureList.getAt(i), sizeof(STHaarFeature));
		aTHaarFeatureList.release();

		convertToFastHaarFeature(features->feature, features->fastfeature,
			features->count, (winsize.width + 1), (winsize.height + 1));

		return features;
	}

	STHaarFeature*	haarFeature(const char* desc,
		int x0, int y0, int w0, int h0, float wt0,
		int x1, int y1, int w1, int h1, float wt1,
		int x2 /*= 0*/, int y2 /*= 0*/, int w2 /*= 0*/, int h2 /*= 0*/, float wt2 /*= 0*/)
	{
		STHaarFeature* phf;

		assert(CVLIB_HAAR_FEATURE_MAX >= 3);
		assert(strlen(desc) < CVLIB_HAAR_FEATURE_DESC_MAX);

		phf = new STHaarFeature;

		strcpy(&(phf->desc[0]), desc);
		phf->tilted = (phf->desc[0] == 't');

		phf->rect[0].r.x = x0;
		phf->rect[0].r.y = y0;
		phf->rect[0].r.width = w0;
		phf->rect[0].r.height = h0;
		phf->rect[0].weight = wt0;

		phf->rect[1].r.x = x1;
		phf->rect[1].r.y = y1;
		phf->rect[1].r.width = w1;
		phf->rect[1].r.height = h1;
		phf->rect[1].weight = wt1;

		phf->rect[2].r.x = x2;
		phf->rect[2].r.y = y2;
		phf->rect[2].r.width = w2;
		phf->rect[2].r.height = h2;
		phf->rect[2].weight = wt2;

		return phf;
	}

	float evalFastHaarFeature(const SFastHaarFeature* feature, int* sum, int* tilted)
	{
		int* img = 0;
		int i = 0;
		float ret = 0.0F;

		assert(feature);

		img = (feature->tilted) ? tilted : sum;

		assert(img);
		for (i = 0; i < CVLIB_HAAR_FEATURE_MAX; i++)
		{
			if (fabs(feature->rect[i].weight) != 0.0f)
			{
				ret += feature->rect[i].weight *
					(img[feature->rect[i].p0] - img[feature->rect[i].p1] -
						img[feature->rect[i].p2] + img[feature->rect[i].p3]);
			}
		}

		return ret;
	}

	void loadHaarFeature(STHaarFeature* feature, FILE* fp)
	{
		int nrect;
		int j;
		int tmp;
		int weight;

		nrect = 0;
		fscanf(fp, "%d", &nrect);

		assert(nrect <= CVLIB_HAAR_FEATURE_MAX);

		for (j = 0; j < nrect; j++)
		{
			fscanf(fp, "%d %d %d %d %d %d",
				&(feature->rect[j].r.x),
				&(feature->rect[j].r.y),
				&(feature->rect[j].r.width),
				&(feature->rect[j].r.height),
				&tmp, &weight);
			feature->rect[j].weight = (float)weight;
		}
		for (j = nrect; j < CVLIB_HAAR_FEATURE_MAX; j++)
		{
			feature->rect[j].r.x = 0;
			feature->rect[j].r.y = 0;
			feature->rect[j].r.width = 0;
			feature->rect[j].r.height = 0;
			feature->rect[j].weight = 0.0f;
		}
		fscanf(fp, "%s", &(feature->desc[0]));
		feature->tilted = (feature->desc[0] == 't');
	}

	inline void saveHaarFeature(STHaarFeature* feature, FILE* fp)
	{
		fprintf(fp, "%d\n", ((fabs(feature->rect[2].weight) < 0.0001) ? 2 : 3));
		fprintf(fp, "%d %d %d %d %d %d\n",
			feature->rect[0].r.x,
			feature->rect[0].r.y,
			feature->rect[0].r.width,
			feature->rect[0].r.height,
			0,
			(int)(feature->rect[0].weight));
		fprintf(fp, "%d %d %d %d %d %d\n",
			feature->rect[1].r.x,
			feature->rect[1].r.y,
			feature->rect[1].r.width,
			feature->rect[1].r.height,
			0,
			(int)(feature->rect[1].weight));
		if (fabs(feature->rect[2].weight)/* != 0.0F*/)
		{
			fprintf(fp, "%d %d %d %d %d %d\n",
				feature->rect[2].r.x,
				feature->rect[2].r.y,
				feature->rect[2].r.width,
				feature->rect[2].r.height,
				0,
				(int)(feature->rect[2].weight));
		}
		fprintf(fp, "%s\n", &(feature->desc[0]));
	}

	void convertToFastHaarFeature(STHaarFeature* feature, SFastHaarFeature* pFastHF,
		int size, int stepx, int stepy)
	{
		int i = 0;
		int j = 0;

		for (i = 0; i < size; i++)
		{
			pFastHF[i].tilted = feature[i].tilted;
			if (!pFastHF[i].tilted)
			{
				for (j = 0; j < CVLIB_HAAR_FEATURE_MAX; j++)
				{
					pFastHF[i].rect[j].weight = feature[i].rect[j].weight;
					if (fabs(pFastHF[i].rect[j].weight) < 0.0001)
					{
						break;
					}
					CVLib_SUM_OFFSETS(pFastHF[i].rect[j].p0,
						pFastHF[i].rect[j].p1,
						pFastHF[i].rect[j].p2,
						pFastHF[i].rect[j].p3,
						feature[i].rect[j].r, stepx)
				}

			}
			else
			{
				for (j = 0; j < CVLIB_HAAR_FEATURE_MAX; j++)
				{
					pFastHF[i].rect[j].weight = feature[i].rect[j].weight;
					if (fabs(pFastHF[i].rect[j].weight) < 0.0001)
					{
						break;
					}
					CVLib_TILTED_OFFSETS(pFastHF[i].rect[j].p0,
						pFastHF[i].rect[j].p1,
						pFastHF[i].rect[j].p2,
						pFastHF[i].rect[j].p3,
						feature[i].rect[j].r, stepx)
				}
			}
		}

		for (i = 0; i < size; i++)
		{
			for (j = 0; j < CVLIB_HAAR_FEATURE_MAX; j++)
			{
				Rect rect = feature[i].rect[j].r;
				if (rect.x >= stepx - 1 ||
					rect.y >= stepy - 1 ||
					rect.x + rect.width > stepx ||
					rect.y + rect.height > stepy)
					assert(false);
			}
		}
	}

}