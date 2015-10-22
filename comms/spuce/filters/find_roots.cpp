// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
//! \author Tony Kirke
#include <spuce/filters/find_roots.h>
namespace spuce {
/* Copyright (C) 1981-1999 Ken Turkowski. <turk_at_computer.org>
 *
 * All rights reserved.
 *
 * Warranty Information
 *  Even though I have reviewed this software, I make no warranty
 *  or representation, either express or implied, with respect to this
 *  software, its quality, accuracy, merchantability, or fitness for a
 *  particular purpose.  As a result, this software is provided "as is,"
 *  and you, its user, are assuming the entire risk as to its quality
 *  and accuracy.
 *
 * This code may be used and freely distributed as long as it includes
 * this copyright notice and the above warranty information.
 */
/*******************************************************************************
 * FindPolynomialRoots
 *
 * The Bairstow and Newton correction formulae are used for a simultaneous
 * linear and quadratic iterated synthetic division.  The coefficients of
 * a polynomial of degree n are given as a[i] (i=0,i,..., n) where a[0] is
 * the constant term.  The coefficients are scaled by dividing them by
 * their geometric mean.  The Bairstow or Newton iteration method will
 * nearly always converge to the number of figures carried, fig, either to
 * root values or to their reciprocals.  If the simultaneous Newton and
 * Bairstow iteration fails to converge on root values or their
 * reciprocals in maxiter iterations, the convergence requirement will be
 * successively reduced by one decimal figure.  This program anticipates
 * and protects against loss of significance in the quadratic synthetic
 * division.  (Refer to "On Programming the Numerical Solution of
 * Polynomial Equations," by K. W. Ellenberger, Commun. ACM 3 (Dec. 1960),
 * 644-647.)  The real and imaginary part of each root is stated as u[i]
 * and v[i], respectively, together with the corresponding constant,
 * conv[i], used in the convergence test.  This program has been used
 * successfully for over a year on the Bendix G15-D (Intercard System) and
 * has recently been coded for the IBM 709 (Fortran system).
 *
 * ACM algorithm #30 - Numerical Solution of the Polynomial Equation
 * K. W. Ellenberger
 * Missle Division, North American Aviation, Downey, California
 * Converted to C, modified, optimized, and structured by
 * Ken Turkowski
 * CADLINC, Inc., Palo Alto, California
 *******************************************************************************/

std::vector<std::complex<float_type> > find_roots(const std::vector<float_type>& a, long n) {
  const long max_iter = 100;
  const long fig = 10;
  int i, j;
  std::vector<std::complex<float_type> > roots(n);
  std::vector<float_type> h(n + 3);
  std::vector<float_type> b(n + 3);
  std::vector<float_type> c(n + 3);
  std::vector<float_type> d(n + 3);
  std::vector<float_type> e(n + 3);
  /* [-2 : n] */
  float_type K, ps, qs, pt, qt, s, rev;
  int t;
  float_type r = 0;
  float_type p = 0;
  float_type q = 0;
  int index = 0;

  /* Zero elements with negative indices */
  b[2 - 1] = b[2 - 2] = c[2 - 1] = c[2 - 2] = d[2 - 1] = d[2 - 2] = e[2 - 1] = e[2 - 2] = h[2 - 1] = h[2 - 2] = 0.0;

  /* Copy polynomial coefficients to working storage */
  int m = n;
  /* Note reversal of coefficients */
  for (j = n; j >= 0; j--) h[2 + j] = a[m++];

  t = 1;
  //  K = pow((float_type)10.0, (float_type)(fig));				/* Relative accuracy */

  for (; h[2 + n] == 0.0; n--) { /* Look for zero high-order coeff. */
    roots[n] = std::complex<float_type>(0.0, 0.0);
  }

INIT:
  if (n == 0) return (roots);

  ps = qs = pt = qt = s = 0.0;
  rev = 1.0;
  K = pow((float_type)10.0, (float_type)(fig));

  if (n == 1) {
    r = -h[2 + 1] / h[2 + 0];
    goto LINEAR;
  }

  for (j = n; j >= 0; j--) /* Find geometric mean of coeff's */
    if (h[2 + j] != 0.0) s += log(fabs(h[2 + j]));

  s = exp(s / (n + 1));

  for (j = n; j >= 0; j--) /* Normalize coeff's by mean */
    h[2 + j] /= s;

  if (fabs(h[2 + 1] / h[2 + 0]) < fabs(h[2 + n - 1] / h[2 + n])) {
  REVERSE:
    t = -t;
    for (j = (n - 1) / 2; j >= 0; j--) {
      s = h[2 + j];
      h[2 + j] = h[2 + n - j];
      h[2 + n - j] = s;
    }
  }
  if (qs != 0.0) {
    p = ps;
    q = qs;
  } else {
    if (h[2 + n - 2] == 0.0) {
      q = 1.0;
      p = -2.0;
    } else {
      q = h[2 + n] / h[2 + n - 2];
      p = (h[2 + n - 1] - q * h[2 + n - 3]) / h[2 + n - 2];
    }
    if (n == 2) goto QADRTIC;
    r = 0.0;
  }
ITERATE:
  for (i = max_iter; i > 0; i--) {
    for (j = 0; j <= n; j++) { /* BAIRSTOW */
      b[2 + j] = h[2 + j] - p * b[2 + j - 1] - q * b[2 + j - 2];
      c[2 + j] = b[2 + j] - p * c[2 + j - 1] - q * c[2 + j - 2];
    }
    if ((h[2 + n - 1] != 0.0) && (b[2 + n - 1] != 0.0)) {
      if (fabs(h[2 + n - 1] / b[2 + n - 1]) >= K) { b[2 + n] = h[2 + n] - q * b[2 + n - 2]; }
      if (b[2 + n] == 0.0) goto QADRTIC;
      if (K < fabs(h[2 + n] / b[2 + n])) goto QADRTIC;
    }

    for (j = 0; j <= n; j++) {                /* NEWTON */
      d[2 + j] = h[2 + j] + r * d[2 + j - 1]; /* Calculate polynomial at r */
      e[2 + j] = d[2 + j] + r * e[2 + j - 1]; /* Calculate derivative at r */
    }
    if (d[2 + n] == 0.0) goto LINEAR;
    if (K < fabs(h[2 + n] / d[2 + n])) goto LINEAR;

    c[2 + n - 1] = -p * c[2 + n - 2] - q * c[2 + n - 3];
    s = c[2 + n - 2] * c[2 + n - 2] - c[2 + n - 1] * c[2 + n - 3];
    if (s == 0.0) {
      p -= 2.0;
      q *= (q + 1.0);
    } else {
      p += (b[2 + n - 1] * c[2 + n - 2] - b[2 + n] * c[2 + n - 3]) / s;
      q += (-b[2 + n - 1] * c[2 + n - 1] + b[2 + n] * c[2 + n - 2]) / s;
    }
    if (e[2 + n - 1] == 0.0)
      r -= 1.0; /* Minimum step */
    else
      r -= d[2 + n] / e[2 + n - 1]; /* Newton's iteration */
  }
  ps = pt;
  qs = qt;
  pt = p;
  qt = q;
  if (rev < 0.0) K /= 10.0;
  rev = -rev;
  goto REVERSE;

LINEAR:
  if (t < 0) r = 1.0 / r;
  n--;

  roots[index++] = std::complex<float_type>(r, 0.0);

  for (j = n; j >= 0; j--) { /* Polynomial deflation by lin-nomial */
    if ((d[2 + j] != 0.0) && (fabs(h[2 + j] / d[2 + j]) < K))
      h[2 + j] = d[2 + j];
    else
      h[2 + j] = 0.0;
  }

  if (n == 0) return (roots);
  goto ITERATE;

QADRTIC:
  if (t < 0) {
    p /= q;
    q = 1.0 / q;
  }
  n -= 2;

  if (0.0 < (q - (p * p / 4.0))) { /* Two std::complex roots */
    s = sqrt(q - (p * p / 4.0));
    roots[index] = std::complex<float_type>(-p / 2.0, s);
    roots[index + 1] = std::complex<float_type>(-p / 2.0, -s);
  } else { /* Two real roots */
    s = sqrt(((p * p / 4.0)) - q);
    if (p < 0.0)
      roots[index] = std::complex<float_type>(-0.5 * p + s, 0.0);
    else
      roots[index] = std::complex<float_type>(-0.5 * p - s, 0.0);

    roots[index + 1] = std::complex<float_type>(real(q / roots[index]), 0.0);
  }
  index += 2;

  for (j = n; j >= 0; j--) { /* Polynomial deflation by quadratic */
    if ((b[2 + j] != 0.0) && (fabs(h[2 + j] / b[2 + j]) < K))
      h[2 + j] = b[2 + j];
    else
      h[2 + j] = 0.0;
  }
  goto INIT;
}

}  // namespace spuce
