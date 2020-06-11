/*!
 * \file	Plex.cpp
 * \ingroup base
 * \brief	
 * \author	
 */

#include "Plex.h"
#include "Template.h"

namespace cvlib
{

Plex* Plex::create(Plex*& head, uint nMax, uint cbElement)
{
	Plex* pNewPlex = (Plex*)malloc(nMax*cbElement+sizeof(Plex));
	pNewPlex->pNext = NULL;

	Plex* pLastPlex=head;
	if (pLastPlex)
	{
		while (pLastPlex->pNext)
			pLastPlex=pLastPlex->pNext;
		pLastPlex->pNext=pNewPlex;
	}
	else
		head=pNewPlex;

	return pNewPlex;
}
	
void Plex::freeDataChain()
{
	Array<Plex*> plexAray;
	Plex* pLastPlex=this;
	while (pLastPlex->pNext)
	{
		pLastPlex=pLastPlex->pNext;
		plexAray.add(pLastPlex);
	}
	for (int i=0; i<plexAray.getSize(); i++)
		free((void*)plexAray[i]);
	free(this);
}

}
