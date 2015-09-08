#include "FFTHelper.hpp"

#define kiss_fft_scalar float
#include "kiss_fft.h"
#include "kiss_fft.c"

template <class Type>
FFTHelper<Type>::FFTHelper(int nfft, int inverse_fft)
{
    
}

// Explicit template instantiation
template class FFTHelper<kiss_fft_scalar>;
