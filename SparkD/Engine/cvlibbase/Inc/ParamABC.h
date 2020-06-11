
/*!
 * \file	ParamABC.h
 * \ingroup base
 * \brief	
 * \author	
 */

#pragma once

#include "Object.h"
#include <stdio.h>
#include "Template.h"
#include "PtrArray.h"

namespace cvlib
{

	class IniFile;

	class CVLIB_DECLSPEC ParamABC : public Object
	{
	public:
		// Construction and Destruction
		ParamABC();
		virtual ~ParamABC();

		int	load(const char* szFilename);
		virtual int load(IniFile* /*pFile*/) { return 1; };
		int	save(const char* szFilename);
		virtual int save(IniFile* /*pFile*/) { return 1; };
	};

	typedef enum { VAL_INT, VAL_FLOAT } ValType;

	typedef union _tagVALUE
	{
		int i;
		float fl;
	} ParamValue;

	struct CVLIB_DECLSPEC ParamRange
	{
		char	m_szName[32];
		ParamValue	m_rMin;
		ParamValue	m_rMax;
		ParamValue	m_rDefault;
	};

	class CVLIB_DECLSPEC ParamInfo
	{
	public:
		ParamInfo() { m_pvalue = 0; m_pRange = NULL; }
		ParamInfo(float* value, const ParamRange* pRange) :m_pRange(pRange), m_pvalue(value) {}

		//	template<typename _Tp> _Tp& value() {return *(_Tp*)m_pvalue;}
		float& value() { return *m_pvalue; }

		const ParamRange*	range() { return m_pRange; }
	private:
		float*	m_pvalue;
		const ParamRange*	m_pRange;
	};

	class CVLIB_DECLSPEC IParamABC
	{
	public:
		IParamABC() {}
		virtual ~IParamABC() {}

		virtual int getMethodCount() { return 1; }
		virtual int getParamCount(int /*nMethodID*/) { return m_Params.getSize(); }
		virtual ParamInfo* getParamInfo(int /*nMethodID*/, int nValueID) { return (ParamInfo*)m_Params[nValueID]; }
	protected:
		PtrArray m_Params;
	};


	// #define DECLARE_PARAM_INTERFACE ()			\
	// public:\
	// 	static int GetMethodCount();		\
	// 	static int GetParamCount(int nMethodID);		\
	// 	static void GetParamInfo(int nMethodID, int nValueID, ParamInfo* pParamInfo);\
	// private:



#define IMPLEMENT_PARAM(valuename, paramvalue)			\
	m_param_##valuename = ParamInfo(&paramvalue, Entry(#valuename));					\
	m_Params.add(&m_param_##valuename);				\



#define DECLARE_PARAMRANGE \
private:		\
	static const ParamRange _paramEntries[];\
	const ParamRange* Entry(const char* szName);\


#define BEGIN_PARAMRANGE(theClass)			\
	const ParamRange theClass::_paramEntries[]=\
{\


#define END_PARAMRANGE(theClass)								\
	{"", 0, 0, 0}										\
	};													\
	const ParamRange* theClass::Entry(const char* szName)\
	{														\
		int i;\
		for (i=0; i<sizeof(theClass::_paramEntries)/sizeof(theClass::_paramEntries[0]); i++)\
	{\
		if (!strcmp(theClass::_paramEntries[i].m_szName, szName))\
					break;\
	}\
		return &theClass::_paramEntries[i];\
	}\


#define DEF_PARAMRANGE(paramname, parammin, parammax, paramdefault) \
{#paramname, parammin, parammax, paramdefault},	\


#define DECLARE_PARAM(valuename)	\
private:	ParamInfo m_param_##valuename;\

}