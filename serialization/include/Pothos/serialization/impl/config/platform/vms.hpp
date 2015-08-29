//  (C) Copyright Artyom Beilis 2010.  
//  Use, modification and distribution are subject to the  
//  Boost Software License, Version 1.0. (See accompanying file  
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) 

#ifndef POTHOS_CONFIG_PLATFORM_VMS_HPP 
#define POTHOS_CONFIG_PLATFORM_VMS_HPP 

#define POTHOS_PLATFORM "OpenVMS" 

#undef  POTHOS_HAS_STDINT_H 
#define POTHOS_HAS_UNISTD_H 
#define POTHOS_HAS_NL_TYPES_H 
#define POTHOS_HAS_GETTIMEOFDAY 
#define POTHOS_HAS_DIRENT_H 
#define POTHOS_HAS_PTHREADS 
#define POTHOS_HAS_NANOSLEEP 
#define POTHOS_HAS_CLOCK_GETTIME 
#define POTHOS_HAS_PTHREAD_MUTEXATTR_SETTYPE 
#define POTHOS_HAS_LOG1P 
#define POTHOS_HAS_EXPM1 
#define POTHOS_HAS_THREADS 
#undef  POTHOS_HAS_SCHED_YIELD 

#endif 
