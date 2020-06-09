
/*!
 * \file	cvlibbaseVersion.cpp
 * \ingroup cvlibbase
 * \brief   cvlibbase
 * \author  
 * \history	1.0.20	20110525
 * \history	1.1		20110523
 */

#include "cvlibbaseVersion.h"

#define CVLIBBASE_VERSION_MAJOR 2
#define CVLIBBASE_VERSION_MINOR 5
#define CVLIBBASE_VERSION_BUILD 1
#define CVLIBBASE_TITLE	"cvlibbase 2.5.1"
#define CVLIBBASE_COPYRIGHT	"Copyright (C) 2007-2016"

namespace cvlib
{

void GetCVLIBBASEInfo(SLibraryInfo* pLibInfo)
{
	pLibInfo->nMajor = CVLIBBASE_VERSION_MAJOR;
	pLibInfo->nMinor = CVLIBBASE_VERSION_MINOR;
	pLibInfo->nBuild = CVLIBBASE_VERSION_BUILD;
	strcpy(pLibInfo->szTitle, CVLIBBASE_TITLE);
	strcpy(pLibInfo->szCopyright, CVLIBBASE_COPYRIGHT);
}

}