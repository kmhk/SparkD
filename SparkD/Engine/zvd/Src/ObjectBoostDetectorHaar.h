/*
 *  Computer Vision Library
 *  Copyright 2006-2011 by  Information Center
 *  All rights reserved.
 */

 /*!
 * \file
 * \brief
 * \author
 */ 

#pragma once

#include "ObjectBoostDetectorABC.h"
#include "CascadeClassifierObjectHaar.h"

namespace cvlib {

class CVLIB_DECLSPEC ObjectBoostDetectorHaar : public ObjectBoostDetectorABC
{
public:
	ObjectBoostDetectorHaar(){}
	ObjectBoostDetectorHaar(const ObjectBoostDetectorHaar& from) {assert(false);}
	virtual void release();
	virtual const void* extractLayerFeature(const Mat& image);
protected:
	virtual CascadeClassifierObjectDetectABC* newCascadeClassifierFromFolder(const char* szFilename);
	virtual CascadeClassifierObjectDetectABC* newCascadeClassifierFromOneFile(const char* szFilename);
	virtual CascadeClassifierObjectDetectABC* newCascadeClassifierFromOneFile(XFile* pfile);
protected:
	virtual const void* updateFeature(int nX, int nY, const Mat& pmImage);
	virtual int exportSampleFeature(int nX, int nY);
private:
	Mati	m_mSum;
	Mati	m_mTilted;
	Matd	m_mSqSum;
	double*		m_prSqSum;
	HaarElem::SHaarBasisData	m_data;
};
	
}