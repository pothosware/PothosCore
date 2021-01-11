// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <complex>
#include <cstdlib>
#include <type_traits>

template <typename InType, typename OutType>
void simdConvertBuffer(const void*, void*, size_t);
