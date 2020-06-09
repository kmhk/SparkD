#include "ColorSpace.h"
#include "cvlibmacros.h"

namespace cvlib
{

	static float D65X = 0.9505f;
	static float D65Y = 1.0f;
	static float D65Z = 1.0890f;

	#undef R2Y
	#undef G2Y
	#undef B2Y

	enum
	{
		yuv_shift = 14,
		xyz_shift = 12,
		R2Y = 4899,
		G2Y = 9617,
		B2Y = 1868,
		BLOCK_SIZE = 256
	};

	template<typename _Tp> struct RGB2Gray
	{
		typedef _Tp channel_type;

		RGB2Gray(int _srccn, int blueIdx, const float* _coeffs) : srccn(_srccn)
		{
			static const float coeffs0[] = { 0.299f, 0.587f, 0.114f };
			memcpy(coeffs, _coeffs ? _coeffs : coeffs0, 3 * sizeof(coeffs[0]));
			if (blueIdx == 0)
				std::swap(coeffs[0], coeffs[2]);
		}

		void operator()(const _Tp* src, _Tp* dst, int n) const
		{
			int scn = srccn;
			float cb = coeffs[0], cg = coeffs[1], cr = coeffs[2];
			for (int i = 0; i < n; i++, src += scn)
				dst[i] = saturate_cast<_Tp>(src[0] * cb + src[1] * cg + src[2] * cr);
		}
		int srccn;
		float coeffs[3];
	};

	static int calcRGBTab = 0;
	static int tab[256 * 3];
	template<> struct RGB2Gray<uchar>
	{
		typedef uchar channel_type;

		RGB2Gray(int _srccn, int blueIdx, const int* coeffs) : srccn(_srccn)
		{
			if (calcRGBTab == 0) {
				const int coeffs0[] = { R2Y, G2Y, B2Y };
				if (!coeffs) coeffs = coeffs0;

				int b = 0, g = 0, r = (1 << (yuv_shift - 1));
				int dr = coeffs[blueIdx ^ 2], dg = coeffs[1], db = coeffs[blueIdx];

				for (int i = 0; i < 256; i++, b += db, g += dg, r += dr)
				{
					tab[i] = r;
					tab[i + 256] = g;
					tab[i + 512] = b;
				}
				calcRGBTab = 1;
			}
		}
		void operator()(const uchar* src, uchar* dst, int n) const
		{
			int scn = srccn;
			const int* _tab = tab;
			if (scn > 3) {
				for (int i = 0; i < n; i++, src += scn)
					dst[i] = (uchar)((_tab[src[0]] + _tab[src[1] + 256] + _tab[src[2] + 512]) >> yuv_shift);
			}
			else {
				for (int i = 0; i < n; i++, src += 3)
					dst[i] = (uchar)((_tab[src[0]] + _tab[src[1] + 256] + _tab[src[2] + 512]) >> yuv_shift);
			}
		}
		int srccn;
	//	int tab[256 * 3];
	};


	void ColorSpace::RGBtoGray(const Mat& colorImg, Mat& mgray, enBMPFormatType ct)
	{
		int cn = colorImg.channels();
		if (cn == 1)
		{
			mgray.create(colorImg, true);
			return;
		}
		if (mgray.size() != colorImg.size() || cn == 3 || cn == 4)
		{
			mgray.release();
			mgray.create(colorImg.size(), colorImg.type());
		}
		int nH = colorImg.rows(), nW = colorImg.cols();
		if (ct == CT_RGB || ct == CT_RGBA)
		{
			RGB2Gray<uchar> convert(colorImg.channels(), 2, 0);
			Size sz = getContinuousSize(colorImg, 1);
//	#pragma omp parallel for schedule(dynamic)
			for (int i = 0; i < sz.height; i++)
			{
				const uchar* pcolor = colorImg.data.ptr[i];
				uchar* pgray = mgray.data.ptr[i];
				convert(pcolor, pgray, sz.width);
			}
		}
		else if (ct == CT_BGR || ct == CT_ABGR)
		{
			for (int i = 0; i < nH; i++)
			{
				uchar* pcolor = colorImg.data.ptr[i];
				uchar* pgray = mgray.data.ptr[i];
				for (int j = cn - 3, k = 0; k < nW; j += cn, k++)
					pgray[k] = (uchar)((114 * pcolor[j] + 587 * pcolor[j + 1] + 299 * pcolor[j + 2]) / 1000);
			}
		}
	}
	typedef void(*SplitFunc)(const Mat& src, Mat& x, Mat& y, Mat& z);
	template<typename _Tp> static void splitFunc_(const Mat& src, Mat& x, Mat& y, Mat& z)
	{
		int i;
		int rows = src.rows();
		int cols = src.cols();
		int d = 0;
		int cn = src.channels();
		for (int k = 0; k < rows; k++)
		{
			d = 0;
			const _Tp* pcolor = (const _Tp*)src.data.ptr[k];
			_Tp* px = (_Tp*)x.data.ptr[k];
			_Tp* py = (_Tp*)y.data.ptr[k];
			_Tp* pz = (_Tp*)z.data.ptr[k];
			if (cn == 1)
			{
				for (i = 0; i < cols; i++, d++)
				{
					px[i] = pcolor[d];
				}
			}
			else if (cn == 2)
			{
				for (i = 0; i < cols; i++, d += 2)
				{
					px[i] = pcolor[d];
					py[i] = pcolor[d + 1];
				}
			}
			else if (cn == 3)
			{
				for (i = 0; i < cols; i++, d += 3)
				{
					px[i] = pcolor[d];
					py[i] = pcolor[d + 1];
					pz[i] = pcolor[d + 2];
				}
			}
			else if (cn == 4)
			{
				for (i = 0; i < cols; i++, d += 4)
				{
					px[i] = pcolor[d];
					py[i] = pcolor[d + 1];
					pz[i] = pcolor[d + 2];
				}
			}
		}
	}
	void ColorSpace::split(const Mat& color, Mat& x, Mat& y, Mat& z)
	{
		x.create(color.size(), color.type());
		y.create(color.size(), color.type());
		z.create(color.size(), color.type());
		SplitFunc splitFuncs[] = { splitFunc_<char>, splitFunc_<uchar>, splitFunc_<short>, splitFunc_<int>, splitFunc_<float>, splitFunc_<double> };
		return splitFuncs[color.type()](color, x, y, z);
	}
	void ColorSpace::split(const Mat& color, std::vector<Mat>& channels)
	{
		if (channels.size() == 0) {
			if (color.channels() == 1)
				channels.push_back(color);
			else {
				Mat t;
				channels.push_back(t);
				channels.push_back(t);
				channels.push_back(t);
				Mat& x = channels[0];
				Mat& y = channels[1];
				Mat& z = channels[2];
				split(color, x, y, z);
			}
		}
		else {
			if (channels.size() < color.channels())
				return;
			if (color.channels() == 1)
				channels[0] = color;
			else {
				Mat& x = channels[0];
				Mat& y = channels[1];
				Mat& z = channels[2];
				split(color, x, y, z);
			}
		}
	}

	void ColorSpace::toRGB(const Mat& color1, Mat& color2, enBMPFormatType ct)
	{
		color2.create(color1.rows(), color1.cols(), MAT_Tuchar3);
		int s = 0;
		const uchar* psrc = color1.data.ptr[0];
		uchar* pdst = color2.data.ptr[0];
		int datalen = color1.rows()*color1.cols() * 3;
		int i;
		switch (ct)
		{
		case CT_RGB:
			color2 = color1;
		case CT_RGBA:
			for (i = 0; i < datalen; i += 3, s += 4)
			{
				pdst[i] = psrc[s];
				pdst[i + 1] = psrc[s + 1];
				pdst[i + 2] = psrc[s + 2];
			}
			break;
		case CT_BGR:
			for (i = 0; i < datalen; i += 3, s += 3)
			{
				pdst[i] = psrc[s + 2];
				pdst[i + 1] = psrc[s + 1];
				pdst[i + 2] = psrc[s];
			}
			break;
		case CT_BGRA:
			for (i = 0; i < datalen; i += 3, s += 4)
			{
				pdst[i] = psrc[s + 2];
				pdst[i + 1] = psrc[s + 1];
				pdst[i + 2] = psrc[s];
			}
			break;
		case CT_ABGR:
			for (i = 0; i < datalen; i += 3, s += 4)
			{
				pdst[i] = psrc[s + 3];
				pdst[i + 1] = psrc[s + 2];
				pdst[i + 2] = psrc[s + 1];
			}
			break;
		case CT_ARGB:
			for (i = 0; i < datalen; i += 3, s += 4)
			{
				pdst[i] = psrc[s + 1];
				pdst[i + 1] = psrc[s + 2];
				pdst[i + 2] = psrc[s + 3];
			}
			break;
        default:
            assert(false);
		}
	}
	void ColorSpace::toRGBA(const Mat& color1, Mat& color2, enBMPFormatType ct)
	{
		int rows = color1.rows();
		int cols = color1.cols();
		if (rows == 0 || cols == 0)
			return;

		color2.create(rows, cols, MAT_Tuchar4);
		int s = 0;
		const uchar* psrc = color1.data.ptr[0];
		uchar* pdst = color2.data.ptr[0];
		int datalen = rows*cols * 4;
		int i;
		switch (ct)
		{
		case CT_DefaultMode:
		{
			int cn = color1.channels();
			if (cn == 1) {
				for (i = 0; i < datalen; i += 4, s++)
				{
					pdst[i] = psrc[s];
					pdst[i + 1] = psrc[s];
					pdst[i + 2] = psrc[s];
					pdst[i + 3] = 255;
				}
			}
			else if (cn == 3) {
				for (i = 0; i < datalen; i += 4, s += 3)
				{
					pdst[i] = psrc[s];
					pdst[i + 1] = psrc[s + 1];
					pdst[i + 2] = psrc[s + 2];
					pdst[i + 3] = 255;
				}
			}
			else {
				color2 = color1;
			}
		}
		break;
		case CT_GRAY:
			for (i = 0; i < datalen; i += 4, s++)
			{
				pdst[i] = psrc[s];
				pdst[i + 1] = psrc[s];
				pdst[i + 2] = psrc[s];
				pdst[i + 3] = 255;
			}
			break;
		case CT_RGBA:
			color2 = color1;
			break;
		case CT_RGB:
			for (i = 0; i < datalen; i += 4, s += 3)
			{
				pdst[i] = psrc[s];
				pdst[i + 1] = psrc[s + 1];
				pdst[i + 2] = psrc[s + 2];
				pdst[i + 3] = 255;
			}
			break;
		case CT_BGR:
			for (i = 0; i < datalen; i += 4, s += 3)
			{
				pdst[i] = psrc[s + 2];
				pdst[i + 1] = psrc[s + 1];
				pdst[i + 2] = psrc[s];
				pdst[i + 3] = 255;
			}
			break;
		case CT_ABGR:
			for (i = 0; i < datalen; i += 4, s += 4)
			{
				pdst[i] = psrc[s + 3];
				pdst[i + 1] = psrc[s + 2];
				pdst[i + 2] = psrc[s + 1];
				pdst[i + 3] = psrc[s];
			}
			break;
		case CT_ARGB:
			for (i = 0; i < datalen; i += 4, s += 4)
			{
				pdst[i] = psrc[s + 1];
				pdst[i + 1] = psrc[s + 2];
				pdst[i + 2] = psrc[s + 3];
				pdst[i + 3] = psrc[s];
			}
			break;
        default:
                assert(false);
		}
	}
	void ColorSpace::RGBtoBGRA(const Mat& color1, Mat& color2)
	{
		color2.create(color1.rows(), color1.cols(), MAT_Tuchar4);
		uchar* psrc = color1.data.ptr[0];
		uchar* pdst = color2.data.ptr[0];
		int datalen = color1.rows()*color1.cols() * 4;
		int scn = color1.channels();
		int s = 0;
		if (scn == 4)
		{
			for (int i = 0; i < datalen; i += 4, s += scn)
			{
				pdst[i] = psrc[s + 2];
				pdst[i + 1] = psrc[s + 1];
				pdst[i + 2] = psrc[s];
				pdst[i + 3] = psrc[s + 3];
			}
		}
		else
		{
			for (int i = 0; i < datalen; i += 4, s += scn)
			{
				pdst[i] = psrc[s + 2];
				pdst[i + 1] = psrc[s + 1];
				pdst[i + 2] = psrc[s];
				pdst[i + 3] = 255;
			}
		}
	}
	void ColorSpace::GraytoRGB(const Mat& gray, Mat& color, enBMPFormatType ct)
	{
		if (gray.channels() >= 3)
		{
			color = gray;
			return;
		}
		if (gray.size() != color.size())
		{
			color.release();
			if (ct == CT_RGB || ct == CT_BGR)
				color.create(gray.size(), (TYPE)CVLIB_MAKETYPE(gray.type(), 3));
			else
				color.create(gray.size(), (TYPE)CVLIB_MAKETYPE(gray.type(), 4));
		}
		int cn = color.channels();
		int nH = color.rows(), nW = color.cols();
		int elemsize = CVLIB_ELEM_SIZE(gray.type());
		if (ct == CT_RGB || ct == CT_BGR)
		{
			for (int i = 0; i < nH; i++)
			{
				int d = 0;
				int d1 = 0;
				uchar* pcolor = color.data.ptr[i];
				uchar* pgray = gray.data.ptr[i];
				for (int k = 0; k < nW; k++, d += cn, d1 += elemsize)
				{
					memcpy(&pcolor[d*elemsize], &pgray[d1], elemsize);
					memcpy(&pcolor[(d + 1)*elemsize], &pgray[d1], elemsize);
					memcpy(&pcolor[(d + 2)*elemsize], &pgray[d1], elemsize);
					if (cn == 4)
						pcolor[(d + 3)*elemsize] = pgray[d1];
				}
			}
		}
		else if (ct == CT_RGBA || ct == CT_ABGR)
		{
			for (int i = 0; i < nH; i++)
			{
				int d = 0;
				int d1 = 0;
				uchar* pcolor = color.data.ptr[i];
				uchar* pgray = gray.data.ptr[i];
				if (ct == CT_RGBA)
				{
					for (int k = 0; k < nW; k++, d += cn, d1 += elemsize)
					{
						memcpy(&pcolor[d*elemsize], &pgray[d1], elemsize);
						memcpy(&pcolor[(d + 1)*elemsize], &pgray[d1], elemsize);
						memcpy(&pcolor[(d + 2)*elemsize], &pgray[d1], elemsize);
						pcolor[(d + 3)*elemsize] = 255;
					}
				}
				else
				{
					for (int k = 0; k < nW; k++, d += cn, d1 += elemsize)
					{
						memcpy(&pcolor[(d + 1)*elemsize], &pgray[d1], elemsize);
						memcpy(&pcolor[(d + 2)*elemsize], &pgray[d1], elemsize);
						memcpy(&pcolor[(d + 3)*elemsize], &pgray[d1], elemsize);
						pcolor[(d)*elemsize] = pgray[d1] = 255;
					}
				}
			}
		}
	}

	void ColorSpace::RGBtoHSV(const uchar* pbR, float *prH_)
	{
		float *prH = &prH_[0];
		float *prS = &prH_[1];
		float *prV = &prH_[2];
		float rMax, rMin;
		float rR, rG, rB;
		float rcosta, rthita, rH;
		float pi = 3.142592f;

		rR = pbR[0] / 255.0F;
		rG = pbR[1] / 255.0F;
		rB = pbR[2] / 255.0F;
		rMax = MAX(rR, MAX(rG, rB));
		rMin = MIN(rR, MIN(rG, rB));
		if (rMax == 0)
		{	/*black*/
			*prH = *prS = *prV = 0;
		}
		else
		{
			if (rMax == rMin)
			{
				*prH = *prS = 0;
				*prV = rMax;
			}
			else if (rMin == 1)
			{/*white*/
				*prH = *prS = 0;
				*prV = 1;
			}
			else
			{
				rcosta = (rR - (rG + rB) / 2.0f) / (float)sqrt((rR - rG) * (rR - rG) + (rR - rB)*(rG - rB));
				rthita = (float)acos(rcosta);
				rH = (rG >= rB) ? rthita : 2.0f * pi - rthita;
				*prH = (float)(rH * 180.0 / pi);
				*prV = (float)((rR + rG + rB) / 3.0);
				*prS = (rMax != 0) ? (float)(1.0 - rMin / (*prV)) : 0.0f;
			}
		}
	}

	void ColorSpace::RGBtoHSV(const Mat& src, Mat& dst)
	{
		assert(src.channels() == 3);
		dst.create(src.size(), MAT_Tfloat3);
		uchar*	pbyte = src.data.ptr[0];
		float*	pfloat = dst.data.fl[0];
		int d = 0;
		for (int i = 0; i < src.size().area(); i++, d += 3)
			RGBtoHSV(&pbyte[d], &pfloat[d]);
	}

	void ColorSpace::RGBtoLab(const uchar* bR, float *prL)
	{
		float arX[3];
		RGBtoXYZ(bR, arX);
		XYZtoLab(arX, prL);
	}

	void ColorSpace::RGBtoLab(const Mat& src, Mat& dst)
	{
		assert(src.type1() == MAT_Tbyte3);

		dst.create(src.size(), MAT_Tfloat3);
		uchar* pbR = src.data.ptr[0];
		float* prL = dst.data.fl[0];

		for (int i = 0; i < src.rows()*src.cols() * 3; i += 3)
		{
			RGBtoLab(&pbR[i], &prL[i]);
		}
	}

	void ColorSpace::RGBtoXYZ(const uchar* pbR, float *prX_)
	{
		uchar bR = pbR[0];
		uchar bG = pbR[1];
		uchar bB = pbR[2];
		float *prX = &prX_[0];
		float *prY = &prX_[1];
		float *prZ = &prX_[2];
		// normalize red, green, blue values
		float rLinear = (float)bR / 255.0f;
		float gLinear = (float)bG / 255.0f;
		float bLinear = (float)bB / 255.0f;

		// convert to a sRGB form
		float r = (rLinear > 0.04045f) ? (float)pow((rLinear + 0.055) / (1.0 + 0.055), 2.2) : (rLinear / 12.92f);
		float g = (gLinear > 0.04045f) ? (float)pow((gLinear + 0.055) / (1.0 + 0.055), 2.2) : (gLinear / 12.92f);
		float b = (bLinear > 0.04045f) ? (float)pow((bLinear + 0.055) / (1.0 + 0.055), 2.2) : (bLinear / 12.92f);

		*prX = r*0.4124f + g*0.3576f + b*0.1805f;
		*prY = r*0.2126f + g*0.7152f + b*0.0722f;
		*prZ = r*0.0193f + g*0.1192f + b*0.9505f;
		// 	*prX = rLinear*0.4124f + gLinear*0.3576f + bLinear*0.1805f;
		// 	*prY = rLinear*0.2126f + gLinear*0.7152f + bLinear*0.0722f;
		// 	*prZ = rLinear*0.0193f + gLinear*0.1192f + bLinear*0.9505f;

		return;
	}

	void ColorSpace::RGBtoYUV(const uchar* pbR, float *prY_)
	{
		uchar bR = pbR[0], bG = pbR[1], bB = pbR[2];
		float *prY = prY_;
		float *prU = prY_ + 1;
		float *prV = prY_ + 2;
		//	Y = (int)( 0.257 * R + 0.504 * G + 0.098 * B);
		//	U = (int)( 0.439 * R - 0.368 * G - 0.071 * B + 128);
		//	V = (int)(-0.148 * R - 0.291 * G + 0.439 * B + 128);
		*prY = (float)(0.299f * bR + 0.587f * bG + 0.114f * bB);
		*prU = (float)((bB - *prY) * 0.565f + 128);
		*prV = (float)((bR - *prY) * 0.713f + 128);

		*prY = MIN(255, MAX(0, *prY));
		*prU = MIN(255, MAX(0, *prU));
		*prV = MIN(255, MAX(0, *prV));
		return;
	}

	void ColorSpace::RGBtoYUV(const Mat& src, Mat& dst)
	{
		assert(src.type1() == MAT_Tuchar3);
		dst.create(src.size(), MAT_Tfloat3);
		uchar* pbR = src.data.ptr[0];
		float* prL = dst.data.fl[0];

		for (int i = 0; i < src.rows() * src.cols() * 3; i += 3)
		{
			RGBtoYUV(&pbR[i], &prL[i]);
		}
	}

	void ColorSpace::XYZtoRGB(const float* px, uchar* pbR)
	{
		float x = px[0];
		float y = px[1];
		float z = px[2];
		float Clinear[3];
		Clinear[0] = x*3.2410f - y*1.5374f - z*0.4986f; // red
		Clinear[1] = -x*0.9692f + y*1.8760f + z*0.0416f; // green
		Clinear[2] = x*0.0556f - y*0.2040f + z*1.0570f; // blue

		for (int i = 0; i < 3; i++)
			Clinear[i] = (Clinear[i] <= 0.0031308) ? 12.92f*Clinear[i] : (1.0f + 0.055f)* (float)pow(Clinear[i], (1.0f / 2.4f)) - 0.055f;

		*pbR = (uchar)(Clinear[0] * 255.0 + 0.5);
		*(pbR + 1) = (uchar)(Clinear[1] * 255.0 + 0.5);
		*(pbR + 2) = (uchar)(Clinear[2] * 255.0 + 0.5);
	}

	/// <summary>
	/// XYZ to L*a*b* transformation function.
	/// </summary>
	/// <param name="t"></param>
	/// <returns></returns>
	float ColorSpace::Fxyz(float t)
	{
		return ((t > 0.008856) ? (float)pow(t, (1.0f / 3.0f)) : (7.787f*t + 16.0f / 116.0f));
	}

	/// <summary>
	/// Converts CIEXYZ to CIELab structure.
	/// </summary>
	void ColorSpace::XYZtoLab(const float* px, float* prL_)
	{
		float y = px[1];
		float z = px[2];
		// 	*prL = (float)(116.0f * pow(y/D65Y, 1.0/3.0) -16.0f);
		// 	if (*prL < 0) *prL = 0.0f;
		// 	*pra = 500.0f * (float)(pow(x/D65X, 1.0/3.0) - pow(y/D65Y, 1.0/3.0));
		// 	*prb = 200.0f * (float)(pow(y/D65Y, 1.0/3.0) - pow(z/D65Z, 1.0/3.0));

		*prL_ = 116.0f * Fxyz(y / D65Y) - 16.0f;
		*(prL_ + 1) = 500.0f * (Fxyz(px[0] / D65X) - Fxyz(y / D65Y));
		*(prL_ + 2) = 200.0f * (Fxyz(y / D65Y) - Fxyz(z / D65Z));
	}

	void ColorSpace::LabtoXYZ(const float* pl, float* x)
	{
		float theta = 6.0f / 29.0f;

		float fy = (*pl + 16) / 116.0f;
		float fx = fy + (pl[1] / 500.0f);
		float fz = fy - (pl[2] / 200.0f);

		*x = (fx > theta) ? D65X * (fx*fx*fx) : (fx - 16.0f / 116.0f) * 3 * (theta*theta)*D65X;
		*(x + 1) = (fy > theta) ? D65Y * (fy*fy*fy) : (fy - 16.0f / 116.0f) * 3 * (theta*theta)*D65Y;
		*(x + 2) = (fz > theta) ? D65Z * (fz*fz*fz) : (fz - 16.0f / 116.0f) * 3 * (theta*theta)*D65Z;
	}

	/// <summary>
	/// Converts CIELab to COLOR.
	/// </summary>
	void ColorSpace::LabtoRGB(const float* l, uchar* red)
	{
		float x[3];
		LabtoXYZ(l, x);
		XYZtoRGB(x, red);
	}

	void ColorSpace::RGBtoXYZ(const Mat& src, Mat& dst)
	{
		assert(src.type1() == MAT_Tbyte3);
		assert(dst.type1() == MAT_Tfloat3);
		uchar** ppbR = src.data.ptr;
		float** pprX = dst.data.fl;

		for (int iH = 0; iH < src.rows(); iH++)
			for (int iW = 0; iW < src.cols(); iW++)
			{
				RGBtoXYZ(&ppbR[iH][iW * 3], &pprX[iH][iW * 3]);
			}
	}

	void ColorSpace::XYZtoRGB(const Mat& src, Mat& dst)
	{
		assert(src.type1() == MAT_Tfloat3);
		assert(dst.type1() == MAT_Tbyte3);
		float* prX = src.data.fl[0];
		uchar* pbR = dst.data.ptr[0];
		for (int i = 0; i < src.rows()*src.cols() * 3; i += 3)
		{
			XYZtoRGB(&prX[i], &pbR[i]);
		}
	}

	void ColorSpace::XYZtoLab(const Mat& src, Mat& dst)
	{
		assert(src.type1() == MAT_Tfloat3);
		assert(dst.type1() == MAT_Tfloat3);
		float* prX = src.data.fl[0];
		float* pbR = dst.data.fl[0];

		for (int i = 0; i < src.rows() * src.cols() * 3; i += 3)
		{
			XYZtoLab(&prX[i], &pbR[i]);
		}
	}

	void ColorSpace::LabtoXYZ(const Mat& src, Mat& dst)
	{
		assert(src.type1() == MAT_Tfloat3);
		assert(dst.type1() == MAT_Tfloat3);
		const float* prX = src.data.fl[0];
		float* pbR = dst.data.fl[0];

		for (int i = 0; i < src.rows() * src.cols() * 3; i += 3)
		{
			LabtoXYZ(&prX[i], &pbR[i]);
		}
	}

	void ColorSpace::LabtoRGB(const Mat& src, Mat& dst)
	{
		assert(src.type1() == MAT_Tfloat3);
		dst.create(src.size(), MAT_Tuchar3);
		const float* prL = src.data.fl[0];
		uchar* pbR = dst.data.ptr[0];

		for (int i = 0; i < src.rows()*src.cols() * 3; i += 3)
		{
			LabtoRGB(&prL[i], &pbR[i]);
		}
	}

	void ColorSpace::RGBtoHSL(const uchar* pbR, float *prH)
	{
		uchar bR = pbR[0], bG = pbR[1], bB = pbR[2];
		float *prS = prH + 1;
		float *prL = prH + 2;
		float h = 0, s = 0, l = 0;

		// normalizes red-green-blue values
		float nRed = (float)bR / 255.0f;
		float nGreen = (float)bG / 255.0f;
		float nBlue = (float)bB / 255.0f;

		float MAX = MAX(nRed, MAX(nGreen, nBlue));
		float MIN = MIN(nRed, MIN(nGreen, nBlue));

		// hue
		if (MAX == MIN)
		{
			h = 0; // undefined
		}
		else if (MAX == nRed && nGreen >= nBlue)
		{
			h = 60.0f*(nGreen - nBlue) / (MAX - MIN);
		}
		else if (MAX == nRed && nGreen < nBlue)
		{
			h = 60.0f*(nGreen - nBlue) / (MAX - MIN) + 360.0f;
		}
		else if (MAX == nGreen)
		{
			h = 60.0f*(nBlue - nRed) / (MAX - MIN) + 120.0f;
		}
		else if (MAX == nBlue)
		{
			h = 60.0f*(nRed - nGreen) / (MAX - MIN) + 240.0f;
		}

		// luminance
		l = (MAX + MIN) / 2.0f;

		// saturation
		if (l == 0 || MAX == MIN)
		{
			s = 0;
		}
		else if (0 < l && l <= 0.5)
		{
			s = (MAX - MIN) / (MAX + MIN);
		}
		else if (l > 0.5)
		{
			s = (MAX - MIN) / (2 - (MAX + MIN)); //(MAX-MIN > 0)?
		}

		*prH = h;
		*prS = s;
		*prL = l;
	}

	void ColorSpace::RGBtoHSL(const Mat& src, Mat& dst)
	{
		assert(src.type1() == MAT_Tuchar3);
		dst.create(src.size(), MAT_Tfloat3);

		for (int iH = 0; iH < src.rows(); iH++) {
			const uchar* pbR = src.data.ptr[iH];
			float* prR = dst.data.fl[iH];
			for (int iW = 0; iW < src.cols(); iW++, pbR += 3, prR += 3)
				RGBtoHSL(pbR, prR);
		}
	}

	void ColorSpace::HSLtoRGB(const Mat& src, Mat& dst)
	{
		assert(src.type1() == MAT_Tfloat3);
		dst.create(src.size(), MAT_Tuchar3);

		for (int iH = 0; iH < src.rows(); iH++) {
			const float* prR = src.data.fl[iH];
			uchar* pbR = dst.data.ptr[iH];
			for (int iW = 0; iW < src.cols(); iW++, pbR += 3, prR += 3)
				HSLtoRGB(prR, pbR);
		}
	}

	void ColorSpace::HSLtoRGB(const float* ph, uchar* pbR)
	{
		float h = *ph;
		float s = *(ph + 1);
		float l = *(ph + 2);
		uchar* pbG = pbR + 1;
		uchar* pbB = pbR + 2;
		if (s == 0)
		{
			*pbR = (uchar)(l * 255.0f);
			*pbG = *pbR;
			*pbB = *pbR;
		}
		else
		{
			float q = (l < 0.5) ? (l * (1.0f + s)) : (l + s - (l*s));
			float p = (2.0f * l) - q;

			float Hk = h / 360.0f;
			float* _Tp = new float[3];
			_Tp[0] = Hk + (1.0f / 3.0f);	// Tr
			_Tp[1] = Hk;				// Tb
			_Tp[2] = Hk - (1.0f / 3.0f);	// Tg

			for (int i = 0; i < 3; i++)
			{
				if (_Tp[i] < 0) _Tp[i] += 1.0f;
				if (_Tp[i] > 1) _Tp[i] -= 1.0f;

				if ((_Tp[i] * 6) < 1)
				{
					_Tp[i] = p + ((q - p)*6.0f*_Tp[i]);
				}
				else if ((_Tp[i] * 2.0f) < 1) //(1.0/6.0)<=_Tp[i] && _Tp[i]<0.5
				{
					_Tp[i] = q;
				}
				else if ((_Tp[i] * 3.0f) < 2) // 0.5<=_Tp[i] && _Tp[i]<(2.0/3.0)
				{
					_Tp[i] = p + (q - p) * ((2.0f / 3.0f) - _Tp[i]) * 6.0f;
				}
				else _Tp[i] = p;
			}

			*pbR = (uchar)(_Tp[0] * 255.0f);
			*pbG = (uchar)(_Tp[1] * 255.0f);
			*pbB = (uchar)(_Tp[2] * 255.0f);

			delete[]_Tp;
		}
	}

	void ColorSpace::YUVtoRGB(const float* py, uchar* pbR)
	{
		float y = *py, u = *(py + 1), v = *(py + 2);
		uchar* pbG = pbR + 1;
		uchar* pbB = pbR + 2;
		int U, V, R, G, B;
		float Y = y;
		U = (int)u - 128, V = (int)v - 128;

		//	R = (int)(1.164 * Y + 2.018 * U);
		//	G = (int)(1.164 * Y - 0.813 * V - 0.391 * U);
		//	B = (int)(1.164 * Y + 1.596 * V);
		R = (int)(Y + 1.403f * V);
		G = (int)(Y - 0.344f * U - 0.714f * V);
		B = (int)(Y + 1.770f * U);

		*pbR = (uchar)MIN(255, MAX(0, R));
		*pbG = (uchar)MIN(255, MAX(0, G));
		*pbB = (uchar)MIN(255, MAX(0, B));
		return;
	}

	void ColorSpace::YUVtoRGB(const Mat& src, Mat& dst)
	{
		assert(src.type1() == MAT_Tfloat3);
		dst.create(src.size(), MAT_Tuchar3);
		float* prY = src.data.fl[0];
		uchar* pbR = dst.data.ptr[0];
		for (int i = 0; i < src.rows() * src.cols() * 3; i += 3)
		{
			YUVtoRGB(&prY[i], &pbR[i]);
		}
	}

	float ColorSpace::GetColorDistance(float r1, float g1, float b1, float r2, float g2, float b2)
	{
		float a = r2 - r1;
		float b = g2 - g1;
		float c = b2 - b1;

		return (float)sqrt(a*a + b*b + c*c);
	}

	void ColorSpace::extractChannel(const Mat& src, Mat& dst, int ch)
	{
		dst = Mat::zeros(src.size(), src.type());
		if (src.channels() <= ch)
			return;
		Size sz = src.size();
		TYPE type = src.type();
		int cn = src.channels();
		int ix, iy, icn;
		switch (type)
		{
		case MAT_Tbyte:
			for (iy = 0; iy < sz.height; iy++)
			{
				const uchar* psrc = src.data.ptr[iy];
				uchar* pdst = dst.data.ptr[iy];
				for (ix = 0, icn = 0; ix < sz.width; ix++, icn += cn)
					pdst[ix] = psrc[icn + ch];
			}
			break;
		case MAT_Tshort:
			for (iy = 0; iy < sz.height; iy++)
			{
				const short* psrc = src.data.s[iy];
				short* pdst = dst.data.s[iy];
				for (ix = 0, icn = 0; ix < sz.width; ix++, icn += cn)
					pdst[ix] = psrc[icn + ch];
			}
			break;
		case MAT_Tint:
			for (iy = 0; iy < sz.height; iy++)
			{
				const int* psrc = src.data.i[iy];
				int* pdst = dst.data.i[iy];
				for (ix = 0, icn = 0; ix < sz.width; ix++, icn += cn)
					pdst[ix] = psrc[icn + ch];
			}
			break;
		case MAT_Tfloat:
			for (iy = 0; iy < sz.height; iy++)
			{
				const float* psrc = src.data.fl[iy];
				float* pdst = dst.data.fl[iy];
				for (ix = 0, icn = 0; ix < sz.width; ix++, icn += cn)
					pdst[ix] = psrc[icn + ch];
			}
			break;
		case MAT_Tdouble:
			for (iy = 0; iy < sz.height; iy++)
			{
				const double* psrc = src.data.db[iy];
				double* pdst = dst.data.db[iy];
				for (ix = 0, icn = 0; ix < sz.width; ix++, icn += cn)
					pdst[ix] = psrc[icn + ch];
			}
			break;
        default:
            assert(false);
		}
	}
	void ColorSpace::merge(std::vector<Mat>& srcs, Mat& dst)
	{
		Mat** p = new Mat*[srcs.size()];
		for (size_t i = 0; i < srcs.size(); i++) {
			p[i] = &srcs[i];
		}
		merge(p, (int)srcs.size(), dst);
		delete[]p;
	}
	void ColorSpace::merge(Mat** const srcs, int n, Mat& dst)
	{
		if (srcs == 0 || n == 0)
			return;
		int rows = srcs[0]->rows();
		int cols = srcs[0]->cols();
		int cn = MIN(n, 4);
		TYPE src_type = srcs[0]->type();
		TYPE dst_type = (TYPE)CVLIB_MAKETYPE(src_type, cn);
		dst.create(rows, cols, dst_type);
		for (int i = 0; i < rows; i++)
		{
			if (src_type == MAT_Tuchar || src_type == MAT_Tchar)
			{
				uchar* pdst = dst.data.ptr[i];
				for (int j = 0, jcn = 0; j < cols; j++, jcn += cn)
				{
					for (int icn = 0; icn < cn; icn++)
					{
						uchar* psrc = srcs[icn]->data.ptr[i];
						pdst[jcn + icn] = psrc[j];
					}
				}
			}
			else if (src_type == MAT_Tint)
			{
				int* pdst = dst.data.i[i];
				for (int j = 0, jcn = 0; j < cols; j++, jcn += cn)
				{
					for (int icn = 0; icn < cn; icn++)
					{
						int* psrc = srcs[icn]->data.i[i];
						pdst[jcn + icn] = psrc[j];
					}
				}
			}
			else if (src_type == MAT_Tfloat)
			{
				float* pdst = dst.data.fl[i];
				for (int j = 0, jcn = 0; j < cols; j++, jcn += cn)
				{
					for (int icn = 0; icn < cn; icn++)
					{
						float* psrc = srcs[icn]->data.fl[i];
						pdst[jcn + icn] = psrc[j];
					}
				}
			}
			else if (src_type == MAT_Tshort)
			{
				short* pdst = dst.data.s[i];
				for (int j = 0, jcn = 0; j < cols; j++, jcn += cn)
				{
					for (int icn = 0; icn < cn; icn++)
					{
						short* psrc = srcs[icn]->data.s[i];
						pdst[jcn + icn] = psrc[j];
					}
				}
			}
			else if (src_type == MAT_Tdouble)
			{
				double* pdst = dst.data.db[i];
				for (int j = 0, jcn = 0; j < cols; j++, jcn += cn)
				{
					for (int icn = 0; icn < cn; icn++)
					{
						double* psrc = srcs[icn]->data.db[i];
						pdst[jcn + icn] = psrc[j];
					}
				}
			}
		}
	}

	void ColorSpace::merge(const Mat& src, const Mat& malpha, Mat& dst)
	{
		if (src.size() != malpha.size())
			return;
		dst.create(src.size(), MAT_Tuchar4);

		const uchar* psrc = src.data.ptr[0];
		uchar* pdst = dst.data.ptr[0];
		uchar* palpha = malpha.data.ptr[0];
		int datalen = src.size().area() * 4;
		if (src.channels() == 3)
		{
			int s = 0;
			int i, k = 0;
			for (i = 0; i < datalen; i += 4, s += 3, k++)
			{
				pdst[i] = psrc[s];
				pdst[i + 1] = psrc[s + 1];
				pdst[i + 2] = psrc[s + 2];
				pdst[i + 3] = palpha[k];
			}
		}
		else if (src.channels() == 1)
		{
			int s = 0;
			int i, k = 0;
			for (i = 0; i < datalen; i += 4, s++, k++)
			{
				pdst[i] = psrc[s];
				pdst[i + 1] = psrc[s];
				pdst[i + 2] = psrc[s];
				pdst[i + 3] = palpha[k];
			}
		}
	}
}