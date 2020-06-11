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
#include "cvlibbase/Inc/cvlibbaseVersion.h"
#include "cvlibbase/Inc/cvlibbaseDef.h"

// basis classes
#include "cvlibbase/Inc/Object.h"
#include "cvlibbase/Inc/Exception.h"
#include "cvlibbase/Inc/ExifInfo.h"
#include "cvlibbase/Inc/Algorithm.h"
#include "cvlibbase/Inc/BitOperation.h"
#include "cvlibbase/Inc/ParamABC.h"
#include "cvlibbase/Inc/PtrArray.h"
#include "cvlibbase/Inc/PtrList.h"
#include "cvlibbase/Inc/PtrQueue.h"
#include "cvlibbase/Inc/PtrHash.h"
#include "cvlibbase/Inc/Random.h"
#include "cvlibbase/Inc/Timer.h"
#include "cvlibbase/Inc/String.hpp"
#include "cvlibbase/Inc/StringArray.h"
#include "cvlibbase/Inc/cvlibstructs.h"
#include "cvlibbase/Inc/EnumString.h"
#include "cvlibbase/Inc/Tree.h"
#include "cvlibbase/Inc/Plex.h"
#include "cvlibbase/Inc/AutoBuffer.h"
#include "cvlibbase/Inc/IILog.h"
#include "cvlibbase/Inc/IPDebug.h"
#include "cvlibbase/Inc/ImageList.h"
#include "cvlibbase/Inc/cvbase.h"
#include "cvlibbase/Inc/SparseMat.h"
#include "cvlibbase/Inc/internal.h"

// matrix and vector modules
#include "cvlibbase/Inc/Mat.h"
#include "cvlibbase/Inc/mat3.h"
#include "cvlibbase/Inc/Vec.h"
#include "cvlibbase/Inc/MatOperation.h"

// drawing apis
#include "cvlibbase/Inc/Drawing.h"

// colorspace
#include "cvlibbase/Inc/ColorSpace.h"

// mathematics classes and functions
#include "cvlibbase/Inc/CholeskyDecomposition.h"
#include "cvlibbase/Inc/EigenvalueDecomposition.h"
#include "cvlibbase/Inc/LUDecomposition.h"
#include "cvlibbase/Inc/QRDecomposition.h"
#include "cvlibbase/Inc/SingularValueDecomposition.h"
#include "cvlibbase/Inc/Statistics.h"
#include "cvlibbase/Inc/Distance.h"

// Utilitiy classes
#include "cvlibbase/Inc/cvlibutil.h"

// File Operation classes
#include "cvlibbase/Inc/IniFile.h"
#include "cvlibbase/Inc/XFile.h"
#include "cvlibbase/Inc/XFileDisk.h"
#include "cvlibbase/Inc/XFileMem.h"

// template classes
#include "cvlibbase/Inc/Template.h"
#include "cvlibbase/Inc/TemplateFactory.h"

// pipe modules
#include "cvlibbase/Inc/PumpABC.h"

#include "cvlibbase/Inc/CommandLineParameters.h"
#include "cvlibbase/Inc/parallel.h"
