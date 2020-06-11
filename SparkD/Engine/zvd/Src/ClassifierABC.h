
/*!
 * \file    ClassifierABC.h
 * \ingroup	base
 * \brief
 * \author
 */

#pragma once

#include "cvlibbase/Inc/Algorithm.h"
#include "cvlibbase/Inc/Vec.h"

namespace cvlib
{

#define CLASSIFIER_UNKNOWN	    ""
#define CLASSIFIER_SET			"CSet"
#define CLASSIFIER_FISHER		"Fisher"

#define CLASSIFIER_LINEAR		"Linear"
#define CLASSIFIER_AFFINE		"Affine"
#define CLASSIFIER_LDA			"LDA"
#define CLASSIFIER_RLDA			"RLDA"
#define CLASSIFIER_DLDA			"DLDA"
#define CLASSIFIER_NLDA			"NLDA"
#define CLASSIFIER_NDA			"NDA"
#define CLASSIFIER_NNFA			"NNFA"
#define CLASSIFIER_NNSA			"NNSA"
#define CLASSIFIER_PNFA			"PNFA"
#define CLASSIFIER_PNSA			"PNSA"
#define CLASSIFIER_SSDA			"SSDA"
#define CLASSIFIER_NEDA			"NEDA"

#define CLASSIFIER_PCA			"PCA"
#define CLASSIFIER_KPCA			"KPCA"

#define CLASSIFIER_GDA			"GDA"
#define CLASSIFIER_KFA			"KFA"
#define CLASSIFIER_KDDA			"KDDA"

#define CLASSIFIER_MAP			"MAP"
#define CLASSIFIER_ML			"ML"

#define CLASSIFIER_RBOOST		"RBoost"
#define CLASSIFIER_REALBOOST	"RealBoost"
#define CLASSIFIER_RSTUMPBOOST	"RStumpBoost"
#define CLASSIFIER_STUMP		"StumpBoost"
#define CLASSIFIER_KNN			"KNN"
#define CLASSIFIER_SVM			"SVM"
#define CLASSIFIER_STAGE		"Stage"

	/**
	 @brief
	 */
#define VERSION_LEN		16

	class CVLIB_DECLSPEC MachineABC : public Algorithm
	{
	public:
		MachineABC();
		virtual ~MachineABC();
		virtual void	release();
		void	setVersion(const char* szVersion);
		void	getVersion(char* szVersion) const;
		virtual int		fromFile(XFile* pFile);
		virtual int		toFile(XFile* pFile) const;
	protected:
		char	m_szVersion[VERSION_LEN];
	};

	class CVLIB_DECLSPEC IMapperABC
	{
	public:
		virtual	Vec*	project(const Vec* pvIn) const = 0;
		virtual	void	project(const Vec* pvIn, Vec* pvOut) const = 0;
	};

	class CVLIB_DECLSPEC MapperABC : public MachineABC, public IMapperABC
	{
	public:
		virtual MapperABC* New() const = 0;
		virtual int inLength() const = 0;
		virtual int outLength() const = 0;
	};

	class CVLIB_DECLSPEC IClassifierABC
	{
	public:
		virtual float	eval(Vec* pvFea) = 0;
	};

	class CVLIB_DECLSPEC ClassifierABC : public MachineABC, public IClassifierABC
	{
	public:
		virtual ClassifierABC* New() const = 0;
	};

	class CVLIB_DECLSPEC MachineSet : public MachineABC
	{
	public:
		MachineSet();
		virtual ~MachineSet();

		int add(MachineABC* pClassifier);

		void removeAt(int nIdx);
		void detachAt(int nIdx);

		int getSize()	const { return m_nNum; }
		int getActiveSize() const { return m_nActiveNum; }
		void setActiveSize(int nNum) { m_nActiveNum = nNum; }

		virtual int		fromIniFile(const char* szFilename);
		virtual int		fromFile(XFile* pFile);
		virtual int		toFile(XFile* pFile) const;

		virtual void	release();
		virtual void	detachAll();

	protected:
		MachineABC** m_pClassifierArray;
		int	m_nNum;
		int m_nActiveNum;
	};

	class CVLIB_DECLSPEC ClassifierSet : public IClassifierABC, public MachineSet
	{
	public:
		ClassifierSet();
		virtual ~ClassifierSet();

		ClassifierABC* getAt(int nIdx) const;
		ClassifierABC** getClassifiers() { return (ClassifierABC**)m_pClassifierArray; }
		virtual float	eval(Vec* /*pvFea*/) { return 0.0f; }
	};

	class CVLIB_DECLSPEC MapperSet : public IMapperABC, public MachineSet
	{
	public:
		MapperSet();
		virtual ~MapperSet();

		MapperABC* getAt(int nIdx) const;
		MapperABC** getClassifiers() { return (MapperABC**)m_pClassifierArray; }

		virtual	Vec*	project(const Vec* /*pvIn*/) const { return NULL; }
		virtual	void	project(const Vec* /*pvIn*/, Vec* /*pvOut*/) const { return; }
	};

}