
/*!
 * \file
 * \brief .
 * \author
 */


#pragma once

#include "cvlibbase/Inc/cvbase.h"
#include "cvlibbase/Inc/Mat.h"
#include "cvlibbase/Inc/AutoBuffer.h"
#include "FindContour.h"

namespace cvlib
{

/********************************IPContour Infomations****************************************/

struct CVLIB_DECLSPEC Chain
{
	CVLIB_SEQUENCE_FIELDS()
    Point2i  origin;
};

#define CVLIB_CONTOUR_FIELDS()  \
    CVLIB_SEQUENCE_FIELDS()     \
    Rect rect;				 \
    int color;               \
int reserved[3];

struct CVLIB_DECLSPEC IPContour
{
    CVLIB_CONTOUR_FIELDS()
};

typedef IPContour Point2DSeq;

struct CVLIB_DECLSPEC ContourInfo 
{
	int flags;
    ContourInfo *next;        /* next contour with the same mark value */
    ContourInfo *parent;      /* information about parent contour */
    Sequence *contour;             /* corresponding contour (may be 0, if rejected) */
    Rect rect;                /* bounding rectangle */
    Point2i origin;             /* origin point (where the contour was traced from) */
    int is_hole;                /* hole flag */
};

struct CVLIB_DECLSPEC _ContourScanner
{    
	MemStorage *storage1;     /* contains fetched contours */
	MemStorage *storage2;     /* contains approximated contours
	(!=storage1 if approx_method2 != approx_method1) */
	MemStorage *cinfo_storage;        /* contains _CvContourInfo nodes */
	Set *cinfo_set;           /* set of _CvContourInfo nodes */
	MemStoragePos initial_pos;        /* starting storage pos */
	MemStoragePos backup_pos; /* beginning of the latest approx. contour */
	MemStoragePos backup_pos2;        /* ending of the latest approx. contour */
	char *img0;                 /* image origin */
	char *img;                  /* current image row */
	int img_step;               /* image step */
	Size img_size;            /* ROI size */
	Point2i offset;             /* ROI offset: coordinates, added to each contour point */
	Point2i pt;                 /* current scanner position */
	Point2i lnbd;               /* position of the last met contour */
	int nbd;                    /* current mark val */
	ContourInfo *l_cinfo;    /* information about latest approx. contour */
	ContourInfo cinfo_temp;  /* temporary var which is used in simple modes */
	ContourInfo frame_info;  /* information about frame */
	Sequence frame;                /* frame itself */
	int approx_method1;         /* approx method when tracing */
	int approx_method2;         /* final approx method */
	int mode;                   /* contour scanning mode:
								0 - external only
								1 - all the contours w/o any hierarchy
								2 - connected components (i.e. two-level structure -
	external contours and holes) */
	int subst_flag;
	int seq_type1;              /* type of fetched contours */
	int header_size1;           /* hdr size of fetched contours */
	int elem_size1;             /* elem size of fetched contours */
	int seq_type2;              /*                                       */
	int header_size2;           /*        the same for approx. contours  */
	int elem_size2;             /*                                       */
    ContourInfo *cinfo_table[126];	
};

typedef struct _ContourScanner* ContourScanner;

/****************************************************************************************\
*                                  Chain Approximation                                   *
\****************************************************************************************/

struct CVLIB_DECLSPEC PtInfo
{
    Point2i pt;
    int k;                      /* support region */
    int s;                      /* curvature value */
    struct PtInfo *next;
};

/* Freeman chain reader state */

#define CVLIB_SEQ_READER_FIELDS()                                      \
    int          header_size;                                       \
    Sequence*       seq;        /* sequence, beign read */             \
    SeqBlock*  block;      /* current block */                    \
    char*        ptr;        /* pointer to element be read next */  \
    char*        block_min;  /* pointer to the beginning of block */\
    char*        block_max;  /* pointer to the end of block */      \
    int          delta_index;/* = seq->first->start_index   */      \
    char*        prev_elem;  /* pointer to previous element */

struct CVLIB_DECLSPEC ChainPtReader
{
    CVLIB_SEQ_READER_FIELDS()
	char		code;
    Point2i		pt;
    char		deltas[8][2];
};

#define CVLIB_IS_SEQ_CHAIN( seq )   \
    (CVLIB_SEQ_KIND(seq) == CVLIB_SEQ_KIND_CURVE && (seq)->elem_size == 1)

#define CVLIB_IS_SEQ_CHAIN_CONTOUR( seq ) \
    (CVLIB_IS_SEQ_CHAIN( seq ) && CVLIB_IS_SEQ_CLOSED( seq ))

//////////////////////////////////////////////////////////////////////////

/************************************* CvSlice ******************************************/

#define CVLIB_READ_CHAIN_POINT( _pt, reader )                              \
{                                                                       \
    (_pt) = (reader).pt;                                                \
    if( (reader).ptr )                                                  \
    {                                                                   \
	CVLIB_READ_SEQ_ELEM( (reader).code, (reader));                     \
	assert( ((reader).code & ~7) == 0 );                            \
	(reader).pt.x += (reader).deltas[(int)(reader).code][0];        \
	(reader).pt.y += (reader).deltas[(int)(reader).code][1];        \
    }                                                               \
}

#define CVLIB_POLY_APPROX_DP 0

#define cvlibContourPerimeter( contour ) arcLength( contour, CVLIB_WHOLE_SEQ, 1 )

#define CVLIB_IS_SEQ_POLYLINE( seq )   \
    (CVLIB_SEQ_KIND(seq) == CVLIB_SEQ_KIND_CURVE && CVLIB_IS_SEQ_POINT_SET(seq))

#define CVLIB_IS_SEQ_POLYGON( seq )   \
    (CVLIB_IS_SEQ_POLYLINE(seq) && CVLIB_IS_SEQ_CLOSED(seq))

inline double AngleBetWeen2Lines(const Point2i* pt1, Point2i* pt2, Point2i* pt0 )
{
    double dx1 = pt1->x - pt0->x;
    double dy1 = pt1->y - pt0->y;
    double dx2 = pt2->x - pt0->x;
    double dy2 = pt2->y - pt0->y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

//////////////////////////////////////////////////////////////////////////

int findContours( Mat* img, MemStorage* storage, Sequence** firstContour, int mode=CVLIB_RETR_LIST, 
				 int method=CVLIB_CHAIN_APPROX_SIMPLE, Point2i offset=Point2i() ) ;

ContourScanner startFindContours(Mat* pmImg, MemStorage* storage, int nRetrievalMode, int nApproximageMethod, Point2i startPt);

Sequence* findNextContour( ContourScanner scanner ) ;

int traceContour( char *ptr, int step, char *stop_ptr, int is_hole ) ;

void endProcessContour( ContourScanner scanner ) ;
int fetchContour( char *ptr, int step, Point2i pt, Sequence* contour, int _method ) ;

int fetchContourEx( char* ptr, int step, Point2i pt, Sequence* contour, int  _method,  int nbd, Rect* _rect ) ;

int approximateChainTC89( Chain* chain, int header_size, MemStorage* storage, 
                         Sequence** contour, int method ) ;

Sequence* endFindContours( ContourScanner *_scanner ) ;

void endSquares( Mat* graySrc, Matrix<Point2i>& squares, int nScale, bool bMulti = false ) ;

float getEdgeValue( Mat* pmEdge, Vector<Point2i>& rect) ;

Sequence* testContour( Sequence* contours, MemStorage* storage, int method, Sequence* rst ) ;

Sequence* approxPoly( const void*  array, int  header_size, MemStorage*  storage, int  method, double  parameter, int parameter2 ) ;

/* the version for integer point coordinates */
int iApproxPolyDP_32s( Sequence* src_contour, int header_size, MemStorage* storage, Sequence** dst_contour, float eps ) ;

double arcLength( const void *array, Slice slice, int is_closed ) ;

/* external contour area function */
double contourArea( const void *array, Slice slice ) ;

int checkContourConvexity( const Arr* array ) ;

Sequence* pointSeqFromMat( int seq_kind, const Arr* arr, IPContour* contour_header, SeqBlock* block ) ;
int getSeqReaderPos( SeqReader* reader );

/* area of a whole sequence */
int iContourArea( const Sequence* contour, double *area ) ;

/* area of a contour sector */
int iContourSecArea( Sequence * contour, Slice slice, double *area ) ;


/*=====================================Memory Functions=========================================*/

Rect boundingRect( Arr* array, int update ) ;

//SeqWiriter funcs


int startReadChainPoints( Chain * chain, ChainPtReader * reader ) ;

/*=====================================Memory Functions=========================================*/

int findContours(Mat* img, Matrix<Point2i>& contours, int mode = CVLIB_RETR_LIST,
	int method = CVLIB_CHAIN_APPROX_SIMPLE, Point2i offset = Point2i());

int approxPolyDP(const Point2i* src_contour, int count0, Point2i* dst_contour,
	bool is_closed0, double eps, AutoBuffer<Range>* _stack);

//memstorage

}