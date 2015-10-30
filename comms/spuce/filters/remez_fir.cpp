// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
//! \author Tony Kirke
#define _USE_MATH_DEFINES
#include <spuce/typedefs.h>
#include <cmath>
#include <iostream>
#include <spuce/filters/remez_fir.h>
#include <spuce/filters/fir_inv_dft.h>

#define MAXITERATIONS 40
#define GRIDDENSITY 16

#ifndef USE_DEBUG
#define DUMP(x) 
#else
#define DUMP(x) print_std_vector(x,#x)
void print_std_vector(const std::vector<double>& x, std::string n) {
	std::cout << n << " ";
	for (size_t i=0;i<x.size();i++) std::cout << x[i] << " ";
	std::cout << "\n";
}
#endif

namespace spuce {
/********************
 * remez
 *=======
 * Calculates the optimal (in the Chebyshev/minimax sense)
 * FIR filter impulse response given a set of band edges,
 * the desired reponse on those bands, and the weight given to
 * the error in those bands.
 *
 * INPUT:
 * ------
 * int numtaps                     - Number of filter coefficients
 * int numband                     - Number of bands in filter specification
 * std::vector<float_type> bands   - User-specified band edges [2 * numband]
 * std::vector<float_type> des     - User-specified band responses [numband]
 * std::vector<float_type> weight  - User-specified error weights [numband]
 * int type                        - Type of filter
 *
 * OUTPUT:
 * -------
 * std::vector<float_type> filt    - Impulse response of final filter [numtaps]
 *************************************************************************/
bool remez_fir::remez(std::vector<float_type>& filt, int numtaps, int numband, std::vector<float_type>& bands,
                      const std::vector<float_type>& des, const std::vector<float_type>& weight, remez_type type) {
  bool ok = true;
  float_type c;
  int i;
  int iter;
  remez_symmetry symmetry = (type == remez_type::BANDPASS) ? remez_symmetry::POSITIVE : remez_symmetry::NEGATIVE;
  int r = numtaps / 2;  // number of extrema
  if ((numtaps % 2 != 0) && (symmetry == remez_symmetry::POSITIVE)) r++;

  // Predict dense grid size in advance for array sizes
  int gridSize = 0;
  for (i = 0; i < numband; i++) {
    gridSize += (int)floor(0.5 + 2 * r * GRIDDENSITY * (bands[2 * i + 1] - bands[2 * i]));
  }
  if (symmetry == remez_symmetry::NEGATIVE) gridSize--;
  if (gridSize < 1) {
		throw std::runtime_error("Gridsize issue in Remez");
  }

	DUMP(des);
	DUMP(bands);
	DUMP(weight);
	
  std::vector<float_type> grid(gridSize);
  std::vector<float_type> d(gridSize);
  std::vector<float_type> w(gridSize);
  std::vector<float_type> e(gridSize);
  std::vector<float_type> x(r + 1);
  std::vector<float_type> y(r + 1);
  std::vector<float_type> ad(r + 1);
  std::vector<float_type> taps(r + 1);
  std::vector<int> ext(r + 1);

  // Create dense frequency grid
  createDenseGrid(r, numtaps, numband, bands, des, weight, gridSize, grid, d, w, symmetry);

  // initial guess
  for (i = 0; i <= r; i++) ext[i] = i * (gridSize - 1) / r;

  // For Differentiator: (fix grid)
  if (type == remez_type::DIFFERENTIATOR) {
    for (i = 0; i < gridSize; i++)
      if (d[i] > 0.0001) w[i] /= grid[i];
  }

  // For odd or Negative symmetry filters, alter the
  // d[] and w[] according to Parks McClellan
  if (symmetry == remez_symmetry::POSITIVE) {
    if (numtaps % 2 == 0) {
      for (i = 0; i < gridSize; i++) {
        c = cos(M_PI * grid[i]);
        d[i] /= c;
        w[i] *= c;
      }
    }
  } else {
    float_type M_PI_fac = (numtaps % 2 == 0) ? M_PI : 2*M_PI;
    for (i = 0; i < gridSize; i++) {
      c = sin(M_PI_fac * grid[i]);
      d[i] /= c;
      w[i] *= c;
    }
  }

	DUMP(grid);
	DUMP(d);
	DUMP(w);
	
  // Perform the Remez Exchange algorithm
  for (iter = 0; iter < MAXITERATIONS; iter++) {
    // calc x
    for (i = 0; i <= r; i++) x[i] = cos(2*M_PI * grid[ext[i]]);
    // Calculate ad[] function in Parks & Burrus
    ad = calc_d(r, x);
    // calc y
    y = calc_y(r, ext, d, w, ad);
    // calculate error array
    for (i = 0; i < gridSize; i++) e[i] = w[i] * (d[i] - gee(grid[i], r, ad, x, y));
    search(r, ext, gridSize, e);
    if (isDone(r, ext, e)) break;
  }
  // end Remez Exchange algorithm
  if (iter == MAXITERATIONS) {
		throw std::runtime_error("Reached maximum iteration count. Results may be bad");
  }


	
  for (i = 0; i <= r; i++) x[i] = cos(2*M_PI * grid[ext[i]]);
  ad = calc_d(r, x);
  y = calc_y(r, ext, d, w, ad);

  // Find the 'taps' of the filter for use with Frequency
  // Sampling.  If odd or Negative symmetry, fix the taps
  // according to Parks McClellan
  for (i = 0; i <= numtaps / 2; i++) {
    if (symmetry == remez_symmetry::POSITIVE) {
      if (numtaps % 2 != 0)
        c = 1;
      else
        c = cos(M_PI * (float_type)i / numtaps);
    } else {
      if (numtaps % 2 != 0)
        c = sin(2*M_PI * (float_type)i / numtaps);
      else
        c = sin(M_PI * (float_type)i / numtaps);
    }
    taps[i] = gee((float_type)i / numtaps, r, ad, x, y) * c;
  }
  // Frequency sampling design with calculated taps
  if (symmetry == remez_symmetry::POSITIVE)
    filt = inv_dft_symmetric(taps, numtaps);
  else
    filt = inv_dft(taps, numtaps);
  return (ok);
}
/*******************
 * Creates the dense grid of frequencies
 * also creates the Desired Frequency Response function (d[]) and
 * the Weight function (w[]) on that dense grid
 *******************/
void remez_fir::createDenseGrid(int r, int numtaps, int numband, std::vector<float_type>& bands,
                                const std::vector<float_type>& des, const std::vector<float_type>& weight, int gridSize,
                                std::vector<float_type>& grid, std::vector<float_type>& d, std::vector<float_type>& w,
                                remez_symmetry symmetry) {
  float_type lowf, highf;
  float_type delf = 0.5 / (GRIDDENSITY * r);

  // For differentiator, hilbert,
  //  symmetry is odd and grid[0] = max(delf, band[0])
  if ((symmetry == remez_symmetry::NEGATIVE) && (delf > bands[0])) bands[0] = delf;

  int j = 0;
  int k, i;
  for (int band = 0; band < numband; band++) {
    grid[j] = bands[2 * band];
    lowf = bands[2 * band];
    highf = bands[2 * band + 1];
    k = (int)floor((highf - lowf) / delf + 0.5);
    for (i = 0; i < k; i++) {
      d[j] = des[band];
      w[j] = weight[band];
      grid[j] = lowf;
      lowf += delf;
      j++;
    }
    grid[j - 1] = highf;
  }

  // Similar to above, if odd symmetry, last grid point can't be .5
  // - but, if there are even taps, leave the last grid point at .5
  if ((symmetry == remez_symmetry::NEGATIVE) && (grid[gridSize - 1] > (0.5 - delf)) && ((numtaps % 2) != 0)) {
    grid[gridSize - 1] = 0.5 - delf;
  }
}
// Dee function in Parks & Burrus
std::vector<float_type> remez_fir::calc_d(int r, const std::vector<float_type>& x) {
  std::vector<float_type> d(r+1);
  int i, j, k;
  float_type denom, xi;
  int ld = (r - 1) / 15 + 1;
  for (i = 0; i <= r; i++) {
    denom = 1.0;
    xi = x[i];
    for (j = 0; j < ld; j++) {
      for (k = j; k <= r; k += ld)
        if (k != i) denom *= 2.0 * (xi - x[k]);
    }
    if (fabs(denom) < 0.00001) denom = 0.00001;
    d[i] = 1.0 / denom;
  }
  return d;
}
std::vector<float_type> remez_fir::calc_y(int r, const std::vector<int>& ext, const std::vector<float_type>& d,
                                          const std::vector<float_type>& w, const std::vector<float_type>& ad) {
  std::vector<float_type> y(r+1);
  float_type sign, dev, denom, numer;
  int i;

  // Calculate dev
  numer = denom = 0;
  sign = 1;
  for (i = 0; i <= r; i++) {
    numer += ad[i] * d[ext[i]];
    denom += sign * ad[i] / w[ext[i]];
    sign = -sign;
  }
  dev = numer / denom;
  sign = 1;

  // Calculate y[]
  for (i = 0; i <= r; i++) {
    y[i] = d[ext[i]] - sign * dev / w[ext[i]];
    sign = -sign;
  }
  return y;
}
/*********************
 * gee (see p301 Parks & Burrus) Digital Filter Design
 *********************/
float_type remez_fir::gee(float_type freq, int r, const std::vector<float_type>& ad, const std::vector<float_type>& x,
                          const std::vector<float_type>& y) {
  int i;
  float_type c;
  float_type numer = 0;
  float_type denom = 0;
  float_type xc = cos(2*M_PI * freq);
  for (i = 0; i <= r; i++) {
    c = xc - x[i];
    if (fabs(c) < 1.0e-7) {
      numer = y[i];
      denom = 1;
      break;
    }
    c = ad[i] / c;
    denom += c;
    numer += c * y[i];
  }
  return numer / denom;
}

/************************
 * search
 *========
 * Searches for the maxima/minima of the error curve.  If more than
 * r+1 extrema are found, it uses the following heuristic
 * 1) Adjacent non-alternating extrema deleted first.
 * 2) If there are more than one excess extrema, delete the
 *    one with the smallest error.  This will create a non-alternation
 *    condition that is fixed by 1).
 * 3) If there is exactly one excess extremum, delete the smaller
 *    of the first/last extremum
 ************************/

void remez_fir::search(int r, std::vector<int>& ext, int gridSize, const std::vector<float_type>& e) {
  bool up, alt;
  std::vector<int> foundExt(gridSize); /* Array of found extremals */
  int k = 0;
  int i;
  //  Copy found extremals from ext[]
  for (i = 0; i <= r; i++) foundExt[i] = ext[i];
  // Check for extremum at 0.
  if (((e[0] > 0.0) && (e[0] > e[1])) || ((e[0] < 0.0) && (e[0] < e[1]))) foundExt[k++] = 0;

  // Check for extrema inside dense grid
  for (i = 1; i < gridSize - 1; i++) {
    if (((e[i] >= e[i - 1]) && (e[i] > e[i + 1]) && (e[i] > 0.0)) ||
        ((e[i] <= e[i - 1]) && (e[i] < e[i + 1]) && (e[i] < 0.0)))
      foundExt[k++] = i;
  }

  // Check for extremum at 0.5
  int j = gridSize - 1;
  if (((e[j] > 0.0) && (e[j] > e[j - 1])) || ((e[j] < 0.0) && (e[j] < e[j - 1]))) foundExt[k++] = j;

  // Remove extra extremals
  int extra = k - (r + 1);
  int l;
  while (extra > 0) {
    up = e[foundExt[0]] > 0.0;
    // up = true(false) -->  first one is a maximum(minimum)
    l = 0;
    alt = true;
    for (j = 1; j < k; j++) {
      if (fabs(e[foundExt[j]]) < fabs(e[foundExt[l]])) l = j;  // new smallest error.
      if (up && (e[foundExt[j]] < 0.0))
        up = false;  // switch to a minima
      else if (!up && (e[foundExt[j]] > 0.0))
        up = true;  // switch to a maxima
      else {
        alt = false;
        break;  // Ooops, found two non-alternating
      }         // extrema.  Delete smallest of them
    }           // if the loop finishes, all extrema are alternating

    // If there's only one extremal and all are alternating,
    // delete the smallest of the first/last extremals.
    if (alt && (extra == 1)) {
      if (fabs(e[foundExt[k - 1]]) < fabs(e[foundExt[0]]))
        l = foundExt[k - 1];  // Delete last extremal
      else
        l = foundExt[0];  // Delete first extremal
    }

    for (j = l; j < k; j++)  // Loop that does the deletion
      foundExt[j] = foundExt[j + 1];
    k--;
    extra--;
  }
  //  Copy found extremals to ext[]
  for (i = 0; i <= r; i++) ext[i] = foundExt[i];
}
/*******************
 * isDone
 *========
 * Checks to see if the error function is small enough to consider
 * the result to have converged.
 ********************/
bool remez_fir::isDone(int r, const std::vector<int>& ext, const std::vector<float_type>& e) {
  int i;
  float_type min, max, current;
  min = max = fabs(e[ext[0]]);
  for (i = 1; i <= r; i++) {
    current = fabs(e[ext[i]]);
    if (current < min) min = current;
    if (current > max) max = current;
  }
  if (((max - min) / max) < 0.0001) return true;
  return false;
}
}  // namespace spuce
