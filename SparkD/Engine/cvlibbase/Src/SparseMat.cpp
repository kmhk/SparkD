#include "SparseMat.h"

namespace cvlib
{

	//////////////////////////////////////////////////////////////////////////
	MatND::MatND() { msize.p = NULL, m_dims = 0, data.ptr = 0; }
	MatND::MatND(int dims, const int* p, TYPE m_type)
	{
		create(dims, p, m_type);
	}
	MatND::MatND(const MatND& other)
	{
		create(other);
	}
	MatND::~MatND()
	{
		release();
	}
	MatND& MatND::operator=(const MatND& other)
	{
		release();
		int nflag = create(other);
		assert(nflag);
		return *this;
	}
	int MatND::create(const MatND& other)
	{
		int nflag = create(other.m_dims, other.msize.p, other.type(), other.mstep.p);
		memcpy(data.ptr, other.data.ptr, other.total()*CVLIB_ELEM_SIZE(type()));
		return nflag;
	}
	int MatND::create(int _dims, const int* _sz, TYPE _type, const int* _steps, bool autosteps)
	{
		int i;
		m_dims = _dims;
		this->mstep.p = new int[2 * m_dims + 1];
		memset(this->mstep.p, 0, sizeof(int)*(2 * m_dims + 1));
		msize.p = (int*)(mstep.p + m_dims) + 1;
		msize.p[-1] = m_dims;
		this->m_type = _type;
		if (!_sz)
			return 0;

		int esz = CVLIB_ELEM_SIZE(m_type), _total = esz;
		for (i = m_dims - 1; i >= 0; i--)
		{
			int s = _sz[i];
			assert(s >= 0);
			msize.p[i] = s;

			if (_steps)
				mstep.p[i] = i < m_dims - 1 ? _steps[i] : esz;
			else if (autosteps)
			{
				mstep.p[i] = _total;
				int64 total1 = (int64)_total*s;
				if ((uint64)total1 != (int)total1)
					assert(0);
				_total = (int)total1;
			}
		}
		data.ptr = new uchar[total()*esz];
		return 1;
	}
	void MatND::release()
	{
		if (data.ptr)
		{
			delete[]data.ptr; data.ptr = NULL;
			delete[]mstep.p; mstep.p = NULL;
		}
	}
	int MatND::total() const
	{
		int p = 1;
		for (int i = 0; i < m_dims; i++)
			p *= msize[i];
		return p;
	}
	void MatND::zero()
	{
		assert(data.ptr);
		memset(data.ptr, 0, CVLIB_ELEM_SIZE(type())*total());
	}
	typedef void(*convertFunc)(const uchar* _src, uchar* _dst, int len);
	template<typename ST, typename DT> static void
		iConvert(const uchar* _src, uchar* _dst, int len)
	{
		const ST* src = (const ST*)_src;
		DT* dst = (DT*)_dst;
		for (int i = 0; i < len; i++)
			dst[i] = (DT)src[i];
	}
	void MatND::convert(TYPE dstType)
	{
		MatND src(*this);
		release();
		create(m_dims, src.msize.p, dstType, src.mstep.p);
		convertFunc funcs[6][6] =
		{
			{NULL,NULL,NULL,NULL,NULL,NULL},
			{NULL,iConvert<uchar,uchar>,iConvert<uchar,short>,iConvert<uchar,int>,iConvert<uchar,float>,iConvert<uchar,double>},
			{NULL,iConvert<short,uchar>,iConvert<short,short>,iConvert<short,int>,iConvert<short,float>,iConvert<short,double>},
			{NULL,iConvert<int,uchar>,iConvert<int,short>,iConvert<int,int>,iConvert<int,float>,iConvert<int,double>},
			{NULL,iConvert<float,uchar>,iConvert<float,short>,iConvert<float,int>,iConvert<float,float>,iConvert<float,double>},
			{NULL,iConvert<double,uchar>,iConvert<double,short>,iConvert<double,int>,iConvert<double,float>,iConvert<double,double>},
		};
		convertFunc func = funcs[src.type()][type()];
		func(src.data.ptr, data.ptr, total());
	}


	//////////////////////////////////////////////////////////////////////////
	typedef void(*normalizeFunc)(uchar* src, double dmin, double dmax, double smin, double smax, int len);
	template<typename _Tp> static void iNormalize(uchar* _src, double dmin, double dmax, double smin, double smax, int len)
	{
		_Tp* src = (_Tp*)_src;
		double scale = (dmax - dmin)*((smax - smin) > DBL_EPSILON ? 1.0 / (smax - smin) : 1.0);
		for (int i = 0; i < len; i++)
			src[i] = (_Tp)((src[i] - smin)*scale + dmin);
	}
	void MatND::normalize(double rmin, double rmax)
	{
		normalizeFunc func;
		normalizeFunc funcs[6] = { NULL, iNormalize<uchar>, iNormalize<short>, iNormalize<int>, iNormalize<float>, iNormalize<double> };
		func = funcs[(int)m_type];
		double smin, smax;
		minMax(smin, smax);
		func(data.ptr, rmin, rmax, smin, smax, total());
	}
	typedef void(*minmaxFunc)(uchar* src, double& minVal, double& maxVal, int len);
	template<typename _Tp> static void iMinMax(uchar* _src, double& minVal, double& maxVal, int len)
	{
		_Tp* src = (_Tp*)_src;
		for (int i = 0; i < len; i++)
		{
			if (minVal > src[i])
				minVal = src[i];
			if (maxVal < src[i])
				maxVal = src[i];
		}
	}
	void MatND::minMax(double& rmin, double& rmax)
	{
		minmaxFunc func;
		minmaxFunc funcs[6] = { NULL, iMinMax<uchar>, iMinMax<short>, iMinMax<int>, iMinMax<float>, iMinMax<double> };
		func = funcs[(int)m_type];
		if (m_type <= MAT_Tint)
			rmin = INT_MAX, rmax = -INT_MAX;
		else
			rmin = MAX_FLOAT, rmax = -MAX_FLOAT;
		func(data.ptr, rmin, rmax, total());
	}
	typedef void(*minmaxlocFunc)(uchar* src, double* minVal, double* maxVal, int* pnmin, int* pnmax, int len);
	template<typename _Tp> static void iMinMaxLoc(uchar* _src, double* minVal, double* maxVal, int* pnmin, int* pnmax, int len)
	{
		int i;
		_Tp* src = (_Tp*)_src;
		if (minVal && pnmin)
		{
			double minv = -1;
			int imin = (int)*minVal;
			for (i = 0; i < len; i++)
			{
				if (minv > src[i])
				{
					minv = src[i];
					imin = i;
				}
			}
			*minVal = minv;
			*pnmin = imin;
		}
		if (maxVal || pnmax)
		{
			double maxv = *maxVal;
			int imax = -1;
			for (i = 0; i < len; i++)
			{
				if (maxv < src[i])
				{
					maxv = src[i];
					imax = i;
				}
			}
			*maxVal = maxv;
			*pnmax = imax;
		}
	}
	void MatND::minMaxLoc(double* rmin, double* rmax, int* idxmin, int* idxmax)
	{
		minmaxlocFunc func;
		minmaxlocFunc funcs[6] = { NULL, iMinMaxLoc<uchar>, iMinMaxLoc<short>, iMinMaxLoc<int>, iMinMaxLoc<float>, iMinMaxLoc<double> };
		func = funcs[(int)m_type];
		double _rmin, _rmax;
		int _idxmin, _idxmax;
		if (m_type <= MAT_Tint)
			_rmin = INT_MAX, _rmax = -INT_MAX;
		else
			_rmin = MAX_FLOAT, _rmax = -MAX_FLOAT;
		if (rmin || idxmin)
		{
			func(data.ptr, &_rmin, 0, &_idxmin, 0, total());
			if (rmin) *rmin = _rmin;
			if (idxmin) *idxmin = _idxmin;
		}
		if (rmax || idxmax)
		{
			func(data.ptr, 0, &_rmax, 0, &_idxmax, total());
			if (rmax) *rmax = _rmax;
			if (idxmax) *idxmax = _idxmax;
		}
	}



	//////////////////////////////////////////////////////////////////////////
	DefMat::DefMat(int dim, int* sizes, int m_type)
	{
		/* create sparse or ND matrix but not both */
		m_pSparseNode = NULL;
		m_pSparse = NULL;
		// 	m_pND = NULL;
		m_Volume = 0;
		m_Max = 0;
		m_IDXs = NULL;
		m_Dim = 0;
		if (dim > 0 && sizes != 0)
			realloc(dim, sizes, m_type);
	}
	DefMat::~DefMat()
	{
		if (m_pSparse)releaseSparseMat(&m_pSparse);
		// 	if(m_pND) delete m_pND; m_pND=NULL;
		if (m_IDXs) delete[]m_IDXs; m_IDXs = NULL;
	}
	void DefMat::realloc(int dim, int* sizes, int m_type)
	{
		if (m_pSparse)releaseSparseMat(&m_pSparse);
		//	if(m_pND) delete m_pND; m_pND=NULL;

		if (m_type == BYSIZE)
		{
			int size = 0;
			int i;
			for (size = 1, i = 0; i < dim; ++i)
			{
				size *= sizes[i];
			}
			size *= sizeof(int);
			if (size > (2 << 20))
			{ /* if size > 1M */
				m_type = SPARSE;
			}
			else
			{
				m_type = ND;
			}
		}/* define matrix m_type */

		if (m_type == SPARSE)
		{
			m_pSparse = createSparseMat(dim, sizes, MAT_Tint);
			m_Dim = dim;
		}
		// 	if(m_type == ND )
		// 	{
		// 		m_pND = new MatND( dim, sizes, MAT_Tint );
		// 		m_pND->zero();
		// 		m_IDXs = new int[dim];
		// 		m_Dim = dim;
		// 	}
		m_Volume = 0;
		m_Max = 0;
	}
	void DefMat::afterLoad()
	{
		m_Volume = 0;
		m_Max = 0;
		if (m_pSparse)
		{/* calculate Volume of loaded hist */
			SparseMatIterator mat_iterator;
			SparseNode* node = initSparseMatIterator(m_pSparse, &mat_iterator);

			for (; node != 0; node = getNextSparseNode(&mat_iterator))
			{
				int val = *(int*)CVLIB_NODE_VAL(m_pSparse, node); /* get value of the element
																 (assume that the m_type is CVLIB_32SC1) */
				m_Volume += val;
				if (m_Max < val)m_Max = val;
			}
		}/* calculate Volume of loaded hist */
	// 	if(m_pND)
	// 	{/* calculate Volume of loaded hist */
	// 		Mat   mat;
	// 		double  max_val;
	// 		double  vol;
	// 		cvGetMat( m_pND, &mat, NULL, 1 );
	// 
	// 		vol = mat.sum();
	// 		m_Volume = cvutil::round(vol);
	// 		m_Max = cvutil::round(mat.max());
	// 	}/* calculate Volume of loaded hist */
	}
	int* DefMat::getPtr(int* indx)
	{
		if (m_pSparse) return (int*)ptrND(m_pSparse, indx, NULL, 1, NULL);
		//	if(m_pND) return  (int*)ptrND( m_pND, indx, NULL, 1, NULL);
		return NULL;
	}
	int DefMat::getVal(int* indx)
	{
		int* p = getPtr(indx);
		if (p)return p[0];
		return -1;
	}
	int DefMat::add(int* indx, int val)
	{
		int  NewVal;
		int* pVal = getPtr(indx);
		if (pVal == NULL) return -1;
		pVal[0] += val;
		NewVal = pVal[0];
		m_Volume += val;
		if (m_Max < NewVal)m_Max = NewVal;
		return NewVal;
	}
	void DefMat::add(DefMat* pMatAdd)
	{
		int*    pIDXS = NULL;
		int     Val = 0;
		for (Val = pMatAdd->getNext(&pIDXS, 1); pIDXS; Val = pMatAdd->getNext(&pIDXS, 0))
		{
			add(pIDXS, Val);
		}
	}
	int DefMat::setMax(int* indx, int val)
	{
		int  NewVal;
		int* pVal = getPtr(indx);
		if (pVal == NULL) return -1;
		if (val > pVal[0])
		{
			m_Volume += val - pVal[0];
			pVal[0] = val;
		}
		NewVal = pVal[0];
		if (m_Max < NewVal)m_Max = NewVal;
		return NewVal;
	}
	int DefMat::getNext(int** pIDXS, int init)
	{
		int     Val = 0;
		pIDXS[0] = NULL;
		if (m_pSparse)
		{
			m_pSparseNode = (init || m_pSparseNode == NULL) ?
				initSparseMatIterator(m_pSparse, &m_SparseIterator) :
				getNextSparseNode(&m_SparseIterator);

			if (m_pSparseNode)
			{
				int* pVal = (int*)CVLIB_NODE_VAL(m_pSparse, m_pSparseNode);
				if (pVal)Val = pVal[0];
				pIDXS[0] = CVLIB_NODE_IDX(m_pSparse, m_pSparseNode);
			}
		}/* sparce matrix */
	// 	if(m_pND)
	// 	{
	// 		int i;
	// 		if(init)
	// 		{
	// 			for(i=0;i<m_Dim;++i)
	// 			{
	// 				m_IDXs[i] = cvGetDimSize( m_pND, i )-1;
	// 			}
	// 			pIDXS[0] = m_IDXs;
	// 			Val = GetVal(m_IDXs);
	// 		}
	// 		else
	// 		{
	// 			for(i=0;i<m_Dim;++i)
	// 			{
	// 				if((m_IDXs[i]--)>0)
	// 					break;
	// 				m_IDXs[i] = cvGetDimSize( m_pND, i )-1;
	// 			}
	// 			if(i==m_Dim)
	// 			{
	// 				pIDXS[0] = NULL;
	// 			}
	// 			else
	// 			{
	// 				pIDXS[0] = m_IDXs;
	// 				Val = GetVal(m_IDXs);
	// 			}
	// 		}/* get next ND */
	// 	}/* sparce matrix */
		return Val;
	}

	//////////////////////////////////////////////////////////////////////////
	SparseMat* createSparseMat(int dims, const int* sizes, int m_type)
	{
		SparseMat* arr = 0;

		m_type = CVLIB_MAT_TYPE(m_type);
		int pix_size1 = CVLIB_ELEM_SIZE(m_type);
		int pix_size = pix_size1/**CVLIB_MAT_CN(m_type)*/;
		int i, size;
		MemStorage* storage;

		if (pix_size == 0)
			return NULL;

		if (dims <= 0 || dims > CVLIB_MAX_DIM_HEAP)
			return NULL;

		if (!sizes)
			return NULL;

		for (i = 0; i < dims; i++)
		{
			if (sizes[i] <= 0)
				return NULL;
		}

		arr = (SparseMat*)alloc(sizeof(*arr) + MAX(0, dims - CVLIB_MAX_DIM) * sizeof(arr->size[0]));

		arr->m_type = CVLIB_SPARSE_MAT_MAGIC_VAL | m_type;
		arr->dims = dims;
		arr->refcount = 0;
		arr->hdr_refcount = 1;
		memcpy(arr->size, sizes, dims * sizeof(sizes[0]));

		arr->valoffset = (int)alignSize(sizeof(SparseNode), pix_size1);
		arr->idxoffset = (int)alignSize(arr->valoffset + pix_size, sizeof(int));
		size = (int)alignSize(arr->idxoffset + dims * sizeof(int), sizeof(SetElem));

		storage = createMemStorage(CVLIB_SPARSE_MAT_BLOCK);
		arr->heap = createSet(0, sizeof(Set), size, storage);

		arr->hashsize = CVLIB_SPARSE_HASH_SIZE0;
		size = arr->hashsize * sizeof(arr->hashtable[0]);

		arr->hashtable = (void**)alloc(size);
		memset(arr->hashtable, 0, size);
		return arr;
	}

	// Creates MatND and underlying data
	void releaseSparseMat(SparseMat** array)
	{
		if (!array)
			assert(false);

		if (*array)
		{
			SparseMat* arr = *array;

			if (!CVLIB_IS_SPARSE_MAT_HDR(arr))
				return;

			*array = 0;

			releaseMemStorage(&arr->heap->storage);
			cvFree(&arr->hashtable);
			cvFree(&arr);
		}
	}


	// Creates MatND and underlying data
	SparseMat* CloneSparseMat(const SparseMat* src)
	{
		SparseMat* dst = 0;

		if (!CVLIB_IS_SPARSE_MAT_HDR(src))
			return NULL;

		dst = createSparseMat(src->dims, src->size, src->m_type);
		//	cvCopy( src, dst ); 

		{
			SparseMatIterator iterator;
			SparseNode* node;

			dst->dims = src->dims;
			memcpy(dst->size, src->size, src->dims * sizeof(src->size[0]));
			dst->valoffset = src->valoffset;
			dst->idxoffset = src->idxoffset;
			clearSet(dst->heap);

			if (src->heap->active_count >= dst->hashsize*CVLIB_SPARSE_HASH_RATIO)
			{
				cvFree(&dst->hashtable);
				dst->hashsize = src->hashsize;
				dst->hashtable = (void**)alloc(dst->hashsize * sizeof(dst->hashtable[0]));
			}

			memset(dst->hashtable, 0, dst->hashsize * sizeof(dst->hashtable[0]));

			for (node = initSparseMatIterator(src, &iterator);
				node != 0; node = getNextSparseNode(&iterator))
			{
				SparseNode* node_copy = (SparseNode*)setNew(dst->heap);
				int tabidx = node->hashval & (dst->hashsize - 1);
				CVLIB_MEMCPY_AUTO(node_copy, node, dst->heap->elem_size);
				node_copy->next = (SparseNode*)dst->hashtable[tabidx];
				dst->hashtable[tabidx] = node_copy;
			}
		}
		return dst;
	}

	SparseNode* initSparseMatIterator(const SparseMat* mat, SparseMatIterator* iterator)
	{
		SparseNode* node = 0;

		int idx;

		if (!CVLIB_IS_SPARSE_MAT(mat))
			return NULL;

		if (!iterator)
			return NULL;

		iterator->mat = (SparseMat*)mat;
		iterator->node = 0;

		for (idx = 0; idx < mat->hashsize; idx++)
			if (mat->hashtable[idx])
			{
				node = iterator->node = (SparseNode*)mat->hashtable[idx];
				break;
			}

		iterator->curidx = idx;
		return node;
	}

	SparseNode* getNextSparseNode(SparseMatIterator* mat_iterator)
	{
		if (mat_iterator->node->next)
			return mat_iterator->node = mat_iterator->node->next;
		else
		{
			int idx;
			for (idx = ++mat_iterator->curidx; idx < mat_iterator->mat->hashsize; idx++)
			{
				SparseNode* node = (SparseNode*)mat_iterator->mat->hashtable[idx];
				if (node)
				{
					mat_iterator->curidx = idx;
					return mat_iterator->node = node;
				}
			}
			return NULL;
		}
	}

#define ICV_SPARSE_MAT_HASH_MULTIPLIER  33
#define CVLIB_ZERO_CHAR( dst, len )                                                    \
{                                                                                   \
	size_t _icv_memcpy_i_, _icv_memcpy_len_ = (len);                                \
	char* _icv_memcpy_dst_ = (char*)(dst);                                          \
	\
	for( _icv_memcpy_i_ = 0; _icv_memcpy_i_ < _icv_memcpy_len_; _icv_memcpy_i_++ )  \
	_icv_memcpy_dst_[_icv_memcpy_i_] = '\0';                                    \
}

	static uchar* icvGetNodePtr(SparseMat* mat, const int* idx, int* _type,
		int create_node, unsigned* precalc_hashval)
	{
		uchar* ptr = 0;

		int i, tabidx;
		unsigned hashval = 0;
		SparseNode *node;
		assert(CVLIB_IS_SPARSE_MAT(mat));

		if (!precalc_hashval)
		{
			for (i = 0; i < mat->dims; i++)
			{
				int t = idx[i];
				if ((unsigned)t >= (unsigned)mat->size[i])
					return NULL;
				hashval = hashval*ICV_SPARSE_MAT_HASH_MULTIPLIER + t;
			}
		}
		else
		{
			hashval = *precalc_hashval;
		}

		tabidx = hashval & (mat->hashsize - 1);
		hashval &= INT_MAX;

		for (node = (SparseNode*)mat->hashtable[tabidx];
			node != 0; node = node->next)
		{
			if (node->hashval == hashval)
			{
				int* nodeidx = CVLIB_NODE_IDX(mat, node);
				for (i = 0; i < mat->dims; i++)
					if (idx[i] != nodeidx[i])
						break;
				if (i == mat->dims)
				{
					ptr = (uchar*)CVLIB_NODE_VAL(mat, node);
					break;
				}
			}
		}

		if (!ptr && create_node)
		{
			if (mat->heap->active_count >= mat->hashsize*CVLIB_SPARSE_HASH_RATIO)
			{
				void** newtable;
				int newsize = MAX(mat->hashsize * 2, CVLIB_SPARSE_HASH_SIZE0);
				int newrawsize = newsize * sizeof(newtable[0]);

				SparseMatIterator iterator;
				assert((newsize & (newsize - 1)) == 0);

				// resize hash table
				newtable = (void**)alloc(newrawsize);
				memset(newtable, 0, newrawsize);

				node = initSparseMatIterator(mat, &iterator);
				while (node)
				{
					SparseNode* next = getNextSparseNode(&iterator);
					int newidx = node->hashval & (newsize - 1);
					node->next = (SparseNode*)newtable[newidx];
					newtable[newidx] = node;
					node = next;
				}

				cvFree(&mat->hashtable);
				mat->hashtable = newtable;
				mat->hashsize = newsize;
				tabidx = hashval & (newsize - 1);
			}

			node = (SparseNode*)setNew(mat->heap);
			node->hashval = hashval;
			node->next = (SparseNode*)mat->hashtable[tabidx];
			mat->hashtable[tabidx] = node;
			CVLIB_MEMCPY_INT(CVLIB_NODE_IDX(mat, node), idx, mat->dims);
			ptr = (uchar*)CVLIB_NODE_VAL(mat, node);
			if (create_node > 0)
				CVLIB_ZERO_CHAR(ptr, CVLIB_ELEM_SIZE2(mat->m_type));
		}

		if (_type)
			*_type = CVLIB_MAT_TYPE(mat->m_type);

		return ptr;
	}

	/*
	static void icvDeleteNode( SparseMat* mat, const int* idx, unsigned* precalc_hashval )
	{
		int i, tabidx;
		unsigned hashval = 0;
		SparseNode *node, *prev = 0;
		assert( CVLIB_IS_SPARSE_MAT( mat ));

		if( !precalc_hashval )
		{
			for( i = 0; i < mat->dims; i++ )
			{
				int t = idx[i];
				if( (unsigned)t >= (unsigned)mat->size[i] )
					return;
				hashval = hashval*ICV_SPARSE_MAT_HASH_MULTIPLIER + t;
			}
		}
		else
		{
			hashval = *precalc_hashval;
		}

		tabidx = hashval & (mat->hashsize - 1);
		hashval &= INT_MAX;

		for( node = (SparseNode*)mat->hashtable[tabidx];
			node != 0; prev = node, node = node->next )
		{
			if( node->hashval == hashval )
			{
				int* nodeidx = CVLIB_NODE_IDX(mat,node);
				for( i = 0; i < mat->dims; i++ )
					if( idx[i] != nodeidx[i] )
						break;
				if( i == mat->dims )
					break;
			}
		}

		if( node )
		{
			if( prev )
				prev->next = node->next;
			else
				mat->hashtable[tabidx] = node->next;
			SetRemoveByPtr( mat->heap, node );
		}
	}*/

	uchar* ptrND(const void* arr, const int* idx, int* _type, int create_node, unsigned* precalc_hashval)
	{
		uchar* ptr = 0;

		if (!idx)
			return NULL;

		if (CVLIB_IS_SPARSE_MAT(arr))
			ptr = icvGetNodePtr((SparseMat*)arr, idx, _type, create_node, precalc_hashval);
		// 	else if( CV_IS_MATND( arr ))
		// 	{
		// 		MatND* mat = (MatND*)arr;
		// 		int i;
		// 		ptr = mat->data.ptr;
		// 
		// 		for( i = 0; i < mat->Dims(); i++ )
		// 		{
		// 			if( (unsigned)idx[i] >= (unsigned)(mat->msize[i]) )
		// 				return NULL;
		// 			ptr += (size_t)idx[i]*mat->MStep[i];
		// 		}
		// 
		// 		if( _type )
		// 			*_type = CVLIB_MAT_TYPE(mat->type());
		// 	}
		// 	else if( CVLIB_IS_MAT_HDR(arr) || CV_IS_IMAGE_HDR(arr) )
		// 		ptr = Ptr2D( arr, idx[0], idx[1], _type );
		// 	else
		// 		return NULL;
		return ptr;
	}

	uchar* Ptr2D(const void* arr, int y, int x, int* _type)
	{
		uchar* ptr = 0;

		// 	if( CVLIB_IS_MAT( arr ))
		// 	{
		// 		Mat* mat = (Mat*)arr;
		// 		int m_type;
		// 
		// 		if( (unsigned)y >= (unsigned)(mat->rows()) || (unsigned)x >= (unsigned)(mat->cols()) )
		// 			return NULL;
		// 
		// 		m_type = mat->type();
		// 		if( _type )
		// 			*_type = m_type;
		// 
		// 		ptr = &mat->data.ptr[y][x*mat->step()];
		// 	}
		// 	else if( CV_IS_IMAGE( arr ))
		// 	{
		// 		IplImage* img = (IplImage*)arr;
		// 		int pix_size = (img->depth & 255) >> 3;
		// 		int width, height;
		// 		ptr = (uchar*)img->imageData;
		// 
		// 		if( img->dataOrder == 0 )
		// 			pix_size *= img->nChannels;
		// 
		// 		if( img->roi )
		// 		{
		// 			width = img->roi->width;
		// 			height = img->roi->height;
		// 
		// 			ptr += img->roi->yOffset*img->widthStep +
		// 				img->roi->xOffset*pix_size;
		// 
		// 			if( img->dataOrder )
		// 			{
		// 				int coi = img->roi->coi;
		// 				if( !coi )
		// 					return NULL;
		// 				ptr += (coi - 1)*img->imageSize;
		// 			}
		// 		}
		// 		else
		// 		{
		// 			width = img->width;
		// 			height = img->height;
		// 		}
		// 
		// 		if( (unsigned)y >= (unsigned)height ||
		// 			(unsigned)x >= (unsigned)width )
		// 			return NULL;
		// 
		// 		ptr += y*img->widthStep + x*pix_size;
		// 
		// 		if( _type )
		// 		{
		// 			int m_type = icvIplToCvDepth(img->depth);
		// 			if( m_type < 0 || (unsigned)(img->nChannels - 1) > 3 )
		// 				return NULL;
		// 
		// 			*_type = CVLIB_MAKETYPE( m_type, img->nChannels );
		// 		}
		// 	}
		// 	else if( CV_IS_MATND( arr ))
		// 	{
		// 		MatND* mat = (MatND*)arr;
		// 
		// 		if( mat->Dims() != 2 || 
		// 			(unsigned)y >= (unsigned)(mat->msize[0]) ||
		// 			(unsigned)x >= (unsigned)(mat->msize[1]) )
		// 			return NULL;
		// 
		// 		ptr = mat->data.ptr + (size_t)y*mat->MStep[0] + x*mat->MStep[1];
		// 		if( _type )
		// 			*_type = CVLIB_MAT_TYPE(mat->type());
		// 	}
		/*else*/ if (CVLIB_IS_SPARSE_MAT(arr))
		{
			int idx[] = { y, x };
			ptr = icvGetNodePtr((SparseMat*)arr, idx, _type, 1, 0);
		}
		else
		{
			return NULL;
		}

		return ptr;
	}

	void clearSet(Set* set)
	{
		clearSeq((Sequence*)set);
		set->free_elems = 0;
		set->active_count = 0;
	}

	SetElem* setNew(Set* set_header)
	{
		SetElem* elem = set_header->free_elems;
		if (elem)
		{
			set_header->free_elems = elem->next_free;
			elem->flags = elem->flags & ((1 << 26) - 1);
			set_header->active_count++;
		}
		else
			setAdd(set_header, NULL, (SetElem**)&elem);
		return elem;
	}
	void SetRemoveByPtr(Set* set_header, void* elem)
	{
		SetElem* _elem = (SetElem*)elem;
		assert(_elem->flags >= 0);
		_elem->next_free = set_header->free_elems;
		_elem->flags = (_elem->flags & ((1 << 26) - 1)) | (1 << (sizeof(int) * 8 - 1));
		set_header->free_elems = _elem;
		set_header->active_count--;
	}

}