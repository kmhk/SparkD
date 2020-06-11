/*!
 * \file	LineRansac.h
 * \brief	extract the line that contains the most points among the given points
 * \author	
 */

#include "LineRansac.h"
#include "RegressorTrainer.h"
#include "Regressor.h"

using namespace cvlib;

LineRansac::LineRansac()
{
	m_rErrorTh = 2.f;
};
LineRansac::~LineRansac()
{
	release();
}
LineRansac::LineRansac(float rErrorTh)
{
	create(rErrorTh);
}

bool LineRansac::create(float rErrorTh)
{
	m_rErrorTh = rErrorTh;
	return true;
}
void LineRansac::release() {}

bool LineRansac::process(const cvlib::Point2f* psPoint, int nNum, SimpleLine& line, cvlib::Vector<int>& vInlier, int mode)
{
	if (nNum < 2)
		return false;

	if (nNum == 2)
	{
		vInlier.add(0);
		vInlier.add(1);
		if (line.createFrom(psPoint[0], psPoint[1]))
			return true;
		else
			return false;
	}

	float rError, rMaxError = FLT_MAX;
	cvlib::Vector<int> vmaxInlier;
	if (mode == 0)
	{
		for (int i = 0; i < nNum - 1; i++)
		{
			for (int j = i + 1; j < nNum; j++)
			{
				if (!line.createFrom(psPoint[i], psPoint[j]))
					continue;

				Vector<int> vInlier;
				rError = FindInliers(psPoint, nNum, line, vInlier);
				if (vmaxInlier.getSize() < vInlier.getSize() ||
					(vmaxInlier.getSize() == vInlier.getSize() && rMaxError > rError))
				{
					rMaxError = rError;
					vmaxInlier = vInlier;
				}
			}
		}
	}
	else
	{
		int trycount = nNum*(nNum - 1) / 5;
		for (int k = 0; k < trycount; k++)
		{
			int i = (int)(cvlib::Random::boundedUniform()*nNum);
			int j = (int)(cvlib::Random::boundedUniform()*nNum);
			if (!line.createFrom(psPoint[i], psPoint[j]))
				continue;

			cvlib::Vector<int> vInlier;
			rError = FindInliers(psPoint, nNum, line, vInlier);
			if (vmaxInlier.getSize() < vInlier.getSize() ||
				(vmaxInlier.getSize() == vInlier.getSize() && rMaxError > rError))
			{
				rMaxError = rError;
				vmaxInlier = vInlier;
			}
		}
	}

	if (vmaxInlier.getSize() < 2)
	{
		return false;
	}
	else if (vmaxInlier.getSize() == 2)
	{
		vInlier.add(vmaxInlier[0]);
		vInlier.add(vmaxInlier[1]);

		return line.createFrom(psPoint[vmaxInlier[0]], psPoint[vmaxInlier[1]]);
	}
	return GetLineEquationByLsq(psPoint, vmaxInlier, line, vInlier);
}

bool LineRansac::processFast(const cvlib::Point2i* psPoint, int nNum, SimpleLine& line)
{
	if (nNum < 2)
		return false;

	if (nNum == 2)
	{
		if (line.createFrom(psPoint[0], psPoint[1]))
			return true;
		else
			return false;
	}

	float rError, rMaxError = FLT_MAX;
	int maxInlier_count = 0;

	SimpleLine maxline;
	{
		int trycount = nNum*(nNum - 1) / 2;
		for (int k = 0; k < trycount; k++)
		{
			int i = (int)(cvlib::Random::boundedUniform()*nNum);
			int j = (int)(cvlib::Random::boundedUniform()*nNum);
			if (!line.createFrom(psPoint[i], psPoint[j]))
				continue;

			int inlier_count = 0;
			rError = FindInliers(psPoint, nNum, line, inlier_count);
			if (maxInlier_count < inlier_count ||
				(maxInlier_count == inlier_count && rMaxError > rError))
			{
				rMaxError = rError;
				maxInlier_count = inlier_count;
				maxline = line;
			}
		}
	}

	if (maxInlier_count < 2)
	{
		return false;
	}
	return GetLineEquationByLsq(psPoint, nNum, maxline, line);
}

bool LineRansac::processFast(const cvlib::Point2f* psPoint, int nNum, SimpleLine& line)
{
	if (nNum < 2)
		return false;

	if (nNum == 2)
	{
		if (line.createFrom(psPoint[0], psPoint[1]))
			return true;
		else
			return false;
	}

	float rError, rMaxError = FLT_MAX;
	int maxInlier_count = 0;

	SimpleLine maxline;
	{
		int trycount = MIN(100, nNum*(nNum - 1) / 2);
		for (int k = 0; k < trycount; k++)
		{
			int i = (int)(cvlib::Random::boundedUniform()*nNum);
			int j = (int)(cvlib::Random::boundedUniform()*nNum);
			if (!line.createFrom(psPoint[i], psPoint[j]))
				continue;

			int inlier_count = 0;
			rError = FindInliers(psPoint, nNum, line, inlier_count);
			if (maxInlier_count < inlier_count ||
				(maxInlier_count == inlier_count && rMaxError > rError))
			{
				rMaxError = rError;
				maxInlier_count = inlier_count;
				maxline = line;
			}
		}
	}

	if (maxInlier_count < 2)
	{
		return false;
	}
	return GetLineEquationByLsq(psPoint, nNum, maxline, line);
}
float LineRansac::FindInliers(const cvlib::Point2f* psPoint, int nNum, const SimpleLine& line, cvlib::Vector<int>& vInlier) const
{
	int nInlierNum = 0;

	float rY;
	float rDist;

	float rError = 0.f;
	vInlier.setSize(nNum);
	int idx = 0;
	for (int i = 0; i < nNum; i++)
	{
		rY = line.ypos(psPoint[i].x);
		rDist = ABS(rY - psPoint[i].y);
		if (rDist < m_rErrorTh)
		{
			rError += rDist;
			vInlier[idx] = i;
			idx++;
		}
	}
	vInlier.resize(idx);
	return rError;
}
float LineRansac::FindInliers(const cvlib::Point2i* psPoint, int nNum, const SimpleLine& line, cvlib::Vector<int>& vInlier) const
{
	int nInlierNum = 0;

	float rY;
	float rDist;

	float rError = 0.f;
	vInlier.setSize(nNum);
	int idx = 0;
	for (int i = 0; i < nNum; i++)
	{
		rY = line.ypos(psPoint[i].x);
		rDist = ABS(rY - psPoint[i].y);
		if (rDist < m_rErrorTh)
		{
			rError += rDist;
			vInlier[idx] = i;
			idx++;
		}
	}
	vInlier.resize(idx);
	return rError;
}
float LineRansac::FindInliers(const cvlib::Point2i* psPoint, int nNum, const SimpleLine& line, int& inlier_count) const
{
	int nInlierNum = 0;

	float rY;
	float rDist;

	float rError = 0.f;
	inlier_count = 0;
	for (int i = 0; i < nNum; i++)
	{
		rY = line.ypos(psPoint[i].x);
		rDist = ABS(rY - psPoint[i].y);
		if (rDist < m_rErrorTh)
		{
			rError += rDist;
			inlier_count++;
		}
	}
	return rError;
}
float LineRansac::FindInliers(const cvlib::Point2f* psPoint, int nNum, const SimpleLine& line, int& inlier_count) const
{
	int nInlierNum = 0;

	float rY;
	float rDist;

	float rError = 0.f;
	inlier_count = 0;
	for (int i = 0; i < nNum; i++)
	{
		rY = line.ypos(psPoint[i].x);
		rDist = ABS(rY - psPoint[i].y);
		if (rDist < m_rErrorTh)
		{
			rError += rDist;
			inlier_count++;
		}
	}
	return rError;
}
bool LineRansac::GetLineEquationByLsq(const cvlib::Point2i* psPoint, int num, const SimpleLine& maxline, SimpleLine& line) const
{
	Vector<int> vmaxInlier;
	FindInliers(psPoint, num, maxline, vmaxInlier);
	int nNum = vmaxInlier.getSize();
	Mat m_xInputMat(nNum, 1, MAT_Tdouble);
	Vec m_xOutputVec(nNum, MAT_Tdouble);
	Vec m_xVec(1, MAT_Tfloat);

	double** pprInputMat = m_xInputMat.data.db;
	double* prOutputVec = m_xOutputVec.data.db;
	float* prtVec = m_xVec.data.fl;

	int i;
	for (i = 0; i < nNum; i++)
	{
		pprInputMat[i][0] = psPoint[vmaxInlier[i]].x;
		prOutputVec[i] = psPoint[vmaxInlier[i]].y;
	}

	cvlib::LinearRegressorTrainer trainer;
	cvlib::LinearRegressor regressor;

	trainer.setData(&m_xInputMat, &m_xOutputVec);
	int ntrain = trainer.train(&regressor);
	trainer.release();
	if (ntrain == 0) {
		return false;
	}
	line.a = regressor.getCoeffVec().data.fl[0];
	line.b = regressor.getCoeffVec().data.fl[1];

	regressor.release();

	return true;
}
bool LineRansac::GetLineEquationByLsq(const cvlib::Point2f* psPoint, int num, const SimpleLine& maxline, SimpleLine& line) const
{
	cvlib::Vector<int> vmaxInlier;
	FindInliers(psPoint, num, maxline, vmaxInlier);
	int nNum = vmaxInlier.getSize();
	Mat m_xInputMat(nNum, 1, cvlib::MAT_Tdouble);
	Vec m_xOutputVec(nNum, cvlib::MAT_Tdouble);
	Vec m_xVec(1, cvlib::MAT_Tfloat);

	double** pprInputMat = m_xInputMat.data.db;
	double* prOutputVec = m_xOutputVec.data.db;
	float* prtVec = m_xVec.data.fl;

	int i;
	for (i = 0; i < nNum; i++)
	{
		pprInputMat[i][0] = psPoint[vmaxInlier[i]].x;
		prOutputVec[i] = psPoint[vmaxInlier[i]].y;
	}

	LinearRegressorTrainer trainer;
	LinearRegressor regressor;

	trainer.setData(&m_xInputMat, &m_xOutputVec);
	int ntrain = trainer.train(&regressor);
	trainer.release();
	if (ntrain == 0) {
		return false;
	}
	line.a = regressor.getCoeffVec().data.fl[0];
	line.b = regressor.getCoeffVec().data.fl[1];

	regressor.release();

	return true;
}
bool LineRansac::GetLineEquationByLsq(const cvlib::Point2f* psPoint, const cvlib::Vector<int>& vmaxInlier, SimpleLine& line, cvlib::Vector<int>& vInlier) const
{
	int nNum = vmaxInlier.getSize();
	Mat m_xInputMat(nNum, 1, cvlib::MAT_Tdouble);
	Vec m_xOutputVec(nNum, cvlib::MAT_Tdouble);
	Vec m_xVec(1, cvlib::MAT_Tfloat);

	double** pprInputMat = m_xInputMat.data.db;
	double* prOutputVec = m_xOutputVec.data.db;
	float* prtVec = m_xVec.data.fl;

	int i;
	for (i = 0; i < nNum; i++)
	{
		pprInputMat[i][0] = psPoint[vmaxInlier[i]].x;
		prOutputVec[i] = psPoint[vmaxInlier[i]].y;
	}

	cvlib::LinearRegressorTrainer trainer;
	cvlib::LinearRegressor regressor;

	trainer.setData(&m_xInputMat, &m_xOutputVec);
	int ntrain = trainer.train(&regressor);
	trainer.release();
	if (ntrain == 0) {
		return false;
	}
	line.a = regressor.getCoeffVec().data.fl[0];
	line.b = regressor.getCoeffVec().data.fl[1];
	
	vInlier.removeAll();
	for (i = 0; i < vmaxInlier.getSize(); i++)
	{
		prtVec[0] = (float)pprInputMat[i][0];
		float rY = regressor.eval(&m_xVec);
		if (ABS(rY - prOutputVec[i]) < m_rErrorTh)
		{
			vInlier.add(vmaxInlier[i]);
		}
	}

	regressor.release();
	
	return true;
}

SimpleLine createSimpleLineFromRegions(const cvlib::Vector<vec2f>& points, float rerr, int* pvalid, int mode)
{
	SimpleLine plateLine;
	if (points.getSize() < 2)
		return plateLine;

	cvlib::Vector<int> vInlier;
	LineRansac ransac(rerr);
	ransac.process(points.getData(), points.getSize(), plateLine, vInlier, mode);
	if (pvalid)
		*pvalid = vInlier.getSize();
	return plateLine;
}
SimpleLine createSimpleLineFromRegions(const cvlib::Vector<vec2i>& points, float rerr, int* pvalid, int mode)
{
	SimpleLine plateLine;
	if (points.getSize() < 2)
		return plateLine;

	cvlib::Vector<cvlib::vec2f> pointsf(points.getSize());
	for (int i = 0; i < points.getSize(); i++)
		pointsf[i] = points[i];

	return createSimpleLineFromRegions(pointsf, rerr, pvalid, mode);
	/*Vector<int> vInlier;
	LineRansac ransac(5.0f);
	ransac.process(points.getData(), points.getSize(), plateLine, vInlier, mode);
	if (pvalid)
		*pvalid = vInlier.getSize();
	return plateLine;*/
}
SimpleLine createSimpleLineFromRegions(const cvlib::Vector<Rect>& vregions, int* pvalid, int mode)
{
	SimpleLine plateLine;
	if (vregions.getSize() < 2)
		return plateLine;

	cvlib::Vector<vec2f> vregions_centers;
	for (int i = 0; i < vregions.getSize(); i++)
		vregions_centers.add(vregions[i].centerf());

	cvlib::Vector<int> vInlier;
	LineRansac ransac(5.f);
	ransac.process(vregions_centers.getData(), vregions_centers.getSize(), plateLine, vInlier, mode);
	if (pvalid)
		*pvalid = vInlier.getSize();
	return plateLine;
}

SimpleLine createFastSimpleLineFromRegions(const cvlib::Vector<cvlib::vec2f>& vpoints, float rerr)
{
	SimpleLine plateLine;
	if (vpoints.getSize() < 2)
		return plateLine;

	LineRansac ransac(rerr);
	ransac.processFast(vpoints.getData(), vpoints.getSize(), plateLine);
	return plateLine;
}
SimpleLine createFastSimpleLineFromRegions(const cvlib::Vector<vec2i>& vpoints, float rerr)
{
	SimpleLine plateLine;
	if (vpoints.getSize() < 2)
		return plateLine;
	cvlib::Vector<cvlib::vec2f> pointsf(vpoints.getSize());
	for (int i = 0; i < vpoints.getSize(); i++)
		pointsf[i] = vpoints[i];

	LineRansac ransac(rerr);
	ransac.processFast(pointsf.getData(), pointsf.getSize(), plateLine);
	return plateLine;
}
