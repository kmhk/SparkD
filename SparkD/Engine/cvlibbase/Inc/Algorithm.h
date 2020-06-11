/*!
 * \file	Algorithm.h
 * \ingroup base
 * \brief	
 * \author	
 */

#pragma once

#include "PtrArray.h"
#include "String.hpp"
#include "Object.h"

namespace cvlib
{

#define CVLIB_IDLEN			256

	/**
	 * @brief
	 */
	class CVLIB_DECLSPEC Algorithm : public Object
	{
	public:
		//! construction and destruction
		Algorithm();
		virtual ~Algorithm();

		int addAlgorithm(Algorithm* pAlgorithm);
		Algorithm* getAlgorithm(int nIdx) const;
		void SetAlgorithm(int nIdx, Algorithm* pAlgorithm);
		int getAlgorithmCount() const;
		void removeAlgorithms();
		void delAlgorithm(int nIdx);
		virtual const char* getID();
		void setSelfID(const char* szID);
		const char* getSelfID() const;
	protected:
		char m_szID[CVLIB_IDLEN];
		char m_szTotalID[CVLIB_IDLEN];
		PtrArray	m_AlgorithmArray;
	};

}