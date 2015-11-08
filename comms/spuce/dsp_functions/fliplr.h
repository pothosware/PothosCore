#pragma once
// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
#include <vector>
namespace spuce {
//! \file
//! \brief fliplr - same as matlab function
//! \author Tony Kirke
//! \ingroup template_array_functions misc
template <class T> std::vector<T> fliplr(const std::vector<T>& x) {
  auto N = x.size();
  std::vector<T> c(N);
  for (size_t j = 0; j < N; j++) c[N - j] = x[j];
  return (c);
}
}  // namespace spuce
