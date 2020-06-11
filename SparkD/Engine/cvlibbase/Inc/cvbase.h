
/*!
 * \file
 * \brief .
 * \author 
 */

#pragma once

#include "cvlibbaseDef.h"
#include "cvlibutil.h"

namespace cvlib
{

#define CVLIB_MAGIC_MASK 0xFFFF0000

/******************************Mat type****************************/

//#define CVLIB_MAT_CN_MASK          ((CVLIB_CN_MAX - 1) << CVLIB_CN_SHIFT)
//#define CVLIB_MAT_CN(flags)        ((((flags) & CVLIB_MAT_CN_MASK) >> CVLIB_CN_SHIFT) + 1)
//#define CVLIB_MAT_DEPTH_MASK       (CVLIB_DEPTH_MAX - 1)

//#define CVLIB_MAT_TYPE_MASK        (CVLIB_DEPTH_MAX*CVLIB_CN_MAX - 1)
//#define CVLIB_MAT_TYPE(flags)      ((flags) & CVLIB_MAT_TYPE_MASK)
#define CVLIB_MAT_CONT_FLAG_SHIFT  14
#define CVLIB_MAT_CONT_FLAG        (1 << CVLIB_MAT_CONT_FLAG_SHIFT)
#define CVLIB_IS_MAT_CONT(flags)   ((flags) & CVLIB_MAT_CONT_FLAG)
#define CVLIB_IS_CONT_MAT          CVLIB_IS_MAT_CONT
#define CVLIB_MAT_TEMP_FLAG_SHIFT  15
#define CVLIB_MAT_TEMP_FLAG        (1 << CVLIB_MAT_TEMP_FLAG_SHIFT)
#define CVLIB_IS_TEMP_MAT(flags)   ((flags) & CVLIB_MAT_TEMP_FLAG)

#define CVLIB_MAGIC_MASK       0xFFFF0000
#define CVLIB_MAT_MAGIC_VAL    0x42420000

/* default alignment for dynamic data strucutures, resided in storages. */
#define  CVLIB_STRUCT_ALIGN    ((int)sizeof(double))
/****************************************************************************************\
*                                    Sequence types                                      *
\****************************************************************************************/

#define CVLIB_32FC1 CVLIB_MAKETYPE(CVLIB_32F,1)
#define CVLIB_32FC2 CVLIB_MAKETYPE(CVLIB_32F,2)
#define CVLIB_32FC3 CVLIB_MAKETYPE(CVLIB_32F,3)
#define CVLIB_32FC4 CVLIB_MAKETYPE(CVLIB_32F,4)
#define CVLIB_32FC(n) CVLIB_MAKETYPE(CVLIB_32F,(n))

#define CVLIB_32SC1 CVLIB_MAKETYPE(CVLIB_32S,1)
#define CVLIB_32SC2 CVLIB_MAKETYPE(CVLIB_32S,2)
#define CVLIB_32SC3 CVLIB_MAKETYPE(CVLIB_32S,3)
#define CVLIB_32SC4 CVLIB_MAKETYPE(CVLIB_32S,4)
#define CVLIB_32SC(n) CVLIB_MAKETYPE(CVLIB_32S,(n))

#define CVLIB_8SC1 CVLIB_MAKETYPE(CVLIB_8S,1)
#define CVLIB_8SC2 CVLIB_MAKETYPE(CVLIB_8S,2)
#define CVLIB_8SC3 CVLIB_MAKETYPE(CVLIB_8S,3)
#define CVLIB_8SC4 CVLIB_MAKETYPE(CVLIB_8S,4)
#define CVLIB_8SC(n) CVLIB_MAKETYPE(CVLIB_8S,(n))

/* IEEE754 constants and macros */
#define  CVLIB_POS_INF       0x7f800000
#define  CVLIB_NEG_INF       0x807fffff /* CVLIB_TOGGLE_FLT(0xff800000) */
#define  CVLIB_1F            0x3f800000
#define  CVLIB_TOGGLE_FLT(x) ((x)^((int)(x) < 0 ? 0x7fffffff : 0))
#define  CVLIB_TOGGLE_DBL(x) \
    ((x)^((int64)(x) < 0 ? CV_BIG_INT(0x7fffffffffffffff) : 0))

#define CVLIB_SEQ_MAGIC_VAL             0x42990000

#define CVLIB_IS_SEQ(seq) \
((seq) != NULL && (((Sequence*)(seq))->flags & CVLIB_MAGIC_MASK) == CVLIB_SEQ_MAGIC_VAL)

#define CVLIB_SET_MAGIC_VAL             0x42980000
#define CVLIB_IS_SET(set) \
((set) != NULL && (((Sequence*)(set))->flags & CVLIB_MAGIC_MASK) == CVLIB_SET_MAGIC_VAL)

#define CVLIB_SEQ_ELTYPE_BITS           9
#define CVLIB_SEQ_ELTYPE_MASK           ((1 << CVLIB_SEQ_ELTYPE_BITS) - 1)

#define CVLIB_SEQ_ELTYPE_POINT          CVLIB_32SC2  /* (x,y) */
#define CVLIB_SEQ_ELTYPE_CODE           CVLIB_8UC1   /* freeman code: 0..7 */
#define CVLIB_SEQ_ELTYPE_GENERIC        0
#define CVLIB_SEQ_ELTYPE_PTR            CVLIB_USRTYPE1
#define CVLIB_SEQ_ELTYPE_PPOINT         CVLIB_SEQ_ELTYPE_PTR  /* &(x,y) */
#define CVLIB_SEQ_ELTYPE_INDEX          CVLIB_32SC1  /* #(x,y) */
#define CVLIB_SEQ_ELTYPE_GRAPH_EDGE     0  /* &next_o, &next_d, &vtx_o, &vtx_d */
#define CVLIB_SEQ_ELTYPE_GRAPH_VERTEX   0  /* first_edge, &(x,y) */
#define CVLIB_SEQ_ELTYPE_TRIAN_ATR      0  /* vertex of the binary tree   */
#define CVLIB_SEQ_ELTYPE_CONNECTED_COMP 0  /* connected component  */
#define CVLIB_SEQ_ELTYPE_POINT3D        CVLIB_32FC3  /* (x,y,z)  */

#define CVLIB_SEQ_KIND_BITS        3
#define CVLIB_SEQ_KIND_MASK        (((1 << CVLIB_SEQ_KIND_BITS) - 1)<<CVLIB_SEQ_ELTYPE_BITS)

/* types of sequences */
#define CVLIB_SEQ_KIND_GENERIC     (0 << CVLIB_SEQ_ELTYPE_BITS)
#define CVLIB_SEQ_KIND_CURVE       (1 << CVLIB_SEQ_ELTYPE_BITS)
#define CVLIB_SEQ_KIND_BIN_TREE    (2 << CVLIB_SEQ_ELTYPE_BITS)

#define CVLIB_8U   0
#define CVLIB_8S   1
#define CVLIB_16U  2
#define CVLIB_16S  3
#define CVLIB_32S  4
#define CVLIB_32F  5
#define CVLIB_64F  6
#define CVLIB_USRTYPE1 7

#define CVLIB_8UC1 CVLIB_MAKETYPE(CVLIB_8U,1)
#define CVLIB_SEQ_ELTYPE_CODE           CVLIB_8UC1   /* freeman code: 0..7 */

/* chain-coded curves */
#define CVLIB_SEQ_CHAIN           (CVLIB_SEQ_KIND_CURVE  | CVLIB_SEQ_ELTYPE_CODE)
#define CVLIB_SEQ_CHAIN_CONTOUR   (CVLIB_SEQ_FLAG_CLOSED | CVLIB_SEQ_CHAIN)

/* point sets */
#define CVLIB_SEQ_POINT_SET       (CVLIB_SEQ_KIND_GENERIC| CVLIB_SEQ_ELTYPE_POINT)
#define CVLIB_SEQ_POINT3D_SET     (CVLIB_SEQ_KIND_GENERIC| CVLIB_SEQ_ELTYPE_POINT3D)
#define CVLIB_SEQ_POLYLINE        (CVLIB_SEQ_KIND_CURVE  | CVLIB_SEQ_ELTYPE_POINT)
#define CVLIB_SEQ_POLYGON         (CVLIB_SEQ_FLAG_CLOSED | CVLIB_SEQ_POLYLINE )
#define CVLIB_SEQ_CONTOUR         CVLIB_SEQ_POLYGON
#define CVLIB_SEQ_SIMPLE_POLYGON  (CVLIB_SEQ_FLAG_SIMPLE | CVLIB_SEQ_POLYGON  )

#define CVLIB_SEQ_KIND_BITS			3
#define CVLIB_SEQ_ELTYPE_BITS			9

#define CVLIB_SEQ_FLAG_SHIFT       (CVLIB_SEQ_KIND_BITS + CVLIB_SEQ_ELTYPE_BITS)

/* flags for curves */
#define CVLIB_SEQ_FLAG_CLOSED     (1 << CVLIB_SEQ_FLAG_SHIFT)
#define CVLIB_SEQ_FLAG_SIMPLE     (2 << CVLIB_SEQ_FLAG_SHIFT)
#define CVLIB_SEQ_FLAG_CONVEX     (4 << CVLIB_SEQ_FLAG_SHIFT)
#define CVLIB_SEQ_FLAG_HOLE       (8 << CVLIB_SEQ_FLAG_SHIFT)

/* sequence of the integer numbers */
#define CVLIB_SEQ_INDEX           (CVLIB_SEQ_KIND_GENERIC  | CVLIB_SEQ_ELTYPE_INDEX)

#define CVLIB_SEQ_ELTYPE( seq )   ((seq)->flags & CVLIB_SEQ_ELTYPE_MASK)
#define CVLIB_SEQ_KIND( seq )     ((seq)->flags & CVLIB_SEQ_KIND_MASK )

/* flag checking */
#define CVLIB_IS_SEQ_INDEX( seq )      ((CVLIB_SEQ_ELTYPE(seq) == CVLIB_SEQ_ELTYPE_INDEX) && \
(CVLIB_SEQ_KIND(seq) == CVLIB_SEQ_KIND_GENERIC))

#define CVLIB_IS_SEQ_CURVE( seq )      (CVLIB_SEQ_KIND(seq) == CVLIB_SEQ_KIND_CURVE)
#define CVLIB_IS_SEQ_CLOSED( seq )     (((seq)->flags & CVLIB_SEQ_FLAG_CLOSED) != 0)
#define CVLIB_IS_SEQ_CONVEX( seq )     (((seq)->flags & CVLIB_SEQ_FLAG_CONVEX) != 0)
#define CVLIB_IS_SEQ_HOLE( seq )       (((seq)->flags & CVLIB_SEQ_FLAG_HOLE) != 0)
#define CVLIB_IS_SEQ_SIMPLE( seq )     ((((seq)->flags & CVLIB_SEQ_FLAG_SIMPLE) != 0) || \
                                    CVLIB_IS_SEQ_CONVEX(seq))

#define CVLIB_MEMCPY_AUTO( dst, src, len )                                             \
{                                                                                   \
    int _icv_memcpy_i_, _icv_memcpy_len_ = (len);                                \
    char* _icv_memcpy_dst_ = (char*)(dst);                                          \
    const char* _icv_memcpy_src_ = (const char*)(src);                              \
    if( (_icv_memcpy_len_ & (sizeof(int)-1)) == 0 )                                 \
    {                                                                               \
	assert( ((intptr_t)_icv_memcpy_src_&(sizeof(int)-1)) == 0 &&                  \
	((intptr_t)_icv_memcpy_dst_&(sizeof(int)-1)) == 0 );                  \
	for( _icv_memcpy_i_ = 0; _icv_memcpy_i_ < _icv_memcpy_len_;                 \
	_icv_memcpy_i_+=sizeof(int) )                                           \
        {                                                                           \
		*(int*)(_icv_memcpy_dst_+_icv_memcpy_i_) =                              \
		*(const int*)(_icv_memcpy_src_+_icv_memcpy_i_);                         \
        }                                                                           \
    }                                                                               \
    else                                                                            \
    {                                                                               \
	for(_icv_memcpy_i_ = 0; _icv_memcpy_i_ < _icv_memcpy_len_; _icv_memcpy_i_++)\
	_icv_memcpy_dst_[_icv_memcpy_i_] = _icv_memcpy_src_[_icv_memcpy_i_];    \
    }                                                                               \
}

/* initializes 8-element array for fast access to 3x3 neighborhood of a pixel */
#define  CVLIB_INIT_3X3_DELTAS( deltas, step, nch )            \
    ((deltas)[0] =  (nch),  (deltas)[1] = -(step) + (nch),  \
	(deltas)[2] = -(step), (deltas)[3] = -(step) - (nch),	\
	(deltas)[4] = -(nch),  (deltas)[5] =  (step) - (nch),	\
	(deltas)[6] =  (step), (deltas)[7] =  (step) + (nch))

/******************************** Memory storage ****************************************/

struct CVLIB_DECLSPEC MemBlock
{
    struct MemBlock*  prev;
    struct MemBlock*  next;
};

#define CVLIB_STORAGE_MAGIC_VAL    0x42890000

struct CVLIB_DECLSPEC MemStorage
{
    int signature;
    MemBlock* bottom;/* first allocated block */
    MemBlock* top;   /* current memory block - top of the stack */
    MemStorage* parent; /* borrows new blocks from */
    int block_size;  /* block size */
    int free_space;  /* free space in the current block */
};

#define CVLIB_IS_STORAGE(storage)  \
    ((storage) != NULL &&       \
(((MemStorage*)(storage))->signature & CVLIB_MAGIC_MASK) == CVLIB_STORAGE_MAGIC_VAL)


struct CVLIB_DECLSPEC MemStoragePos
{
    MemBlock* top;
    int free_space;
};


typedef union Cv32suf
{
	int i;
	unsigned u;
	float f;
} Cv32suf;

struct CvPair32s32f
{
	int i;
	float val;
};	

/*********************************** Sequence *******************************************/

struct CVLIB_DECLSPEC SeqBlock
{
    struct SeqBlock*  prev;		/* previous sequence block */
    struct SeqBlock*  next;		/* next sequence block */
	int    start_index;			/* index of the first element in the block +
								sequence->first->start_index */
	int    count;				/* number of elements in the block */
	char*  data;				/* pointer to the first element of the block */
};


#define CVLIB_TREE_NODE_FIELDS(node_type)                          \
    int       flags;         /* micsellaneous flags */          \
    int       header_size;   /* size of sequence header */      \
    struct    node_type* h_prev; /* previous sequence */        \
    struct    node_type* h_next; /* next sequence */            \
    struct    node_type* v_prev; /* 2nd previous sequence */    \
	struct    node_type* v_next;  /* 2nd next sequence */

/*
read/write sequence.
Elements can be dynamically inserted to or deleted from the sequence.
*/
#define CVLIB_SEQUENCE_FIELDS()														\
    CVLIB_TREE_NODE_FIELDS(Sequence)													\
    int       total;          /* total number of elements */						\
    int       elem_size;      /* size of sequence element in bytes */				\
    char*     block_max;      /* maximal bound of the last block */					\
    char*     ptr;            /* current write pointer */							\
    int       delta_elems;    /* how many elements allocated when the seq grows */  \
	MemStorage* storage;    /* where the seq is stored */							\
    SeqBlock* free_blocks;  /* free blocks list */									\
    SeqBlock* first; /* pointer to the first sequence block */

struct CVLIB_DECLSPEC Sequence
{
	Sequence(){};
	CVLIB_SEQUENCE_FIELDS()
};


/****************************************************************************************/
/*                            Sequence writer & reader                                  */
/****************************************************************************************/

#define SEQ_WRITER_FIELDS()											\
    int          header_size;										\
    Sequence*       seq;        /* the sequence written */			\
    SeqBlock*  block;      /* current block */						\
    char*        ptr;        /* pointer to free space */			\
    char*        block_min;  /* pointer to the beginning of block*/	\
char*        block_max;  /* pointer to the end of block */

struct CVLIB_DECLSPEC SeqWriter
{
    SEQ_WRITER_FIELDS()
};


#define SEQ_READER_FIELDS()											\
    int          header_size;                                       \
    Sequence*       seq;        /* sequence, beign read */          \
    SeqBlock*  block;      /* current block */						\
    char*        ptr;        /* pointer to element be read next */  \
    char*        block_min;  /* pointer to the beginning of block */\
    char*        block_max;  /* pointer to the end of block */      \
    int          delta_index;/* = seq->first->start_index   */      \
	char*        prev_elem;  /* pointer to previous element */


struct CVLIB_DECLSPEC SeqReader
{
    SEQ_READER_FIELDS()
};

/*************************************** Set ********************************************/
/*
Set.
Order is not preserved. There can be gaps between sequence elements.
After the element has been inserted it stays in the same place all the time.
The MSB(most-significant or sign bit) of the first field (flags) is 0 iff the element exists.
*/
#define CVLIB_SET_ELEM_FIELDS(elem_type)   \
    int  flags;                         \
struct elem_type* next_free;

struct CVLIB_DECLSPEC SetElem
{
    CVLIB_SET_ELEM_FIELDS(SetElem)
};

#define CVLIB_SET_FIELDS()     \
    CVLIB_SEQUENCE_FIELDS()    \
    SetElem* free_elems;	\
	int active_count;

struct CVLIB_DECLSPEC Set
{
    CVLIB_SET_FIELDS()
};


#define CVLIB_SET_ELEM_IDX_MASK   ((1 << 26) - 1)
#define CVLIB_SET_ELEM_FREE_FLAG  (1 << (sizeof(int)*8-1))

/* Checks whether the element pointed by ptr belongs to a set or not */
#define CVLIB_IS_SET_ELEM( ptr )  (((SetElem*)(ptr))->flags >= 0)


struct CVLIB_DECLSPEC TreeNode
{
    int       flags;         /* micsellaneous flags */
    int       header_size;   /* size of sequence header */
    struct    TreeNode* h_prev; /* previous sequence */
    struct    TreeNode* h_next; /* next sequence */
    struct    TreeNode* v_prev; /* 2nd previous sequence */
    struct    TreeNode* v_next; /* 2nd next sequence */
};

typedef void Arr;





//////////////////////////////////////////////////////////////////////////

/************************************* CvSlice ******************************************/

#define CVLIB_WHOLE_SEQ_END_INDEX 0x3fffffff
#define CVLIB_WHOLE_SEQ  Slice(0, CVLIB_WHOLE_SEQ_END_INDEX)


#define  CVLIB_MALLOC_ALIGN    32

#define ICV_FREE_PTR(storage)  \
    ((char*)(storage)->top + (storage)->block_size - (storage)->free_space)

#define ICV_ALIGNED_SEQ_BLOCK_SIZE  \
    (int)alignSize(sizeof(SeqBlock), CVLIB_STRUCT_ALIGN)

#define CVLIB_WRITE_SEQ_ELEM( elem, writer )             \
{                                                     \
    assert( (writer).seq->elem_size == sizeof(elem)); \
    if( (writer).ptr >= (writer).block_max )          \
    {                                                 \
		createSeqBlock( &writer);                   \
    }                                                 \
    assert( (writer).ptr <= (writer).block_max - sizeof(elem));\
    memcpy((writer).ptr, &(elem), sizeof(elem));      \
    (writer).ptr += sizeof(elem);                     \
}

/* move reader position forward */
#define CVLIB_NEXT_SEQ_ELEM( elem_size, reader )                 \
{                                                             \
    if( ((reader).ptr += (elem_size)) >= (reader).block_max ) \
    {                                                         \
		changeSeqBlock( &(reader), 1 );                     \
    }                                                         \
}

/* read element and move read position forward */
#define CVLIB_READ_SEQ_ELEM( elem, reader )                       \
{                                                              \
    assert( (reader).seq->elem_size == sizeof(elem));          \
    memcpy( &(elem), (reader).ptr, sizeof((elem)));            \
    CVLIB_NEXT_SEQ_ELEM( sizeof(elem), reader )                   \
}

/* maximum size of dynamic memory buffer.
cvAlloc reports an error if a larger block is requested. */
#define  CVLIB_MAX_ALLOC_SIZE    (((int)1 << (sizeof(int)*8-2)))

#define CVLIB_MEMCPY_INT( dst, src, len )                                              \
{                                                                                   \
    int _icv_memcpy_i_, _icv_memcpy_len_ = (len);                                \
    int* _icv_memcpy_dst_ = (int*)(dst);                                            \
    const int* _icv_memcpy_src_ = (const int*)(src);                                \
    assert( ((intptr_t)_icv_memcpy_src_&(sizeof(int)-1)) == 0 &&                      \
	((intptr_t)_icv_memcpy_dst_&(sizeof(int)-1)) == 0 );                      \
	\
    for(_icv_memcpy_i_=0;_icv_memcpy_i_<_icv_memcpy_len_;_icv_memcpy_i_++)          \
	_icv_memcpy_dst_[_icv_memcpy_i_] = _icv_memcpy_src_[_icv_memcpy_i_];        \
}

#define CVLIB_GET_LAST_ELEM( seq, block ) \
    ((block)->data + ((block)->count - 1)*((seq)->elem_size))

/* default storage block size */
#define  CVLIB_STORAGE_BLOCK_SIZE   ((1<<16) - 128)

/* type checking macros */
#define CVLIB_IS_SEQ_POINT_SET( seq ) \
    ((CVLIB_SEQ_ELTYPE(seq) == CVLIB_32SC2 || CVLIB_SEQ_ELTYPE(seq) == CVLIB_32FC2))

#define CVLIB_IS_MAT_HDR(mat) \
    ((mat) != NULL && \
    (CVLIB_MAGIC_MASK) == CVLIB_MAT_MAGIC_VAL && \
    ((const Mat*)(mat))->cols() > 0 && ((const Mat*)(mat))->rows() > 0)

#define CVLIB_IS_MAT(mat) \
    (CVLIB_IS_MAT_HDR(mat) && ((const Mat*)(mat))->data.ptr != NULL)

#define ICVLIB_SHIFT_TAB_MAX 32

//////////////////////////////////////////////////////////////////////////

// default <free>
inline static int defaultFree( void* ptr, void* )
{
    // Pointer must be aligned by CVLIB_MALLOC_ALIGN
    if( ((intptr_t)ptr & (CVLIB_MALLOC_ALIGN-1)) != 0 )
        return -49;
    free( *((char**)ptr - 1) );
	
    return 0;
}
inline static void* defaultAlloc( int size, void* )
{
    char *ptr, *ptr0 = (char*)malloc(
        (int)(size + CVLIB_MALLOC_ALIGN*((size >= 4096) + 1) + sizeof(char*)));
	
    if( !ptr0 )
        return 0;
	
    // align the pointer
    ptr = (char*)alignPtr(ptr0 + sizeof(char*) + 1, CVLIB_MALLOC_ALIGN);
    *(char**)(ptr - sizeof(char*)) = ptr0;
	
    return ptr;
}

#define cvFree(ptr) (cvFree_(*(ptr)), *(ptr)=0)

// pointers to allocation functions, initially set to default
typedef void* (/*CV_CDECL*/ *AllocFunc)(int size, void* userdata);
typedef int (/*CV_CDECL*/ *FreeFunc)(void* pptr, void* userdata);

static AllocFunc p_cvAlloc = defaultAlloc;
static FreeFunc p_cvFree = defaultFree;
static void* p_cvAllocUserData = 0;

inline bool cvFree_( void* ptr )
{	
    if( ptr )
    {
        int status = p_cvFree( ptr, p_cvAllocUserData );
        if( status < 0 )
            return false ;
    }
	return true ;
}

/* finds sequence element by its index */
CVLIB_DECLSPEC char* getSeqElem( const Sequence *seq, int index ) ;

/* pushes element to the sequence */
CVLIB_DECLSPEC char* seqPush( Sequence *seq, const void *element ) ;

/* pops the last element out of the sequence */
CVLIB_DECLSPEC int seqPop( Sequence *seq, void *element ) ;

/* recycles a sequence block for the further use */
CVLIB_DECLSPEC int freeSeqBlock( Sequence *seq, int in_front_of ) ;

/* constructs sequence from array without copying any data.
the resultant sequence can't grow above its initial size */
CVLIB_DECLSPEC Sequence* makeSeqHeaderForArray( int seq_flags, int header_size, int elem_size, void *array, 
								int total, Sequence *seq, SeqBlock * block ) ;

CVLIB_DECLSPEC int sliceLength( Slice slice, const Sequence* seq ) ;

/* returns the current reader position */
CVLIB_DECLSPEC int getSeqReaderPos( SeqReader* reader ) ;

/* Adds several new elements to the end of sequence */
CVLIB_DECLSPEC void  seqPushMulti( Sequence* seq, const void* elements, int count, int in_front=0 );
/* removes several elements from the end of sequence */
CVLIB_DECLSPEC char* seqPushFront( Sequence *seq, const void *element );
CVLIB_DECLSPEC void seqPopMulti( Sequence *seq, void *_elements, int count, int front=0 ) ;
CVLIB_DECLSPEC void clearSeq(Sequence *seq);
CVLIB_DECLSPEC void seqPopFront( Sequence* seq, void* element);
CVLIB_DECLSPEC char* seqInsert( Sequence *seq, int before_index, const void *element );
CVLIB_DECLSPEC void seqInsertSlice( Sequence* seq, int index, const Arr* from_arr );
CVLIB_DECLSPEC void seqRemove( Sequence *seq, int index );
CVLIB_DECLSPEC void seqRemoveSlice( Sequence* seq, Slice slice );

/* sets reader position to given absolute or relative
(relatively to the current one) position */
CVLIB_DECLSPEC int setSeqReaderPos( SeqReader* reader, int index, int is_relative = 0 ) ;
CVLIB_DECLSPEC int changeSeqBlock( void* _reader, int direction );
/*=====================================Memory Functions=========================================*/

/****************************************Sequence Operations*************************************/

/****************************************************************************************/
/*                                Operations on sequences                               */
/****************************************************************************************/

#define  CVLIB_SEQ_ELEM( seq, elem_type, index )                    \
	/* assert gives some guarantee that <seq> parameter is valid */  \
	(   assert(sizeof((seq)->first[0]) == sizeof(SeqBlock) &&      \
	(seq)->elem_size == sizeof(elem_type)),                      \
	(elem_type*)((seq)->first && (unsigned)index <               \
	(unsigned)((seq)->first->count) ?                            \
	(seq)->first->data + (index) * sizeof(elem_type) :           \
	getSeqElem( (Sequence*)(seq), (index) )))
#define CVLIB_GET_SEQ_ELEM( elem_type, seq, index ) CVLIB_SEQ_ELEM( (seq), elem_type, (index) )

/* add element to sequence: */
#define CVLIB_WRITE_SEQ_ELEM_VAR( elem_ptr, writer )     \
{                                                     \
	if( (writer).ptr >= (writer).block_max )          \
{                                                 \
	createSeqBlock( &writer);                   \
}                                                 \
	memcpy((writer).ptr, elem_ptr, (writer).seq->elem_size);\
	(writer).ptr += (writer).seq->elem_size;          \
}

/* Move reader position backward: */
#define CVLIB_PREV_SEQ_ELEM( elem_size, reader )                \
{                                                            \
	if( ((reader).ptr -= (elem_size)) < (reader).block_min ) \
{                                                        \
	changeSeqBlock( &(reader), -1 );                   \
}                                                        \
}

/* read element and move read position forward: */
#define CVLIB_READ_SEQ_ELEM( elem, reader )                       \
{                                                              \
	assert( (reader).seq->elem_size == sizeof(elem));          \
	memcpy( &(elem), (reader).ptr, sizeof((elem)));            \
	CVLIB_NEXT_SEQ_ELEM( sizeof(elem), reader )                   \
}

/* read element and move read position backward: */
#define CVLIB_REV_READ_SEQ_ELEM( elem, reader )                     \
{                                                                \
	assert( (reader).seq->elem_size == sizeof(elem));            \
	memcpy(&(elem), (reader).ptr, sizeof((elem)));               \
	CVLIB_PREV_SEQ_ELEM( sizeof(elem), reader )                     \
}


#define CVLIB_READ_CHAIN_POINT( _pt, reader )                              \
{                                                                       \
	(_pt) = (reader).pt;                                                \
	if( (reader).ptr )                                                  \
{                                                                   \
	CVLIB_READ_SEQ_ELEM( (reader).code, (reader));                     \
	assert( ((reader).code & ~7) == 0 );                            \
	(reader).pt.x += (reader).deltas[(int)(reader).code][0];        \
	(reader).pt.y += (reader).deltas[(int)(reader).code][1];        \
}                                                                   \
}

#define CVLIB_CURRENT_POINT( reader )  (*((vec2i*)((reader).ptr)))
#define CVLIB_PREV_POINT( reader )     (*((vec2i*)((reader).prev_elem)))

#define CVLIB_READ_EDGE( pt1, pt2, reader )               \
{                                                      \
	assert( sizeof(pt1) == sizeof(CvPoint) &&          \
	sizeof(pt2) == sizeof(CvPoint) &&          \
	reader.seq->elem_size == sizeof(CvPoint)); \
	(pt1) = CVLIB_PREV_POINT( reader );                   \
	(pt2) = CVLIB_CURRENT_POINT( reader );                \
	(reader).prev_elem = (reader).ptr;                 \
	CVLIB_NEXT_SEQ_ELEM( sizeof(CvPoint), (reader));      \
}

CVLIB_DECLSPEC void* alloc( int size ) ;

CVLIB_DECLSPEC Sequence* createSeq( int seq_flags, int header_size, int elem_size, MemStorage * storage ) ;

CVLIB_DECLSPEC int createSeqBlock( SeqWriter * writer ) ;

CVLIB_DECLSPEC int insertNodeIntoTree( void* _node, void* _parent, void* _frame ) ;

CVLIB_DECLSPEC int setSeqBlockSize( Sequence *seq, int delta_elements ) ;

CVLIB_DECLSPEC int startAppendToSeq( Sequence *seq, SeqWriter * writer ) ;

CVLIB_DECLSPEC int startReadSeq( const Sequence *seq, SeqReader * reader, int reverse=0 ) ;

CVLIB_DECLSPEC int startWriteSeq( int seq_flags, int header_size, int elem_size, MemStorage * storage, SeqWriter * writer ) ;

CVLIB_DECLSPEC Rect boundingRect( Arr* array, int update ) ;

//SeqWiriter funcs

/* calls icvFlushSeqWriter and finishes writing process */
CVLIB_DECLSPEC Sequence* endWriteSeq( SeqWriter * writer ) ;

CVLIB_DECLSPEC int flushSeqWriter( SeqWriter * writer ) ;

CVLIB_DECLSPEC int growSeq( Sequence *seq, int in_front_of ) ;

CVLIB_DECLSPEC int goNextMemBlock( MemStorage * storage ) ;

CVLIB_DECLSPEC int setAdd( Set* set, SetElem* element, SetElem** inserted_element ) ;

//memstorage

CVLIB_DECLSPEC MemStorage* createMemStorage( int block_size=0 ) ;

/* creates empty set */
CVLIB_DECLSPEC Set* createSet( int set_flags, int header_size, int elem_size, MemStorage * storage ) ;

/* creates child memory storage */
CVLIB_DECLSPEC MemStorage* createChildMemStorage( MemStorage * parent ) ;

/* initializes allocated storage */
CVLIB_DECLSPEC int initMemStorage( MemStorage* storage, int block_size ) ;

CVLIB_DECLSPEC int saveMemStoragePos( const MemStorage * storage, MemStoragePos * pos ) ;

CVLIB_DECLSPEC int restoreMemStoragePos( MemStorage * storage, MemStoragePos * pos ) ;

CVLIB_DECLSPEC int clearMemStorage( MemStorage * storage ) ;

CVLIB_DECLSPEC int releaseMemStorage( MemStorage** storage ) ;

CVLIB_DECLSPEC void* memStorageAlloc( MemStorage* storage, int size ) ;

CVLIB_DECLSPEC int destroyMemStorage( MemStorage* storage ) ;
CVLIB_DECLSPEC int iMemCopy( double **buf1, double **buf2, double **buf3, int *b_max );

}
