#pragma once
// forward declarations
#include "mixed_type.h"
//----------------------T1 + T2 operators----------------------------------
//!
template <typename T1, typename T2> inline bool operator ==(std::complex<T1> r, std::complex<T2> l)
{
  return ((real(r) == real(l)) && (imag(r) == imag(l)));  
}

///!
template <typename T1, typename T2> inline bool operator <=(std::complex<T1> r, std::complex<T2> l)
{
  return ((real(r) <= real(l)) && (imag(r) <= imag(l)));  
}
///!

template <typename T1, typename T2> inline bool operator <(std::complex<T1> r, std::complex<T2> l){
  return ((real(r) < real(l)) && (imag(r) < imag(l)));  
}
///!
template <typename T1, typename T2> inline bool operator >=(std::complex<T1> r, std::complex<T2> l)
{
  return ((real(r) >= real(l)) && (imag(r) >= imag(l)));  
}
///!
template <typename T1, typename T2> inline bool operator >(std::complex<T1> r, std::complex<T2> l)
{
  return ((real(r) > real(l)) && (imag(r) > imag(l)));  
}

//!
template <typename T1, typename T2> inline bool operator !=(std::complex<T1> r, std::complex<T2> l)
{
  return ((real(r) != real(l)) || (imag(r) != imag(l)));  
}

template <typename T1, typename T2> std::complex<typename spuce::mixed_type<T1,T2>::dtype>
operator +(std::complex<T2> l, std::complex<T1> r) {
	return(std::complex<typename spuce::mixed_type<T1,T2>::dtype>(
																																(typename spuce::mixed_type<T1,T2>::dtype)std::real(r)+
																																(typename spuce::mixed_type<T1,T2>::dtype)std::real(l),
																																(typename spuce::mixed_type<T1,T2>::dtype)std::imag(r)+
																																(typename spuce::mixed_type<T1,T2>::dtype)std::imag(l)
																																)
				 );
}

template <typename T1, typename T2> std::complex<typename spuce::mixed_type<T1,T2>::dtype>
operator -(std::complex<T2> l, std::complex<T1> r) {
	return(std::complex<typename spuce::mixed_type<T1,T2>::dtype>(
																																(typename spuce::mixed_type<T1,T2>::dtype)std::real(l)-
																																(typename spuce::mixed_type<T1,T2>::dtype)std::real(r),
																																(typename spuce::mixed_type<T1,T2>::dtype)std::imag(l)-
																																(typename spuce::mixed_type<T1,T2>::dtype)std::imag(r)
																																)
				 );
}


template <typename T1, typename T2> std::complex<typename spuce::mixed_type<T1,T2>::dtype>
operator *(std::complex<T2> l, std::complex<T1> r) {
	return(std::complex<typename spuce::mixed_type<T1,T2>::dtype>(
																																(typename spuce::mixed_type<T1,T2>::dtype)
																																((std::real(r)*std::real(l)) - (std::imag(r)*std::imag(l))), 
																																(typename spuce::mixed_type<T1,T2>::dtype)
																																(std::real(r)*std::imag(l) + std::imag(r)*std::real(l)))
				 );
}


template <typename T1, typename T2> std::complex<typename spuce::mixed_type<T1,T2>::dtype> operator *(T1 r, std::complex<T2> l) {
  return(std::complex<typename spuce::mixed_type<T1,T2>::dtype>(r*std::real(l),r*std::imag(l)));
}
template <typename T1, typename T2> std::complex<typename spuce::mixed_type<T1,T2>::dtype> operator *(std::complex<T2> l, T1 r) {
  return(std::complex<typename spuce::mixed_type<T1,T2>::dtype>(r*std::real(l),r*std::imag(l)));
}
template <typename T1, typename T2> std::complex<typename spuce::mixed_type<T1,T2>::dtype> operator +(T1 r, std::complex<T2> l) {
	return(std::complex<typename spuce::mixed_type<T1,T2>::dtype>(
																																(typename spuce::mixed_type<T1,T2>::dtype)r+
																																(typename spuce::mixed_type<T1,T2>::dtype)std::real(l),
																																(typename spuce::mixed_type<T1,T2>::dtype)std::imag(l)
																																)
				 );
}
template <typename T1, typename T2> std::complex<typename spuce::mixed_type<T1,T2>::dtype> operator +(std::complex<T2> l, T1 r) {
	return(std::complex<typename spuce::mixed_type<T1,T2>::dtype>(
																																(typename spuce::mixed_type<T1,T2>::dtype)r+
																																(typename spuce::mixed_type<T1,T2>::dtype)std::real(l),
																																(typename spuce::mixed_type<T1,T2>::dtype)std::imag(l)
																																)
				 );
}
template <typename T1, typename T2> std::complex<typename spuce::mixed_type<T1,T2>::dtype> operator -(T1 r, std::complex<T2> l) {
	return(std::complex<typename spuce::mixed_type<T1,T2>::dtype>(
																																(typename spuce::mixed_type<T1,T2>::dtype)r-
																																(typename spuce::mixed_type<T1,T2>::dtype)std::real(l),
																																(typename spuce::mixed_type<T1,T2>::dtype)-std::imag(l)
																																)
				 );
}
template <typename T1, typename T2> std::complex<typename spuce::mixed_type<T1,T2>::dtype> operator -(std::complex<T2> l, T1 r) {
	return(std::complex<typename spuce::mixed_type<T1,T2>::dtype>(
																																(typename spuce::mixed_type<T1,T2>::dtype)std::real(l)
																																-(typename spuce::mixed_type<T1,T2>::dtype)r,
																																(typename spuce::mixed_type<T1,T2>::dtype)std::imag(l)
																																)
				 );
}


template <typename T1, typename T2> std::complex<typename spuce::mixed_type<T1,T2>::dtype> operator /(T1 r, std::complex<T2> l) {
	return(std::complex<typename spuce::mixed_type<T1,T2>::dtype>((r*conj(l)/(typename spuce::mixed_type<T1,T2>::dtype)norm(l))));
}
template <typename T1, typename T2> std::complex<typename spuce::mixed_type<T1,T2>::dtype> operator /(std::complex<T2> l, T1 r) {
	return(std::complex<typename spuce::mixed_type<T1,T2>::dtype>(std::real(l)/r,std::imag(l)/r));
}

template <typename T1, typename T2> std::complex<typename spuce::mixed_type<T1,T2>::dtype>
operator /(std::complex<T2> r, std::complex<T1> l) {
	return(std::complex<typename spuce::mixed_type<T1,T2>::dtype>((r*conj(l)/(typename spuce::mixed_type<T1,T2>::dtype)norm(l))));
}


