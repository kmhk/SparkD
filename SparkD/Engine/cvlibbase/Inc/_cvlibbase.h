/*!
 \page page1 About cvlibbase-Computer Vision Basis Library 1.0

	All rights reserved.\n
	\n

 \section sec
	cvlib cvlibbase
*/

/*!
 * \file	cvlibbase.h
 * \ingroup cvlibbase
 * \brief   Computer Vision Base Library
 * \author  
 */

#pragma once

// macros
#include "cvlibbaseVersion.h"
#include "cvlibbaseDef.h"

// basis classes
#include "Object.h"
#include "Exception.h"
#include "ExifInfo.h"
#include "Algorithm.h"
#include "BitOperation.h"
#include "ParamABC.h"
#include "PtrArray.h"
#include "PtrList.h"
#include "PtrQueue.h"
#include "PtrHash.h"
#include "Random.h"
#include "Timer.h"
#include "String.hpp"
#include "StringArray.h"
#include "cvlibstructs.h"
#include "EnumString.h"
#include "Tree.h"
#include "Plex.h"
#include "AutoBuffer.h"
#include "IILog.h"
#include "IPDebug.h"
#include "ImageList.h"
#include "cvbase.h"
#include "SparseMat.h"
#include "internal.h"

// matrix and vector modules
#include "Mat.h"
#include "mat3.h"
#include "Vec.h"
#include "MatOperation.h"

// drawing apis
#include "Drawing.h"

// colorspace
#include "ColorSpace.h"

// mathematics classes and functions
#include "CholeskyDecomposition.h"
#include "EigenvalueDecomposition.h"
#include "LUDecomposition.h"
#include "QRDecomposition.h"
#include "SingularValueDecomposition.h"
#include "Statistics.h"
#include "Distance.h"

// Utilitiy classes
#include "cvlibutil.h"

// File Operation classes
#include "IniFile.h"
#include "XFile.h"
#include "XFileDisk.h"
#include "XFileMem.h"

// template classes
#include "Template.h"
#include "TemplateFactory.h"

// pipe modules
#include "PumpABC.h"

#include "CommandLineParameters.h"
#include "parallel.h"
