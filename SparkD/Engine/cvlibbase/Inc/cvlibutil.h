/*! 
 * \file    cvutil.h
 * \ingroup base
 * \brief   cvlib
 * \author  
 */
#pragma once

#include "Mat.h"
#include "Vec.h"
#include "String.hpp"
#include "StringArray.h"
#include "Template.h"
//#include <uchar.h>
#include <string.h>

namespace cvlib
{

	enum {
		DECOMP_LU, DECOMP_SVD, DECOMP_CHOLESKY
	};

	class CVLIB_DECLSPEC cvutil
	{
	public:
		enum enSortFlag { SORT_INC, SORT_DEC };

		static int scanSortDir(const String &path, const String &extension, StringArray& filenames, bool fSubDirectory = false);
		static int scanImagesInFolder(const char* _szworkPath, StringArray& vFilenames, bool fsubDirectory = false);
		static int scanFoldersInFolder(const char* _szworkPath, StringArray& vFilenames);

		static int scanSortDirW(const std::wstring &path, const std::wstring &extension, std::vector<std::wstring>& filenames, bool fSubDirectory = false);
		static int scanImagesInFolderW(const std::wstring& szworkPath, std::vector<std::wstring>& vFilenames, bool fsubDirectory = false);
		static std::wstring addBackSlashW(const std::wstring &path);

		static String removeExt(const String &s);
		static String addBackSlash(const String &path);
		static String getExt(const String &s);
		static String getPath(const String &fullfilename);
		static String getFilename(const String &filename);

		static int mkDir(const char* filename);
		static int mkDir(const wchar_t* filename);
		static int round(double a);
		static int floor(double value);
		static int ceil(double value);

		static int compareStr(const void *arg1, const void *arg2);

		static void sort(int* pValues, int nNum, enSortFlag nFlag);
		static void sortIdx(const int* pValues, int nNum, int* pnIdx, int nK, enSortFlag nFlag);
		static void sortIdx(const float* pValues, int nNum, int* pnIdx, int nK, enSortFlag nFlag);
		static void sortIdx(const double* pValues, int nNum, int* pnIdx, int nK, enSortFlag nFlag);

		static int		findMin(const int* pValue, int nNum, int* pnIdx = NULL);
		static float	findMin(const float* pValue, int nNum, int* pnIdx = NULL);
		static double	findMin(const double* pValue, int nNum, int* pnIdx = NULL);
		static void		findMinIdx(const Matf* pms, int nNum, Matb* pmIdx);

		static int		findMax(const int* pValue, int nNum, int* pnIdx = NULL);
		static float	findMax(const float* pValue, int nNum, int* pnIdx = NULL);
		static double	findMax(const double* pValue, int nNum, int* pnIdx = NULL);
		static void		findMaxIdx(const Matf* pms, int nNum, Matb* pmIdx);

		static void		calcElementVar(const Vector<Vecf>& vVectors, Vecf &varVec, Vecf *vpMean = NULL);
		static void		pseudoInvert(const Mat& A, Mat& inverse);
		static void		solve(const Mat& A, const Mat& B, Mat& X, int mode = DECOMP_LU);
		static void		computeSVD(const Mat& _aarr, Mat& _w, Mat& _u, Mat& _vt, int flags);
		static bool		makeDataCpp(const char* szdatafile, const char* szcpp, const char* varname);
		static bool		mergeMultipleFiles(const char* szdatafile, const Vector<String>& files);
	};


}
