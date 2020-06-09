
#include "_cvlibbase.h"
#include "IntegralImage.h"
#include "binarizewolfjolion.h"

using namespace std;
using namespace cvlib;

enum NiblackVersion
{
	NIBLACK=0,
    SAUVOLA,
    WOLFJOLION,
};

#define uget(x,y)    at<unsigned char>(y,x)
#define uset(x,y,v)  at<unsigned char>(y,x)=v;
#define fget(x,y)    at<float>(y,x)
#define fset(x,y,v)  at<float>(y,x)=v;

// *************************************************************
// glide a window across the image and
// *************************************************************
// create two maps: mean and standard deviation.
//

double calcLocalStats(const Mat &im, Mat &map_m, Mat &map_s, int winx, int winy) {
	Mat im_sum, im_sum_sq;
	cvlib::ip::integralImage(im, im_sum, im_sum_sq);
	assert(im_sum.type1() == MAT_Tint && im_sum_sq.type1() == MAT_Tdouble);

	double m, s, max_s, sum, sum_sq;
	int wxh = winx / 2;
	int wyh = winy / 2;
	int x_firstth = wxh;
	int y_firstth = wyh;
	int y_lastth = im.rows() - wyh - 1;
	double winarea = winx*winy;

	max_s = 0;
	for (int j = y_firstth; j <= y_lastth; j++) {
		sum = sum_sq = 0;

		// for sum array iterator pointer
		int* sum_top_left = im_sum.data.i[j - wyh];
		int* sum_top_right = sum_top_left + winx;
		int* sum_bottom_left = im_sum.data.i[j - wyh + winy];
		int* sum_bottom_right = sum_bottom_left + winx;

		// for sum_sq array iterator pointer
		double *sum_eq_top_left = im_sum_sq.data.db[j - wyh];
		double *sum_eq_top_right = sum_eq_top_left + winx;
		double *sum_eq_bottom_left = im_sum_sq.data.db[j - wyh + winy];
		double *sum_eq_bottom_right = sum_eq_bottom_left + winx;

		sum = (*sum_bottom_right + *sum_top_left) - (*sum_top_right + *sum_bottom_left);
		sum_sq = (*sum_eq_bottom_right + *sum_eq_top_left) - (*sum_eq_top_right + *sum_eq_bottom_left);

		m = sum / winarea;
		s = sqrt((sum_sq - m*sum) / winarea);
		if (s > max_s) max_s = s;

		float *map_m_data = map_m.data.fl[j] + x_firstth;
		float *map_s_data = map_s.data.fl[j] + x_firstth;
		*map_m_data++ = m;
		*map_s_data++ = s;

		// Shift the window, add and remove	new/old values to the histogram
		for (int i = 1; i <= im.cols() - winx; i++) {
			sum_top_left++, sum_top_right++, sum_bottom_left++, sum_bottom_right++;

			sum_eq_top_left++, sum_eq_top_right++, sum_eq_bottom_left++, sum_eq_bottom_right++;

			sum = (*sum_bottom_right + *sum_top_left) - (*sum_top_right + *sum_bottom_left);
			sum_sq = (*sum_eq_bottom_right + *sum_eq_top_left) - (*sum_eq_top_right + *sum_eq_bottom_left);

			m = sum / winarea;
			s = sqrt((sum_sq - m*sum) / winarea);
			if (s > max_s) max_s = s;

			*map_m_data++ = m;
			*map_s_data++ = s;
		}
	}

	return max_s;
}



/**********************************************************
 * The binarization routine
 **********************************************************/


void NiblackSauvolaWolfJolion(const Mat& im, Mat& output, NiblackVersion version, int winx, int winy, double k, double dR)
{
	double m, s, max_s;
	double th = 0;
	double min_I, max_I;
	int wxh = winx / 2;
	int wyh = winy / 2;
	int x_firstth = wxh;
	int x_lastth = im.cols() - wxh - 1;
	int y_lastth = im.rows() - wyh - 1;
	int y_firstth = wyh;
	// int mx, my;

	// Create local statistics and store them in a double matrices
	Mat map_m = Mat::zeros(im.rows(), im.cols(), MAT_Tfloat);
	Mat map_s = Mat::zeros(im.rows(), im.cols(), MAT_Tfloat);
	max_s = calcLocalStats(im, map_m, map_s, winx, winy);
	float inv_max_s = 1.f / max_s;

	im.minMaxLoc(&min_I, &max_I);

	Mat thsurf(im.size(), MAT_Tfloat);

	// Create the threshold surface, including border processing
	// ----------------------------------------------------
	for (int j = y_firstth; j <= y_lastth; j++) {

		float *th_surf_data = thsurf.data.fl[j] + wxh;
		float *map_m_data = map_m.data.fl[j] + wxh;
		float *map_s_data = map_s.data.fl[j] + wxh;

		// NORMAL, NON-BORDER AREA IN THE MIDDLE OF THE WINDOW:
		for (int i = 0; i <= im.cols() - winx; i++) {
			m = *map_m_data++;
			s = *map_s_data++;

			// Calculate the threshold
			switch (version) {

			case NIBLACK:
				th = m + k*s;
				break;

			case SAUVOLA:
				th = m * (1 + k*(s / dR - 1));
				break;

			case WOLFJOLION:
				th = m + k * (s * inv_max_s - 1) * (m - min_I);
				break;
			}

			// thsurf.fset(i+wxh,j,th);
			*th_surf_data++ = th;


			if (i == 0) {
				// LEFT BORDER
				float *th_surf_ptr = thsurf.data.fl[j];
				for (int i = 0; i <= x_firstth; ++i)
					*th_surf_ptr++ = th;

				// LEFT-UPPER CORNER
				if (j == y_firstth)
				{
					for (int u = 0; u < y_firstth; ++u)
					{
						float *th_surf_ptr = thsurf.data.fl[u];
						for (int i = 0; i <= x_firstth; ++i)
							*th_surf_ptr++ = th;
					}

				}

				// LEFT-LOWER CORNER
				if (j == y_lastth)
				{
					for (int u = y_lastth + 1; u < im.rows(); ++u)
					{
						float *th_surf_ptr = thsurf.data.fl[u];
						for (int i = 0; i <= x_firstth; ++i)
							*th_surf_ptr++ = th;
					}
				}
			}

			// UPPER BORDER
			if (j == y_firstth)
				for (int u = 0; u < y_firstth; ++u)
					thsurf.fset(i + wxh, u, th);

			// LOWER BORDER
			if (j == y_lastth)
				for (int u = y_lastth + 1; u < im.rows(); ++u)
					thsurf.fset(i + wxh, u, th);
		}

		// RIGHT BORDER
		float *th_surf_ptr = thsurf.data.fl[j] + x_lastth;
		for (int i = x_lastth; i < im.cols(); ++i)
			// thsurf.fset(i,j,th);
			*th_surf_ptr++ = th;

		// RIGHT-UPPER CORNER
		if (j == y_firstth)
		{
			for (int u = 0; u < y_firstth; ++u)
			{
				float *th_surf_ptr = thsurf.data.fl[u] + x_lastth;
				for (int i = x_lastth; i < im.cols(); ++i)
					*th_surf_ptr++ = th;
			}
		}

		// RIGHT-LOWER CORNER
		if (j == y_lastth)
		{
			for (int u = y_lastth + 1; u < im.rows(); ++u)
			{
				float *th_surf_ptr = thsurf.data.fl[u] + x_lastth;
				for (int i = x_lastth; i < im.cols(); ++i)
					*th_surf_ptr++ = th;
			}
		}
	}

	for (int y = 0; y < im.rows(); ++y)
	{
		unsigned char *im_data = im.data.ptr[y];
		float *th_surf_data = thsurf.data.fl[y];
		unsigned char *output_data = output.data.ptr[y];
		for (int x = 0; x < im.cols(); ++x)
		{
			*output_data = *im_data >= *th_surf_data ? 255 : 0;
			im_data++;
			th_surf_data++;
			output_data++;
		}
	}
}

/**********************************************************
 * The main function
 **********************************************************/

namespace cvlib
{
	void binarization(const Mat& image, Mat& bin, enPageBinary mode, int winx, int winy, float optK)
	{
		bool didSpecifyK = true;
		NiblackVersion versionCode = (NiblackVersion)mode;

		// Treat the window size
		if (winx == 0 || winy == 0) {
			winy = (int)(2.0 * image.rows() - 1) / 3;
			winx = (int)image.cols() - 1 < winy ? image.cols() - 1 : winy;
			// if the window is too big, than we asume that the image
			// is not a single text box, but a document page: set
			// the window size to a fixed constant.
			if (winx > 100)
				winx = winy = 40;
		}

		// Threshold
		bin.create(image.size(), MAT_Tuchar);
		NiblackSauvolaWolfJolion(image, bin, versionCode, winx, winy, optK, 128);
	}

}
