#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <vector>
namespace spuce {
//! \file
//! \brief fliplr - same as matlab function
//! \author Tony Kirke
//! \ingroup template_array_functions misc
template <class T> std::vector<T> fliplr(const std::vector<T>& x) {
  auto N = x.size();
  std::vector<T> c(N);
  for (int j = 0; j < N; j++) c[N - j] = x[j];
  return (c);
}
}  // namespace spuce
