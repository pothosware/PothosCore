#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <vector>
namespace spuce {
//! \file
// \brief Template partial convolve function
//! \author Tony Kirke,  Copyright(c) 2001
//! \ingroup template_array_functions misc
template <class T> std::vector<T> partial_convolve(const std::vector<T>& x, const std::vector<T>& y, int N, int M) {
  int i, j;
  auto L = M + N - 1;
  std::vector<T> c(L);
  for (i = 0; i < L; i++) {
    c[i] = (T)0;
    for (j = 0; j < N; j++) {
      if ((i - j >= 0) & (i - j < M)) c[i] += x[j] * y[i - j];
    }
  }
  return (c);
}
}  // namespace spuce
