
/*! 
 * \file EigenvalueDecomposition.cpp
 * \ingroup math
 * \brief 
 * \author
 */

#include "EigenvalueDecomposition.h"

namespace cvlib
{
	
static	double Hypot (double a, double b)
{
	double r;
	if (fabs(a) > fabs(b)) 
	{
		r = b/a;
		r = fabs(a) * sqrt(1 + r * r);
	} 
	else if (b != 0) 
	{
		r = a/b;
		r = fabs(b) * sqrt(1 + r * r);
	} 
	else 
	{
		r = 0.0;
	}
	return r;
}

void  EigenvalueDecomposition::tred2()
{
	//  This is derived from the Algol procedures tred2 by
	//  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
	//  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
	//  Fortran subroutine in EISPACK.
	int i, j, k;
	for (j = 0; j < n; j++)
	{
		d[j] = V[n - 1][j];
	}
	
	// Householder reduction to tridiagonal form.
	
	for (i = n - 1; i > 0; i--)
	{
		// scale to avoid under/overflow.
		
		double scale = 0.0;
		double h = 0.0;
		for (k = 0; k < i; k++)
		{
			scale = scale + fabs(d[k]);
		}
		if (scale == 0.0)
		{
			e[i] = d[i - 1];
			for (j = 0; j < i; j++)
			{
				d[j] = V[i - 1][j];
				V[i][j] = 0.0;
				V[j][i] = 0.0;
			}
		}
		else
		{
			// Generate Householder vector.
			
			for (k = 0; k < i; k++)
			{
				d[k] /= scale;
				h += d[k] * d[k];
			}
			double f = d[i - 1];
			double g = sqrt(h);
			if (f > 0)
			{
				g = - g;
			}
			e[i] = scale * g;
			h = h - f * g;
			d[i - 1] = f - g;
			for (j = 0; j < i; j++)
			{
				e[j] = 0.0;
			}
			
			// Apply similarity transformation to remaining columns.
			
			for (j = 0; j < i; j++)
			{
				f = d[j];
				V[j][i] = f;
				g = e[j] + V[j][j] * f;
				for (int k2 = j + 1; k2 <= i - 1; k2++)
				{
					g += V[k2][j] * d[k2];
					e[k2] += V[k2][j] * f;
				}
				e[j] = g;
			}
			f = 0.0;
			for (j = 0; j < i; j++)
			{
				e[j] /= h;
				f += e[j] * d[j];
			}
			double hh = f / (h + h);
			for (j = 0; j < i; j++)
			{
				e[j] -= hh * d[j];
			}
			for (j = 0; j < i; j++)
			{
				f = d[j];
				g = e[j];
				for (int k2 = j; k2 <= i - 1; k2++)
				{
					V[k2][j] -= (f * e[k2] + g * d[k2]);
				}
				d[j] = V[i - 1][j];
				V[i][j] = 0.0;
			}
		}
		d[i] = h;
	}
	
	// Accumulate transformations.
	
	for (i = 0; i < n - 1; i++)
	{
		V[n - 1][i] = V[i][i];
		V[i][i] = 1.0;
		double h = d[i + 1];
		if (h != 0.0)
		{
			for (k = 0; k <= i; k++)
			{
				d[k] = V[k][i + 1] / h;
			}
			for (int j2 = 0; j2 <= i; j2++)
			{
				double g = 0.0;
				for (k = 0; k <= i; k++)
				{
					g += V[k][i + 1] * V[k][j2];
				}
				for (k = 0; k <= i; k++)
				{
					V[k][j2] -= g * d[k];
				}
			}
		}
		for (k = 0; k <= i; k++)
		{
			V[k][i + 1] = 0.0;
		}
	}
	for (j = 0; j < n; j++)
	{
		d[j] = V[n - 1][j];
		V[n - 1][j] = 0.0;
	}
	V[n - 1][n - 1] = 1.0;
	e[0] = 0.0;
}
		
void  EigenvalueDecomposition::tql2()
{
	//  This is derived from the Algol procedures tql2, by
	//  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
	//  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
	//  Fortran subroutine in EISPACK.
	
	int i;
	for (i = 1; i < n; i++)
	{
		e[i - 1] = e[i];
	}
	e[n - 1] = 0.0;
	
	double f = 0.0;
	double tst1 = 0.0;
	double eps = pow(2.0, - 52.0);
	for (int l = 0; l < n; l++)
	{
		// find small subdiagonal element
		
		tst1 = MAX(tst1, fabs(d[l]) + fabs(e[l]));
		int m = l;
		while (m < n)
		{
			if (fabs(e[m]) <= eps * tst1)
			{
				break;
			}
			m++;
		}
		
		// If m == l, d[l] is an eigenvalue,
		// otherwise, iterate.
		
		if (m > l)
		{
			int iter = 0;
			do 
			{
				iter = iter + 1; // (Could check iteration count here.)
				
				// Compute implicit shift
				
				double g = d[l];
				double p = (d[l + 1] - g) / (2.0 * e[l]);
				double r = Hypot(p, 1.0);
				if (p < 0)
				{
					r = - r;
				}
				d[l] = e[l] / (p + r);
				d[l + 1] = e[l] * (p + r);
				double dl1 = d[l + 1];
				double h = g - d[l];
				for (i = l + 2; i < n; i++)
				{
					d[i] -= h;
				}
				f = f + h;
				
				// Implicit QL transformation.
				
				p = d[m];
				double c = 1.0;
				double c2 = c;
				double c3 = c;
				double el1 = e[l + 1];
				double s = 0.0;
				double s2 = 0.0;
				for (i = m - 1; i >= l; i--)
				{
					c3 = c2;
					c2 = c;
					s2 = s;
					g = c * e[i];
					h = c * p;
					r = Hypot(p, e[i]);
					e[i + 1] = s * r;
					s = e[i] / r;
					c = p / r;
					p = c * d[i] - s * g;
					d[i + 1] = h + s * (c * g + s * d[i]);
					
					// Accumulate transformation.
					
					for (int k = 0; k < n; k++)
					{
						h = V[k][i + 1];
						V[k][i + 1] = s * V[k][i] + c * h;
						V[k][i] = c * V[k][i] - s * h;
					}
				}
				p = (- s) * s2 * c3 * el1 * e[l] / dl1;
				e[l] = s * p;
				d[l] = c * p;
				
				// Check for convergence.
			}
			while (fabs(e[l]) > eps * tst1);
		}
		d[l] = d[l] + f;
		e[l] = 0.0;
	}
	
	// sort eigenvalues and corresponding vectors.
	
	for (i = 0; i < n - 1; i++)
	{
		int k = i;
		double p = d[i];
		for (int j = i + 1; j < n; j++)
		{
			if (d[j] > p)
			{
				k = j;
				p = d[j];
			}
		}
		if (k != i)
		{
			d[k] = d[i];
			d[i] = p;
			for (int j = 0; j < n; j++)
			{
				p = V[j][i];
				V[j][i] = V[j][k];
				V[j][k] = p;
			}
		}
	}
}

void  EigenvalueDecomposition::orthes()
{
	//  This is derived from the Algol procedures orthes and ortran,
	//  by Martin and Wilkinson, Handbook for Auto. Comp.,
	//  Vol.ii-Linear Algebra, and the corresponding
	//  Fortran subroutines in EISPACK.
	
	int low = 0;
	int high = n - 1;
	int m, i, j;
	for (m = low + 1; m <= high - 1; m++)
	{
		
		// scale column.
		
		double scale = 0.0;
		for (i = m; i <= high; i++)
		{
			scale = scale + fabs(H[i][m - 1]);
		}
		if (scale != 0.0)
		{
			
			// Compute Householder transformation.
			double h = 0.0;
			for (i = high; i >= m; i--)
			{
				ort[i] = H[i][m - 1] / scale;
				h += ort[i] * ort[i];
			}
			double g = sqrt(h);
			if (ort[m] > 0)
			{
				g = - g;
			}
			h = h - ort[m] * g;
			ort[m] = ort[m] - g;
			
			// Apply Householder similarity transformation
			// H = (I-u*u'/h)*H*(I-u*u')/h)
			
			for (j = m; j < n; j++)
			{
				double f = 0.0;
				for (i = high; i >= m; i--)
				{
					f += ort[i] * H[i][j];
				}
				f = f / h;
				for (i = m; i <= high; i++)
				{
					H[i][j] -= f * ort[i];
				}
			}
			
			for (i = 0; i <= high; i++)
			{
				double f = 0.0;
				for (j = high; j >= m; j--)
				{
					f += ort[j] * H[i][j];
				}
				f = f / h;
				for (j = m; j <= high; j++)
				{
					H[i][j] -= f * ort[j];
				}
			}
			ort[m] = scale * ort[m];
			H[m][m - 1] = scale * g;
		}
	}
	
	// Accumulate transformations (Algol's ortran).
	
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < n; j++)
		{
			V[i][j] = (i == j?1.0:0.0);
		}
	}
	
	for (m = high - 1; m >= low + 1; m--)
	{
		if (H[m][m - 1] != 0.0)
		{
			for (i = m + 1; i <= high; i++)
			{
				ort[i] = H[i][m - 1];
			}
			for (j = m; j <= high; j++)
			{
				double g = 0.0;
				for (i = m; i <= high; i++)
				{
					g += ort[i] * V[i][j];
				}
				// Double division avoids possible underflow
				g = (g / ort[m]) / H[m][m - 1];
				for (i = m; i <= high; i++)
				{
					V[i][j] += g * ort[i];
				}
			}
		}
	}
}
		
void  EigenvalueDecomposition::cdiv(double xr, double xi, double yr, double yi)
{
	double r, d_;
	if (fabs(yr) > fabs(yi))
	{
		r = yi / yr;
		d_ = yr + r * yi;
		cdivr = (xr + r * xi) / d_;
		cdivi = (xi - r * xr) / d_;
	}
	else
	{
		r = yr / yi;
		d_ = yi + r * yr;
		cdivr = (r * xr + xi) / d_;
		cdivi = (r * xi - xr) / d_;
	}
}

void  EigenvalueDecomposition::hqr2()
{
	//  This is derived from the Algol procedure hqr2,
	//  by Martin and Wilkinson, Handbook for Auto. Comp.,
	//  Vol.ii-Linear Algebra, and the corresponding
	//  Fortran subroutine in EISPACK.
	
	// Initialize
	
	int nn = this->n;
	int n_ = nn - 1;
	int low = 0;
	int high = nn - 1;
	double eps = pow(2.0, - 52.0);
	double exshift = 0.0;
	double p = 0, q = 0, r = 0, s = 0, z = 0, t, w, x, y;
	
	// Store roots isolated by balanc and compute matrix norm
	int i, j, k;
	double norm = 0.0;
	for (i = 0; i < nn; i++)
	{
		if (i < low || i > high)
		{
			d[i] = H[i][i];
			e[i] = 0.0;
		}
		for (j = MAX(i - 1, 0); j < nn; j++)
		{
			norm = norm + fabs(H[i][j]);
		}
	}
	
	// Outer loop over eigenvalue index
	
	int iter = 0;
	while (n_ >= low)
	{
		
		// Look for single small sub-diagonal element
		
		int l = n_;
		while (l > low)
		{
			s = fabs(H[l - 1][l - 1]) + fabs(H[l][l]);
			if (s == 0.0)
			{
				s = norm;
			}
			if (fabs(H[l][l - 1]) < eps * s)
			{
				break;
			}
			l--;
		}
		
		// Check for convergence
		// One root found
		
		if (l == n_)
		{
			H[n_][n_] = H[n_][n_] + exshift;
			d[n_] = H[n_][n_];
			e[n_] = 0.0;
			n_--;
			iter = 0;
			
			// Two roots found
		}
		else if (l == n_ - 1)
		{
			w = H[n_][n_ - 1] * H[n_ - 1][n_];
			p = (H[n_ - 1][n_ - 1] - H[n_][n_]) / 2.0;
			q = p * p + w;
			z = sqrt(fabs(q));
			H[n_][n_] = H[n_][n_] + exshift;
			H[n_ - 1][n_ - 1] = H[n_ - 1][n_ - 1] + exshift;
			x = H[n_][n_];
			
			// Real pair
			
			if (q >= 0)
			{
				if (p >= 0)
				{
					z = p + z;
				}
				else
				{
					z = p - z;
				}
				d[n_ - 1] = x + z;
				d[n_] = d[n_ - 1];
				if (z != 0.0)
				{
					d[n_] = x - w / z;
				}
				e[n_ - 1] = 0.0;
				e[n_] = 0.0;
				x = H[n_][n_ - 1];
				s = fabs(x) + fabs(z);
				p = x / s;
				q = z / s;
				r = sqrt(p * p + q * q);
				p = p / r;
				q = q / r;
				
				// Row modification
				
				for (int j2 = n_ - 1; j2 < nn; j2++)
				{
					z = H[n_ - 1][j2];
					H[n_ - 1][j2] = q * z + p * H[n_][j2];
					H[n_][j2] = q * H[n_][j2] - p * z;
				}
				
				// Column modification
				
				for (i = 0; i <= n_; i++)
				{
					z = H[i][n_ - 1];
					H[i][n_ - 1] = q * z + p * H[i][n_];
					H[i][n_] = q * H[i][n_] - p * z;
				}
				
				// Accumulate transformations
				
				for (i = low; i <= high; i++)
				{
					z = V[i][n_ - 1];
					V[i][n_ - 1] = q * z + p * V[i][n_];
					V[i][n_] = q * V[i][n_] - p * z;
				}
				
				// Complex pair
			}
			else
			{
				d[n_ - 1] = x + p;
				d[n_] = x + p;
				e[n_ - 1] = z;
				e[n_] = - z;
			}
			n_ = n_ - 2;
			iter = 0;
			
			// No convergence yet
		}
		else
		{
			
			// Form shift
			
			x = H[n_][n_];
			y = 0.0;
			w = 0.0;
			if (l < n_)
			{
				y = H[n_ - 1][n_ - 1];
				w = H[n_][n_ - 1] * H[n_ - 1][n_];
			}
			
			// Wilkinson's original ad hoc shift
			
			if (iter == 10)
			{
				exshift += x;
				for (i = low; i <= n_; i++)
				{
					H[i][i] -= x;
				}
				s = fabs(H[n_][n_ - 1]) + fabs(H[n_ - 1][n_ - 2]);
				x = y = 0.75 * s;
				w = (- 0.4375) * s * s;
			}
			
			// MATLAB's new ad hoc shift
			
			if (iter == 30)
			{
				s = (y - x) / 2.0;
				s = s * s + w;
				if (s > 0)
				{
					s = sqrt(s);
					if (y < x)
					{
						s = - s;
					}
					s = x - w / ((y - x) / 2.0 + s);
					for (i = low; i <= n_; i++)
					{
						H[i][i] -= s;
					}
					exshift += s;
					x = y = w = 0.964;
				}
			}
			
			iter = iter + 1; // (Could check iteration count here.)
			
			// Look for two consecutive small sub-diagonal elements
			
			int m = n_ - 2;
			while (m >= l)
			{
				z = H[m][m];
				r = x - z;
				s = y - z;
				p = (r * s - w) / H[m + 1][m] + H[m][m + 1];
				q = H[m + 1][m + 1] - z - r - s;
				r = H[m + 2][m + 1];
				s = fabs(p) + fabs(q) + fabs(r);
				p = p / s;
				q = q / s;
				r = r / s;
				if (m == l)
				{
					break;
				}
				if (fabs(H[m][m - 1]) * (fabs(q) + fabs(r)) < eps * (fabs(p) * (fabs(H[m - 1][m - 1]) + fabs(z) + fabs(H[m + 1][m + 1]))))
				{
					break;
				}
				m--;
			}
			
			for (i = m + 2; i <= n_; i++)
			{
				H[i][i - 2] = 0.0;
				if (i > m + 2)
				{
					H[i][i - 3] = 0.0;
				}
			}
			
			// Double QR step involving rows l:n and columns m:n
			
			for (k = m; k <= n_ - 1; k++)
			{
				bool notlast = (k != n_ - 1);
				if (k != m)
				{
					p = H[k][k - 1];
					q = H[k + 1][k - 1];
					r = (notlast?H[k + 2][k - 1]:0.0);
					x = fabs(p) + fabs(q) + fabs(r);
					if (x != 0.0)
					{
						p = p / x;
						q = q / x;
						r = r / x;
					}
				}
				if (x == 0.0)
				{
					break;
				}
				s = sqrt(p * p + q * q + r * r);
				if (p < 0)
				{
					s = - s;
				}
				if (s != 0)
				{
					if (k != m)
					{
						H[k][k - 1] = (- s) * x;
					}
					else if (l != m)
					{
						H[k][k - 1] = - H[k][k - 1];
					}
					p = p + s;
					x = p / s;
					y = q / s;
					z = r / s;
					q = q / p;
					r = r / p;
					
					// Row modification
					
					for (j = k; j < nn; j++)
					{
						p = H[k][j] + q * H[k + 1][j];
						if (notlast)
						{
							p = p + r * H[k + 2][j];
							H[k + 2][j] = H[k + 2][j] - p * z;
						}
						H[k][j] = H[k][j] - p * x;
						H[k + 1][j] = H[k + 1][j] - p * y;
					}
					
					// Column modification
					
					for (i = 0; i <= MIN(n_, k + 3); i++)
					{
						p = x * H[i][k] + y * H[i][k + 1];
						if (notlast)
						{
							p = p + z * H[i][k + 2];
							H[i][k + 2] = H[i][k + 2] - p * r;
						}
						H[i][k] = H[i][k] - p;
						H[i][k + 1] = H[i][k + 1] - p * q;
					}
					
					// Accumulate transformations
					
					for (i = low; i <= high; i++)
					{
						p = x * V[i][k] + y * V[i][k + 1];
						if (notlast)
						{
							p = p + z * V[i][k + 2];
							V[i][k + 2] = V[i][k + 2] - p * r;
						}
						V[i][k] = V[i][k] - p;
						V[i][k + 1] = V[i][k + 1] - p * q;
					}
				} // (s != 0)
			} // k loop
		} // check convergence
	} // while (n >= low)
	
	// Backsubstitute to find vectors of upper triangular form
	
	if (norm == 0.0)
	{
		return ;
	}
	
	for (n_ = nn - 1; n_ >= 0; n_--)
	{
		p = d[n_];
		q = e[n_];
		
		// Real vector
		
		if (q == 0)
		{
			int l = n_;
			H[n_][n_] = 1.0;
			for (i = n_ - 1; i >= 0; i--)
			{
				w = H[i][i] - p;
				r = 0.0;
				for (j = l; j <= n_; j++)
				{
					r = r + H[i][j] * H[j][n_];
				}
				if (e[i] < 0.0)
				{
					z = w;
					s = r;
				}
				else
				{
					l = i;
					if (e[i] == 0.0)
					{
						if (w != 0.0)
						{
							H[i][n_] = (- r) / w;
						}
						else
						{
							H[i][n_] = (- r) / (eps * norm);
						}
						
						// Solve real equations
					}
					else
					{
						x = H[i][i + 1];
						y = H[i + 1][i];
						q = (d[i] - p) * (d[i] - p) + e[i] * e[i];
						t = (x * s - z * r) / q;
						H[i][n_] = t;
						if (fabs(x) > fabs(z))
						{
							H[i + 1][n_] = (- r - w * t) / x;
						}
						else
						{
							H[i + 1][n_] = (- s - y * t) / z;
						}
					}
					
					// Overflow control
					
					t = fabs(H[i][n_]);
					if ((eps * t) * t > 1)
					{
						for (j = i; j <= n_; j++)
						{
							H[j][n_] = H[j][n_] / t;
						}
					}
				}
			}
			
			// Complex vector
		}
		else if (q < 0)
		{
			int l = n_ - 1;
			
			// Last vector component imaginary so matrix is triangular
			
			if (fabs(H[n_][n_ - 1]) > fabs(H[n_ - 1][n_]))
			{
				H[n_ - 1][n_ - 1] = q / H[n_][n_ - 1];
				H[n_ - 1][n_] = (- (H[n_][n_] - p)) / H[n_][n_ - 1];
			}
			else
			{
				cdiv(0.0, - H[n_ - 1][n_], H[n_ - 1][n_ - 1] - p, q);
				H[n_ - 1][n_ - 1] = cdivr;
				H[n_ - 1][n_] = cdivi;
			}
			H[n_][n_ - 1] = 0.0;
			H[n_][n_] = 1.0;
			for (i = n_ - 2; i >= 0; i--)
			{
				double ra, sa, vr, vi;
				ra = 0.0;
				sa = 0.0;
				for (j = l; j <= n_; j++)
				{
					ra = ra + H[i][j] * H[j][n_ - 1];
					sa = sa + H[i][j] * H[j][n_];
				}
				w = H[i][i] - p;
				
				if (e[i] < 0.0)
				{
					z = w;
					r = ra;
					s = sa;
				}
				else
				{
					l = i;
					if (e[i] == 0)
					{
						cdiv(- ra, - sa, w, q);
						H[i][n_ - 1] = cdivr;
						H[i][n_] = cdivi;
					}
					else
					{
						
						// Solve complex equations
						
						x = H[i][i + 1];
						y = H[i + 1][i];
						vr = (d[i] - p) * (d[i] - p) + e[i] * e[i] - q * q;
						vi = (d[i] - p) * 2.0 * q;
						if (vr == 0.0 && vi == 0.0)
						{
							vr = eps * norm * (fabs(w) + fabs(q) + fabs(x) + fabs(y) + fabs(z));
						}
						cdiv(x * r - z * ra + q * sa, x * s - z * sa - q * ra, vr, vi);
						H[i][n_ - 1] = cdivr;
						H[i][n_] = cdivi;
						if (fabs(x) > (fabs(z) + fabs(q)))
						{
							H[i + 1][n_ - 1] = (- ra - w * H[i][n_ - 1] + q * H[i][n_]) / x;
							H[i + 1][n_] = (- sa - w * H[i][n_] - q * H[i][n_ - 1]) / x;
						}
						else
						{
							cdiv(- r - y * H[i][n_ - 1], - s - y * H[i][n_], z, q);
							H[i + 1][n_ - 1] = cdivr;
							H[i + 1][n_] = cdivi;
						}
					}
					
					// Overflow control
					
					t = MAX(fabs(H[i][n_ - 1]), fabs(H[i][n_]));
					if ((eps * t) * t > 1)
					{
						for (int j2 = i; j2 <= n_; j2++)
						{
							H[j2][n_ - 1] = H[j2][n_ - 1] / t;
							H[j2][n_] = H[j2][n_] / t;
						}
					}
				}
			}
		}
	}
	
	// Vectors of isolated roots
	
	for (i = 0; i < nn; i++)
	{
		if (i < low || i > high)
		{
			for (int j2 = i; j2 < nn; j2++)
			{
				V[i][j2] = H[i][j2];
			}
		}
	}
	
	// Back transformation to get eigenvectors of original matrix
	
	for (j = nn - 1; j >= low; j--)
	{
		for (int i2 = low; i2 <= high; i2++)
		{
			z = 0.0;
			for (int k2 = low; k2 <= MIN(j, high); k2++)
			{
				z = z + V[i2][k2] * H[k2][j];
			}
			V[i2][j] = z;
		}
	}
}

EigenvalueDecomposition::EigenvalueDecomposition(Mat* pArg, Matd* /*pmD=NULL*/, Matd* /*pmV=NULL*/)
{
	d = NULL;
	e = NULL;
	V = NULL;
	H = NULL;
	ort = NULL;

	assert (pArg->type() == MAT_Tdouble);

	double** A = pArg->data.db;
	n = pArg->cols();
	V = new double*[n];
	V[0] = new double[n*n];
	for (int i = 1; i < n; i++)
	{
		V[i] = &V[0][i*n];
	}
	d = new double[n];
	e = new double[n];
	
	m_fIsSymmetric = true;
	for (int j = 0; (j < n) && m_fIsSymmetric; j++)
	{
		for (int i = 0; (i < n) && m_fIsSymmetric; i++)
		{
			m_fIsSymmetric = (A[i][j] == A[j][i]);
		}
	}
	
	if (m_fIsSymmetric)
	{
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				V[i][j] = A[i][j];
			}
		}
		
		// Tridiagonalize.
		tred2();
		
		// Diagonalize.
		tql2();
	}
	else
	{
		H = new double*[n];
		for (int i2 = 0; i2 < n; i2++)
		{
			H[i2] = new double[n];
		}
		ort = new double[n];
		
		for (int j = 0; j < n; j++)
		{
			for (int i = 0; i < n; i++)
			{
				H[i][j] = A[i][j];
			}
		}
		
		// Reduce to Hessenberg form.
		orthes();
		
		// Reduce Hessenberg to real Schur form.
		hqr2();
	}
}

EigenvalueDecomposition::~EigenvalueDecomposition()
{
	if (V)
	{
		delete []V[0];
		delete []V;
		V = NULL;
	}
	if (d)
	{
		delete []d;
		d = NULL;
	}
	if (e)
	{
		delete []e;
		e = NULL;
	}
	if (H)
	{
		for (int i2 = 0; i2 < n; i2++)
		{
			delete []H[i2];
		}
		delete []H;
		H = NULL;
	}
	if (ort)
	{
		delete []ort;
		ort = NULL;
	}
}

const double* EigenvalueDecomposition::realEigenvalues() const
{
	return d;
}

const double* EigenvalueDecomposition::imagEigenvalues() const
{
	return e;
}

Mat* EigenvalueDecomposition::D() const
{
	Mat* X = new Mat(n, n, MAT_Tdouble);
	double** D = X->data.db;
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			D[i][j] = 0.0;
		}
		D[i][i] = d[i];
		if (e[i] > 0 && i < n-1)
		{
			D[i][i + 1] = e[i];
		}
		else if (e[i] < 0 && i > 0)
		{
			D[i][i - 1] = e[i];
		}
	}
	return X;
}

Mat* EigenvalueDecomposition::GetV() const
{
	return new Mat(V[0], n, n, MAT_Tdouble);
}

}
