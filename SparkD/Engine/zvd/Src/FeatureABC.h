/*
 *  cvlib - Computer Vision Library
 *  All rights reserved.
 */

/*!
 * \file	FeatureABC.h
 * \brief	
 * \author	
 */
#pragma once

#include "cvlibbase/Inc/_cvlibbase.h"

namespace cvlib
{

#define FEATURE_SET "FSet"

	class CVLIB_DECLSPEC FeatureABC
	{

	public:
		FeatureABC() {}
		virtual ~FeatureABC() {}

		virtual FeatureABC* clone() const = 0;
		virtual int read(XFile*) = 0;
		virtual int write(XFile*) const = 0;
		bool write(const char* szfilename) const;
		virtual void release() = 0;

		virtual Vec* getBinData() { return NULL; };
		virtual int setBinData(void*) { return 1; };

		virtual float dist(const FeatureABC*) const = 0;
		virtual int dimension() const = 0;
		virtual void getData(Vec&) {};
		const char* getID() const { return m_szID; }
		virtual bool isMultiple() const { return false; }
	protected:
		char m_szID[CVLIB_MAX_IDLEN];
	};

	class CVLIB_DECLSPEC FeatureSet : public FeatureABC
	{
	public:
		//! Construction and Destruction
		FeatureSet();
		virtual ~FeatureSet();

		int add(FeatureABC* pFeature);
		void removeAt(int nIdx);
		int getSize() const { return m_nNum; }
		const FeatureABC* getAt(int nIdx) const;
		FeatureABC* getAt(int nIdx);
		void setAt(int nIdx, FeatureABC* pfeature);
		FeatureABC** getFeatures() { return m_pFeatureArray; }

		virtual FeatureABC* clone() const;
		virtual int read(XFile* pFile);
		virtual int write(XFile* pFile) const;
		virtual void release();

		virtual float dist(const FeatureABC*) const { return 1.0f; }
		virtual int dimension() const;
		virtual void getData(Vec&) {}
		virtual bool isMultiple() const { return true; }

		static FeatureSet* createFromID(const char* szID);
	protected:
		FeatureABC** m_pFeatureArray;
		int	m_nNum;
	};

	class CVLIB_DECLSPEC FeatureFactory : public FactoryTemplate<FeatureABC>
	{
	protected:
		FeatureFactory();
	public:
		virtual ~FeatureFactory();
		static FeatureFactory* Instance();
		virtual void	init();
		virtual void	release();
		void addFeature(FeatureABC* p) { add(p, p->getID()); }
		FeatureABC*	readFeature(const char* szPath);
		FeatureABC*	readFeature(XFile* pFile);
	protected:
		static FeatureFactory* m_pFeatureFactory;
	};

}