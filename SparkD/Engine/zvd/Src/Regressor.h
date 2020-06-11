/*!
 * \file
 * \brief
 * \author
 */
#pragma once

#include "ClassifierABC.h"

namespace cvlib
{

	class CVLIB_DECLSPEC LinearRegressor : public ClassifierABC
	{
		friend class LinearRegressorTrainer;
	public:
		//! Construction and Destruction
		LinearRegressor();
		virtual ~LinearRegressor();
		virtual ClassifierABC* New() const { return new LinearRegressor; }

		//! Initialization
		virtual void	release();

		//! Operations for loading and saving
		virtual int		fromFile(XFile* pFile);
		virtual int		toFile(XFile* pFile) const;

		//! Operations
		virtual float	eval(Vec* pvFea);

		float getConstCoeff();
		void setConstCoeff(float rConf);
		Vec& getCoeffVec() { return m_vCoeff; }
	protected:
		Vec m_vCoeff;
	};

	/**
	 @brief
	 */
	class CVLIB_DECLSPEC QuadraticRegressor : public LinearRegressor
	{
		friend class QuadraticRegressorTrainer;
	public:
		virtual ClassifierABC* New() const { return new QuadraticRegressor; }
		virtual void	release();
		virtual float	eval(Vec* pvFea);
		static const Vec* iConvert(const Vec* pvFea);
	};

	class CVLIB_DECLSPEC LeastSquareMachine : public LinearRegressor
	{
		friend class LeastSquareTrainer;
	public:
		virtual ClassifierABC* New() const { return new LeastSquareMachine; }
		virtual float	eval(Vec* pvFea);
	};

}