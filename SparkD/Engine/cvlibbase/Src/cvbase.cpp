/*!
 * \file
 * \brief .
 * \author 
 */

#include "cvbase.h"
#include "Mat.h"

namespace cvlib
{

//////////////////////////////////////////////////////////////////////////

/*************************************************************************************/
int saveMemStoragePos( const MemStorage * storage, MemStoragePos * pos )
{	
    if( !storage || !pos )
        return -27;
	
    pos->top = storage->top;
    pos->free_space = storage->free_space;
	return 1 ;
}
/*************************************************************************************/
int restoreMemStoragePos( MemStorage * storage, MemStoragePos * pos )
{	
    if( !storage || !pos )
        return -27;
    if( pos->free_space > storage->block_size )
        return -201;		
	
    storage->top = pos->top;
    storage->free_space = pos->free_space;
	
    if( !storage->top )
    {
        storage->top = storage->bottom;
        storage->free_space = storage->top ? storage->block_size - sizeof(MemBlock) : 0;
    }
    return 1 ;
}
/*************************************************************************************/
int clearMemStorage( MemStorage * storage )
{	
    if( !storage )
        return -27;
	
    if( storage->parent )
    {
        destroyMemStorage( storage );
    }
    else
    {
        storage->top = storage->bottom;
        storage->free_space = storage->bottom ? storage->block_size - sizeof(MemBlock) : 0;
    }
	return 1 ;
}

/*************************************************************************************/
int destroyMemStorage( MemStorage* storage )
{	
    int k = 0;
	
    MemBlock *block;
    MemBlock *dst_top = 0;
	
    if( !storage )
        return -27 ;
	
    if( storage->parent )
        dst_top = storage->parent->top;
	
    for( block = storage->bottom; block != 0; k++ )
    {
        MemBlock *temp = block;
		
        block = block->next;
        if( storage->parent )
        {
            if( dst_top )
            {
                temp->prev = dst_top;
                temp->next = dst_top->next;
                if( temp->next )
                    temp->next->prev = temp;
                dst_top = dst_top->next = temp;
            }
            else
            {
                dst_top = storage->parent->bottom = storage->parent->top = temp;
                temp->prev = temp->next = 0;
                storage->free_space = storage->block_size - sizeof( *temp );
            }
        }
        else
        {
            cvFree( &temp );
        }
    }
	
    storage->top = storage->bottom = 0;
    storage->free_space = 0;
	
    return 1;
}

/*************************************************************************************/
int insertNodeIntoTree( void* _node, void* _parent, void* _frame )
{	
    TreeNode* node = (TreeNode*)_node;
    TreeNode* parent = (TreeNode*)_parent;
	
    if( !node || !parent )
        return -27 ;
	
    node->v_prev = _parent != _frame ? parent : 0;
    node->h_next = parent->v_next;
	
    assert( parent->v_next != node );
	
    if( parent->v_next )
        parent->v_next->h_prev = node;
    parent->v_next = node;
	
	return 1 ;
}
/*************************************************************************************/
/* creates empty sequence */
Sequence* createSeq( int seq_flags, int header_size, int elem_size, MemStorage * storage )
{
    Sequence *seq = 0;   
	
    if( !storage )
        return NULL;
    if( header_size < (int)sizeof( Sequence ) || elem_size <= 0 )
        return NULL ;
	
    /* allocate sequence header */
    seq = (Sequence*)memStorageAlloc( storage, header_size ) ;

    memset( seq, 0, header_size );
	
    seq->header_size = header_size;
    seq->flags = (seq_flags & ~CVLIB_MAGIC_MASK) | CVLIB_SEQ_MAGIC_VAL;
    {
        int elemtype = CVLIB_MAT_TYPE(seq_flags);
        int typesize = CVLIB_ELEM_SIZE2(elemtype);
		
        if( elemtype != CVLIB_SEQ_ELTYPE_GENERIC && typesize != 0 && typesize != elem_size )
			return NULL ;
    }
    seq->elem_size = elem_size;
    seq->storage = storage;
	
    setSeqBlockSize( seq, (1 << 10)/elem_size ) ;  
	
    return seq;
}
/*************************************************************************************/
/* adjusts <delta_elems> field of sequence. It determines how much the sequence
grows if there are no free space inside the sequence buffers */
int setSeqBlockSize( Sequence *seq, int delta_elements )
{
    int elem_size;
    int useful_block_size;    
	
    if( !seq || !seq->storage )
        return -27 ;
    if( delta_elements < 0 )
        return -211 ;
	
    useful_block_size = alignLeft(seq->storage->block_size - 
		sizeof(MemBlock) - sizeof(SeqBlock), CVLIB_STRUCT_ALIGN) ;
    elem_size = seq->elem_size;
	
    if( delta_elements == 0 )
    {
        delta_elements = (1 << 10) / elem_size;
        delta_elements = MAX( delta_elements, 1 );
    }
    if( delta_elements * elem_size > useful_block_size )
    {
        delta_elements = useful_block_size / elem_size;
        if( delta_elements == 0 )
            return -211 ;
    }
	
    seq->delta_elems = delta_elements;
	
   return 1 ;
}
/*************************************************************************************/
void* memStorageAlloc( MemStorage* storage, int size )
{
    char *ptr = 0;    
    
    if( !storage )
        return NULL ;
	
    if( size > INT_MAX )
        return NULL;
	
    assert( storage->free_space % CVLIB_STRUCT_ALIGN == 0 );
	
    if( (int)storage->free_space < size )
    {
        int max_free_space = alignLeft(storage->block_size - sizeof(MemBlock), CVLIB_STRUCT_ALIGN);

        if( max_free_space < size )
            return NULL;
		
        goNextMemBlock( storage ) ;
    }
	
    ptr = ICV_FREE_PTR(storage);
    assert( (intptr_t)ptr % CVLIB_STRUCT_ALIGN == 0 );
    storage->free_space = alignLeft(storage->free_space - (int)size, CVLIB_STRUCT_ALIGN );    
	
    return ptr;
}

/*************************************************************************************/
/* initializes sequence writer */
int startAppendToSeq( Sequence *seq, SeqWriter * writer )
{	
    if( !seq || !writer )
        return -27 ;
	
    memset( writer, 0, sizeof( *writer ));
    writer->header_size = sizeof( SeqWriter );
	
    writer->seq = seq;
    writer->block = seq->first ? seq->first->prev : 0;
    writer->ptr = seq->ptr;
    writer->block_max = seq->block_max;
	return 1 ;
}

/*************************************************************************************/
int createSeqBlock( SeqWriter * writer )
{    
    Sequence *seq;
	
    if( !writer || !writer->seq )
        return -27 ;
	
    seq = writer->seq;
	
    flushSeqWriter( writer );	
    growSeq( seq, 0 ) ;
	
    writer->block = seq->first->prev;
    writer->ptr = seq->ptr;
    writer->block_max = seq->block_max;
	
    return 1 ;
}

/*************************************************************************************/
/* updates sequence header */
int flushSeqWriter( SeqWriter * writer )
{
    Sequence *seq = 0;    
	
    if( !writer )
        return -27 ;
	
    seq = writer->seq;
    seq->ptr = writer->ptr;
	
    if( writer->block )
    {
        int total = 0;
        SeqBlock *first_block = writer->seq->first;
        SeqBlock *block = first_block;
		
        writer->block->count = (int)((writer->ptr - writer->block->data) / seq->elem_size);
        assert( writer->block->count > 0 );
		
        do
        {
            total += block->count;
            block = block->next;
        }
        while( block != first_block );
		
        writer->seq->total = total;
    }
	
    return 1 ;
}
/*************************************************************************************/
int growSeq( Sequence *seq, int in_front_of )
{
    SeqBlock *block;

    if( !seq )
        return -27;
    block = seq->free_blocks;

    if( !block )
    {
        int elem_size = seq->elem_size;
        int delta_elems = seq->delta_elems;
        MemStorage *storage = seq->storage;

        if( seq->total >= delta_elems*4 )
            setSeqBlockSize( seq, delta_elems*2 );

        if( !storage )
            return -27 ;

        /* if there is a free space just after last allocated block
           and it's big enough then enlarge the last block
           (this can happen only if the new block is added to the end of sequence */
        if( (unsigned)(ICV_FREE_PTR(storage) - seq->block_max) < CVLIB_STRUCT_ALIGN &&
            storage->free_space >= seq->elem_size && !in_front_of )
        {
            int delta = storage->free_space / elem_size;

            delta = MIN( delta, delta_elems ) * elem_size;
            seq->block_max += delta;
            storage->free_space = alignLeft((int)(((char*)storage->top + storage->block_size) -
                                              seq->block_max), CVLIB_STRUCT_ALIGN );
            return 1;
        }
        else
        {
            int delta = elem_size * delta_elems + ICV_ALIGNED_SEQ_BLOCK_SIZE;

            /* try to allocate <delta_elements> elements */
            if( storage->free_space < delta )
            {
                int small_block_size = MAX(1, delta_elems/3)*elem_size +
                                       ICV_ALIGNED_SEQ_BLOCK_SIZE;
                /* try to allocate smaller part */
                if( storage->free_space >= small_block_size + CVLIB_STRUCT_ALIGN )
                {
                    delta = (storage->free_space - ICV_ALIGNED_SEQ_BLOCK_SIZE)/seq->elem_size;
                    delta = delta*seq->elem_size + ICV_ALIGNED_SEQ_BLOCK_SIZE;
                }
                else
                {
                    goNextMemBlock( storage);
                    assert( storage->free_space >= delta );
                }
            }

            block = (SeqBlock*)memStorageAlloc( storage, delta ) ;
            block->data = (char*)alignPtr( block + 1, CVLIB_STRUCT_ALIGN );
            block->count = delta - ICV_ALIGNED_SEQ_BLOCK_SIZE;
            block->prev = block->next = 0;
        }
    }
    else
    {
        seq->free_blocks = block->next;
    }

    if( !(seq->first) )
    {
        seq->first = block;
        block->prev = block->next = block;
    }
    else
    {
        block->prev = seq->first->prev;
        block->next = seq->first;
        block->prev->next = block->next->prev = block;
    }

    /* for free blocks the <count> field means total number of bytes in the block.
       And for used blocks it means a current number of sequence
       elements in the block */
    assert( block->count % seq->elem_size == 0 && block->count > 0 );

    if( !in_front_of )
    {
        seq->ptr = block->data;
        seq->block_max = block->data + block->count;
        block->start_index = block == block->prev ? 0 :
            block->prev->start_index + block->prev->count;
    }
    else
    {
        int delta = block->count / seq->elem_size;
        block->data += block->count;

        if( block != block->prev )
        {
            assert( seq->first->start_index == 0 );
            seq->first = block;
        }
        else
        {
            seq->block_max = seq->ptr = block->data;
        }

        block->start_index = 0;

        for( ;; )
        {
            block->start_index += delta;
            block = block->next;
            if( block == seq->first )
                break;
        }
    }

    block->count = 0;

    return 1 ;
}
/*************************************************************************************/
int goNextMemBlock( MemStorage * storage )
{    
    if( !storage )
        return -27;
	
    if( !storage->top || !storage->top->next )
    {
        MemBlock *block = 0;
		
        if( !(storage->parent) )
        {
			block = (MemBlock *)alloc( storage->block_size );
            if( !block )
				return -27 ;
        }
        else
        {
            MemStorage *parent = storage->parent;
            MemStoragePos parent_pos;
			
            saveMemStoragePos( parent, &parent_pos );
            goNextMemBlock( parent );
			
            block = parent->top;
            restoreMemStoragePos( parent, &parent_pos );
			
            if( block == parent->top )  /* the single allocated block */
            {
                assert( parent->bottom == block );
                parent->top = parent->bottom = 0;
                parent->free_space = 0;
            }
            else
            {
                /* cut the block from the parent's list of blocks */
                parent->top->next = block->next;
                if( block->next )
                    block->next->prev = parent->top;
            }
        }
		
        /* link block */
        block->next = 0;
        block->prev = storage->top;
		
        if( storage->top )
            storage->top->next = block;
        else
            storage->top = storage->bottom = block;
    }
	
    if( storage->top->next )
        storage->top = storage->top->next;
    storage->free_space = storage->block_size - sizeof(MemBlock);
    assert( storage->free_space % CVLIB_STRUCT_ALIGN == 0 );

    return 1 ;
}
/*************************************************************************************/
void*  alloc( int size )
{
    void* ptr = 0;   
    
    if( (int)size > CVLIB_MAX_ALLOC_SIZE )
        return NULL ;
	
    ptr = p_cvAlloc( size, p_cvAllocUserData );

    if( !ptr )
        return NULL ;
	
    return ptr;
}
/*************************************************************************************/
/* calls icvFlushSeqWriter and finishes writing process */
Sequence* endWriteSeq( SeqWriter * writer )
{
    Sequence *seq = 0 ;	
    
    if( !writer )
        return NULL ;
	
    flushSeqWriter( writer ) ;
    seq = writer->seq;
	
    /* truncate the last block */
    if( writer->block && writer->seq->storage )
    {
        MemStorage *storage = seq->storage;
        char *storage_block_max = (char *) storage->top + storage->block_size;
		
        assert( writer->block->count > 0 );
		
        if( (unsigned)((storage_block_max - storage->free_space)
            - seq->block_max) < CVLIB_STRUCT_ALIGN )
        {
            storage->free_space = alignLeft((int)(storage_block_max - seq->ptr), CVLIB_STRUCT_ALIGN);
            seq->block_max = seq->ptr;
        }
    }
	
    writer->ptr = 0;   
	
    return seq;
}
/*************************************************************************************/

/*************************************************************************************/
/* adds new element to the set */
int setAdd( Set* set, SetElem* element, SetElem** inserted_element )
{
    int id = -1;	
    SetElem *free_elem;
	
    if( !set )
        return -27 ;
	
    if( !(set->free_elems) )
    {
        int count = set->total;
        int elem_size = set->elem_size;
        char *ptr;
        growSeq( (Sequence *) set, 0 );
		
        set->free_elems = (SetElem*) (ptr = set->ptr);
        for( ; ptr + elem_size <= set->block_max; ptr += elem_size, count++ )
        {
            ((SetElem*)ptr)->flags = count | CVLIB_SET_ELEM_FREE_FLAG;
            ((SetElem*)ptr)->next_free = (SetElem*)(ptr + elem_size);
        }
        assert( count <= CVLIB_SET_ELEM_IDX_MASK+1 );
        ((SetElem*)(ptr - elem_size))->next_free = 0;
        set->first->prev->count += count - set->total;
        set->total = count;
        set->ptr = set->block_max;
    }
	
    free_elem = set->free_elems;
    set->free_elems = free_elem->next_free;
	
    id = free_elem->flags & CVLIB_SET_ELEM_IDX_MASK;
    if( element )
        CVLIB_MEMCPY_INT( free_elem, element, (int)set->elem_size/sizeof(int) );
	
    free_elem->flags = id;
    set->active_count++;
	
    if( inserted_element )
        *inserted_element = free_elem;   
	
    return id;
}
/*************************************************************************************/
int startWriteSeq( int seq_flags, int header_size,
				int elem_size, MemStorage * storage, SeqWriter * writer )
{
    Sequence *seq = 0;
	
    if( !storage || !writer )
        return -27 ;
	
    seq = createSeq( seq_flags, header_size, elem_size, storage ) ;
    if( !startAppendToSeq( seq, writer ) )
		return -27 ;

	return 1 ;
}

/*************************************************************************************/
/* initializes sequence reader */
int startReadSeq( const Sequence *seq, SeqReader * reader, int reverse )
{
    SeqBlock *first_block;
    SeqBlock *last_block;
    
    if( reader )
    {
        reader->seq = 0;
        reader->block = 0;
        reader->ptr = reader->block_max = reader->block_min = 0;
    }    
	
    if( !seq || !reader )
        return -27 ;
	
    reader->header_size = sizeof( SeqReader );
    reader->seq = (Sequence*)seq;
	
    first_block = seq->first;
	
    if( first_block )
    {
        last_block = first_block->prev;
        reader->ptr = first_block->data;
        reader->prev_elem = CVLIB_GET_LAST_ELEM( seq, last_block );
        reader->delta_index = seq->first->start_index;
		
        if( reverse )
        {
            char *temp = reader->ptr;
			
            reader->ptr = reader->prev_elem;
            reader->prev_elem = temp;
			
            reader->block = last_block;
        }
        else
        {
            reader->block = first_block;
        }
		
        reader->block_min = reader->block->data;
        reader->block_max = reader->block_min + reader->block->count * seq->elem_size;
    }
    else
    {
        reader->delta_index = 0;
        reader->block = 0;
		reader->ptr = reader->prev_elem = reader->block_min = reader->block_max = 0;
    }
	return 1 ;
}

/*************************************************************************************/
/* changes the current reading block to the previous or to the next */
int changeSeqBlock( void* _reader, int direction )
{	
    SeqReader* reader = (SeqReader*)_reader;
    
    if( !reader )
        return -27 ;
	
    if( direction > 0 )
    {
        reader->block = reader->block->next;
        reader->ptr = reader->block->data;
    }
    else
    {
        reader->block = reader->block->prev;
        reader->ptr = CVLIB_GET_LAST_ELEM( reader->seq, reader->block );
    }
    reader->block_min = reader->block->data;
    reader->block_max = reader->block_min + reader->block->count * reader->seq->elem_size;
	
    return 1 ;;
}

/*************************************************************************************/
/* releases memory storage */
int releaseMemStorage( MemStorage** storage )
{
    MemStorage *st;
	
    if( !storage )
        return -27 ;
	
    st = *storage;
    *storage = 0;
	
    if( st )
    {
        destroyMemStorage( st );
        cvFree( &st );
    }
    return 1 ;
}
/*************************************************************************************/
MemStorage* createMemStorage( int block_size )
{
    MemStorage *storage = 0;    
	
    storage = (MemStorage *)alloc( sizeof( MemStorage ));
    initMemStorage( storage, block_size ) ;
    
	
//     if( cvGetErrStatus() < 0 )
//         cvFree( &storage );
	
    return storage;
}
/*************************************************************************************/
/* initializes allocated storage */
int initMemStorage( MemStorage* storage, int block_size )
{    
    if( !storage )
        return -27 ;
	
    if( block_size <= 0 )
        block_size = CVLIB_STORAGE_BLOCK_SIZE;
	
    block_size = (int)alignSize( block_size, CVLIB_STRUCT_ALIGN );
    assert( sizeof(MemBlock) % CVLIB_STRUCT_ALIGN == 0 );
	
    memset( storage, 0, sizeof( *storage ));
    storage->signature = CVLIB_STORAGE_MAGIC_VAL;
    storage->block_size = block_size;
	
    return 1 ;
}
/*************************************************************************************/
MemStorage* createChildMemStorage( MemStorage * parent )
{
    MemStorage *storage = 0;
    
    if( !parent )
        return NULL;
	
    storage = createMemStorage(parent->block_size);
    storage->parent = parent;	
   
//     if( cvGetErrStatus() < 0 )
//         cvFree( &storage );
	
    return storage;
}
/*************************************************************************************/
Set* createSet( int set_flags, int header_size, int elem_size, MemStorage * storage )
{
    Set *set = 0;
    
    if( !storage )
        return NULL;
    if( header_size < (int)sizeof( Set ) || elem_size < (int)sizeof(void*)*2 ||
        (elem_size & (sizeof(void*)-1)) != 0 )
        return NULL ;
	
    set = (Set*) createSeq( set_flags, header_size, elem_size, storage );
    set->flags = (set->flags & ~CVLIB_MAGIC_MASK) | CVLIB_SET_MAGIC_VAL;   
	
    return set;
}

/*************************************************************************************/
char* getSeqElem( const Sequence *seq, int index )
{
    SeqBlock *block;
    int count, total = seq->total;
	
    if( (unsigned)index >= (unsigned)total )
    {
        index += index < 0 ? total : 0;
        index -= index >= total ? total : 0;
        if( (unsigned)index >= (unsigned)total )
            return 0;
    }
	
    block = seq->first;
    if( index + index <= total )
    {
        while( index >= (count = block->count) )
        {
            block = block->next;
            index -= count;
        }
    }
    else
    {
        do
        {
            block = block->prev;
            total -= block->count;
        }
        while( index < total );
        index -= total;
    }
	
    return block->data + index * seq->elem_size;
}
/*************************************************************************************/
/* pushes element to the sequence */
char* seqPush( Sequence *seq, const void *element )
{
    char *ptr = 0;
    int elem_size;   
	
    if( !seq )
        return NULL;
	
    elem_size = seq->elem_size;
    ptr = seq->ptr;
	
    if( ptr >= seq->block_max )
    {
        growSeq( seq, 0 ) ;
		
        ptr = seq->ptr;
        assert( ptr + elem_size <= seq->block_max /*&& ptr == seq->block_min */  );
    }
	
    if( element )
        CVLIB_MEMCPY_AUTO( ptr, element, elem_size );
    seq->first->prev->count++;
    seq->total++;
    seq->ptr = ptr + elem_size;    
	
    return ptr;
}
/*************************************************************************************/
/* pops the last element out of the sequence */
int seqPop( Sequence *seq, void *element )
{
    char *ptr;
    int elem_size;   
	
    if( !seq )
        return -27 ;
    if( seq->total <= 0 )
        return -201 ;
	
    elem_size = seq->elem_size;
    seq->ptr = ptr = seq->ptr - elem_size;
	
    if( element )
        CVLIB_MEMCPY_AUTO( element, ptr, elem_size );
    seq->ptr = ptr;
    seq->total--;
	
    if( --(seq->first->prev->count) == 0 )
    {
        freeSeqBlock( seq, 0 );
        assert( seq->ptr == seq->block_max );
    }
	
   return 1 ;
}
/*************************************************************************************/
/* recycles a sequence block for the further use */
int freeSeqBlock( Sequence *seq, int in_front_of )
{	
    SeqBlock *block = seq->first;
	
    assert( (in_front_of ? block : block->prev)->count == 0 );
	
    if( block == block->prev )  /* single block case */
    {
        block->count = (int)(seq->block_max - block->data) + block->start_index * seq->elem_size;
        block->data = seq->block_max - block->count;
        seq->first = 0;
        seq->ptr = seq->block_max = 0;
        seq->total = 0;
    }
    else
    {
        if( !in_front_of )
        {
            block = block->prev;
            assert( seq->ptr == block->data );
			
            block->count = (int)(seq->block_max - seq->ptr);
            seq->block_max = seq->ptr = block->prev->data +
                block->prev->count * seq->elem_size;
        }
        else
        {
            int delta = block->start_index;
			
            block->count = delta * seq->elem_size;
            block->data -= block->count;
			
            /* update start indices of sequence blocks */
            for( ;; )
            {
                block->start_index -= delta;
                block = block->next;
                if( block == seq->first )
                    break;
            }
			
            seq->first = block->next;
        }
		
        block->prev->next = block->next;
        block->next->prev = block->prev;
    }
	
    assert( block->count > 0 && block->count % seq->elem_size == 0 );
    block->next = seq->free_blocks;
    seq->free_blocks = block;
	return 1 ;
}

/*************************************************************************************/
/* constructs sequence from array without copying any data.
the resultant sequence can't grow above its initial size */
Sequence* makeSeqHeaderForArray( int seq_flags, int header_size, int elem_size,
						void *array, int total, Sequence *seq, SeqBlock * block )
{
    Sequence* result = 0;    
	
    if( elem_size <= 0 || header_size < (int)sizeof( Sequence ) || total < 0 )
        return NULL ;
	
    if( !seq || ((!array || !block) && total > 0) )
        return NULL ;
	
    memset( seq, 0, header_size );
	
    seq->header_size = header_size;
    seq->flags = (seq_flags & ~CVLIB_MAGIC_MASK) | CVLIB_SEQ_MAGIC_VAL;
    {
        int elemtype = CVLIB_MAT_TYPE(seq_flags);
        int typesize = CVLIB_ELEM_SIZE2(elemtype);
		
        if( elemtype != CVLIB_SEQ_ELTYPE_GENERIC &&
            typesize != 0 && typesize != elem_size )
            return NULL ;
    }
    seq->elem_size = elem_size;
    seq->total = total;
    seq->block_max = seq->ptr = (char *) array + total * elem_size;
	
    if( total > 0 )
    {
        seq->first = block;
        block->prev = block->next = block;
        block->start_index = 0;
        block->count = total;
        block->data = (char *) array;
    }
	
    result = seq;    
	
    return result;
}
/*************************************************************************************/
int sliceLength( Slice slice, const Sequence* seq )
{
    int total = seq->total;
    int length = slice.end_index - slice.start_index;
    
    if( length != 0 )
    {
        if( slice.start_index < 0 )
            slice.start_index += total;
        if( slice.end_index <= 0 )
            slice.end_index += total;

        length = slice.end_index - slice.start_index;
    }

    if( length < 0 )
    {
        length += total;
        /*if( length < 0 )
            length += total;*/
    }
    else if( length > total )
        length = total;

    return length;
}
/*************************************************************************************/
/* sets reader position to given absolute or relative
(relatively to the current one) position */
int setSeqReaderPos( SeqReader* reader, int index, int is_relative )
{	
    SeqBlock *block;
    int elem_size, count, total;
	
    if( !reader || !reader->seq )
        return -27 ;
	
    total = reader->seq->total;
    elem_size = reader->seq->elem_size;
	
    if( !is_relative )
    {
        if( index < 0 )
        {
            if( index < -total )
                return -211 ;
            index += total;
        }
        else if( index >= total )
        {
            index -= total;
            if( index >= total )
                return -211 ;
        }
		
        block = reader->seq->first;
        if( index >= (count = block->count) )
        {
            if( index + index <= total )
            {
                do
                {
                    block = block->next;
                    index -= count;
                }
                while( index >= (count = block->count) );
            }
            else
            {
                do
                {
                    block = block->prev;
                    total -= block->count;
                }
                while( index < total );
                index -= total;
            }
        }
        reader->ptr = block->data + index * elem_size;
        if( reader->block != block )
        {
            reader->block = block;
            reader->block_min = block->data;
            reader->block_max = block->data + block->count * elem_size;
        }
    }
    else
    {
        char* ptr = reader->ptr;
        index *= elem_size;
        block = reader->block;
		
        if( index > 0 )
        {
            while( ptr + index >= reader->block_max )
            {
                int delta = (int)(reader->block_max - ptr);
                index -= delta;
                reader->block = block = block->next;
                reader->block_min = ptr = block->data;
                reader->block_max = block->data + block->count*elem_size;
            }
            reader->ptr = ptr + index;
        }
        else
        {
            while( ptr + index < reader->block_min )
            {
                int delta = (int)(ptr - reader->block_min);
                index += delta;
                reader->block = block = block->prev;
                reader->block_min = block->data;
                reader->block_max = ptr = block->data + block->count*elem_size;
            }
            reader->ptr = ptr + index;
        }
    }
    return 1 ;
}

static const char icvPower2ShiftTab[] =
{
    (char)0, (char)1, (char)-1, (char)2, (char)-1, (char)-1, (char)-1, (char)3, (char)-1, (char)-1, (char)-1, (char)-1, (char)-1, (char)-1, (char)-1, (char)4,
	(char)-1, (char)-1, (char)-1, (char)-1, (char)-1, (char)-1, (char)-1, (char)-1, (char)-1, (char)-1, (char)-1, (char)-1, (char)-1, (char)-1, (char)-1, (char)5
};
/* returns the current reader position */
int getSeqReaderPos( SeqReader* reader )
{
    int elem_size;
    int index = -1;	
    
    if( !reader || !reader->ptr )
        return -27 ;
	
    elem_size = reader->seq->elem_size;
    if( elem_size <= ICVLIB_SHIFT_TAB_MAX && (index = icvPower2ShiftTab[elem_size - 1]) >= 0 )
        index = (int)((reader->ptr - reader->block_min) >> index);
    else
        index = (int)((reader->ptr - reader->block_min) / elem_size);
	
    index += reader->block->start_index - reader->delta_index;    
	
    return index;
}
/*************************************************************************************/
int iMemCopy( double **buf1, double **buf2, double **buf3, int *b_max )
{
    int bb;

    if( (*buf1 == NULL && *buf2 == NULL) || *buf3 == NULL )
        return -2 ;

    bb = *b_max;
    if( *buf2 == NULL )
    {
        *b_max = 2 * (*b_max);
        *buf2 = (double *)alloc( (*b_max) * sizeof( double ));

        if( *buf2 == NULL )
            return -3;

        memcpy( *buf2, *buf3, bb * sizeof( double ));

        *buf3 = *buf2;
        cvFree( buf1 );
        *buf1 = NULL;
    }
    else
    {
        *b_max = 2 * (*b_max);
        *buf1 = (double *) alloc( (*b_max) * sizeof( double ));

        if( *buf1 == NULL )
            return -3 ;

        memcpy( *buf1, *buf3, bb * sizeof( double ));

        *buf3 = *buf1;
        cvFree( buf2 );
        *buf2 = NULL;
    }
    return 1;
}

/*************************************************************************************/
void seqPushMulti( Sequence *seq, const void *_elements, int count, int front )
{
	char *elements = (char *) _elements;

	if( !seq )
		assert( 0 );//"NULL sequence pointer";
	if( count < 0 )
		assert(0);//"number of removed elements is negative" );

	int elem_size = seq->elem_size;

	if( !front )
	{
		while( count > 0 )
		{
			int delta = (int)((seq->block_max - seq->ptr) / elem_size);

			delta = MIN( delta, count );
			if( delta > 0 )
			{
				seq->first->prev->count += delta;
				seq->total += delta;
				count -= delta;
				delta *= elem_size;
				if( elements )
				{
					memcpy( seq->ptr, elements, delta );
					elements += delta;
				}
				seq->ptr += delta;
			}

			if( count > 0 )
				growSeq( seq, 0 );
		}
	}
	else
	{
		SeqBlock* block = seq->first;

		while( count > 0 )
		{
			int delta;

			if( !block || block->start_index == 0 )
			{
				growSeq( seq, 1 );

				block = seq->first;
				assert( block->start_index > 0 );
			}

			delta = MIN( block->start_index, count );
			count -= delta;
			block->start_index -= delta;
			block->count += delta;
			seq->total += delta;
			delta *= elem_size;
			block->data -= delta;

			if( elements )
				memcpy( block->data, elements + count*elem_size, delta );
		}
	}
}
/* removes several elements from the end of sequence */
void seqPopMulti( Sequence *seq, void *_elements, int count, int front )
{
    char *elements = (char *) _elements;    
	
    if( !seq )
        assert(false);
    if( count < 0 )
		assert(false);
	
    count = MIN( count, seq->total );
	
    if( !front )
    {
        if( elements )
            elements += count * seq->elem_size;
		
        while( count > 0 )
        {
            int delta = seq->first->prev->count;
			
            delta = MIN( delta, count );
            assert( delta > 0 );
			
            seq->first->prev->count -= delta;
            seq->total -= delta;
            count -= delta;
            delta *= seq->elem_size;
            seq->ptr -= delta;
			
            if( elements )
            {
                elements -= delta;
                memcpy( elements, seq->ptr, delta );
            }
			
            if( seq->first->prev->count == 0 )
                freeSeqBlock( seq, 0 );
        }
    }
    else
    {
        while( count > 0 )
        {
            int delta = seq->first->count;
			
            delta = MIN( delta, count );
            assert( delta > 0 );
			
            seq->first->count -= delta;
            seq->total -= delta;
            count -= delta;
            seq->first->start_index += delta;
            delta *= seq->elem_size;
			
            if( elements )
            {
                memcpy( elements, seq->first->data, delta );
                elements += delta;
            }
			
            seq->first->data += delta;
            if( seq->first->count == 0 )
                freeSeqBlock( seq, 1 );
        }
    }
}
void clearSeq(Sequence *seq)
{
    if ( seq )
	    seqPopMulti( seq, 0, seq->total );
}
void seqPopFront( Sequence *seq, void *element )
{
    int elem_size;
    SeqBlock *block;

    if( !seq )
        return;
    if( seq->total <= 0 )
        return;

    elem_size = seq->elem_size;
    block = seq->first;

    if( element )
        memcpy( element, block->data, elem_size );
    block->data += elem_size;
    block->start_index++;
    seq->total--;

    if( --(block->count) == 0 )
        freeSeqBlock( seq, 1 );
}
char* SeqInsert( Sequence *seq, int before_index, const void *element )
{
    int elem_size;
    int block_size;
    SeqBlock *block;
    int delta_index;
    int total;
    char* ret_ptr = 0;

    if( !seq )
        return 0;

    total = seq->total;
    before_index += before_index < 0 ? total : 0;
    before_index -= before_index > total ? total : 0;

    if( (unsigned)before_index > (unsigned)total )
        return 0;

    if( before_index == total )
    {
        ret_ptr = seqPush( seq, element );
    }
    else if( before_index == 0 )
    {
        ret_ptr = seqPushFront( seq, element );
    }
    else
    {
        elem_size = seq->elem_size;

        if( before_index >= total >> 1 )
        {
            char *ptr = seq->ptr + elem_size;

            if( ptr > seq->block_max )
            {
                growSeq( seq, 0 );

                ptr = seq->ptr + elem_size;
                assert( ptr <= seq->block_max );
            }

            delta_index = seq->first->start_index;
            block = seq->first->prev;
            block->count++;
            block_size = (int)(ptr - block->data);

            while( before_index < block->start_index - delta_index )
            {
                SeqBlock *prev_block = block->prev;

                memmove( block->data + elem_size, block->data, block_size - elem_size );
                block_size = prev_block->count * elem_size;
                memcpy( block->data, prev_block->data + block_size - elem_size, elem_size );
                block = prev_block;

                /* Check that we don't fall into an infinite loop: */
                assert( block != seq->first->prev );
            }

            before_index = (before_index - block->start_index + delta_index) * elem_size;
            memmove( block->data + before_index + elem_size, block->data + before_index,
                     block_size - before_index - elem_size );

            ret_ptr = block->data + before_index;

            if( element )
                memcpy( ret_ptr, element, elem_size );
            seq->ptr = ptr;
        }
        else
        {
            block = seq->first;

            if( block->start_index == 0 )
            {
                growSeq( seq, 1 );

                block = seq->first;
            }

            delta_index = block->start_index;
            block->count++;
            block->start_index--;
            block->data -= elem_size;

            while( before_index > block->start_index - delta_index + block->count )
            {
                SeqBlock *next_block = block->next;

                block_size = block->count * elem_size;
                memmove( block->data, block->data + elem_size, block_size - elem_size );
                memcpy( block->data + block_size - elem_size, next_block->data, elem_size );
                block = next_block;

                /* Check that we don't fall into an infinite loop: */
                assert( block != seq->first );
            }

            before_index = (before_index - block->start_index + delta_index) * elem_size;
            memmove( block->data, block->data + elem_size, before_index - elem_size );

            ret_ptr = block->data + before_index - elem_size;

            if( element )
                memcpy( ret_ptr, element, elem_size );
        }

        seq->total = total + 1;
    }

    return ret_ptr;
}
char* seqPushFront( Sequence *seq, const void *element )
{
    char* ptr = 0;
    int elem_size;
    SeqBlock *block;

    if( !seq )
        return 0;

    elem_size = seq->elem_size;
    block = seq->first;

    if( !block || block->start_index == 0 )
    {
        growSeq( seq, 1 );

        block = seq->first;
        assert( block->start_index > 0 );
    }

    ptr = block->data -= elem_size;

    if( element )
        memcpy( ptr, element, elem_size );
    block->count++;
    block->start_index--;
    seq->total++;

    return ptr;
}
void SeqInsertSlice( Sequence* seq, int index, const Arr* from_arr )
{
    SeqReader reader_to, reader_from;
    int i, elem_size, total, from_total;
    Sequence from_header, *from = (Sequence*)from_arr;
    SeqBlock block;

    if( !CVLIB_IS_SEQ(seq) )
        return;

    if( !CVLIB_IS_SEQ(from))
    {
        Mat* mat = (Mat*)from;
		if( !mat->data.ptr)
            return;//( CV_StsBadArg, "Source is not a sequence nor matrix" );

		if( !mat->isContinuous() || (mat->rows() != 1 && mat->cols() != 1) )
            return;//( CV_StsBadArg, "The source array must be 1d coninuous vector" );

        from = makeSeqHeaderForArray( CVLIB_SEQ_KIND_GENERIC, sizeof(from_header), mat->elemSize(),
                                                 mat->data.ptr[0], mat->cols() + mat->rows() - 1,
                                                 &from_header, &block );
		
    }

    if( seq->elem_size != from->elem_size )
        return;

    from_total = from->total;

    if( from_total == 0 )
        return;

    total = seq->total;
    index += index < 0 ? total : 0;
    index -= index > total ? total : 0;

    if( (unsigned)index > (unsigned)total )
        return;// ( CV_StsOutOfRange, "" );

    elem_size = seq->elem_size;

    if( index < (total >> 1) )
    {
        seqPushMulti( seq, 0, from_total, 1 );

        startReadSeq( seq, &reader_to );
        startReadSeq( seq, &reader_from );
        setSeqReaderPos( &reader_from, from_total );

        for( i = 0; i < index; i++ )
        {
            memcpy( reader_to.ptr, reader_from.ptr, elem_size );
            CVLIB_NEXT_SEQ_ELEM( elem_size, reader_to );
            CVLIB_NEXT_SEQ_ELEM( elem_size, reader_from );
        }
    }
    else
    {
        seqPushMulti( seq, 0, from_total );

        startReadSeq( seq, &reader_to );
        startReadSeq( seq, &reader_from );
        setSeqReaderPos( &reader_from, total );
        setSeqReaderPos( &reader_to, seq->total );

        for( i = 0; i < total - index; i++ )
        {
            CVLIB_PREV_SEQ_ELEM( elem_size, reader_to );
            CVLIB_PREV_SEQ_ELEM( elem_size, reader_from );
            memcpy( reader_to.ptr, reader_from.ptr, elem_size );
        }
    }

    startReadSeq( from, &reader_from );
    setSeqReaderPos( &reader_to, index );

    for( i = 0; i < from_total; i++ )
    {
        memcpy( reader_to.ptr, reader_from.ptr, elem_size );
        CVLIB_NEXT_SEQ_ELEM( elem_size, reader_to );
        CVLIB_NEXT_SEQ_ELEM( elem_size, reader_from );
    }
}
void SeqRemove( Sequence *seq, int index )
{
    char *ptr;
    int elem_size;
    int block_size;
    SeqBlock *block;
    int delta_index;
    int total, front = 0;

    if( !seq )
        return;

    total = seq->total;

    index += index < 0 ? total : 0;
    index -= index >= total ? total : 0;

    if( (unsigned) index >= (unsigned) total )
        return;//"Invalid index"

    if( index == total - 1 )
    {
        seqPop( seq, 0 );
    }
    else if( index == 0 )
    {
        seqPopFront( seq, 0 );
    }
    else
    {
        block = seq->first;
        elem_size = seq->elem_size;
        delta_index = block->start_index;
        while( block->start_index - delta_index + block->count <= index )
            block = block->next;

        ptr = block->data + (index - block->start_index + delta_index) * elem_size;

        front = index < total >> 1;
        if( !front )
        {
            block_size = block->count * elem_size - (int)(ptr - block->data);

            while( block != seq->first->prev )  /* while not the last block */
            {
                SeqBlock *next_block = block->next;

                memmove( ptr, ptr + elem_size, block_size - elem_size );
                memcpy( ptr + block_size - elem_size, next_block->data, elem_size );
                block = next_block;
                ptr = block->data;
                block_size = block->count * elem_size;
            }

            memmove( ptr, ptr + elem_size, block_size - elem_size );
            seq->ptr -= elem_size;
        }
        else
        {
            ptr += elem_size;
            block_size = (int)(ptr - block->data);

            while( block != seq->first )
            {
                SeqBlock *prev_block = block->prev;

                memmove( block->data + elem_size, block->data, block_size - elem_size );
                block_size = prev_block->count * elem_size;
                memcpy( block->data, prev_block->data + block_size - elem_size, elem_size );
                block = prev_block;
            }

            memmove( block->data + elem_size, block->data, block_size - elem_size );
            block->data += elem_size;
            block->start_index++;
        }

        seq->total = total - 1;
        if( --block->count == 0 )
            freeSeqBlock( seq, front );
    }
}
void SeqRemoveSlice( Sequence* seq, Slice slice )
{
    int total, length;

    if( !CVLIB_IS_SEQ(seq) )
        return;//"Invalid sequence header" );

    length = sliceLength( slice, seq );
    total = seq->total;

    if( slice.start_index < 0 )
        slice.start_index += total;
    else if( slice.start_index >= total )
        slice.start_index -= total;

    if( (unsigned)slice.start_index >= (unsigned)total )
        return;//"start slice index is out of range"

    slice.end_index = slice.start_index + length;

    if( slice.end_index < total )
    {
        SeqReader reader_to, reader_from;
        int elem_size = seq->elem_size;

        startReadSeq( seq, &reader_to );
        startReadSeq( seq, &reader_from );

        if( slice.start_index > total - slice.end_index )
        {
            int i, count = seq->total - slice.end_index;
            setSeqReaderPos( &reader_to, slice.start_index );
            setSeqReaderPos( &reader_from, slice.end_index );

            for( i = 0; i < count; i++ )
            {
                memcpy( reader_to.ptr, reader_from.ptr, elem_size );
                CVLIB_NEXT_SEQ_ELEM( elem_size, reader_to );
                CVLIB_NEXT_SEQ_ELEM( elem_size, reader_from );
            }

            seqPopMulti( seq, 0, slice.end_index - slice.start_index );
        }
        else
        {
            int i, count = slice.start_index;
            setSeqReaderPos( &reader_to, slice.end_index );
            setSeqReaderPos( &reader_from, slice.start_index );

            for( i = 0; i < count; i++ )
            {
                CVLIB_PREV_SEQ_ELEM( elem_size, reader_to );
                CVLIB_PREV_SEQ_ELEM( elem_size, reader_from );

                memcpy( reader_to.ptr, reader_from.ptr, elem_size );
            }

            seqPopMulti( seq, 0, slice.end_index - slice.start_index, 1 );
        }
    }
    else
    {
        seqPopMulti( seq, 0, total - slice.start_index );
        seqPopMulti( seq, 0, slice.end_index - total, 1 );
    }
}

}