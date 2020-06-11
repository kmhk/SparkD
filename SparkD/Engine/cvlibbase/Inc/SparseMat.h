
/*!
* \file	SparseMat.h
* \ingroup cvlibvp
* \brief   SparseMat
* \author  
*/
#pragma once

#include "cvlibbaseDef.h"
#include "cvbase.h"

namespace cvlib
{

	/* default memory block for sparse array elements */
#define  CVLIB_SPARSE_MAT_BLOCK    (1<<12)

	/* initial hash table size */
#define  CVLIB_SPARSE_HASH_SIZE0    (1<<10)

	/* maximal average node_count/hash_size ratio beyond which hash table is resized */
#define  CVLIB_SPARSE_HASH_RATIO    3


#define CVLIB_MAX_DIM 32
#define CVLIB_MAX_DIM_HEAP       (1 << 16)

	class CVLIB_DECLSPEC MatND
	{
	public:
		union
		{
			unsigned char*	ptr;
			short* s;
			int* i;
			float* fl;
			double* db;
		} data;

		struct CVLIB_DECLSPEC MSize
		{
			MSize(int* _p = 0);
			Size operator()() const;
			const int& operator[](int i) const;
			int& operator[](int i);
			operator const int*() const;
			bool operator == (const MSize& sz) const;
			bool operator != (const MSize& sz) const;

			int* p;
		};

		struct CVLIB_DECLSPEC MStep
		{
			MStep();
			MStep(int s);
			const int& operator[](int i) const;
			int& operator[](int i);
			operator int() const;
			MStep& operator = (int s);

			int* p;
			int buf[2];
		protected:
			MStep& operator = (const MStep&);
		};

		MSize msize;
		MStep mstep;

	public:
		MatND();
		MatND(int dims, const int* p, TYPE type);
		MatND(const MatND& other);
		~MatND();
		int create(int dims, const int* p, TYPE type, const int* _steps = 0, bool autosteps = false);
		int create(const MatND& other);
		void release();
		MatND& operator=(const MatND& other);
		inline bool isValid() const { return data.ptr != NULL; }
		inline TYPE type() const { return m_type; }
		inline int dims() const { return m_dims; }
		int total() const;
		void normalize(double rmin, double rmax);
		void minMax(double& rmin, double& rmax);
		void minMaxLoc(double* rmin, double* rmax, int* idxmin, int* idxmax);
		void zero();
		void convert(TYPE dstType);
	protected:
		TYPE m_type;
		int m_dims;
	};

#define CVLIB_SPARSE_MAT_MAGIC_VAL    0x42440000

	struct CVLIB_DECLSPEC SparseMat
	{
	public:
		int m_type;
		int dims;
		int* refcount;
		int hdr_refcount;

		struct Set* heap;
		void** hashtable;
		int hashsize;
		int valoffset;
		int idxoffset;
		int size[CVLIB_MAX_DIM];
	};

#define SPARSE  0
#define ND      1
#define BYSIZE  -1

	struct CVLIB_DECLSPEC SparseNode
	{
		unsigned hashval;
		struct SparseNode* next;
	};
	struct CVLIB_DECLSPEC SparseMatIterator
	{
		SparseMat* mat;
		SparseNode* node;
		int curidx;
	};

#define CVLIB_IS_SPARSE_MAT_HDR(mat) \
	((mat) != NULL && \
	(((const SparseMat*)(mat))->m_type & CVLIB_MAGIC_MASK) == CVLIB_SPARSE_MAT_MAGIC_VAL)

#define CVLIB_IS_SPARSE_MAT(mat) \
	CVLIB_IS_SPARSE_MAT_HDR(mat)

#define CVLIB_NODE_VAL(mat,node)   ((void*)((uchar*)(node) + (mat)->valoffset))
#define CVLIB_NODE_IDX(mat,node)   ((int*)((uchar*)(node) + (mat)->idxoffset))


	class CVLIB_DECLSPEC DefMat
	{
	private:
		SparseMatIterator m_SparseIterator;
		SparseNode*       m_pSparseNode;
		int*                m_IDXs;
		int                 m_Dim;

	public:
		SparseMat*        m_pSparse;
		//	MatND*				m_pND;
		int                 m_Volume;
		int                 m_Max;

		DefMat(int dim = 0, int* sizes = NULL, int type = SPARSE);
		~DefMat();
		void realloc(int dim, int* sizes, int type = SPARSE);
		void afterLoad();
		int* getPtr(int* indx);
		int getVal(int* indx);
		int add(int* indx, int val);
		void add(DefMat* pMatAdd);
		int setMax(int* indx, int val);
		int getNext(int** pIDXS, int init = 0);
	};

	inline MatND::MSize::MSize(int* _p) : p(_p) {}
	inline Size MatND::MSize::operator()() const
	{
		assert(p[-1] <= 2);
		return Size(p[1], p[0]);
	}
	inline const int& MatND::MSize::operator[](int i) const { return p[i]; }
	inline int& MatND::MSize::operator[](int i) { return p[i]; }
	inline MatND::MSize::operator const int*() const { return p; }

	inline bool MatND::MSize::operator == (const MSize& sz) const
	{
		int d = p[-1], dsz = sz.p[-1];
		if (d != dsz)
			return false;
		if (d == 2)
			return p[0] == sz.p[0] && p[1] == sz.p[1];

		for (int i = 0; i < d; i++)
			if (p[i] != sz.p[i])
				return false;
		return true;
	}

	inline bool MatND::MSize::operator != (const MSize& sz) const
	{
		return !(*this == sz);
	}

	inline MatND::MStep::MStep() { p = buf; p[0] = p[1] = 0; }
	inline MatND::MStep::MStep(int s) { p = buf; p[0] = s; p[1] = 0; }
	inline const int& MatND::MStep::operator[](int i) const { return p[i]; }
	inline int& MatND::MStep::operator[](int i) { return p[i]; }
	inline MatND::MStep::operator int() const
	{
		assert(p == buf);
		return buf[0];
	}
	inline MatND::MStep& MatND::MStep::operator = (int s)
	{
		assert(p == buf);
		buf[0] = s;
		return *this;
	}

	CVLIB_DECLSPEC SparseMat* createSparseMat(int dims, const int* sizes, int type);
	CVLIB_DECLSPEC void releaseSparseMat(SparseMat** array);
	CVLIB_DECLSPEC SparseMat* cloneSparseMat(const SparseMat* src);

	CVLIB_DECLSPEC SparseNode* initSparseMatIterator(const SparseMat* mat, SparseMatIterator* iterator);
	CVLIB_DECLSPEC SparseNode* getNextSparseNode(SparseMatIterator* mat_iterator);
	CVLIB_DECLSPEC uchar* ptrND(const void* arr, const int* idx, int* _type, int create_node, unsigned* precalc_hashval);
	CVLIB_DECLSPEC void cvCopy(const void* srcarr, void* dstarr, const void* maskarr = NULL);

	CVLIB_DECLSPEC uchar* ptr2D(const void* arr, int y, int x, int* _type);
	CVLIB_DECLSPEC void clearSet(Set* set);
	CVLIB_DECLSPEC void setRemoveByPtr(Set* set_header, void* elem);
	CVLIB_DECLSPEC SetElem* setNew(Set* set_header);
	CVLIB_DECLSPEC void clearSeq(Sequence *seq);

}