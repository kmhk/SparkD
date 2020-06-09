#pragma once

#include <vector>
#include <cmath>
#include "_cvlibbase.h"

using namespace std;

namespace cvlib
{

	// ----------------------------------------------------------------------------------------

	// ----------------------------------------------------------------------------------------

	class strip_transform
	{
	public:

		strip_transform()
		{
			sin_angle = 0;
			cos_angle = 1;
			translate.x = 0;
			translate.y = 0;
		}

		strip_transform(const double& angle, const Point2d& translate_)
		{
			sin_angle = std::sin(angle);
			cos_angle = std::cos(angle);
			translate = translate_;
		}

		template <typename T>
		const Point2_<T> operator() (const Point2_<T>& p) const
		{
			double x = cos_angle*p.x - sin_angle*p.y;
			double y = sin_angle*p.x + cos_angle*p.y;

			return Point2_<T>(x, y) + translate;
		}

		const Matd get_m() const
		{
			Matd temp;
			temp[0][0] = cos_angle;
			temp[0][1] = -sin_angle;
			temp[1][0] = sin_angle;
			temp[1][1] = cos_angle;
			return temp;
		}

		const Point2d get_b() const { return translate; }

		inline friend void toFile(const strip_transform& item, XFile* pfile)
		{
			pfile->write(&item.sin_angle, sizeof(item.sin_angle), 1);
			pfile->write(&item.cos_angle, sizeof(item.cos_angle), 1);
			pfile->write(&item.translate, sizeof(item.translate), 1);
		}

		inline friend void fromFile(strip_transform& item, XFile* pfile)
		{
			pfile->read(&item.sin_angle, sizeof(item.sin_angle), 1);
			pfile->read(&item.cos_angle, sizeof(item.cos_angle), 1);
			pfile->read(&item.translate, sizeof(item.translate), 1);
		}

	private:
		double sin_angle;
		double cos_angle;
		Point2d translate;
	};

	// ----------------------------------------------------------------------------------------

	class strip_transform_affine
	{
	public:

		strip_transform_affine()
		{
			m = 1.f, b = 0.f;
		}

		strip_transform_affine(const float& m_, const float& b_) :m(m_), b(b_)
		{}

		const float operator() (const float& p) const
		{
			return m*p + b;
		}

		const float& get_m() const { return m; }

		const float& get_b() const { return b; }

		inline friend void toFile(const strip_transform_affine& item, XFile* pfile)
		{
			pfile->write(&item.m, sizeof(item.m), 1);
			pfile->write(&item.b, sizeof(item.b), 1);
		}

		inline friend void fromFile(strip_transform_affine& item, XFile* pfile)
		{
			pfile->read(&item.m, sizeof(item.m), 1);
			pfile->read(&item.b, sizeof(item.b), 1);
		}

	private:
		float m;
		float b;
	};

	// ----------------------------------------------------------------------------------------

	inline strip_transform_affine operator* (
		const strip_transform_affine& lhs,
		const strip_transform_affine& rhs
		)
	{
		return strip_transform_affine(lhs.get_m()*rhs.get_m(), lhs.get_m()*rhs.get_b() + lhs.get_b());
	}

	// ----------------------------------------------------------------------------------------

	inline strip_transform_affine inv(
		const strip_transform_affine& trans
	)
	{
		float im = 1.f / trans.get_m();
		return strip_transform_affine(im, -im*trans.get_b());
	}

	// ----------------------------------------------------------------------------------------

	template <typename T>
	strip_transform_affine find_strip_affine_transform(
		const std::vector<T>& from_points,
		const std::vector<T>& to_points
	)
	{
		// make sure requires clause is not broken
		assert(from_points.size() == to_points.size() && from_points.size() >= 2);

		float f1 = from_points[0], f2 = from_points[1];
		float t1 = to_points[0], t2 = to_points[1];
		float m, b;
		m = (t2 - t1) / (f2 - f1);
		b = t1 - m*f1;

		return strip_transform_affine(m, b);
	}

	// ----------------------------------------------------------------------------------------

	template <typename T>
	strip_transform_affine find_similarity_transform(
		const std::vector<T>& from_points,
		const std::vector<T>& to_points
	)
	{
		// make sure requires clause is not broken
		assert(from_points.size() == to_points.size() && from_points.size() >= 2);
		// We use the formulas from the paper: Least-squares estimation of transformation
		// parameters between two point patterns by Umeyama.  They are equations 34 through
		// 43.

		T mean_from = 0, mean_to = 0;
		double sigma_from = 0, sigma_to = 0;
		double cov = 0;
		cov = 0;

		for (unsigned long i = 0; i < from_points.size(); ++i)
		{
			mean_from += from_points[i];
			mean_to += to_points[i];
		}
		int count = (int)from_points.size();
		mean_from *= (T)(1.0 / count);
		mean_to *= (T)(1.0 / count);

		for (int i = 0; i < count; ++i)
		{
			T t1 = from_points[i] - mean_from;
			sigma_from += t1 * t1;
			T t2 = to_points[i] - mean_to;
			sigma_to += t2 * t2;
			cov += t2*t1;
		}

		sigma_from = sqrt(sigma_from / count);
		sigma_to = sqrt(sigma_to / count);
		cov /= count;

		double m = sigma_to / sigma_from;
		double b = mean_to - m*mean_from;

		return strip_transform_affine((float)m, (float)b);
	}

	// ----------------------------------------------------------------------------------------
}
