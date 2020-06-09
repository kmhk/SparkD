/*!
 * \file	PumpABC.cpp
 * \ingroup base
 * \brief	
 * \author	
 */

#include <assert.h>
#include "PumpABC.h"
#include "Tree.h"
#include "Mat.h"

namespace cvlib
{

const char PumpABC::PumpABC_id_[CVLIB_MAX_PUMP_IDLEN] = "PumpABC";
const char* PumpABC::id () { return PumpABC_id_; } 

PumpABC::PumpABC()
{
	m_numInOut=0;
}

void PumpABC::regDataEx( int l, void* pdata, const String& type/*=""*/)
{
	assert (m_linktypes[l]==LT_OUTPUT);
	getPin(l)->setData(pdata);
	getPin(l)->setType(type);
}

void* PumpABC::getDataEx(int l)
{
	return getPin(l)->data();
}

const String& PumpABC::getDataTypeEx(int l)
{
	return getPin(l)->type();
}

int PumpABC::getOutputNum()
{
	int nret=0;
	for (int i=0; i<m_numInOut; i++)
	{
		if (m_linktypes[i] == LT_OUTPUT)
			nret++;
	}
	return nret;
}

void PumpABC::resetLinks()
{
	for (int i=0; i<m_numInOut; i++)
	{
		if (m_linktypes[i] == LT_OUTPUT)
		{
			if (!m_pumpDatas[i].pPin)
				m_pumpDatas[i].pPin=new Pin();
			m_pumpDatas[i].pPin->m_LinkType=LT_OUTPUT;
			m_pumpDatas[i].pPin->setPump(this);
			m_pumpDatas[i].pSrc = this;
			m_pumpDatas[i].pSink = NULL;
		}
		else
			m_pumpDatas[i].pPin = NULL;
		if (m_linktypes[i] < LT_OUTPUT)
			m_pumpDatas[i].pSink = this;
		else
			m_pumpDatas[i].pSink = NULL;
		m_pumpDatas[i].nSrcLink = 0;
		m_pumpDatas[i].pSrc = NULL;
	}
}

bool PumpABC::connect (int to, PumpABC* pfrom, int from)
{
	assert (to>=0 && to < MAX_LINKS);
	if (m_linktypes[to] == LT_OUTPUT)
		return false;
	PumpABC* pRequestPump=pfrom->RequestJoin(from, this);
	if (pRequestPump==NULL)
		return false;
	m_pumpDatas[to].pSrc=pRequestPump;
	m_pumpDatas[to].nSrcLink=from;
	m_pumpDatas[to].pPin=pRequestPump->getPin(from);
	return true;
}

bool PumpABC::disconnect(int to)
{
	assert (to>=0 && to < MAX_LINKS);
	if (m_linktypes[to] == LT_OUTPUT)
		return false;
	m_pumpDatas[to].pSrc=NULL;
	m_pumpDatas[to].nSrcLink=0;
	m_pumpDatas[to].pPin=NULL;
	return true;
}

void PumpABC::PumpRelease()
{
	releaseData();
	for (int i=0; i<m_numInOut; i++)
	{
		if (m_linktypes[i] == LT_OUTPUT && m_pumpDatas[i].pPin)
		{
			delete m_pumpDatas[i].pPin;
			m_pumpDatas[i].pPin = NULL;
		}
	}
	m_numInOut=0;
}

PumpABC* PumpABC::RequestJoin(int& to, PumpABC* pfrom)
{
	assert (to>=0 && to < MAX_LINKS);
	if (m_linktypes[to] != LT_OUTPUT)
		return NULL;
	m_pumpDatas[to].pSink = pfrom;
	return this;
}

//void PumpABC::RequestAdd(Array<PumpABC*>& pumpAray/*, Array<Pin*>& pinAray*/)
void PumpABC::RequestAdd(PtrArray& pumpAray/*, Array<Pin*>& pinAray*/)
{
	pumpAray.add(this);
}

// whether an input/output is connected to a pipe?
bool PumpABC::isConnected( int l )
{
	assert(l >= 0 && l < MAX_LINKS);
	return m_pumpDatas[l].pPin != 0;
}

bool PumpABC::isConnected( int l, LinkType type )
{
	assert(l >= 0 && l < MAX_LINKS);
	return m_linktypes[l] == type;
}

// to receive Pin, to which given input/output of the pump is connected
Pin* PumpABC::getPin( int l)
{
	assert(isConnected(l));
	return m_pumpDatas[l].pPin;
}

class PumpTree : public Tree
{
public:
	PumpTree(const char* id)
	{
		strcpy(m_id, id); m_nLevel=0;
	}
	~PumpTree()
	{
	}

	char m_id[CVLIB_MAX_PUMP_IDLEN];
	int m_nLevel;

	bool Search(const String& t, PumpTree*& pPump)
	{
		if (m_id == t)
		{
			pPump=this;
			return true;
		}
		Tree* first_;
		for (first_ =first(); first_; first_ = first_->next())
		{
			if (((PumpTree*)first_)->Search(t, pPump))
				break;
		}
		if (first_)
		{
			pPump=((PumpTree*)first_);
			return true;
		}
		return false;
	}

	void Output(FILE* pFile)
	{
		for (int i=0; i<m_nLevel; i++)
			fprintf (pFile, "\t");
		fprintf (pFile, "%s\n", m_id);
		Tree* first_;
		for (first_ =first(); first_; first_ = first_->next())
			((PumpTree*)first_)->Output(pFile);
	}

	bool add (const PumpABC::id_t& id, const PumpABC::id_t& baseid)
	{
		PumpTree* pParentTree=NULL;
		if (!Search(baseid, pParentTree))
			return false;
		PumpTree* pChild=NULL;
		if (!pParentTree->Search(id, pChild))
		{
			PumpTree* pNew=new PumpTree(id);
			pNew->m_nLevel=pParentTree->m_nLevel+1;
			pParentTree->addLast(pNew);
		}
		else
			assert(false);
		return true;
	}
};

// static PumpTree mPumpTree(PumpABC::id());
// 
// int OutputPumpTree(const char* szFilename)
// {
// 	FILE* pFile=fopen(szFilename, "w");
// 	if (pFile)
// 	{
// 		mPumpTree.Output(pFile);
// 		fclose(pFile);
// 		return 1;
// 	}
// 	return 0;
// }

// _register_creator::_register_creator(const PumpABC::id_t& id, const PumpABC::id_t& baseid, FuncCreator func)
// {
//  	FuncCreator other;
//  	if (!_creator_map().lookup(id, other))
//  		_creator_map().setAt(id, func);
// 	FILE* pFile=fopen("c:/pump_list.txt", "a+");
// 	fprintf(pFile, "%s-->%s\n", id, baseid);
// 	fclose(pFile);
//	assert(mPumpTree.add(id, baseid));
// }

/************************************************************************/
/*                                                                      */
/************************************************************************/
IMPLEMENT_PUMP(MatToImagePump, PumpABC)

MatToImagePump::MatToImagePump()
{
	m_numInOut=2;
	m_linktypes[0]=LT_INPUT; 
	m_linktypes[1]=LT_OUTPUT; 
	memset (m_pumpDatas, 0, sizeof(m_pumpDatas[0])*MAX_LINKS);
	resetLinks(); 
}

PumpRet MatToImagePump::pumpProcess() 
{
	assert (false);
// 	Mat* pmRet = (Mat*)getData();
// 	Mat* pImage = new Mat;
// 	pmRet->toImage(*pImage);
// 	regDataEx(1, pImage, "image");
	return PR_AFTER;
}

void MatToImagePump::releaseData()
{
	if (getPin(1)->data())
	{
		delete ((Mat*)getPin(1)->data());
		getPin(1)->setData(NULL);
	}
}

//--------------------------------------------------------------------------
//
//								Pipe
//
//--------------------------------------------------------------------------

Pipe::~Pipe()
{
	freePipe();
}

void Pipe::freePipe()
{
	for (int i=0; i<m_Pumps.getSize(); i++)
	{
		PumpABC* pPump=(PumpABC*)m_Pumps[i];
		delete pPump;
	}
	m_Pumps.removeAll();
	m_OrderPumps.removeAll();
}

// to call processes of pumps connected to a pipe
// returns true, if the pipe has got into wait state
bool Pipe::callPumps(PipePassword)
{
	if (!m_fBuild)
	{
		if (!buildOrder())
			return false;
	}

	int i;
	for (i=0; i<m_OrderPumps.getSize(); i++)
	{
		PumpABC* pPump=(PumpABC*)m_OrderPumps[i];
		PumpRet pr;
		do
		{
			pr = pPump->pumpProcess();
		} while (pr==PR_NOW);

		if (pr>=PR_NEVER)
			return false;
	}

	return true;
}

Pipe& Pipe::add(PumpABC* pump)
{
	m_Pumps.add(pump);
	m_fBuild=false;
	return *this;
}

bool Pipe::buildOrder()
{
	int i;
	int* pnFlag=new int[m_Pumps.getSize()];
	int** ppnBuild = new int*[m_Pumps.getSize()];

	memset (pnFlag, 0, sizeof(pnFlag[0])*m_Pumps.getSize());
	for (i=0; i<m_Pumps.getSize(); i++)
	{
		ppnBuild[i] = new int[((PumpABC*)m_Pumps[i])->getParameterNum()];
		for (int j=0; j<((PumpABC*)m_Pumps[i])->getParameterNum(); j++)
			ppnBuild[i][j] = ((PumpABC*)m_Pumps[i])->m_linktypes[j];
	}
	m_OrderPumps.removeAll();

	// ½å¶®ºåº· °é¾¢
	int nPrevOrder;
	do 
	{
		nPrevOrder=m_OrderPumps.getSize();
		for (i=0; i<m_Pumps.getSize(); i++)
		{
			if ( pnFlag[i])
				continue;
			PumpABC* pPump=(PumpABC*)m_Pumps[i];
			bool fComplete=true;
			for (int j=0; j<pPump->getParameterNum() && fComplete; j++)
			{
				if (ppnBuild[i][j] == LT_INPUT || ppnBuild[i][j] == LT_INOUT)
				{
					PumpABC* pSrcPump=pPump->m_pumpDatas[j].pSrc;
					if (pSrcPump)
					{
						int k;
						for (k=0; k<m_Pumps.getSize(); k++)
						{
							if (m_Pumps[k] == pSrcPump && pnFlag[k])
								break;
						}
						if (k==m_Pumps.getSize())
							fComplete=false;
					}
				}
			}
			if (fComplete)
			{
				m_OrderPumps.add(pPump);
				pnFlag[i]=1;
			}
		}
	} while (nPrevOrder < m_OrderPumps.getSize());

	delete []pnFlag;
	for (i=0; i<m_Pumps.getSize(); i++)
		delete []ppnBuild[i];
	delete []ppnBuild;

	//src, dst¼ÇÂÝ
	m_Srcs.removeAll();
	m_Dsts.removeAll();
	for (i=0; i<m_OrderPumps.getSize(); i++)
	{
		PumpABC* pPump=(PumpABC*)m_OrderPumps[i];
		int j;
		for (j=0; j<pPump->getParameterNum(); j++)
		{
			if ( (pPump->m_linktypes[j]==LT_INPUT || pPump->m_linktypes[j]==LT_INOUT) && pPump->m_pumpDatas[j].pSrc == NULL)
				m_Srcs.add(&pPump->m_pumpDatas[j]);
		}
		for (j=0; j<pPump->getParameterNum(); j++)
		{
			if (pPump->m_linktypes[j]==LT_OUTPUT && pPump->m_pumpDatas[j].pSink == NULL)
				m_Dsts.add(&pPump->m_pumpDatas[j]);
		}
	}

	m_fBuild=false;
	if (m_OrderPumps.getSize() != m_Pumps.getSize())
		return false;
	m_fBuild=true;
	return true;
}

Mat* Pipe::getOutputImage()
{
	PumpABC* pPump=(PumpABC*)m_OrderPumps[m_OrderPumps.getSize()-1];
	Mat* image = (Mat*)pPump->getDataEx(pPump->getParameterNum()-1);
	pPump->getPin(pPump->getParameterNum()-1)->setData(NULL);
	return image;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
IMPLEMENT_PUMP(CompoundPump, PumpABC)
CompoundPump::CompoundPump()
{
	m_fBuild = false;
}

CompoundPump::~CompoundPump()
{
	m_numInOut=0;
	PumpRelease();
}
	
PumpRet CompoundPump::pumpProcess()
{
	if (!m_fBuild)
	{
		if (!buildOrder())
			return PR_NEVER;
	}
	int i;
	for (i=0; i<m_OrderPumps.getSize(); i++)
	{
		PumpABC* pPump=(PumpABC*)m_OrderPumps[i];
		PumpRet pr;
		do
		{
			pr = pPump->pumpProcess();
		} while (pr==PR_NOW);
		
		if (pr>=PR_NEVER)
			return PR_NEVER;
	}

	return PR_AFTER;
}

CompoundPump& CompoundPump::add(PumpABC* pump)
{
	pump->RequestAdd(m_Pumps);
	m_fBuild=false;
	return *this;
}

CompoundPump& CompoundPump::detachAt(int at)
{
	m_Pumps.removeAt(at);
	m_fBuild=false;
	return *this;
}

CompoundPump& CompoundPump::detachAll()
{
	m_Pumps.removeAll();
	m_fBuild=false;
	return *this;
}

Pin* CompoundPump::getPin( int l )
{
	if (!m_fBuild)
	{
		if (!buildOrder())
			return NULL;
	}
	return PumpABC::getPin(l);
}

bool CompoundPump::isConnected( int l )
{
	if (!m_fBuild)
	{
		if (!buildOrder())
			return false;
	}
	return PumpABC::isConnected(l);
}

bool CompoundPump::isConnected( int l, LinkType /*type*/ )
{
	if (!m_fBuild)
	{
		if (!buildOrder())
			return false;
	}
	return PumpABC::isConnected(l);
}

bool CompoundPump::connect (int to, PumpABC* pfrom, int from)
{
	assert (to>=0 && to < MAX_LINKS);
	assert (getPumpCount() != 0);

	if (m_linktypes[to] == LT_OUTPUT)
		return false;

	if (!m_fBuild)
	{
		if (!buildOrder())
			return false;
	}

	PumpABC* pRequestPump=pfrom->RequestJoin(from, this);
	if (pRequestPump==NULL)
		return false;
	m_pumpDatas[to].pSrc = pRequestPump;
	m_pumpDatas[to].pPin = pRequestPump->getPin(from);
	m_pumpDatas[to].nSrcLink = from;

	((PumpData*)m_Srcs[to])->pSrc = pRequestPump;
	((PumpData*)m_Srcs[to])->nSrcLink = from;
	((PumpData*)m_Srcs[to])->pPin = pRequestPump->getPin(from);
	return true;
}

bool CompoundPump::disconnect(int to)
{
	assert (to>=0 && to < MAX_LINKS);
	assert (getPumpCount() != 0);
	
	if (m_linktypes[to] == LT_OUTPUT)
		return false;
	
	if (!m_fBuild)
	{
		if (!buildOrder())
			return false;
	}
	m_pumpDatas[to].pSrc = NULL;
	m_pumpDatas[to].pPin = NULL;
	m_pumpDatas[to].nSrcLink = 0;
	
	((PumpData*)m_Srcs[to])->pSrc = NULL;
	((PumpData*)m_Srcs[to])->nSrcLink = 0;
	((PumpData*)m_Srcs[to])->pPin = NULL;
	return true;
}

PumpABC* CompoundPump::RequestJoin(int& to, PumpABC* pfrom)
{
	assert (to>=0 && to < MAX_LINKS);
	if (m_linktypes[to] != LT_OUTPUT)
		return NULL;
	m_pumpDatas[to].pSink = pfrom;
	((PumpData*)m_Dsts[to-m_Srcs.getSize()])->pSink = pfrom;
	return this;
}

//void CompoundPump::RequestAdd(Array<PumpABC*>& pumpAray/*, Array<Pin*>& pinAray*/)
void CompoundPump::RequestAdd(PtrArray& pumpAray/*, Array<Pin*>& pinAray*/)
{
	pumpAray.add(this);
}

void CompoundPump::releaseData()
{
	for (int i=0; i<m_Pumps.getSize(); i++)
	{
		PumpABC* pPump=(PumpABC*)m_Pumps[i];
		delete pPump;
	}
	m_Pumps.removeAll();
	m_OrderPumps.removeAll();
}

bool CompoundPump::iBuildOrder()
{
	int i;
	int* pnFlag=new int[m_Pumps.getSize()];
	int** ppnBuild = new int*[m_Pumps.getSize()];
	
	memset (pnFlag, 0, sizeof(pnFlag[0])*m_Pumps.getSize());
	for (i=0; i<m_Pumps.getSize(); i++)
	{
		ppnBuild[i] = new int[((PumpABC*)m_Pumps[i])->getParameterNum()];
		for (int j=0; j<((PumpABC*)m_Pumps[i])->getParameterNum(); j++)
			ppnBuild[i][j] = ((PumpABC*)m_Pumps[i])->m_linktypes[j];
	}
	m_OrderPumps.removeAll();
	
	// ½å¶®ºåº· °é¾¢
	int nPrevOrder;
	do 
	{
		nPrevOrder=m_OrderPumps.getSize();
		for (i=0; i<m_Pumps.getSize(); i++)
		{
			if ( pnFlag[i])
				continue;
			PumpABC* pPump=(PumpABC*)m_Pumps[i];
			bool fComplete=true;
			for (int j=0; j<pPump->getParameterNum() && fComplete; j++)
			{
				if (ppnBuild[i][j] == LT_INPUT || ppnBuild[i][j] == LT_INOUT)
				{
					PumpABC* pSrcPump=pPump->m_pumpDatas[j].pSrc;
					if (pSrcPump)
					{
						int k;
						for (k=0; k<m_Pumps.getSize(); k++)
						{
							if (m_Pumps[k] == pSrcPump && pnFlag[k])
								break;
						}
						if (k==m_Pumps.getSize())
							fComplete=false;
					}
				}
			}
			if (fComplete)
			{
				m_OrderPumps.add(pPump);
				pnFlag[i]=1;
			}
		}
	} while (nPrevOrder < m_OrderPumps.getSize());
	
	delete []pnFlag;
	for (i=0; i<m_Pumps.getSize(); i++)
		delete []ppnBuild[i];
	delete []ppnBuild;
	
	m_Srcs.removeAll();
	m_Dsts.removeAll();
	for (i=0; i<m_OrderPumps.getSize(); i++)
	{
		PumpABC* pPump=(PumpABC*)m_OrderPumps[i];
		int j;
		for (j=0; j<pPump->getParameterNum(); j++)
		{
			if ( (pPump->m_linktypes[j]==LT_INPUT || pPump->m_linktypes[j]==LT_INOUT) && pPump->m_pumpDatas[j].pSrc == NULL)
				m_Srcs.add(&pPump->m_pumpDatas[j]);
		}
		for (j=0; j<pPump->getParameterNum(); j++)
		{
			if (pPump->m_linktypes[j]==LT_OUTPUT && pPump->m_pumpDatas[j].pSink == NULL)
				m_Dsts.add(&pPump->m_pumpDatas[j]);
		}
	}
	
	m_fBuild=false;
	if (m_OrderPumps.getSize() != m_Pumps.getSize())
		return false;
	m_fBuild=true;
	return true;
}

bool CompoundPump::buildOrder()
{
	if (!m_fBuild)
		m_fBuild= iBuildOrder();
	if (!m_fBuild)
		return false;

	if (m_Srcs.getSize()+m_Dsts.getSize()>=MAX_LINKS)
		return false;

	m_numInOut=m_Srcs.getSize()+m_Dsts.getSize();
	int i, k;
	for (i=0; i<m_Srcs.getSize(); i++)
		m_linktypes[i]=LT_INPUT;
	for (k=0; k<m_Dsts.getSize(); k++, i++)
		m_linktypes[i]=((PumpData*)m_Dsts[k])->pPin->m_LinkType;
	
	for (i=0; i<m_Srcs.getSize(); i++)
		m_pumpDatas[i]=*(PumpData*)m_Srcs[i];
	for (k=0; k<m_Dsts.getSize(); k++, i++)
		m_pumpDatas[i]=*(PumpData*)m_Dsts[k];
	return true;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
IMPLEMENT_PUMP(BranchPump, PumpABC)
BranchPump::BranchPump(PumpABC* pfrom, int from)
{
	m_linktypes[0]=LT_INPUT; 
	for (int i=1; i<MAX_LINKS; i++)
		m_linktypes[i]=LT_OUTPUT; 
	memset (m_pumpDatas, 0, sizeof(m_pumpDatas[0])*MAX_LINKS);

	m_numInOut=1;
	resetLinks(); 
	connect(0, pfrom, from);
}

PumpRet BranchPump::pumpProcess()
{
	int i;
	void* pdata=getData();
	for (i=1; i<m_numInOut; i++)
		regDataEx(i, pdata, getDataType());
	return PR_AFTER;
}

bool BranchPump::connect (int to, PumpABC* pfrom, int from)
{
	if (to != 0)
		return false;
	PumpABC* pRequestPump=pfrom->RequestJoin(from, this);
	if (pRequestPump==NULL)
		return false;
	m_pumpDatas[to].pSrc=pRequestPump;
	m_pumpDatas[to].nSrcLink=from;
	m_pumpDatas[to].pPin=pRequestPump->getPin(from);
	return true;
}

PumpABC* BranchPump::RequestJoin(int& to, PumpABC* pfrom)
{
	if (to == 0)
		return NULL;
	m_numInOut ++;
	resetLinks();
	PumpABC::RequestJoin (to, pfrom);
	return this;
}

}
