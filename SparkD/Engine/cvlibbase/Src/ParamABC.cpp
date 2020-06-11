/*!
 * \file	ParamABC.cpp
 * \ingroup base
 * \brief	
 * \author	
 */

#include "ParamABC.h"
#include "IniFile.h"

namespace cvlib 
{

ParamABC::ParamABC()
{
}

ParamABC::~ParamABC()
{
}

int	ParamABC::load (const char* szFilename)
{
	IniFile iniFile;
	iniFile.path(String(szFilename));
	if (!iniFile.readFile())
		return 0;
	return load (&iniFile);
}

int	ParamABC::save (const char* szFilename)
{
	IniFile iniFile;
	iniFile.path(String(szFilename));
	if (!ParamABC::save (&iniFile))
		return 0;
	if (!iniFile.writeFile())
		return 0;
	return 1;
}

}
