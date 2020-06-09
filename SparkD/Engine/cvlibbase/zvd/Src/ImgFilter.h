
#pragma once

#pragma warning(disable: 4251)

#include "_cvlibbase.h"
#include "cvlibipbase.h"
#include <vector>

#ifdef __cplusplus

/*! \namespace cvlib
Namespace where all the C++ cvlib functionality resides
*/
namespace cvlib {
	namespace ip {

		using std::vector;
		/*!
		The Base Class for 1D or Row-wise Filters

		This is the base class for linear or non-linear filters that process 1D data.
		In particular, such filters are used for the "horizontal" filtering parts in separable filters.

		Several functions in OpenCV return Ptr<BaseRowFilter> for the specific types of filters,
		and those pointers can be used directly or within cvlib::FilterEngine.
		*/
		class CVLIB_DECLSPEC BaseRowFilter
		{
		public:
			//! the default constructor
			BaseRowFilter();
			//! the destructor
			virtual ~BaseRowFilter();
			//! the filtering operator. Must be overrided in the derived classes. The horizontal border interpolation is done outside of the class.
			virtual void operator()(const uchar* src, uchar* dst,
				int width, int cn) = 0;
			int ksize, anchor;
		};


		/*!
		The Base Class for column-wise Filters

		This is the base class for linear or non-linear filters that process columns of 2D arrays.
		Such filters are used for the "vertical" filtering parts in separable filters.

		Several functions in OpenCV return Ptr<BaseColumnFilter> for the specific types of filters,
		and those pointers can be used directly or within cvlib::FilterEngine.

		Unlike cvlib::BaseRowFilter, cvlib::BaseColumnFilter may have some context information,
		i.e. box filter keeps the sliding sum of elements. To reset the state BaseColumnFilter::reset()
		must be called (e.g. the method is called by cvlib::FilterEngine)
		*/
		class CVLIB_DECLSPEC BaseColumnFilter
		{
		public:
			//! the default constructor
			BaseColumnFilter();
			//! the destructor
			virtual ~BaseColumnFilter();
			//! the filtering operator. Must be overrided in the derived classes. The vertical border interpolation is done outside of the class.
			virtual void operator()(const uchar** src, uchar* dst, int dststep,
				int dstcount, int width) = 0;
			//! resets the internal buffers, if any
			virtual void reset();
			int ksize, anchor;
		};

		/*!
		The Base Class for Non-Separable 2D Filters.

		This is the base class for linear or non-linear 2D filters.

		Several functions in OpenCV return Ptr<BaseFilter> for the specific types of filters,
		and those pointers can be used directly or within cvlib::FilterEngine.

		Similar to cvlib::BaseColumnFilter, the class may have some context information,
		that should be reset using BaseFilter::reset() method before processing the new array.
		*/
		class CVLIB_DECLSPEC BaseFilter
		{
		public:
			//! the default constructor
			BaseFilter();
			//! the destructor
			virtual ~BaseFilter();
			//! the filtering operator. The horizontal and the vertical border interpolation is done outside of the class.
			virtual void operator()(const uchar** src, uchar* dst, int dststep,
				int dstcount, int width, int cn) = 0;
			//! resets the internal buffers, if any
			virtual void reset();
			Size ksize;
			Point anchor;
		};

		/*!
		The Main Class for Image Filtering.

		The class can be used to apply an arbitrary filtering operation to an image.
		It contains all the necessary intermediate buffers, it computes extrapolated values
		of the "virtual" pixels outside of the image etc.
		Pointers to the initialized cvlib::FilterEngine instances
		are returned by various OpenCV functions, such as cvlib::createSeparableLinearFilter(),
		cvlib::createLinearFilter(), cvlib::createGaussianFilter(), cvlib::createDerivFilter(),
		cvlib::createBoxFilter() and cvlib::createMorphologyFilter().

		Using the class you can process large images by parts and build complex pipelines
		that include filtering as some of the stages. If all you need is to apply some pre-defined
		filtering operation, you may use cvlib::filter2D(), cvlib::erode(), cvlib::dilate() etc.
		functions that create FilterEngine internally.

		Here is the example on how to use the class to implement Laplacian operator, which is the sum of
		second-order derivatives. More complex variant for different types is implemented in cvlib::Laplacian().

		\code
		void laplace_f(const Mat& src, Mat& dst)
		{
		assert( src.type() == MAT_Tfloat );
		// make sure the destination array has the proper size and type
		dst.create(src.size(), src.type());

		// get the derivative and smooth kernels for d2I/dx2.
		// for d2I/dy2 we could use the same kernels, just swapped
		Mat kd, ks;
		getSobelKernels( kd, ks, 2, 0, ksize, false, ktype );

		// let's process 10 source rows at once
		int DELTA = MIN(10, src.rows);
		Ptr<FilterEngine> Fxx = createSeparableLinearFilter(src.type(),
		dst.type(), kd, ks, Point(-1,-1), 0, borderType, borderType, Scalar() );
		Ptr<FilterEngine> Fyy = createSeparableLinearFilter(src.type(),
		dst.type(), ks, kd, Point(-1,-1), 0, borderType, borderType, Scalar() );

		int y = Fxx->start(src), dsty = 0, dy = 0;
		Fyy->start(src);
		const uchar* sptr = src.data + y*src.step;

		// allocate the buffers for the spatial image derivatives;
		// the buffers need to have more than DELTA rows, because at the
		// last iteration the output may take max(kd.rows-1,ks.rows-1)
		// rows more than the input.
		Mat Ixx( DELTA + kd.rows - 1, src.cols, dst.type() );
		Mat Iyy( DELTA + kd.rows - 1, src.cols, dst.type() );

		// inside the loop we always pass DELTA rows to the filter
		// (note that the "proceed" method takes care of possibe overflow, since
		// it was given the actual image height in the "start" method)
		// on output we can get:
		//  * < DELTA rows (the initial buffer accumulation stage)
		//  * = DELTA rows (settled state in the middle)
		//  * > DELTA rows (then the input image is over, but we generate
		//                  "virtual" rows using the border mode and filter them)
		// this variable number of output rows is dy.
		// dsty is the current output row.
		// sptr is the pointer to the first input row in the portion to process
		for( ; dsty < dst.rows; sptr += DELTA*src.step, dsty += dy )
		{
		Fxx->proceed( sptr, (int)src.step, DELTA, Ixx.data, (int)Ixx.step );
		dy = Fyy->proceed( sptr, (int)src.step, DELTA, d2y.data, (int)Iyy.step );
		if( dy > 0 )
		{
		Mat dstripe = dst.rowRange(dsty, dsty + dy);
		add(Ixx.rowRange(0, dy), Iyy.rowRange(0, dy), dstripe);
		}
		}
		}
		\endcode
		*/
		class CVLIB_DECLSPEC FilterEngine
		{
		public:
			//! the default constructor
			FilterEngine();
			//! the full constructor. Either _filter2D or both _rowFilter and _columnFilter must be non-empty.
			FilterEngine(const Ptr<BaseFilter>& _filter2D,
				const Ptr<BaseRowFilter>& _rowFilter,
				const Ptr<BaseColumnFilter>& _columnFilter,
				int srcType, int dstType, int bufType,
				int _rowBorderType = BORDER_REPLICATE,
				int _columnBorderType = -1,
				const Scalar& _borderValue = Scalar());
			//! the destructor
			virtual ~FilterEngine();
			//! reinitializes the engine. The previously assigned filters are released.
			void init(const Ptr<BaseFilter>& _filter2D,
				const Ptr<BaseRowFilter>& _rowFilter,
				const Ptr<BaseColumnFilter>& _columnFilter,
				int srcType, int dstType, int bufType,
				int _rowBorderType = BORDER_REPLICATE, int _columnBorderType = -1,
				const Scalar& _borderValue = Scalar());
			//! starts filtering of the specified ROI of an image of size wholeSize.
			virtual int start(Size wholeSize, Rect roi, int maxBufRows = -1);
			//! starts filtering of the specified ROI of the specified image.
			virtual int start(const Mat& src, const Rect& srcRoi = Rect(0, 0, -1, -1), bool isolated = false, int maxBufRows = -1);
			//! processes the next srcCount rows of the image.
			virtual int proceed(const uchar* src, int srcStep, int srcCount,
				uchar* dst, int dstStep);
			//! applies filter to the specified ROI of the image. if srcRoi=(0,0,-1,-1), the whole image is filtered.
			virtual void apply(const Mat& src, Mat& dst,
				const Rect& srcRoi = Rect(0, 0, -1, -1),
				Point dstOfs = Point(0, 0),
				bool isolated = false);
			//! returns true if the filter is separable
			bool isSeparable() const { return (const BaseFilter*)filter2D == 0; }
			//! returns the number
			int remainingInputRows() const;
			int remainingOutputRows() const;

			int srcType, dstType, bufType;
			Size ksize;
			Point anchor;
			int maxWidth;
			Size wholeSize;
			Rect roi;
			int dx1, dx2;
			int rowBorderType, columnBorderType;
			vector<int> borderTab;
			int borderElemSize;
			vector<uchar> ringBuf;
			vector<uchar> srcRow;
			vector<uchar> constBorderValue;
			vector<uchar> constBorderRow;
			int bufStep, startY, startY0, endY, rowCount, dstY;
			vector<uchar*> rows;

			Ptr<BaseFilter> filter2D;
			Ptr<BaseRowFilter> rowFilter;
			Ptr<BaseColumnFilter> columnFilter;
		};

		//! type of the kernel
		enum {
			KERNEL_GENERAL = 0, KERNEL_SYMMETRICAL = 1, KERNEL_ASYMMETRICAL = 2,
			KERNEL_SMOOTH = 4, KERNEL_INTEGER = 8
		};

		CVLIB_DECLSPEC Point normalizeAnchor(Point anchor, Size ksize);

		//! returns type (one of KERNEL_*) of 1D or 2D kernel specified by its coefficients.
		CVLIB_DECLSPEC int getKernelType(const Mat& kernel, Point anchor);

		//! returns the primitive row filter with the specified kernel
		CVLIB_DECLSPEC Ptr<BaseRowFilter> getLinearRowFilter(int srcType, int bufType,
			const Mat& kernel, int anchor,
			int symmetryType);

		//! returns the primitive column filter with the specified kernel
		CVLIB_DECLSPEC Ptr<BaseColumnFilter> getLinearColumnFilter(int bufType, int dstType,
			const Mat& kernel, int anchor,
			int symmetryType, double delta = 0,
			int bits = 0);

		//! returns 2D filter with the specified kernel
		CVLIB_DECLSPEC Ptr<BaseFilter> getLinearFilter(int srcType, int dstType,
			const Mat& kernel,
			Point anchor = Point(-1, -1),
			double delta = 0, int bits = 0);

		//! returns the separable linear filter engine
		CVLIB_DECLSPEC Ptr<FilterEngine> createSeparableLinearFilter(int srcType, int dstType,
			const Mat& rowKernel, const Mat& columnKernel,
			Point anchor = Point(-1, -1), double delta = 0,
			int rowBorderType = BORDER_DEFAULT,
			int columnBorderType = -1,
			const Scalar& borderValue = Scalar());

		//! returns the non-separable linear filter engine
		CVLIB_DECLSPEC Ptr<FilterEngine> createLinearFilter(int srcType, int dstType,
			const Mat& kernel, Point _anchor = Point(-1, -1),
			double delta = 0, int rowBorderType = BORDER_DEFAULT,
			int columnBorderType = -1, const Scalar& borderValue = Scalar());

		//! initializes kernels of the generalized Sobel operator
		CVLIB_DECLSPEC void getDerivKernels(Mat& kx, Mat& ky,
			int dx, int dy, int ksize,
			bool normalize = false, int ktype = MAT_Tfloat);


		//! copies 2D array to a larger destination array with extrapolation of the outer part of src using the specified border mode
		CVLIB_DECLSPEC void copyMakeBorder(const Mat& src, Mat& dst,
			int top, int bottom, int left, int right,
			int borderType, const Scalar& value = Scalar());

		//! applies non-separable 2D linear filter to the image
		CVLIB_DECLSPEC void filter2D(const Mat& src, Mat& dst, int ddepth,
			const Mat& kernel, Point anchor = Point(-1, -1),
			double delta = 0, int borderType = BORDER_DEFAULT);

		//! applies separable 2D linear filter to the image
		CVLIB_DECLSPEC void sepFilter2D(const Mat& src, Mat& dst, int ddepth,
			const Mat& kernelX, const Mat& kernelY,
			Point anchor = Point(-1, -1),
			double delta = 0, int borderType = BORDER_DEFAULT);


		//! interpolation algorithm
		enum
		{
			INTER_NEAREST = 0, //!< nearest neighbor interpolation
			INTER_LINEAR = 1, //!< bilinear interpolation
			INTER_CUBIC = 2, //!< bicubic interpolation
			INTER_AREA = 3, //!< area-based (or super) interpolation
			INTER_LANCZOS4 = 4, //!< Lanczos interpolation over 8x8 neighborhood
			INTER_MAX = 7,
			WARP_INVERSE_MAP = 8
		};

		enum
		{
			INTER_BITS = 5, INTER_BITS2 = INTER_BITS * 2,
			INTER_TAB_SIZE = (1 << INTER_BITS),
			INTER_TAB_SIZE2 = INTER_TAB_SIZE*INTER_TAB_SIZE
		};

		CVLIB_DECLSPEC void preprocess2DKernel(const Mat& kernel, vector<Point>& coords, vector<uchar>& coeffs);

	}
}

#endif /* __cplusplus */
