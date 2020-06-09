
/*!
 * \file	PumpABC.h
 * \ingroup base
 * \brief	.
 * \author	
 */

#pragma once

#include "PtrArray.h"
#include "String.hpp"
#include "Mat.h"

namespace cvlib
{

#pragma warning( disable : 4172 )

	enum PipePassword { PIPE_PASSWORD };

	enum PipeStatus {
		PS_ZERO,
		PS_COMPLETE,
		PS_CANTDO,
		PS_OK,
		PS_BREAK,
		PS_ABORT
	};

#define MAX_LINKS 8
	enum LinkType { LT_NONE, LT_INPUT, LT_INOUT, LT_OUTPUT, LT_MAX };

	enum PumpRet {
		PR_NOW, 	// the pump can be called once again immediately
		PR_AFTER,	// the pump can be called once again
		PR_NEVER,	// the pump has completed operation, and it cannot be called once more
		PR_BREAK,	// the pump has interrupted operation of the pipeline
		PR_ABORT	// the pump has interrupted operation of the pipeline for its restarting
	};

	enum PipeLineRet {
		PLR_FALSE = 0,
		PLR_TRUE = 1,
		PLR_ABORT = 2
	};

	class Pin;
	class PumpABC;
	class CompoundPump;

	class CVLIB_DECLSPEC Pin
	{
	public:
		Pin() :m_fMany(false), m_data(0), m_pPump(0), m_LinkType(LT_MAX) {}
		PumpABC* GetPump();

		inline void setData(void* data) { m_data = data; }
		inline void* data() const { return m_data; }
		inline const String& type() const { return m_sztype; }
		inline void setType(const String& szType) { m_sztype = szType; }
		inline LinkType& link() { return m_LinkType; }
		inline void setPump(PumpABC* pPump) { m_pPump = pPump; }
	private:
		bool m_fMany;
		String m_sztype;
		void* m_data;
		PumpABC*	m_pPump;
	public:
		LinkType	m_LinkType;
	};

	struct CVLIB_DECLSPEC PumpData
	{
		Pin* pPin;
		PumpABC* pSrc;
		PumpABC* pSink;
		int nSrcLink;
	};

#define CVLIB_MAX_PUMP_IDLEN 64

	class CVLIB_DECLSPEC PumpABC
	{
		friend class Pipe;
		friend class CompoundPump;
		friend class BranchPump;
	private:
		static const char PumpABC_id_[CVLIB_MAX_PUMP_IDLEN];
	public:
		typedef String id_t;
		static const char* id();

		PumpABC();
		virtual ~PumpABC() { PumpRelease(); }

		virtual PumpABC* clone() const { return NULL; }

		virtual Mat* pumpProcess(Mat* pImage) { return pImage; }


		virtual PumpRet pumpProcess() { return PR_AFTER; }
		void PumpRelease();

		virtual Pin* getPin(int l);
		virtual bool isConnected(int l);
		virtual bool isConnected(int l, LinkType type);
		virtual bool connect(int to, PumpABC* pfrom, int from);
		virtual bool disconnect(int to);

		void* getData() { return getDataEx(0); }
		void* getDataEx(int l);
		const String& getDataType() { return getDataTypeEx(0); }
		const String& getDataTypeEx(int l);

		void regData(void* pdata, const String& type = "") { regDataEx(0, pdata, type); }
		void regDataEx(int l, void* pdata, const String& type = "");

		void resetLinks();

		int getOutputNum();
		int getParameterNum() const { return m_numInOut; }

	protected:
		virtual void releaseData() {}
		virtual PumpABC* RequestJoin(int& to, PumpABC* pfrom);
		virtual void RequestAdd(PtrArray& pumpAray);

		int m_numInOut; // total number of inputs/outputs
		LinkType m_linktypes[MAX_LINKS];
		PumpData m_pumpDatas[MAX_LINKS];
	};


	//////////////////////////////////////////////////////////////////////////////
	// Helper macros for declaring PumpClass compatible classes

	typedef PumpABC* (*FuncCreator) ();

	struct CVLIB_DECLSPEC _register_creator {
		_register_creator(const PumpABC::id_t&, const PumpABC::id_t&, FuncCreator);
	};

	/// Use a prefix @a p to allow several classes in a same .cpp file
#define DECLARE_PUMP(cls)                             \
private:\
	static const char cls##_id_[CVLIB_MAX_PUMP_IDLEN];\
public:\
	static const char* id ();\
	static PumpABC* New () { return new cls; }					\
	virtual PumpABC* clone() const {return new cls(*this);}		\
private:


#define IMPLEMENT_PUMP(cls, basecls)\
    const char cls::cls##_id_[CVLIB_MAX_PUMP_IDLEN] = #cls;\
	const char* cls::id () { return cls##_id_;}


/************************************************************************/
/*                                                                      */
/************************************************************************/
	template<class TYPE>
	class CVLIB_DECLSPEC InputDataPump : public PumpABC
	{
	public:
		InputDataPump(const TYPE* pdata, const String& szType);
		virtual ~InputDataPump() { PumpRelease(); }
	protected:
		virtual void releaseData();
	};

	template<class TYPE>
	InputDataPump<TYPE>::InputDataPump(const TYPE* pdata, const String& szType)
	{
		m_numInOut = 1;
		m_linktypes[0] = LT_OUTPUT;
		memset(m_pumpDatas, 0, sizeof(m_pumpDatas[0])*MAX_LINKS);
		resetLinks();
		m_pumpDatas[0].pPin->setData((void*)pdata);
		m_pumpDatas[0].pPin->setType(szType);
	}

	template<class TYPE>
	void InputDataPump<TYPE>::releaseData()
	{
		if (getPin(0)->data())
		{
			delete (TYPE*)getPin(0)->data();
			getPin(0)->setData(NULL);
		}
	}


	class CVLIB_DECLSPEC MatToImagePump : public PumpABC
	{
		DECLARE_PUMP(MatToImagePump)
	public:
		MatToImagePump();
		virtual ~MatToImagePump() { PumpRelease(); }
		virtual PumpRet pumpProcess();
	protected:
		virtual void releaseData();
	};

	class CVLIB_DECLSPEC BranchPump : public PumpABC
	{
		DECLARE_PUMP(BranchPump)
	public:
		BranchPump() {}
		BranchPump(PumpABC* pfrom, int from);
		virtual PumpRet pumpProcess();

		virtual bool connect(int to, PumpABC* pfrom, int from);
	protected:
		virtual PumpABC* RequestJoin(int& to, PumpABC* pfrom);
	};

	class CVLIB_DECLSPEC CompoundPump : public PumpABC
	{
		DECLARE_PUMP(CompoundPump)
	public:
		CompoundPump();
		virtual ~CompoundPump();

		CompoundPump& add(PumpABC* pump);
		CompoundPump& detachAt(int at);
		CompoundPump& detachAll();
		PumpABC*	getAt(int at) { return (PumpABC*)m_Pumps[at]; }
		int	getPumpCount() const { return m_Pumps.getSize(); }

		virtual PumpRet pumpProcess();
		virtual Pin* getPin(int l);
		virtual bool isConnected(int l);
		virtual bool isConnected(int l, LinkType type);
		virtual bool connect(int to, PumpABC* pfrom, int from);
		virtual bool disconnect(int to);
	protected:
		virtual PumpABC* RequestJoin(int& to, PumpABC* pfrom);
		virtual void RequestAdd(PtrArray& pumpAray/*, Array<Pin*>& pinAray*/);
		virtual void releaseData();
		// to reset the status:

		bool m_fBuild; // flag handling the construction of a pipe
		PtrArray m_Pumps;
		PtrArray m_OrderPumps;

		PtrArray m_Srcs;
		PtrArray m_Dsts;

		bool buildOrder();
		bool iBuildOrder();
	};

	class CVLIB_DECLSPEC Pipe
	{
	public:
		Pipe() { m_fBuild = false; }
		virtual ~Pipe(); // deletes of Pin

		Pipe& add(PumpABC* pump);
		PumpABC* getAt(int at) { return (PumpABC*)m_Pumps[at]; }
		void detachAt(int at) { m_Pumps.removeAt(at); m_fBuild = false; }
		void detachAll() { m_Pumps.removeAll(); m_fBuild = false; }

		int getPumpCount() { return m_Pumps.getSize(); }

		bool callPumps(PipePassword psw);  // to call processes of pumps one after another
		void freePipe();
		Mat* getOutputImage();
	protected:
		// to reset the status:

		bool m_fBuild; // flag handling the construction of a pipe

		PtrArray m_Pumps;
		PtrArray m_OrderPumps;

		PtrArray m_Srcs;
		PtrArray m_Dsts;

		virtual bool buildOrder();
	};// class Pipe

}

#pragma warning( disable : 4172 )
