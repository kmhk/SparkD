/*! 
 * \file    cvlibcommon.cpp
 * \ingroup base
 * \brief   cvlib
 * \author  
 */

#include "cvlibmacros.h"

#ifdef _MSC_VER
#include <sys/stat.h>
#include <direct.h>
#include <windows.h>
#endif

#include "cvlibutil.h"
#include "cvlibbaseDef.h"
#include "LUDecomposition.h"
#include "SingularValueDecomposition.h"
#include "CholeskyDecomposition.h"
#include "Mat.h"
#include "XFileDisk.h"
#include "MatOperation.h"
#include "svd.h"

#if defined _MSC_VER && defined _M_X64 || (defined __GNUC__ && defined __SSE2__&& !defined __APPLE__)
#include <emmintrin.h>
#endif

namespace cvlib
{
	template<typename _Tp> struct _SElem { _Tp value; int idx; };

	template<typename _Tp>
	int compareIncIdx(const void *p1, const void *p2)
	{
		const _SElem<_Tp> *x = (const _SElem<_Tp>*)p1;
		const _SElem<_Tp> *y = (const _SElem<_Tp>*)p2;
		if (x->value > y->value)
			return 1; //sort in assending order
		if (x->value < y->value)
			return -1;
		return  0;
	}

	template<typename _Tp>
	int compareDecIdx(const void *p1, const void *p2)
	{
		const _SElem<_Tp> *x = (const _SElem<_Tp>*)p1;
		const _SElem<_Tp> *y = (const _SElem<_Tp>*)p2;
		if (x->value < y->value)
			return 1; //sort in assending order
		if (x->value > y->value)
			return -1;
		return  0;
	}

	// local (static) compare function for the qsort() call below
	int cvutil::compareStr(const void *arg1, const void *arg2)
	{
#if CVLIB_OS == CVLIB_OS_WIN32
#if !defined __MINGW32__
		return _stricmp((const char*)(*(String*)arg1), (const char*)(*(String*)arg2));
#else
		return strcmp((const char*)(*(String*)arg1), (const char*)(*(String*)arg2));
#endif
#elif CVLIB_OS == CVLIB_OS_APPLE
		return 1;
#endif
	}


	int cvutil::mkDir(const char* filename)
	{
#ifdef _MSC_VER
		//#if CVLIB_OS == CVLIB_OS_WIN32
		char path[260];
		char* p;
		size_t pos;

		struct _stat st;

		strcpy(path, filename);

		p = path;
		for (; ; )
		{
			pos = strcspn(p, "/\\");

			if (pos == (int)strlen(p)) break;
			if (pos != 0)
			{
				p[pos] = '\0';

				if (p[pos - 1] != ':')
				{
					if (_stat(path, &st) != 0)
					{
						if (_mkdir(path) != 0) return 0;
					}
				}
			}

			p[pos] = '/';

			p += pos + 1;
		}
		return 1;
#elif CVLIB_OS == CVLIB_OS_APPLE
		return 0;
#else
		return 0;
#endif
	}

	int cvutil::mkDir(const wchar_t* filename)
	{
#ifdef _MSC_VER
		//#if CVLIB_OS == CVLIB_OS_WIN32
		wchar_t path[260];
		wchar_t* p;
		size_t pos;

		struct _stat st;

		wcscpy(path, filename);

		p = path;
		for (; ; )
		{
			pos = wcscspn(p, L"/\\");

			if (pos == (int)wcslen(p)) break;
			if (pos != 0)
			{
				p[pos] = '\0';

				if (p[pos - 1] != ':')
				{
					if (_wstat(path, &st) != 0)
					{
						if (_wmkdir(path) != 0) return 0;
					}
				}
			}

			p[pos] = L'/';

			p += pos + 1;
		}
		return 1;
#elif CVLIB_OS == CVLIB_OS_APPLE
		return 0;
#else
		return 0;
#endif
	}
	
	int  cvutil::round(double value)
	{
#if (defined _MSC_VER && defined _M_X64) || (defined __GNUC__ && defined __x86_64__ && defined __SSE2__ && !defined __APPLE__)
		__m128d t = _mm_set_sd(value);
		return _mm_cvtsd_si32(t);
#elif defined _MSC_VER && defined _M_IX86
		int t;
		__asm
		{
			fld value;
			fistp t;
		}
		return t;
#elif defined _MSC_VER && defined _M_ARM && defined HAVE_TEGRA_OPTIMIZATION
		TEGRA_ROUND(value);
#elif defined HAVE_LRINT || defined CV_ICC || defined __GNUC__
#  ifdef HAVE_TEGRA_OPTIMIZATION
		TEGRA_ROUND(value);
#  else
		return (int)lrint(value);
#  endif
#else
		double intpart, fractpart;
		fractpart = modf(value, &intpart);
		if ((fabs(fractpart) != 0.5) || ((((int)intpart) % 2) != 0))
			return (int)(value + (value >= 0 ? 0.5 : -0.5));
		else
			return (int)intpart;
#endif
	}
#if defined __SSE2__ || (defined _M_IX86_FP && 2 == _M_IX86_FP)
#  include "emmintrin.h"
#endif

	int  cvutil::floor(double value)
	{
#if defined _MSC_VER && defined _M_X64 || (defined __GNUC__ && defined __SSE2__ && !defined __APPLE__)
		__m128d t = _mm_set_sd(value);
		int i = _mm_cvtsd_si32(t);
		return i - _mm_movemask_pd(_mm_cmplt_sd(t, _mm_cvtsi32_sd(t, i)));
#elif defined __GNUC__
		int i = (int)value;
		return i - (i > value);
#else
		int i = round(value);
		float diff = (float)(value - i);
		return i - (diff < 0);
#endif
	}


	int  cvutil::ceil(double value)
	{
#if defined _MSC_VER && defined _M_X64 || (defined __GNUC__ && defined __SSE2__&& !defined __APPLE__)
		__m128d t = _mm_set_sd(value);
		int i = _mm_cvtsd_si32(t);
		return i + _mm_movemask_pd(_mm_cmplt_sd(_mm_cvtsi32_sd(t, i), t));
#elif defined __GNUC__
		int i = (int)value;
		return i + (i < value);
#else
		int i = round(value);
		float diff = (float)(i - value);
		return i + (diff < 0);
#endif
	}
	/*	int cvutil::round(double a)
		{
			if (a > 0)
				return (int)(a+0.5);
			else
				return (int)(a-0.5);
		}*/
	int compareIncInt(const void *p1, const void *p2)
	{
		int x = *(int*)p1;
		int y = *(int*)p2;

		if (x > y)
			return 1; //sort in assending order
		if (x < y)
			return -1;
		return  0;
	}

	int compareDecInt(const void *p1, const void *p2)
	{
		int x = *(int*)p1;
		int y = *(int*)p2;

		if (x < y)
			return 1; //sort in assending order
		if (x > y)
			return -1;
		return  0;
	}

	void cvutil::sort(int* pValues, int nNum, enSortFlag nFlag)
	{
		switch (nFlag)
		{
		case SORT_INC:
			qsort(pValues, nNum, sizeof(int), compareIncInt);
			break;
		case SORT_DEC:
			qsort(pValues, nNum, sizeof(int), compareDecInt);
			break;
		default:
			assert(false);
		}
	}

	template <typename _Tp>
	static void _sortIdx(const _Tp* values, int count, int* pnIdx, int nK, cvutil::enSortFlag nFlag)
	{
		_SElem<_Tp>* pElems = new _SElem<_Tp>[count];
		int i;
		for (i = 0; i < count; i++)
		{
			pElems[i].value = values[i];
			pElems[i].idx = i;
		}
		switch (nFlag)
		{
		case cvutil::SORT_INC:
			qsort(pElems, count, sizeof(_SElem<_Tp>), compareIncIdx<_Tp>);
			break;
		case cvutil::SORT_DEC:
			qsort(pElems, count, sizeof(_SElem<_Tp>), compareDecIdx<_Tp>);
			break;
		default:
			assert(false);
		}

		if (pnIdx)
		{
			for (i = 0; i < MIN(nK, count); i++)
				pnIdx[i] = pElems[i].idx;
		}
		delete[]pElems;
	}
	void cvutil::sortIdx(const int* pValues, int nNum, int* pnIdx, int nK, enSortFlag nFlag)
	{
		_sortIdx<int>(pValues, nNum, pnIdx, nK, nFlag);
	}

	void cvutil::sortIdx(const float* pValues, int nNum, int* pnIdx, int nK, enSortFlag nFlag)
	{
		_sortIdx<float>(pValues, nNum, pnIdx, nK, nFlag);
	}

	void cvutil::sortIdx(const double* pValues, int nNum, int* pnIdx, int nK, enSortFlag nFlag)
	{
		_sortIdx<double>(pValues, nNum, pnIdx, nK, nFlag);
	}

	template <typename _Tp>
	static _Tp _findMin(const _Tp* values, int count, int* pnIdx)
	{
		if (count == 1)
		{
			if (pnIdx)
				*pnIdx = 0;
			return values[0];
		}
		int nIdx = 0;
		_Tp rMin = values[0];
		for (int i = 1; i < count; i++)
		{
			if (values[i] < rMin)
			{
				rMin = values[i];
				nIdx = i;
			}
		}
		if (pnIdx)
			*pnIdx = nIdx;
		return rMin;
	}
	int		cvutil::findMin(const int* pValue, int nNum, int* pnIdx/* = NULL*/)
	{
		return _findMin<int>(pValue, nNum, pnIdx);
	}

	float	cvutil::findMin(const float* pValue, int nNum, int* pnIdx/* = NULL*/)
	{
		return _findMin<float>(pValue, nNum, pnIdx);
	}

	double	cvutil::findMin(const double* pValue, int nNum, int* pnIdx/* = NULL*/)
	{
		return _findMin<double>(pValue, nNum, pnIdx);
	}

	template <typename _Tp>
	static _Tp _findMax(const _Tp* values, int count, int* pnIdx)
	{
		if (count == 1)
		{
			if (pnIdx)
				*pnIdx = 0;
			return values[0];
		}
		int nIdx = 0;
		_Tp rMax = values[0];
		for (int i = 1; i < count; i++)
		{
			if (values[i] > rMax)
			{
				rMax = values[i];
				nIdx = i;
			}
		}
		if (pnIdx)
			*pnIdx = nIdx;
		return rMax;
	}
	int		cvutil::findMax(const int* pValue, int nNum, int* pnIdx /*= NULL*/)
	{
		return _findMax<int>(pValue, nNum, pnIdx);
	}

	float	cvutil::findMax(const float* pValue, int nNum, int* pnIdx /*= NULL*/)
	{
		return _findMax<float>(pValue, nNum, pnIdx);
	}

	double	cvutil::findMax(const double* pValue, int nNum, int* pnIdx /*= NULL*/)
	{
		return _findMax<double>(pValue, nNum, pnIdx);
	}

	/**
	  @memo     Scans and sorts a directory for files.

	  @doc      Scans and sorts a directory for files with a specfied extension.

	  @param    path		Path to read from.
	  @param    extension	The file extension to search for. ex. "hips".

	  @return  The filenames found without any path.

	*/
	int cvutil::scanSortDir(const String &path, const String &extension, StringArray& filenames, bool fSubDirectory)
	{
#ifdef _MSC_VER
		//#if CVLIB_OS == CVLIB_OS_WIN32
#ifndef __QT__
		WIN32_FIND_DATAA fd;
		HANDLE h;
		String searchPath;

		// add terminatin backslash (if needed)
		String pathBS = cvutil::addBackSlash(path);
		if (fSubDirectory)
		{
			searchPath = pathBS + String("*.*");
			h = FindFirstFileA(searchPath, &fd);
			if (h == INVALID_HANDLE_VALUE)
				return 0;	// path does not exist
			if (fd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY && fd.cFileName[0] != '.')
			{
				scanSortDir(pathBS + String(fd.cFileName), extension, filenames, fSubDirectory);
			}
			while (FindNextFileA(h, &fd))
			{
				if (((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) && fd.cFileName[0] != '.')
				{
					scanSortDir(pathBS + String(fd.cFileName), extension, filenames, fSubDirectory);
				}
			}

		}

		// build and sort list of filenames
		searchPath = pathBS + String("*.") + extension;
		h = FindFirstFileA(searchPath, &fd);
		if (h == INVALID_HANDLE_VALUE)
			return 0;	// path does not exist
		if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, ".."))
			filenames.add(pathBS + String(fd.cFileName));
		while (FindNextFile(h, &fd)) {
			if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, ".."))
				filenames.add(pathBS + String(fd.cFileName));
		}
		FindClose(h);

		// sort the filenames
		qsort((void *)(filenames.getData()), (size_t)filenames.getSize(),
			sizeof(String), compareStr);
#endif
#endif	
		return filenames.getSize();
	}

	int cvutil::scanSortDirW(const std::wstring &path, const std::wstring &extension, std::vector<std::wstring>& filenames, bool fSubDirectory)
	{
#ifdef _MSC_VER
		//#if CVLIB_OS == CVLIB_OS_WIN32
#ifndef __QT__
		WIN32_FIND_DATAW fd;
		HANDLE h;
		std::wstring searchPath;

		// add terminatin backslash (if needed)
		std::wstring pathBS = cvutil::addBackSlashW(path);
		if (fSubDirectory)
		{
			searchPath = pathBS + std::wstring(L"*.*");
			h = FindFirstFileW(searchPath.c_str(), &fd);
			if (h == INVALID_HANDLE_VALUE)
				return 0;	// path does not exist
			if (fd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY && fd.cFileName[0] != '.')
			{
				scanSortDirW(pathBS + std::wstring(fd.cFileName), extension, filenames, fSubDirectory);
			}
			while (FindNextFileW(h, &fd))
			{
				if (((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) && fd.cFileName[0] != '.')
				{
					scanSortDirW(pathBS + std::wstring(fd.cFileName), extension, filenames, fSubDirectory);
				}
			}

		}

		// build and sort list of filenames
		searchPath = pathBS + std::wstring(L"*.") + extension;
		h = FindFirstFileW(searchPath.c_str(), &fd);
		if (h == INVALID_HANDLE_VALUE)
			return 0;	// path does not exist
		if (wcscmp(fd.cFileName, L".") && wcscmp(fd.cFileName, L".."))
			filenames.push_back(pathBS + std::wstring(fd.cFileName));
		while (FindNextFileW(h, &fd)) {
			if (wcscmp(fd.cFileName, L".") && wcscmp(fd.cFileName, L".."))
				filenames.push_back(pathBS + std::wstring(fd.cFileName));
		}
		FindClose(h);

		// sort the filenames
		std::sort(filenames.begin(), filenames.end()); 
//		qsort((void *)(filenames.begin()), (size_t)filenames.size(),
//			sizeof(String), compareStr);
#endif
#endif	
		return filenames.size();
	}

	/**
	  @memo    Removes the extension of a file name.

	  @doc      Removes the extension of a file name.

	  @param    s	Input file name.

	  @return   File name without extension.

	*/
	String cvutil::removeExt(const String &s)
	{
		return s.left(s.reverseFind('.'));
	}


	/**
	  @memo    Returns the extension of a file name.

	  @doc      Returns the extension of a file name.

	  @param    s	Input filename

	  @return   The extension.

	*/
	String cvutil::getExt(const String &s) {

		return s.right(s.length() - s.reverseFind('.') - 1);
	}

	/**
	  @memo     Ensures that a string is terminated with a backslash

	  @doc      Ensures that a string is terminated with a backslash.
				If the already has a terminating backslash, nothing
				is done.

	  @param    path	Input string.

	  @return   Backslash-terminated output string.

	*/
	String cvutil::addBackSlash(const String &path)
	{
		int len = path.length();

		if (len > 0) {

			if (path[len - 1] == '\\') {

				return path;
			}
			else if (path[len - 1] == '/') {
				return path;
			}
		}
		else {

			return path;
		}
		return path + "/";
	}
	std::wstring cvutil::addBackSlashW(const std::wstring &path)
	{
		int len = path.length();

		if (len > 0) {

			if (path[len - 1] == L'\\') {

				return path;
			}
			else if (path[len - 1] == L'/') {
				return path;
			}
		}
		else {

			return path;
		}
		return path + L"/";
	}

	/**
	  @memo     Returns the path of a filename.

	  @doc      Returns the path of a filename.

	  @param    fullfilename	Filename including any path.

	  @return   The path to the filename.

	*/
	String cvutil::getPath(const String &fullfilename)
	{
		int slashpos1 = fullfilename.reverseFind('\\');
		int slashpos2 = fullfilename.reverseFind('/');
		int slashpos = MAX(slashpos1, slashpos2);
		if (slashpos == -1) {

			// no path
			return "";
		}
		return fullfilename.left(slashpos + 1);
	}


	/**
	  @memo     Returns the file name of a path+file name string.

	  @doc      Returns the file name of a path+file name string.

	  @param    filename	Full qualified filename including path.

	  @return   The file name including any extension, but without any path.

	*/
	String cvutil::getFilename(const String &filename)
	{
		int slashpos = -1;
		int a = filename.reverseFind('\\');
		int b = filename.reverseFind('/');
		slashpos = MAX(a, b);
		if (slashpos == -1) {

			// no path
			return filename;
		}

		return filename.right(filename.length() - slashpos - 1);
	}

	/**
	  @memo     Calculates the variance of each component in a set of vectors.
	  @param    cVectors	Input set of vectors.
	  @param	varVec		A vector containing the variance of each compoment
							in cVectors.
	  @param    vpMean      Optional vector pointer to return the mean vector in.
	  @return   Nothing.
	*/
	void cvutil::calcElementVar(const Vector<Vecf>& vVectors, Vecf &varVec, Vecf *vpMean)
	{
		assert(vVectors.getSize() > 0);
		if (vpMean)
			vpMean->resize(vVectors.getAt(0).length());

		Vecf elemVec(vVectors.getSize());
		varVec.resize(vVectors[0].length());

		for (int elem = 0; elem < vVectors[0].length(); elem++)
		{
			for (int vecNb = 0; vecNb < vVectors.getSize(); vecNb++)
				elemVec[vecNb] = vVectors[vecNb][elem];
			double mean;
			varVec[elem] = (float)elemVec.var(&mean);
			if (vpMean)
				(*vpMean)[elem] = (float)mean;
		}
	}

	void cvutil::findMaxIdx(const Matf* pms, int nNum, Matb* pmIdx)
	{
		int nLen = pms->rows() * pms->cols();
		for (int k = 0; k < nLen; k++)
		{
			float rMax = -1E7;
			int nMax = -1;
			for (int i = 0; i < nNum; i++)
			{
				if (rMax < pms[i].data.fl[0][k])
				{
					rMax = pms[i].data.fl[0][k];
					nMax = i;
				}
			}
			assert(nMax != -1);
			pmIdx->data.ptr[0][k] = (uchar)nMax;
		}
	}

	void cvutil::findMinIdx(const Matf* pms, int nNum, Matb* pmIdx)
	{
		int nLen = pms->rows() * pms->cols();
		for (int k = 0; k < nLen; k++)
		{
			float rMin = 1E7;
			int nMin = -1;
			for (int i = 0; i < nNum; i++)
			{
				if (rMin > pms[i].data.fl[0][k])
				{
					rMin = pms[i].data.fl[0][k];
					nMin = i;
				}
			}
			assert(nMin != -1);
			pmIdx->data.ptr[0][k] = (uchar)nMin;
		}
	}

	int cvutil::scanImagesInFolder(const char* szworkPath, StringArray& vFilenames, bool fsubDirectory)
	{
		scanSortDir(szworkPath, "bmp", vFilenames, fsubDirectory);
		scanSortDir(szworkPath, "jpg", vFilenames, fsubDirectory);
		scanSortDir(szworkPath, "jpeg", vFilenames, fsubDirectory);
		scanSortDir(szworkPath, "tif", vFilenames, fsubDirectory);
		scanSortDir(szworkPath, "tiff", vFilenames, fsubDirectory);
		scanSortDir(szworkPath, "png", vFilenames, fsubDirectory);
		scanSortDir(szworkPath, "pbm", vFilenames, fsubDirectory);
		scanSortDir(szworkPath, "ppm", vFilenames, fsubDirectory);
		scanSortDir(szworkPath, "pgm", vFilenames, fsubDirectory);
		return vFilenames.getSize();
	}
	int cvutil::scanImagesInFolderW(const std::wstring& szworkPath, std::vector<std::wstring>& vFilenames, bool fsubDirectory)
	{
		scanSortDirW(szworkPath, L"bmp", vFilenames, fsubDirectory);
		scanSortDirW(szworkPath, L"jpg", vFilenames, fsubDirectory);
		scanSortDirW(szworkPath, L"jpeg", vFilenames, fsubDirectory);
		scanSortDirW(szworkPath, L"tif", vFilenames, fsubDirectory);
		scanSortDirW(szworkPath, L"tiff", vFilenames, fsubDirectory);
		scanSortDirW(szworkPath, L"png", vFilenames, fsubDirectory);
		scanSortDirW(szworkPath, L"pbm", vFilenames, fsubDirectory);
		scanSortDirW(szworkPath, L"ppm", vFilenames, fsubDirectory);
		scanSortDirW(szworkPath, L"pgm", vFilenames, fsubDirectory);
		return vFilenames.size();
	}

	int cvutil::scanFoldersInFolder(const char* szworkPath, StringArray& vFilenames)
	{
#ifdef _MSC_VER
		//#if CVLIB_OS == CVLIB_OS_WIN32
#ifndef __QT__
		WIN32_FIND_DATAA fd;
		HANDLE h;
		String searchPath;

		// add terminatin backslash (if needed)
		String pathBS = cvutil::addBackSlash(szworkPath);
		vFilenames.removeAll();

		// build and sort list of filenames
		searchPath = pathBS + String("*.*");
		h = FindFirstFileA(searchPath, &fd);
		if (h == INVALID_HANDLE_VALUE)
			return 0;	// path does not exist
		if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..")) {
			String szext = cvutil::getExt(fd.cFileName);
			if (szext.isEmpty())
				vFilenames.add(pathBS + String(fd.cFileName));
		}
		while (FindNextFile(h, &fd)) {
			if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, ".."))
			{
				if (strrchr(fd.cFileName, '.') == 0)
					vFilenames.add(pathBS + String(fd.cFileName));
			}
		}
		FindClose(h);

		// sort the filenames
		qsort((void *)(vFilenames.getData()), (size_t)vFilenames.getSize(),
			sizeof(String), compareStr);
#endif
#endif	
		return vFilenames.getSize();
	}

	void cvutil::pseudoInvert(const Mat& _A, Mat& PseudoIMat)
	{
		int nRow = _A.rows();
		int nCol = _A.cols();
		Mat A;
		_A.convertTo(A, MAT_Tdouble);
		int i;
		SingularValueDecomposition SVDecomp((Mat*)&A);

		int nMin = MIN(nRow, nCol);
		for (i = 0; i < nMin; i++)
		{
			if (SVDecomp.singularValues()[i] < 1E-3) {
				nMin = i;
				break;
			}
		}

		PseudoIMat.create(nCol, nRow, MAT_Tdouble);
		Mat V(nCol, nCol, MAT_Tdouble);
		Mat U(nMin, nRow, MAT_Tdouble);
		Mat SingularV(nCol, nMin, MAT_Tdouble);
		Mat matTemp(nCol, nMin, MAT_Tdouble);

		SingularV.zero();
		PseudoIMat.zero();

		Mat* pmU = SVDecomp.GetU();
		Mat* pmV = SVDecomp.GetV();
		for (i = 0; i < nRow; i++)
		{
			for (int j = 0; j < nMin; j++)
			{
				U.data.db[j][i] = pmU->data.db[i][j];
			}
		}

		for (i = 0; i < nCol; i++)
		{
			for (int j = 0; j < nCol; j++)
			{
				V.data.db[i][j] = pmV->data.db[i][j];
			}
		}

		for (i = 0; i < nMin; i++)
			SingularV.data.db[i][i] = (float)1 / SVDecomp.singularValues()[i];

		delete pmU;
		delete pmV;

		MatOp::mul(&matTemp, &V, &SingularV);
		MatOp::mul(&PseudoIMat, &matTemp, &U);
	}

	void cvutil::solve(const Mat& _A, const Mat& B, Mat& X, int mode)
	{
		assert(_A.type1() == B.type1());
		assert(_A.type1() == MAT_Tfloat || _A.type1() == MAT_Tdouble);
		if (mode == DECOMP_LU)
		{
			Mat A;
			_A.convertTo(A, MAT_Tdouble);
			LUD lud(&A);
			Mat* pmR = lud.solve((Mat*)&B);
			Mat M(3, 3, MAT_Tdouble);
			memcpy(M.data.db[0], pmR->data.db[0], sizeof(double) * 8);
			M.data.db[2][2] = 1.0;
			delete pmR;
			X = *pmR;
		}
		else if (mode == DECOMP_SVD)
		{
			Mat PseudoIMat;
			pseudoInvert(_A, PseudoIMat);
			PseudoIMat.convert(_A.type());

			X.create(PseudoIMat.rows(), B.cols(), B.type());
			MatOp::mul(&X, &PseudoIMat, &B);
		}
		else if (mode == DECOMP_CHOLESKY)
		{
			Mat A;
			_A.convertTo(A, MAT_Tdouble);
			CholeskyDecomposition chd(&A);
			Mat* pmR = chd.solve((Mat*)&B);
			if (pmR != 0) {
				X = *pmR;
				delete pmR;
			}
			else {
				X.zero();
			}
		}
	}

	void cvutil::computeSVD(const Mat& _aarr, Mat& _w, Mat& _u, Mat& _vt, int flags)
	{
		icomputeSVD(_aarr, _w, _u, _vt, flags);
	}

	bool cvutil::makeDataCpp(const char* szdatafile, const char* szcpp, const char* varname)
	{
		uchar* szbuf = 0;
		long len = 0;
		{
			FILE* fp = fopen(szdatafile, "rb");
			if (!fp)
				return false;
			fseek(fp, 0, SEEK_END);
			len = ftell(fp);
			szbuf = new uchar[len];
			fseek(fp, 0, SEEK_SET);
			fread(szbuf, 1, len, fp);
			fclose(fp);
		}
		FILE* fp = fopen(szcpp, "wb");
		fprintf(fp, "unsigned char %s[%ld]={\r\n", varname, len);
		for (int i = 0; i<len; i++)
		{
			if (i % 128 == 0 && i != 0)
				fprintf(fp, "\r\n");
			if (i == len - 1)
				fprintf(fp, "%d", szbuf[i]);
			else
				fprintf(fp, "%d,", szbuf[i]);
		}

		fprintf(fp, "\r\n};\r\n");
		fclose(fp);
		delete[]szbuf;

		return true;
	}

	bool		cvutil::mergeMultipleFiles(const char* szdatafile, const Vector<String>& files)
	{
		XFileDisk xfile;
		if (!xfile.open(szdatafile, "wb"))
			return false;

		for (int i = 0; i < files.getSize(); i++) {
			XFileDisk subdisk;
			if (!subdisk.open(files[i], "rb"))
				return false;
			int sub_size = subdisk.size();
			std::vector<unsigned char> subbuffer;
			subbuffer.resize(sub_size);
			if (subdisk.read(&subbuffer[0], sub_size, 1) != 1)
				return false;

			if (xfile.write(&subbuffer[0], sub_size, 1) != 1)
				return false;
		}

		xfile.close();
		return true;
	}

}
