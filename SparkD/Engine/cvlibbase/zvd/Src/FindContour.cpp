/*!
 * \file
 * \brief .
 * \author
 */

#include "FindContour.h"
#include "iFindContour.h"
#include "Thresh.h"
#include "Drawing.h"

#pragma warning (push)
#pragma warning (disable : 4127)

namespace cvlib
{

/* initializes 8-element array for fast access to 3x3 neighborhood of a pixel */
#define  CVLIB_INIT_3X3_DELTAS( deltas, step, nch )            \
    ((deltas)[0] =  (nch),  (deltas)[1] = -(step) + (nch),  \
	(deltas)[2] = -(step), (deltas)[3] = -(step) - (nch),  \
	(deltas)[4] = -(nch),  (deltas)[5] =  (step) - (nch),  \
(deltas)[6] =  (step), (deltas)[7] =  (step) + (nch))

static const Point2i icvCodeDeltas[8] =
    { Point2i(1, 0), Point2i(1, -1), Point2i(0, -1), Point2i(-1, -1), Point2i(-1, 0), Point2i(-1, 1), Point2i(0, 1), Point2i(1, 1) };

//////////////////////////////////////////////////////////////////////////

/*=========================FindNextContours relation functions=======================*/


/*************************************************************************************/
int findContours( Mat* img, MemStorage* storage, Sequence** firstContour, int mode, int method, Point2i offset )
{
	ContourScanner scanner = 0;
    Sequence *contour = 0;
    int count = -1;	
    
    if( !firstContour )
        return -27 ;
	
    if( method == CVLIB_LINK_RUNS )
    {
        if( offset.x != 0 || offset.y != 0 )
            return -211 ;
		
//        count = FindContoursInInterval( img, storage, firstContour, cntHeaderSize );
    }
    else
    {
        scanner = startFindContours( img, storage, mode, method, offset ) ;
        assert( scanner );		
		
        do
        {
            count++;
            contour = findNextContour( scanner );
        }
        while( contour != 0 );
		
        *firstContour = endFindContours( &scanner );
    }    
	
    return count;
}
/*************************************************************************************/
ContourScanner startFindContours(Mat* pmImg, MemStorage* storage, int nRetrievalMode, int nApproximageMethod, Point2i startPt)
{
	int y;
	ContourScanner scanner = 0;
	scanner = (ContourScanner)alloc(sizeof(*scanner));

	memset(scanner, 0, sizeof(*scanner));
	int header_size = sizeof(IPContour);
	int nH = pmImg->rows();
	int nW = pmImg->cols();

	scanner->img_size.height = nH - 1;
	scanner->img_size.width = nW - 1;
	scanner->storage1 = scanner->storage2 = storage;
	scanner->img0 = (char *)pmImg->data.ptr[0];
	scanner->img = (char *)(pmImg->data.ptr[0] + nW);
	scanner->img_step = nW;
	scanner->mode = nRetrievalMode;
	scanner->offset = startPt;
	scanner->pt.x = scanner->pt.y = 1;
	scanner->lnbd.x = 0;
	scanner->lnbd.y = 1;
	scanner->nbd = 2;
	scanner->mode = nRetrievalMode;
	scanner->frame_info.contour = &(scanner->frame);
	scanner->frame_info.is_hole = 1;
	scanner->frame_info.next = 0;
	scanner->frame_info.parent = 0;
	scanner->frame_info.rect = Rect(0, 0, nW, nH);
	scanner->l_cinfo = 0;
	scanner->subst_flag = 0;

	scanner->frame.flags = CVLIB_SEQ_FLAG_HOLE;
	scanner->approx_method2 = scanner->approx_method1 = nApproximageMethod;

	if (nApproximageMethod == CVLIB_CHAIN_APPROX_TC89_L1 || nApproximageMethod == CVLIB_CHAIN_APPROX_TC89_KCOS)
		scanner->approx_method1 = CVLIB_CHAIN_CODE;

	if (scanner->approx_method1 == CVLIB_CHAIN_CODE)
	{
		scanner->seq_type1 = CVLIB_SEQ_CHAIN_CONTOUR;
		scanner->header_size1 = scanner->approx_method1 == scanner->approx_method2 ?
			header_size : sizeof(Chain);
		scanner->elem_size1 = sizeof(char);
	}
	else
	{
		scanner->seq_type1 = CVLIB_SEQ_POLYGON;
		scanner->header_size1 = scanner->approx_method1 == scanner->approx_method2 ?
			header_size : sizeof(IPContour);
		scanner->elem_size1 = sizeof(Point2i);
	}

	scanner->header_size2 = header_size;

	if (scanner->approx_method2 == CVLIB_CHAIN_CODE)
	{
		scanner->seq_type2 = scanner->seq_type1;
		scanner->elem_size2 = scanner->elem_size1;
	}
	else
	{
		scanner->seq_type2 = CVLIB_SEQ_POLYGON;
		scanner->elem_size2 = sizeof(Point2i);
	}

	scanner->seq_type1 = scanner->approx_method1 == CVLIB_CHAIN_CODE ?
		CVLIB_SEQ_CHAIN_CONTOUR : CVLIB_SEQ_POLYGON;

	scanner->seq_type2 = scanner->approx_method2 == CVLIB_CHAIN_CODE ?
		CVLIB_SEQ_CHAIN_CONTOUR : CVLIB_SEQ_POLYGON;

	saveMemStoragePos(storage, &(scanner->initial_pos));

	if (nApproximageMethod > CVLIB_CHAIN_APPROX_SIMPLE)
	{
		scanner->storage1 = createChildMemStorage(scanner->storage2);
	}

	if (nRetrievalMode > CVLIB_RETR_LIST)
	{
		scanner->cinfo_storage = createChildMemStorage(scanner->storage2);
		scanner->cinfo_set = createSet(0, sizeof(Set), sizeof(ContourInfo),
			scanner->cinfo_storage);
		if (scanner->cinfo_storage == 0 || scanner->cinfo_set == 0)
			return 0;
	}

	/* make zero borders */
	unsigned char** ppImg = pmImg->data.ptr;

	memset(ppImg[0], 0, nW);
	memset(ppImg[nH - 1], 0, nW);

	for (y = 1; y < nH - 1; y++)
		ppImg[y][0] = ppImg[y][nW - 1] = 0;

	/* converts all pixels to 0 or 1 */
	ip::Threshold(*pmImg, *pmImg, 0, 1, ip::THRESH_BINARY);
	return scanner;
}

/*************************************************************************************/
Sequence* findNextContour( ContourScanner scanner )
{
	char *img0;
    char *img;
    int step;
    int width, height;
    int x, y;
    int prev;
    Point2i lnbd;
    Sequence *contour = 0;
    int nbd;
    int mode;
    int result = 1 ;

	if( !scanner )
		return NULL ;
    
	endProcessContour( scanner );

    /* initialize local state */
	
    img0 = scanner->img0;
    img = scanner->img;
    step = scanner->img_step;
    x = scanner->pt.x;
    y = scanner->pt.y;
    width = scanner->img_size.width;
    height = scanner->img_size.height;
    mode = scanner->mode;
    lnbd = scanner->lnbd;
    nbd = scanner->nbd;

    prev = img[x - 1];

    for( ; y < height; y++, img += step )
    {
        for( ; x < width; x++ )
        {
            int p = img[x];

            if( p != prev )
            {
                ContourInfo *par_info = 0;
                ContourInfo *l_cinfo = 0;
                Sequence *seq = 0;
                int is_hole = 0;
                Point2i origin;

                if( !(prev == 0 && p == 1) )    /* if not external contour */
                {
                    /* check hole */
                    if( p != 0 || prev < 1 )
                        goto resume_scan;

                    if( prev & -2 )
                    {
                        lnbd.x = x - 1;
                    }
                    is_hole = 1;
                }

                if( mode == 0 && (is_hole || img0[lnbd.y * step + lnbd.x] > 0) )
                    goto resume_scan;

                origin.y = y;
                origin.x = x - is_hole;

                /* find contour parent */
                if( mode <= 1 || (!is_hole && mode == 2) || lnbd.x <= 0 )
                {
                    par_info = &(scanner->frame_info);
                }
                else
                {
                    int lval = img0[lnbd.y * step + lnbd.x] & 0x7f;
                    ContourInfo *cur = scanner->cinfo_table[lval - 2];

                    assert( lval >= 2 );

                    /* find the first bounding contour */
                    while( cur )
                    {
                        if( (unsigned) (lnbd.x - cur->rect.x) < (unsigned) cur->rect.width &&
                            (unsigned) (lnbd.y - cur->rect.y) < (unsigned) cur->rect.height )
                        {
                            if( par_info )
                            {
                                if( traceContour( scanner->img0 +
                                                 par_info->origin.y * step +
                                                 par_info->origin.x, step, img + lnbd.x,
                                                 par_info->is_hole ) > 0 )
                                    break;
                            }
                            par_info = cur;
                        }
                        cur = cur->next;
                    }

                    assert( par_info != 0 );

                    /* if current contour is a hole and previous contour is a hole or
                       current contour is external and previous contour is external then
                       the parent of the contour is the parent of the previous contour else
                       the parent is the previous contour itself. */
                    if( par_info->is_hole == is_hole )
                    {
                        par_info = par_info->parent;
                        /* every contour must have a parent
                           (at least, the frame of the image) */
                        if( !par_info )
                            par_info = &(scanner->frame_info);
                    }

                    /* hole flag of the parent must differ from the flag of the contour */
                    assert( par_info->is_hole != is_hole );
                    if( par_info->contour == 0 )        /* removed contour */
                        goto resume_scan;
                }

                lnbd.x = x - is_hole;

                saveMemStoragePos( scanner->storage2, &(scanner->backup_pos) );

                seq = createSeq( scanner->seq_type1, scanner->header_size1,
                                   scanner->elem_size1, scanner->storage1 );
                if( !seq )
                {
                    result = -3 ;
                    goto exit_func;
                }
                seq->flags |= is_hole ? CVLIB_SEQ_FLAG_HOLE : 0;

                /* initialize header */
                if( mode <= 1 )
                {
                    l_cinfo = &(scanner->cinfo_temp);
                    result = fetchContour( img + x - is_hole, step,
                                          Point2i( origin.x + scanner->offset.x,
                                                   origin.y + scanner->offset.y),
                                          seq, scanner->approx_method1 );
                    if( result < 0 )
                        goto exit_func;
                }
                else
                {
                    union { ContourInfo* ci; SetElem* se; } v;
                    v.ci = l_cinfo;
                    setAdd( scanner->cinfo_set, 0, &v.se );
                    l_cinfo = v.ci;

                    result = fetchContourEx( img + x - is_hole, step,
                                            Point2i( origin.x + scanner->offset.x,
                                                     origin.y + scanner->offset.y),
                                            seq, scanner->approx_method1,
                                            nbd, &(l_cinfo->rect) );
                    if( result < 0 )
                        goto exit_func;
                    l_cinfo->rect.x -= scanner->offset.x;
                    l_cinfo->rect.y -= scanner->offset.y;

                    l_cinfo->next = scanner->cinfo_table[nbd - 2];
                    scanner->cinfo_table[nbd - 2] = l_cinfo;

                    /* change nbd */
                    nbd = (nbd + 1) & 127;
                    nbd += nbd == 0 ? 3 : 0;
                }

                l_cinfo->is_hole = is_hole;
                l_cinfo->contour = seq;
                l_cinfo->origin = origin;
                l_cinfo->parent = par_info;

                if( scanner->approx_method1 != scanner->approx_method2 )
                {
                    result = approximateChainTC89( (Chain *) seq,
                                                  scanner->header_size2,
                                                  scanner->storage2,
                                                  &(l_cinfo->contour),
                                                  scanner->approx_method2 );
                    if( result < 0 )
                        goto exit_func;
                    clearMemStorage( scanner->storage1 );
                }

                l_cinfo->contour->v_prev = l_cinfo->parent->contour;

                if( par_info->contour == 0 )
                {
                    l_cinfo->contour = 0;
                    if( scanner->storage1 == scanner->storage2 )
                    {
                        restoreMemStoragePos( scanner->storage1, &(scanner->backup_pos) );
                    }
                    else
                    {
                        clearMemStorage( scanner->storage1 );
                    }
                    p = img[x];
                    goto resume_scan;
                }

                saveMemStoragePos( scanner->storage2, &(scanner->backup_pos2) );
                scanner->l_cinfo = l_cinfo;
                scanner->pt.x = x + 1;
                scanner->pt.y = y;
                scanner->lnbd = lnbd;
                scanner->img = (char *) img;
                scanner->nbd = nbd;
                contour = l_cinfo->contour;

                result = 0;
                goto exit_func;
              resume_scan:
                prev = p;
                /* update lnbd */
                if( prev & -2 )
                {
                    lnbd.x = x;
                }
            }                   /* end of prev != p */
        }                       /* end of loop on x */

        lnbd.x = 0;
        lnbd.y = y + 1;
        x = 1;
        prev = 0;

    }                           /* end of loop on y */

	exit_func:

    if( result != 0 )
        contour = 0;
    if( result < 0 )
        return 0;    
    return contour;
}

/*************************************************************************************/
int traceContour( char *ptr, int step, char *stop_ptr, int is_hole )
{
    int deltas[16];
    char *i0 = ptr, *i1, *i3, *i4;
    int s, s_end;
	
    /* initialize local state */
    CVLIB_INIT_3X3_DELTAS( deltas, step, 1 );
    memcpy( deltas + 8, deltas, 8 * sizeof( deltas[0] ));
	
    assert( (*i0 & -2) != 0 );
	
    s_end = s = is_hole ? 0 : 4;
	
    do
    {
        s = (s - 1) & 7;
        i1 = i0 + deltas[s];
        if( *i1 != 0 )
            break;
    }
    while( s != s_end );
	
    i3 = i0;
	
    /* check single pixel domain */
    if( s != s_end )
    {
        /* follow border */
        for( ;; )
        {
            s_end = s;
			
            for( ;; )
            {
                i4 = i3 + deltas[++s];
                if( *i4 != 0 )
                    break;
            }
			
            if( i3 == stop_ptr || (i4 == i0 && i3 == i1) )
                break;
			
            i3 = i4;
            s = (s + 4) & 7;
        }                       /* end of border following loop */
    }
    return i3 == stop_ptr;
}

/*************************************************************************************/
void endProcessContour( ContourScanner scanner )
{
    ContourInfo *l_cinfo = scanner->l_cinfo;
	
    if( l_cinfo )
    {
        if( scanner->subst_flag )
        {
            MemStoragePos temp;
			
            saveMemStoragePos( scanner->storage2, &temp );
			
            if( temp.top == scanner->backup_pos2.top &&
                temp.free_space == scanner->backup_pos2.free_space )
            {
                restoreMemStoragePos( scanner->storage2, &scanner->backup_pos );
            }
            scanner->subst_flag = 0;
        }
		
        if( l_cinfo->contour )
        {
            insertNodeIntoTree( l_cinfo->contour, l_cinfo->parent->contour,
				&(scanner->frame) );
        }
        scanner->l_cinfo = 0;
    }
}


/*************************************************************************************/
int fetchContour( char *ptr, int step, Point2i pt, Sequence* contour, int _method )
{
    const char      nbd = 2;
    int             deltas[16];
    SeqWriter     writer;
    char            *i0 = ptr, *i1, *i3, *i4 = 0;
    int             prev_s = -1, s, s_end;
    int             method = _method - 1;

    assert( (unsigned) _method <= CVLIB_CHAIN_APPROX_SIMPLE );

    /* initialize local state */
    CVLIB_INIT_3X3_DELTAS( deltas, step, 1 );
    memcpy( deltas + 8, deltas, 8 * sizeof( deltas[0] ));

    /* initialize writer */
    startAppendToSeq( contour, &writer );

    if( method < 0 )
        ((Chain *) contour)->origin = pt;

    s_end = s = CVLIB_IS_SEQ_HOLE( contour ) ? 0 : 4;

    do
    {
        s = (s - 1) & 7;
        i1 = i0 + deltas[s];
        if( *i1 != 0 )
            break;
    }
    while( s != s_end );

    if( s == s_end )            /* single pixel domain */
    {
        *i0 = (char) (nbd | -128);
        if( method >= 0 )
        {
            CVLIB_WRITE_SEQ_ELEM( pt, writer );
        }
    }
    else
    {
        i3 = i0;
        prev_s = s ^ 4;

        /* follow border */
        for( ;; )
        {
            s_end = s;

            for( ;; )
            {
                i4 = i3 + deltas[++s];
                if( *i4 != 0 )
                    break;
            }
            s &= 7;

            /* check "right" bound */
            if( (unsigned) (s - 1) < (unsigned) s_end )
            {
                *i3 = (char) (nbd | -128);
            }
            else if( *i3 == 1 )
            {
                *i3 = nbd;
            }

            if( method < 0 )
            {
                char _s = (char) s;

                CVLIB_WRITE_SEQ_ELEM( _s, writer );
            }
            else
            {
                if( s != prev_s || method == 0 )
                {
                    CVLIB_WRITE_SEQ_ELEM( pt, writer );
                    prev_s = s;
                }

                pt.x += icvCodeDeltas[s].x;
                pt.y += icvCodeDeltas[s].y;

            }

            if( i4 == i0 && i3 == i1 )
                break;

            i3 = i4;
            s = (s + 4) & 7;
        }                       /* end of border following loop */
    }

    endWriteSeq( &writer );

    if( _method != CVLIB_CHAIN_CODE )
        boundingRect( contour, 1 );

    assert( writer.seq->total == 0 && writer.seq->first == 0 ||
            writer.seq->total > writer.seq->first->count ||
            (writer.seq->first->prev == writer.seq->first &&
             writer.seq->first->next == writer.seq->first) );

    return 1;
}

/*************************************************************************************/
/* Calculates bounding rectagnle of a point set or retrieves already calculated */
Rect boundingRect( Arr* array, int update )
{
    SeqReader reader;
    Rect  rect(0, 0, 0, 0);
    Sequence* ptseq = 0;    
    int  xmin = 0, ymin = 0, xmax = -1, ymax = -1, i;
    int calculate = update;

    if( CVLIB_IS_SEQ( array ))
    {
        ptseq = (Sequence*)array;
        if( !CVLIB_IS_SEQ_POINT_SET( ptseq ))
            return rect;

        if( ptseq->header_size < (int)sizeof(IPContour))
        {
            /*if( update == 1 )
                CV_ERROR( CV_StsBadArg, "The header is too small to fit the rectangle, "
                                        "so it could not be updated" );*/
            update = 0;
            calculate = 1;
        }
    }    
    if( !calculate )
    {
        rect = ((IPContour*)ptseq)->rect;
        return rect ;
    }
    
    if( ptseq->total )
    {   
        int  is_float = CVLIB_SEQ_ELTYPE(ptseq) == CVLIB_32FC2;
        startReadSeq( ptseq, &reader, 0 );

        if( !is_float )
        {
            Point2i pt;
            /* init values */
            CVLIB_READ_SEQ_ELEM( pt, reader );
            xmin = xmax = pt.x;
            ymin = ymax = pt.y;

            for( i = 1; i < ptseq->total; i++ )
            {            
                CVLIB_READ_SEQ_ELEM( pt, reader );
        
                if( xmin > pt.x )
                    xmin = pt.x;
        
                if( xmax < pt.x )
                    xmax = pt.x;

                if( ymin > pt.y )
                    ymin = pt.y;

                if( ymax < pt.y )
                    ymax = pt.y;
            }
        }
        else
        {
            Point2i pt;
            Cv32suf v;
            /* init values */
            CVLIB_READ_SEQ_ELEM( pt, reader );
            xmin = xmax = CVLIB_TOGGLE_FLT(pt.x);
            ymin = ymax = CVLIB_TOGGLE_FLT(pt.y);

            for( i = 1; i < ptseq->total; i++ )
            {            
                CVLIB_READ_SEQ_ELEM( pt, reader );
                pt.x = CVLIB_TOGGLE_FLT(pt.x);
                pt.y = CVLIB_TOGGLE_FLT(pt.y);
        
                if( xmin > pt.x )
                    xmin = pt.x;
        
                if( xmax < pt.x )
                    xmax = pt.x;

                if( ymin > pt.y )
                    ymin = pt.y;

                if( ymax < pt.y )
                    ymax = pt.y;
            }

            v.i = CVLIB_TOGGLE_FLT(xmin); xmin = (int)floor(v.f);
            v.i = CVLIB_TOGGLE_FLT(ymin); ymin = (int)floor(v.f);
            /* because right and bottom sides of
               the bounding rectangle are not inclusive
               (note +1 in width and height calculation below),
               cvutil::floor is used here instead of cvCeil */
            v.i = CVLIB_TOGGLE_FLT(xmax); xmax = (int)floor(v.f);
            v.i = CVLIB_TOGGLE_FLT(ymax); ymax = (int)floor(v.f);
        }
    }

    rect.x = xmin;
    rect.y = ymin;
    rect.width = xmax - xmin + 1;
    rect.height = ymax - ymin + 1;

    if( update )
        ((IPContour*)ptseq)->rect = rect;

    return rect;
}
/*************************************************************************************/
int fetchContourEx( char* ptr, int step, Point2i pt, Sequence* contour, 
				   int  _method,  int nbd, Rect* _rect )
{
    int         deltas[16];
    SeqWriter	writer;
    char        *i0 = ptr, *i1, *i3, *i4;
    Rect		rect;
    int         prev_s = -1, s, s_end;
    int         method = _method - 1;

    assert( (unsigned) _method <= CVLIB_CHAIN_APPROX_SIMPLE );
    assert( 1 < nbd && nbd < 128 );

    /* initialize local state */
    CVLIB_INIT_3X3_DELTAS( deltas, step, 1 );
    memcpy( deltas + 8, deltas, 8 * sizeof( deltas[0] ));

    /* initialize writer */
    startAppendToSeq( contour, &writer );

    if( method < 0 )
        ((Chain *)contour)->origin = pt;

    rect.x = rect.width = pt.x;
    rect.y = rect.height = pt.y;

    s_end = s = CVLIB_IS_SEQ_HOLE( contour ) ? 0 : 4;

    do
    {
        s = (s - 1) & 7;
        i1 = i0 + deltas[s];
        if( *i1 != 0 )
            break;
    }
    while( s != s_end );

    if( s == s_end )            /* single pixel domain */
    {
        *i0 = (char) (nbd | 0x80);
        if( method >= 0 )
        {
            CVLIB_WRITE_SEQ_ELEM( pt, writer );
        }
    }
    else
    {
        i3 = i0;

        prev_s = s ^ 4;

        /* follow border */
        for( ;; )
        {
            s_end = s;

            for( ;; )
            {
                i4 = i3 + deltas[++s];
                if( *i4 != 0 )
                    break;
            }
            s &= 7;

            /* check "right" bound */
            if( (unsigned) (s - 1) < (unsigned) s_end )
            {
                *i3 = (char) (nbd | 0x80);
            }
            else if( *i3 == 1 )
            {
                *i3 = (char) nbd;
            }

            if( method < 0 )
            {
                char _s = (char) s;
                CVLIB_WRITE_SEQ_ELEM( _s, writer );
            }
            else if( s != prev_s || method == 0 )
            {
                CVLIB_WRITE_SEQ_ELEM( pt, writer );
            }

            if( s != prev_s )
            {
                /* update bounds */
                if( pt.x < rect.x )
                    rect.x = pt.x;
                else if( pt.x > rect.width )
                    rect.width = pt.x;

                if( pt.y < rect.y )
                    rect.y = pt.y;
                else if( pt.y > rect.height )
                    rect.height = pt.y;
            }

            prev_s = s;
            pt.x += icvCodeDeltas[s].x;
            pt.y += icvCodeDeltas[s].y;

            if( i4 == i0 && i3 == i1 )  break;

            i3 = i4;
            s = (s + 4) & 7;
        }                       /* end of border following loop */
    }

    rect.width -= rect.x - 1;
    rect.height -= rect.y - 1;

    endWriteSeq( &writer );

    if( _method != CVLIB_CHAIN_CODE )
        ((IPContour*)contour)->rect = rect;

    assert( writer.seq->total == 0 && writer.seq->first == 0 ||
            writer.seq->total > writer.seq->first->count ||
            (writer.seq->first->prev == writer.seq->first &&
             writer.seq->first->next == writer.seq->first) );

    if( _rect )  *_rect = rect;

    return 1;
}
/*************************************************************************************/
/* curvature: 0 - 1-curvature, 1 - k-cosine curvature. */
int approximateChainTC89( Chain* chain, int header_size, MemStorage* storage, 
                         Sequence** contour, int method )
{
    static const int abs_diff[] = { 1, 2, 3, 4, 3, 2, 1, 0, 1, 2, 3, 4, 3, 2, 1 };

    char            local_buffer[1 << 16];
    char*           buffer = local_buffer;
    int             buffer_size;

    PtInfo       temp;
    PtInfo       *array, *first = 0, *current = 0, *prev_current = 0;
    int             i, j, i1, i2, s, len;
    int             count;

    ChainPtReader reader;
    SeqWriter     writer;
    Point2i        pt = chain->origin;
   
    assert( chain && contour && buffer );

    buffer_size = (chain->total + 8) * sizeof( PtInfo );

    *contour = 0;

    if( !CVLIB_IS_SEQ_CHAIN_CONTOUR( chain ))
        return -12 ;

    if( header_size < (int)sizeof(IPContour) )
        return -12 ;
    
    startWriteSeq( (chain->flags & ~CVLIB_SEQ_ELTYPE_MASK) | CVLIB_SEQ_ELTYPE_POINT,
                     header_size, sizeof( Point2i ), storage, &writer );
    
    if( chain->total == 0 )
    {        
        CVLIB_WRITE_SEQ_ELEM( pt, writer );
        goto exit_function;
    }

    startReadChainPoints( chain, &reader );

    if( method > CVLIB_CHAIN_APPROX_SIMPLE && buffer_size > (int)sizeof(local_buffer))
    {
        buffer = (char *) alloc( buffer_size );
        if( !buffer )
            return -3 ;
    }

    array = (PtInfo*) buffer;
    count = chain->total;

    temp.next = 0;
    current = &temp;

    /* Pass 0.
       Restores all the digital curve points from the chain code.
       Removes the points (from the resultant polygon)
       that have zero 1-curvature */
    for( i = 0; i < count; i++ )
    {
        int prev_code = *reader.prev_elem;

        reader.prev_elem = reader.ptr;
        CVLIB_READ_CHAIN_POINT( pt, reader );

        /* calc 1-curvature */
        s = abs_diff[reader.code - prev_code + 7];

        if( method <= CVLIB_CHAIN_APPROX_SIMPLE )
        {
            if( method == CVLIB_CHAIN_APPROX_NONE || s != 0 )
            {
                CVLIB_WRITE_SEQ_ELEM( pt, writer );
            }
        }
        else
        {
            if( s != 0 )
                current = current->next = array + i;
            array[i].s = s;
            array[i].pt = pt;
        }
    }

    //assert( pt.x == chain->origin.x && pt.y == chain->origin.y );

    if( method <= CVLIB_CHAIN_APPROX_SIMPLE )
        goto exit_function;

    current->next = 0;

    len = i;
    current = temp.next;

    assert( current );

    /* Pass 1.
       Determines support region for all the remained points */
    do
    {
        Point2i pt0;
        int k, l = 0, d_num = 0;

        i = (int)(current - array);
        pt0 = array[i].pt;

        /* determine support region */
        for( k = 1;; k++ )
        {
            int lk, dk_num;
            int dx, dy;
            Cv32suf d;

            assert( k <= len );

            /* calc indices */
            i1 = i - k;
            i1 += i1 < 0 ? len : 0;
            i2 = i + k;
            i2 -= i2 >= len ? len : 0;

            dx = array[i2].pt.x - array[i1].pt.x;
            dy = array[i2].pt.y - array[i1].pt.y;

            /* distance between p_(i - k) and p_(i + k) */
            lk = dx * dx + dy * dy;

            /* distance between p_i and the line (p_(i-k), p_(i+k)) */
            dk_num = (pt0.x - array[i1].pt.x) * dy - (pt0.y - array[i1].pt.y) * dx;
            d.f = (float) (((double) d_num) * lk - ((double) dk_num) * l);

            if( k > 1 && (l >= lk || ((d_num > 0 && d.i <= 0) || (d_num < 0 && d.i >= 0))))
                break;

            d_num = dk_num;
            l = lk;
        }

        current->k = --k;

        /* determine cosine curvature if it should be used */
        if( method == CVLIB_CHAIN_APPROX_TC89_KCOS )
        {
            /* calc k-cosine curvature */
            for( j = k, s = 0; j > 0; j-- )
            {
                double temp_num;
                int dx1, dy1, dx2, dy2;
                Cv32suf sk;

                i1 = i - j;
                i1 += i1 < 0 ? len : 0;
                i2 = i + j;
                i2 -= i2 >= len ? len : 0;

                dx1 = array[i1].pt.x - pt0.x;
                dy1 = array[i1].pt.y - pt0.y;
                dx2 = array[i2].pt.x - pt0.x;
                dy2 = array[i2].pt.y - pt0.y;

                if( (dx1 | dy1) == 0 || (dx2 | dy2) == 0 )
                    break;

                temp_num = dx1 * dx2 + dy1 * dy2;
                temp_num =
                    (float) (temp_num /
                             sqrt( ((double)dx1 * dx1 + (double)dy1 * dy1) *
                                   ((double)dx2 * dx2 + (double)dy2 * dy2) ));
                sk.f = (float) (temp_num + 1.1);

                assert( 0 <= sk.f && sk.f <= 2.2 );
                if( j < k && sk.i <= s )
                    break;

                s = sk.i;
            }
            current->s = s;
        }
        current = current->next;
    }
    while( current != 0 );

    prev_current = &temp;
    current = temp.next;

    /* Pass 2.
       Performs non-maxima supression */
    do
    {
        int k2 = current->k >> 1;

        s = current->s;
        i = (int)(current - array);

        for( j = 1; j <= k2; j++ )
        {
            i2 = i - j;
            i2 += i2 < 0 ? len : 0;

            if( array[i2].s > s )
                break;

            i2 = i + j;
            i2 -= i2 >= len ? len : 0;

            if( array[i2].s > s )
                break;
        }

        if( j <= k2 )           /* exclude point */
        {
            prev_current->next = current->next;
            current->s = 0;     /* "clear" point */
        }
        else
            prev_current = current;
        current = current->next;
    }
    while( current != 0 );

    /* Pass 3.
       Removes non-dominant points with 1-length support region */
    current = temp.next;
    assert( current );
    prev_current = &temp;

    do
    {
        if( current->k == 1 )
        {
            s = current->s;
            i = (int)(current - array);

            i1 = i - 1;
            i1 += i1 < 0 ? len : 0;

            i2 = i + 1;
            i2 -= i2 >= len ? len : 0;

            if( s <= array[i1].s || s <= array[i2].s )
            {
                prev_current->next = current->next;
                current->s = 0;
            }
            else
                prev_current = current;
        }
        else
            prev_current = current;
        current = current->next;
    }
    while( current != 0 );

    if( method == CVLIB_CHAIN_APPROX_TC89_KCOS )
        goto copy_vect;

    /* Pass 4.
       Cleans remained couples of points */
    assert( temp.next );

    if( array[0].s != 0 && array[len - 1].s != 0 )      /* specific case */
    {
        for( i1 = 1; i1 < len && array[i1].s != 0; i1++ )
        {
            array[i1 - 1].s = 0;
        }
        if( i1 == len )
            goto copy_vect;     /* all points survived */
        i1--;

        for( i2 = len - 2; i2 > 0 && array[i2].s != 0; i2-- )
        {
            array[i2].next = 0;
            array[i2 + 1].s = 0;
        }
        i2++;

        if( i1 == 0 && i2 == len - 1 )  /* only two points */
        {
            i1 = (int)(array[0].next - array);
            array[len] = array[0];      /* move to the end */
            array[len].next = 0;
            array[len - 1].next = array + len;
        }
        temp.next = array + i1;
    }

    current = temp.next;
    first = prev_current = &temp;
    count = 1;

    /* do last pass */
    do
    {
        if( current->next == 0 || current->next - current != 1 )
        {
            if( count >= 2 )
            {
                if( count == 2 )
                {
                    int s1 = prev_current->s;
                    int s2 = current->s;

                    if( s1 > s2 || s1 == s2 && prev_current->k <= current->k )
                        /* remove second */
                        prev_current->next = current->next;
                    else
                        /* remove first */
                        first->next = current;
                }
                else
                    first->next->next = current;
            }
            first = current;
            count = 1;
        }
        else
            count++;
        prev_current = current;
        current = current->next;
    }
    while( current != 0 );

  copy_vect:

    /* gather points */
    current = temp.next;
    assert( current );

    do
    {
        CVLIB_WRITE_SEQ_ELEM( current->pt, writer );
        current = current->next;
    }
    while( current != 0 );

	exit_function:

    *contour = endWriteSeq( &writer );

    assert( writer.seq->total > 0 );

    if( buffer != local_buffer )
        cvFree( &buffer );
    return 1 ;
}

/*************************************************************************************/

int startReadChainPoints( Chain * chain, ChainPtReader * reader )
{
    int i;    
	
    if( !chain || !reader )
        return -27 ;
	
    if( chain->elem_size != 1 || chain->header_size < (int)sizeof(Chain))
        return -1 ;
	
    startReadSeq( (Sequence *) chain, (SeqReader *) reader, 0 );
//    CV_CHECK();
	
    reader->pt = chain->origin;
	
    for( i = 0; i < 8; i++ )
    {
        reader->deltas[i][0] = (char) icvCodeDeltas[i].x;
        reader->deltas[i][1] = (char) icvCodeDeltas[i].y;
    }
	return 1 ;
}
/*************************************************************************************/
Sequence* endFindContours( ContourScanner* _scanner )
{
    ContourScanner scanner;
    Sequence *first = 0;    
	
    if( !_scanner )
        return NULL ;
    scanner = *_scanner;
	
    if( scanner )
    {
        endProcessContour( scanner );
		
        if( scanner->storage1 != scanner->storage2 )
            releaseMemStorage( &(scanner->storage1) );
		
        if( scanner->cinfo_storage )
            releaseMemStorage( &(scanner->cinfo_storage) );
		
        first = scanner->frame.v_next;
        cvFree( _scanner );
    }	
    return first;
}
/*************************************************************************************/
Sequence* testContour( Sequence* contours, MemStorage* storage, int /*method*/, Sequence* rst )
{
	// test each contour
	Sequence* result ;
//	Sequence* squares = createSeq( 0, sizeof(Sequence), sizeof(Point2i), storage );
	
	int i ;
	double s, t ;
	
	while( contours )
	{
		// approximate contour with accuracy proportional
		// to the contour perimeter
		result = approxPoly( contours, sizeof(IPContour), storage,
			CVLIB_POLY_APPROX_DP, cvlibContourPerimeter(contours)*0.02, 0 );
		// square contours should have 4 vertices after approximation
		// relatively large area (to filter out noisy contours)
		// and be convex.
		// Note: absolute value of an area is used because
		// area may be positive or negative - in accordance with the
		// contour orientation
		if( result->total == 4 &&
			fabs(contourArea(result,CVLIB_WHOLE_SEQ)) > 1000 &&
			checkContourConvexity(result) )
		{
			s = 0;
			
			for( i = 0; i < 5; i++ )
			{
				// find minimum Angle between joint
				// edges (maximum of cosine)
				if( i >= 2 )
				{
					t = fabs(AngleBetWeen2Lines(
						(Point2i*)getSeqElem( result, i ),
						(Point2i*)getSeqElem( result, i-2 ),
						(Point2i*)getSeqElem( result, i-1 )));
					s = s > t ? s : t;
				}
			}
			
			// if cosines of all angles are small
			// (all angles are ~90 degree) then write quandrange
			// vertices to resultant sequence 
			if( s < 0.3 )
				for( i = 0; i < 4; i++ )
				{
					seqPush( rst, (Point2i*)getSeqElem( result, i ));				
				}
		}
		
		// take the next contour
		contours = contours->h_next;
	}	
//	return squares ;
	return NULL ;
}
/*************************************************************************************/
Sequence* approxPoly( const void*  array, int  header_size, MemStorage*  storage, int  method, 
						double  parameter, int parameter2 )
{
	Sequence* dst_seq = 0;
    Sequence *prev_contour = 0, *parent = 0;
    IPContour contour_header;
    Sequence* src_seq = 0;
    SeqBlock block;
    int recursive = 0;
    
    if( CVLIB_IS_SEQ( array ))
    {
        src_seq = (Sequence*)array;
        if( !CVLIB_IS_SEQ_POLYLINE( src_seq ))
            return NULL ;

        recursive = parameter2;

        if( !storage )
            storage = src_seq->storage;
    }
    else
    {
        src_seq = pointSeqFromMat( CVLIB_SEQ_KIND_CURVE | (parameter2 ? CVLIB_SEQ_FLAG_CLOSED : 0), 
									 array, &contour_header, &block ) ;

		if( src_seq == NULL )
			return NULL ;
    }

    if( !storage )
        return NULL ;

    if( header_size < 0 )
        return NULL ;

    if( header_size == 0 )
        header_size = src_seq->header_size;

    if( !CVLIB_IS_SEQ_POLYLINE( src_seq ))
		return NULL ;

    if( header_size == 0 )
        header_size = src_seq->header_size;

    if( header_size < (int)sizeof(IPContour) )
        return NULL ;

    if( method != CVLIB_POLY_APPROX_DP )
        return NULL ;

    while( src_seq != 0 )
    {
        Sequence *contour = 0;

        switch (method)
        {
        case CVLIB_POLY_APPROX_DP:
            if( parameter < 0 )
                return NULL ;

            if( CVLIB_SEQ_ELTYPE(src_seq) == CVLIB_32SC2 )
            {
                iApproxPolyDP_32s( src_seq, header_size, storage, &contour, (float)parameter ) ;
            }
            else
            {
                //icvApproxPolyDP_32f( src_seq, header_size, storage, &contour, (float)parameter );
            }
            break;
        default:
            assert(0);
            return NULL ;
        }

        assert( contour );

        if( header_size >= (int)sizeof(IPContour))
            boundingRect( contour, 1 );

        contour->v_prev = parent;
        contour->h_prev = prev_contour;

        if( prev_contour )
            prev_contour->h_next = contour;
        else if( parent )
            parent->v_next = contour;
        prev_contour = contour;
        if( !dst_seq )
            dst_seq = prev_contour;

        if( !recursive )
            break;

        if( src_seq->v_next )
        {
            assert( prev_contour != 0 );
            parent = prev_contour;
            prev_contour = 0;
            src_seq = src_seq->v_next;
        }
        else
        {
            while( src_seq->h_next == 0 )
            {
                src_seq = src_seq->v_prev;
                if( src_seq == 0 )
                    break;
                prev_contour = parent;
                if( parent )
                    parent = parent->v_prev;
            }
            if( src_seq )
                src_seq = src_seq->h_next;
        }
    }    
    return dst_seq;
}
/*************************************************************************************/
double arcLength( const void *array, Slice slice, int is_closed )
{
    double perimeter = 0;	
    int i, j = 0, count;
    const int N = 16;
    float buf[N];
    Mat buffer ;
	buffer.create( buf, 1, N, MAT_Tfloat) ;

    SeqReader reader;
    IPContour contour_header;
    Sequence* contour = 0;
    SeqBlock block;
	
    if( CVLIB_IS_SEQ( array ))
    {
        contour = (Sequence*)array;
        if( !CVLIB_IS_SEQ_POLYLINE( contour ))
            return -5 ;
        if( is_closed < 0 )
            is_closed = CVLIB_IS_SEQ_CLOSED( contour );
    }
    else
    {
        is_closed = is_closed > 0;
        contour = pointSeqFromMat( CVLIB_SEQ_KIND_CURVE | (is_closed ? CVLIB_SEQ_FLAG_CLOSED : 0), 
								   array, &contour_header, &block ) ;
		if( contour == NULL )
			return -5 ;
    }
	
    if( contour->total > 1 )
    {
        int is_float = CVLIB_SEQ_ELTYPE( contour ) == CVLIB_32FC2;
        
        startReadSeq( contour, &reader, 0 );
        setSeqReaderPos( &reader, slice.start_index );
        count = sliceLength( slice, contour );
		
        count -= !is_closed && count == contour->total;
		
        /* scroll the reader by 1 point */
        reader.prev_elem = reader.ptr;
        CVLIB_NEXT_SEQ_ELEM( sizeof(Point2i), reader );
		
        for( i = 0; i < count; i++ )
        {
            float dx, dy;
			
            if( !is_float )
            {
                Point2i* pt = (Point2i*)reader.ptr;
                Point2i* prev_pt = (Point2i*)reader.prev_elem;
				
                dx = (float)pt->x - (float)prev_pt->x;
                dy = (float)pt->y - (float)prev_pt->y;
            }
            else
            {
                Point2f* pt = (Point2f*)reader.ptr;
                Point2f* prev_pt = (Point2f*)reader.prev_elem;
				
                dx = pt->x - prev_pt->x;
                dy = pt->y - prev_pt->y;
            }
			
            reader.prev_elem = reader.ptr;
            CVLIB_NEXT_SEQ_ELEM( contour->elem_size, reader );
			
            buffer.data.fl[0][j] = dx * dx + dy * dy;
            if( ++j == N || i == count - 1 )
            {
                buffer.resizeCols( j ) ;
				buffer.sqrt();
                for( ; j > 0; j-- )
                    perimeter += buffer.data.fl[0][j-1];
            }
        }
    }
    return perimeter;
}
/*************************************************************************************/
/* external contour area function */
double contourArea( const void *array, Slice slice )
{
    double area = 0;
    IPContour contour_header;
    Sequence* contour = 0;
    SeqBlock block;
	
    if( CVLIB_IS_SEQ( array ))
    {
        contour = (Sequence*)array;
        if( !CVLIB_IS_SEQ_POLYLINE( contour ))
            return  -5 ;
    }
    else
    {
        contour = pointSeqFromMat( CVLIB_SEQ_KIND_CURVE, array, &contour_header, &block ) ;
		
		if( contour == NULL )
			return -5 ;
    }
	
    if( sliceLength( slice, contour ) == contour->total )
    {
        iContourArea( contour, &area ) ;
    }
    else
    {
        if( CVLIB_SEQ_ELTYPE( contour ) != CVLIB_32SC2 )
            return -210 ;

        iContourSecArea( contour, slice, &area ) ;
    }    
	
    return area;
}
/*************************************************************************************/
int checkContourConvexity( const Arr* array )
{
    int flag = -1;
    int i;
    int orientation = 0;
    SeqReader reader;
    IPContour contour_header;
    SeqBlock block;
    Sequence* contour = (Sequence*)array;

    if( CVLIB_IS_SEQ(contour) )
    {
        if( !CVLIB_IS_SEQ_POLYGON(contour))
            return -210 ;
    }
    else
    {
        contour = pointSeqFromMat( CVLIB_SEQ_KIND_CURVE|CVLIB_SEQ_FLAG_CLOSED, array, &contour_header, &block );
    }

    if( contour->total == 0 )
        return 0 ;

    startReadSeq( contour, &reader, 0 );
    
    flag = 1;

    if( CVLIB_SEQ_ELTYPE( contour ) == CVLIB_32SC2 )
    {
        Point2i *prev_pt = (Point2i*)reader.prev_elem;
        Point2i *cur_pt = (Point2i*)reader.ptr;
    
        int dx0 = cur_pt->x - prev_pt->x;
        int dy0 = cur_pt->y - prev_pt->y;

        for( i = 0; i < contour->total; i++ )
        {
            int dxdy0, dydx0;
            int dx, dy;

            /*int orient; */
            CVLIB_NEXT_SEQ_ELEM( sizeof(Point2i), reader );
            prev_pt = cur_pt;
            cur_pt = (Point2i *) reader.ptr;

            dx = cur_pt->x - prev_pt->x;
            dy = cur_pt->y - prev_pt->y;
            dxdy0 = dx * dy0;
            dydx0 = dy * dx0;

            /* find orientation */
            /*orient = -dy0 * dx + dx0 * dy;
               orientation |= (orient > 0) ? 1 : 2;
             */
            orientation |= (dydx0 > dxdy0) ? 1 : ((dydx0 < dxdy0) ? 2 : 3);

            if( orientation == 3 )
            {
                flag = 0;
                break;
            }

            dx0 = dx;
            dy0 = dy;
        }
    }
    else
    {
        assert( CVLIB_SEQ_ELTYPE(contour) == CVLIB_32FC2 );

        Point2f *prev_pt = (Point2f*)reader.prev_elem;
        Point2f *cur_pt = (Point2f*)reader.ptr;
    
        float dx0 = cur_pt->x - prev_pt->x;
        float dy0 = cur_pt->y - prev_pt->y;

        for( i = 0; i < contour->total; i++ )
        {
            float dxdy0, dydx0;
            float dx, dy;

            /*int orient; */
            CVLIB_NEXT_SEQ_ELEM( sizeof(Point2f), reader );
            prev_pt = cur_pt;
            cur_pt = (Point2f*) reader.ptr;

            dx = cur_pt->x - prev_pt->x;
            dy = cur_pt->y - prev_pt->y;
            dxdy0 = dx * dy0;
            dydx0 = dy * dx0;

            /* find orientation */
            /*orient = -dy0 * dx + dx0 * dy;
               orientation |= (orient > 0) ? 1 : 2;
             */
            orientation |= (dydx0 > dxdy0) ? 1 : ((dydx0 < dxdy0) ? 2 : 3);

            if( orientation == 3 )
            {
                flag = 0;
                break;
            }

            dx0 = dx;
            dy0 = dy;
        }
    }
   
    return flag;
}


/*************************************************************************************/
Sequence* pointSeqFromMat( int seq_kind, const Arr* arr,
								 IPContour* contour_header, SeqBlock* block )
{
    Sequence* contour = 0;    
	
    assert( arr != 0 && contour_header != 0 && block != 0 );	
    
    int eltype;
    Mat* mat = (Mat*)arr;	
    
    if( !CVLIB_IS_MAT( mat ))
        return NULL ; 
	
//     eltype = CVLIB_MAT_TYPE( mat->type );
//     if( eltype != CVLIB_32SC2 && eltype != CVLIB_32FC2 )
//         CV_ERROR( CV_StsUnsupportedFormat,
//         "The matrix can not be converted to point sequence because of "
//         "inappropriate element type" );
	
//     if( mat->width != 1 && mat->height != 1 || !CVLIB_IS_MAT_CONT(mat->type))
//         CV_ERROR( CV_StsBadArg,
//         "The matrix converted to point sequence must be "
//         "1-dimensional and continuous" );
	
    makeSeqHeaderForArray(
		(seq_kind & (CVLIB_SEQ_KIND_MASK|CVLIB_SEQ_FLAG_CLOSED)) | eltype,
		sizeof(IPContour), CVLIB_ELEM_SIZE2(eltype), mat->data.ptr,
		mat->cols() * mat->rows(), (Sequence*)contour_header, block );
	
    contour = (Sequence*)contour_header;    
	
    return contour;
}

/*************************************************************************************/
/* area of a whole sequence */
int iContourArea( const Sequence* contour, double *area )
{
    if( contour->total )
    {
        SeqReader reader;
        int lpt = contour->total;
        double a00 = 0, xi_1, yi_1;
        int is_float = CVLIB_SEQ_ELTYPE(contour) == CVLIB_32FC2;
		
        startReadSeq( contour, &reader, 0 );
		
        if( !is_float )
        {
            xi_1 = ((Point2i*)(reader.ptr))->x;
            yi_1 = ((Point2i*)(reader.ptr))->y;
        }
        else
        {
            xi_1 = ((Point2f*)(reader.ptr))->x;
            yi_1 = ((Point2f*)(reader.ptr))->y;
        }
        CVLIB_NEXT_SEQ_ELEM( contour->elem_size, reader );
        
        while( lpt-- > 0 )
        {
            double dxy, xi, yi;
			
            if( !is_float )
            {
                xi = ((Point2i*)(reader.ptr))->x;
                yi = ((Point2i*)(reader.ptr))->y;
            }
            else
            {
                xi = ((Point2f*)(reader.ptr))->x;
                yi = ((Point2f*)(reader.ptr))->y;
            }
            CVLIB_NEXT_SEQ_ELEM( contour->elem_size, reader );
			
            dxy = xi_1 * yi - xi * yi_1;
            a00 += dxy;
            xi_1 = xi;
            yi_1 = yi;
        }
		
        *area = a00 * 0.5;
    }
    else
        *area = 0;
	
    return 1;
}
/*************************************************************************************/
/* area of a contour sector */
int iContourSecArea( Sequence * contour, Slice slice, double *area )
{
    Point2i pt;                 /*  pointer to points   */
    Point2i pt_s, pt_e;         /*  first and last points  */
    SeqReader reader;         /*  points reader of contour   */

    int p_max = 2, p_ind;
    int lpt, flag, i;
    double a00;                 /* unnormalized moments m00    */
    double xi, yi, xi_1, yi_1, x0, y0, dxy, sk, sk1, t;
    double x_s, y_s, nx, ny, dx, dy, du, dv;
    double eps = 1.e-5;
    double *p_are1, *p_are2, *p_are;

    assert( contour != NULL );

    if( contour == NULL )
        return -2 ;

    if( !CVLIB_IS_SEQ_POLYGON( contour ))
        return -12 ;

    lpt = sliceLength( slice, contour );
    /*if( n2 >= n1 )
        lpt = n2 - n1 + 1;
    else
        lpt = contour->total - n1 + n2 + 1;*/

    if( contour->total && lpt > 2 )
    {
        a00 = x0 = y0 = xi_1 = yi_1 = 0;
        sk1 = 0;
        flag = 0;
        dxy = 0;
        p_are1 = (double *) alloc( p_max * sizeof( double ));

        if( p_are1 == NULL )
            return -3 ;

        p_are = p_are1;
        p_are2 = NULL;

        startReadSeq( contour, &reader, 0 );
        setSeqReaderPos( &reader, slice.start_index );
        CVLIB_READ_SEQ_ELEM( pt_s, reader );
        p_ind = 0;
        setSeqReaderPos( &reader, slice.end_index );
        CVLIB_READ_SEQ_ELEM( pt_e, reader );

/*    normal coefficients    */
        nx = pt_s.y - pt_e.y;
        ny = pt_e.x - pt_s.x;
        setSeqReaderPos( &reader, slice.start_index );

        while( lpt-- > 0 )
        {
            CVLIB_READ_SEQ_ELEM( pt, reader );

            if( flag == 0 )
            {
                xi_1 = (double) pt.x;
                yi_1 = (double) pt.y;
                x0 = xi_1;
                y0 = yi_1;
                sk1 = 0;
                flag = 1;
            }
            else
            {
                xi = (double) pt.x;
                yi = (double) pt.y;

/****************   edges intersection examination   **************************/
                sk = nx * (xi - pt_s.x) + ny * (yi - pt_s.y);
                if( fabs( sk ) < eps && lpt > 0 || sk * sk1 < -eps )
                {
                    if( fabs( sk ) < eps )
                    {
                        dxy = xi_1 * yi - xi * yi_1;
                        a00 = a00 + dxy;
                        dxy = xi * y0 - x0 * yi;
                        a00 = a00 + dxy;

                        if( p_ind >= p_max )
                            iMemCopy( &p_are1, &p_are2, &p_are, &p_max );

                        p_are[p_ind] = a00 / 2.;
                        p_ind++;
                        a00 = 0;
                        sk1 = 0;
                        x0 = xi;
                        y0 = yi;
                        dxy = 0;
                    }
                    else
                    {
/*  define intersection point    */
                        dv = yi - yi_1;
                        du = xi - xi_1;
                        dx = ny;
                        dy = -nx;
                        if( fabs( du ) > eps )
                            t = ((yi_1 - pt_s.y) * du + dv * (pt_s.x - xi_1)) /
                                (du * dy - dx * dv);
                        else
                            t = (xi_1 - pt_s.x) / dx;
                        if( t > eps && t < 1 - eps )
                        {
                            x_s = pt_s.x + t * dx;
                            y_s = pt_s.y + t * dy;
                            dxy = xi_1 * y_s - x_s * yi_1;
                            a00 += dxy;
                            dxy = x_s * y0 - x0 * y_s;
                            a00 += dxy;
                            if( p_ind >= p_max )
                                iMemCopy( &p_are1, &p_are2, &p_are, &p_max );

                            p_are[p_ind] = a00 / 2.;
                            p_ind++;

                            a00 = 0;
                            sk1 = 0;
                            x0 = x_s;
                            y0 = y_s;
                            dxy = x_s * yi - xi * y_s;
                        }
                    }
                }
                else
                    dxy = xi_1 * yi - xi * yi_1;

                a00 += dxy;
                xi_1 = xi;
                yi_1 = yi;
                sk1 = sk;

            }
        }

        xi = x0;
        yi = y0;
        dxy = xi_1 * yi - xi * yi_1;

        a00 += dxy;

        if( p_ind >= p_max )
            iMemCopy( &p_are1, &p_are2, &p_are, &p_max );

        p_are[p_ind] = a00 / 2.;
        p_ind++;

/*     common area calculation    */
        *area = 0;
        for( i = 0; i < p_ind; i++ )
            (*area) += fabs( p_are[i] );

        if( p_are1 != NULL )
            cvFree( &p_are1 );
        else if( p_are2 != NULL )
            cvFree( &p_are2 );

        return 1 ;
    }
    else
        return -1 ;
}

/*************************************************************************************/
/* the version for integer point coordinates */
int iApproxPolyDP_32s( Sequence* src_contour, int header_size, MemStorage* storage, Sequence** dst_contour, float eps )
{
    int             init_iters = 3;
    Slice         slice, right_slice;
    SeqReader     reader, reader2;
    SeqWriter     writer;
    Point2i         start_pt(INT_MIN, INT_MIN), end_pt(0, 0), pt(0,0);
    int             i = 0, j, count = src_contour->total, new_count;
    int             is_closed = CVLIB_IS_SEQ_CLOSED( src_contour );
    int             le_eps = 0;
    MemStorage*   temp_storage = 0;
    Sequence*          stack = 0;
    
    assert( CVLIB_SEQ_ELTYPE(src_contour) == CVLIB_32SC2 );
    startWriteSeq( src_contour->flags, header_size, sizeof(pt), storage, &writer );

    if( src_contour->total == 0  )
    {
        *dst_contour = endWriteSeq( &writer );
        return 1;
    }

    temp_storage = createChildMemStorage( storage );

    assert( src_contour->first != 0 );
    stack = createSeq( 0, sizeof(Sequence), sizeof(Slice), temp_storage );
    eps *= eps;
    startReadSeq( src_contour, &reader, 0 );

    if( !is_closed )
    {
        right_slice.start_index = count;
        end_pt = *(Point2i*)(reader.ptr);
        start_pt = *(Point2i*)getSeqElem( src_contour, -1 );

        if( start_pt.x != end_pt.x || start_pt.y != end_pt.y )
        {
            slice.start_index = 0;
            slice.end_index = count - 1;
            seqPush( stack, &slice );
        }
        else
        {
            is_closed = 1;
            init_iters = 1;
        }
    }
    
    if( is_closed )
    {
        /* 1. find approximately two farthest points of the contour */
        right_slice.start_index = 0;

        for( i = 0; i < init_iters; i++ )
        {
            int max_dist = 0;
            setSeqReaderPos( &reader, right_slice.start_index, 1 );
            CVLIB_READ_SEQ_ELEM( start_pt, reader );   /* read the first point */

            for( j = 1; j < count; j++ )
            {
                int dx, dy, dist;

                CVLIB_READ_SEQ_ELEM( pt, reader );
                dx = pt.x - start_pt.x;
                dy = pt.y - start_pt.y;

                dist = dx * dx + dy * dy;

                if( dist > max_dist )
                {
                    max_dist = dist;
                    right_slice.start_index = j;
                }
            }

            le_eps = max_dist <= eps;
        }

        /* 2. initialize the stack */
        if( !le_eps )
        {
            slice.start_index = getSeqReaderPos( &reader );
            slice.end_index = right_slice.start_index += slice.start_index;

            right_slice.start_index -= right_slice.start_index >= count ? count : 0;
            right_slice.end_index = slice.start_index;
            if( right_slice.end_index < right_slice.start_index )
                right_slice.end_index += count;

            seqPush( stack, &right_slice );
            seqPush( stack, &slice );
        }
        else
            CVLIB_WRITE_SEQ_ELEM( start_pt, writer );
    }

    /* 3. run recursive process */
    while( stack->total != 0 )
    {
        seqPop( stack, &slice );

        if( slice.end_index > slice.start_index + 1 )
        {
            int dx, dy, dist, max_dist = 0;
            
            setSeqReaderPos( &reader, slice.end_index );
            CVLIB_READ_SEQ_ELEM( end_pt, reader );

            setSeqReaderPos( &reader, slice.start_index );
            CVLIB_READ_SEQ_ELEM( start_pt, reader );

            dx = end_pt.x - start_pt.x;
            dy = end_pt.y - start_pt.y;

            assert( dx != 0 || dy != 0 );

            for( i = slice.start_index + 1; i < slice.end_index; i++ )
            {
                CVLIB_READ_SEQ_ELEM( pt, reader );
                dist = ABS((pt.y - start_pt.y) * dx - (pt.x - start_pt.x) * dy);

                if( dist > max_dist )
                {
                    max_dist = dist;
                    right_slice.start_index = i;
                }
            }

            le_eps = (double)max_dist * max_dist <= eps * ((double)dx * dx + (double)dy * dy);
        }
        else
        {
            assert( slice.end_index > slice.start_index );
            le_eps = 1;
            /* read starting point */
            setSeqReaderPos( &reader, slice.start_index );
            CVLIB_READ_SEQ_ELEM( start_pt, reader );
        }

        if( le_eps )
        {
            CVLIB_WRITE_SEQ_ELEM( start_pt, writer );
        }
        else
        {
            right_slice.end_index = slice.end_index;
            slice.end_index = right_slice.start_index;
            seqPush( stack, &right_slice );
            seqPush( stack, &slice );
        }
    }

    is_closed = CVLIB_IS_SEQ_CLOSED( src_contour );
    if( !is_closed )
        CVLIB_WRITE_SEQ_ELEM( end_pt, writer );

    *dst_contour = endWriteSeq( &writer );
    
    startReadSeq( *dst_contour, &reader, is_closed );
    CVLIB_READ_SEQ_ELEM( start_pt, reader );

    reader2 = reader;
    CVLIB_READ_SEQ_ELEM( pt, reader );

    new_count = count = (*dst_contour)->total;
    for( i = !is_closed; i < count - !is_closed && new_count > 2; i++ )
    {
        int dx, dy, dist;
        CVLIB_READ_SEQ_ELEM( end_pt, reader );

        dx = end_pt.x - start_pt.x;
        dy = end_pt.y - start_pt.y;
        dist = ABS((pt.x - start_pt.x)*dy - (pt.y - start_pt.y)*dx);
        if( (double)dist * dist <= 0.5*eps*((double)dx*dx + (double)dy*dy) && dx != 0 && dy != 0 )
        {
            new_count--;
            *((Point2i*)reader2.ptr) = start_pt = end_pt;
            CVLIB_NEXT_SEQ_ELEM( sizeof(pt), reader2 );
            CVLIB_READ_SEQ_ELEM( pt, reader );
            i++;
            continue;
        }
        *((Point2i*)reader2.ptr) = start_pt = pt;
        CVLIB_NEXT_SEQ_ELEM( sizeof(pt), reader2 );
        pt = end_pt;
    }

    if( !is_closed )
        *((Point2i*)reader2.ptr) = pt;

    if( new_count < count )
        seqPopMulti( *dst_contour, 0, count - new_count, 0 );

    releaseMemStorage( &temp_storage );

    return 1;
}


/*===================================================================================*/
//////////////////////////////////////////////////////////////////////////
//End process Session
/*************************************************************************************/
float getEdgeValue( Mat* pmEdge, Vector<Point2i>& rect)
{
	float rGraySum = 0;
	int nLenSum = 0;
	for(int i = 0; i <4; i++)
	{
		//CvLineIterator iterator;
        Point2i startpt = rect[i];
        Point2i endpt = rect[(i + 1) % 4];
		int nCount = 0;
		int *pnX;
		int *pnY;
		linePoints(startpt.x, startpt.y, endpt.x, endpt.y, pnX, pnY, nCount);

		uchar** ppbEdge = pmEdge->data.ptr ;
		
        
		for(int j = 0; j < nCount; j++)
		{
			if (!pmEdge->isInside(pnX[j], pnY[j]))
				continue;
			rGraySum += ppbEdge[pnY[j]][pnX[j]];
		}
        
		delete []pnX;
		delete []pnY;
		nLenSum += nCount;
	}
    
	return rGraySum / nLenSum;
}

namespace ip {
	int findContours(Mat* img, Matrix<Point2i>& contours, int mode, int method, Point2i offset)
	{
		Sequence* contoursSeq = 0;
		MemStorage* storage = createMemStorage(0);
		int nret = findContours(img, storage, &contoursSeq, mode, method, offset);
		Sequence* cnt = contoursSeq;
		for (; cnt; cnt = cnt->h_next)
		{
			SeqBlock* pBlock = cnt->first;
			Vector<Point2i> contour;
			contour.setSize(pBlock->count);
			Point2i* pPoint = (Point2i*)pBlock->data;
			for (int k = 0; k < pBlock->count; k++)
				contour[k] = pPoint[k];
			contours.add(contour);
		}
		releaseMemStorage(&storage);
		return nret;
	}

}
/*************************************************************************************/


template<typename _Tp> static int
approxPolyDP_(const Point2_<_Tp>* src_contour, int count0, Point2_<_Tp>* dst_contour,
	bool is_closed0, double eps, AutoBuffer<Range>* _stack)
{
#define PUSH_SLICE(slice) \
        if( top >= stacksz ) \
        { \
            _stack->resize(stacksz*3/2); \
            stack = *_stack; \
            stacksz = _stack->size(); \
        } \
        stack[top++] = slice

#define READ_PT(pt, pos) \
        pt = src_contour[pos]; \
        if( ++pos >= count ) pos = 0

#define READ_DST_PT(pt, pos) \
        pt = dst_contour[pos]; \
        if( ++pos >= count ) pos = 0

#define WRITE_PT(pt) \
        dst_contour[new_count++] = pt

	typedef Point2_<_Tp> PT;
	int             init_iters = 3;
	Range           slice(0, 0), right_slice(0, 0);
	PT              start_pt((_Tp)-1000000, (_Tp)-1000000), end_pt(0, 0), pt(0, 0);
	int             i = 0, j, pos = 0, wpos, count = count0, new_count = 0;
	int             is_closed = is_closed0;
	bool            le_eps = false;
	size_t top = 0, stacksz = _stack->size();
	Range*          stack = *_stack;

	if (count == 0)
		return 0;

	eps *= eps;

	if (!is_closed)
	{
		right_slice.start = count;
		end_pt = src_contour[0];
		start_pt = src_contour[count - 1];

		if (start_pt.x != end_pt.x || start_pt.y != end_pt.y)
		{
			slice.start = 0;
			slice.end = count - 1;
			PUSH_SLICE(slice);
		}
		else
		{
			is_closed = 1;
			init_iters = 1;
		}
	}

	if (is_closed)
	{
		// 1. find approximately two farthest points of the contour
		right_slice.start = 0;

		for (i = 0; i < init_iters; i++)
		{
			double dist, max_dist = 0;
			pos = (pos + right_slice.start) % count;
			READ_PT(start_pt, pos);

			for (j = 1; j < count; j++)
			{
				double dx, dy;

				READ_PT(pt, pos);
				dx = pt.x - start_pt.x;
				dy = pt.y - start_pt.y;

				dist = dx * dx + dy * dy;

				if (dist > max_dist)
				{
					max_dist = dist;
					right_slice.start = j;
				}
			}

			le_eps = max_dist <= eps;
		}

		// 2. initialize the stack
		if (!le_eps)
		{
			right_slice.end = slice.start = pos % count;
			slice.end = right_slice.start = (right_slice.start + slice.start) % count;

			PUSH_SLICE(right_slice);
			PUSH_SLICE(slice);
		}
		else
			WRITE_PT(start_pt);
	}

	// 3. run recursive process
	while (top > 0)
	{
		slice = stack[--top];
		end_pt = src_contour[slice.end];
		pos = slice.start;
		READ_PT(start_pt, pos);

		if (pos != slice.end)
		{
			double dx, dy, dist, max_dist = 0;

			dx = end_pt.x - start_pt.x;
			dy = end_pt.y - start_pt.y;

			assert(dx != 0 || dy != 0);

			while (pos != slice.end)
			{
				READ_PT(pt, pos);
				dist = fabs((pt.y - start_pt.y) * dx - (pt.x - start_pt.x) * dy);

				if (dist > max_dist)
				{
					max_dist = dist;
					right_slice.start = (pos + count - 1) % count;
				}
			}

			le_eps = max_dist * max_dist <= eps * (dx * dx + dy * dy);
		}
		else
		{
			le_eps = true;
			// read starting point
			start_pt = src_contour[slice.start];
		}

		if (le_eps)
		{
			WRITE_PT(start_pt);
		}
		else
		{
			right_slice.end = slice.end;
			slice.end = right_slice.start;
			PUSH_SLICE(right_slice);
			PUSH_SLICE(slice);
		}
	}

	if (!is_closed)
		WRITE_PT(src_contour[count - 1]);

	// last stage: do final clean-up of the approximated contour -
	// remove extra points on the [almost] stright lines.
	is_closed = is_closed0;
	count = new_count;
	pos = is_closed ? count - 1 : 0;
	READ_DST_PT(start_pt, pos);
	wpos = pos;
	READ_DST_PT(pt, pos);

	for (i = !is_closed; i < count - !is_closed && new_count > 2; i++)
	{
		double dx, dy, dist, successive_inner_product;
		READ_DST_PT(end_pt, pos);

		dx = end_pt.x - start_pt.x;
		dy = end_pt.y - start_pt.y;
		dist = fabs((pt.x - start_pt.x)*dy - (pt.y - start_pt.y)*dx);
		successive_inner_product = (pt.x - start_pt.x) * (end_pt.x - pt.x) +
			(pt.y - start_pt.y) * (end_pt.y - pt.y);

		if (dist * dist <= 0.5*eps*(dx*dx + dy*dy) && dx != 0 && dy != 0 &&
			successive_inner_product >= 0)
		{
			new_count--;
			dst_contour[wpos] = start_pt = end_pt;
			if (++wpos >= count) wpos = 0;
			READ_DST_PT(pt, pos);
			i++;
			continue;
		}
		dst_contour[wpos] = start_pt = pt;
		if (++wpos >= count) wpos = 0;
		pt = end_pt;
	}

	if (!is_closed)
		dst_contour[wpos] = pt;

	return new_count;
}

namespace ip {
	int approxPolyDP(const Point2i* src_contour, int count0, Point2i* dst_contour,
		bool is_closed0, double eps, AutoBuffer<Range>* _stack)
	{
		return approxPolyDP_<int>(src_contour, count0, dst_contour, is_closed0, eps, _stack);
	}
}

}

#pragma warning (pop)
