/*
 * FIR filter class, by Mike Perkins
 * 
 * a simple C++ class for linear phase FIR filtering
 *
 * For background, see the post http://www.cardinalpeak.com/blog?p=1841
 *
 * Copyright (c) 2013, Cardinal Peak, LLC.  http://www.cardinalpeak.com
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1) Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2) Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 
 * 3) Neither the name of Cardinal Peak nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * CARDINAL PEAK, LLC BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * 
 * PURPOSE:
 * This object designs digital filters and filters digital data streams
 * 
 * USAGE:
 * Invoke an object of type Filter.  Two constructors are available.
 * One is used for LPF and HPF filters, one is used for BPFs.
 * The arguments to the constructors are as follows:
 * 
 * 		// For LPF or HPF only
 * 		Filter(filterType filt_t, int num_taps, double Fs, double Fx);
 * 		// For BPF only
 * 		Filter(filterType filt_t, int num_taps, double Fs, double Fl, double Fu);
 * 
 * filt_t: is LPF, HPF or BPF
 * num_taps: is the number of taps you want the filter to use
 * Fs: is the sampling frequency of the digital data being filtered
 * Fx: is the "transition" frequency for LPF and HPF filters
 * Fl, Fu: are the upper and lower transition frequencies for BPF filters
 * 
 * Once the filter is created, you can start filtering data.  Here
 * is an example for 51 tap lowpass filtering of an audio stream sampled at
 * 44.1Khz (the CD sampling rate), where the goal is to create a signal
 * of "telephone" bandwidth (4Khz):
 * 
 * Filter *my_filter;
 * 
 * my_filter = new Filter(LPF, 51, 44.1, 4.0)
 * if( my_filter->get_error_flag() != 0 ) // abort in an appropriate manner
 * 
 * while(data_to_be_filtered){
 * 	next_sample = // Get the next sample from the data stream somehow
 * 	filtered_sample = my_filter->do_sample( next_sample );
 *   .
 * 	.
 * 	.
 * }
 * delete my_filter;
 * 
 * Several helper functions are provided:
 *     init(): The filter can be re-initialized with a call to this function
 *     get_taps(double *taps): returns the filter taps in the array "taps"
 *     write_taps_to_file(char *filename): writes the filter taps to a file
 *     write_freqres_to_file(char *filename): output frequency response to a file
 * 
 * Finally, a get_error_flag() function is provided.  Recommended usage
 * is to check the get_error_flag() return value for a non-zero
 * value after the new Filter object is created.  If it is non-zero, print 
 * out the non-zero value and look at the following table to see the
 * error:
 * -1:  Fs <= 0
 * -2:  Fx <= 0 or Fx >= Fs/2
 * -3:  num_taps <= 0 or num_taps >= MAX_NUM_FILTER_TAPS
 * -4:  memory allocation for the needed arrays failed
 * -5:  an invalid filterType was passed into a constructor
 * -10: Fs <= 0 (BPF case)
 * -11: Fl >= Fu
 * -12: Fl <= 0 || Fl >= Fs/2
 * -13: Fu <= 0 || Fu >= Fs/2
 * -14: num_taps <= 0 or num_taps >= MAX_NUM_FILTER_TAPS (BPF case)
 * -15:  memory allocation for the needed arrays failed (BPF case)
 * -16:  an invalid filterType was passed into a constructor (BPF case)
 * 
 * Note that if a non-zero error code value occurs, every call to do_sample()
 * will return the value 0. write_taps_fo_file() will fail and return a -1 (it
 * also returns a -1 if it fails to open the tap file passed into it).
 * get_taps() will have no effect on the array passed in if the error_flag
 * is non-zero. write_freqres_to_file( ) returns different error codes
 * depending on the nature of the error...see the function itself for details.
 * 
 * The filters are designed using the "Fourier Series Method".  This
 * means that the coefficients of a Fourier Series approximation to the
 * frequency response of an ideal filter (LPF, HPF, BPF) are used as
 * the filter taps.  The resulting filters have some ripple in the passband 
 * due to the Gibbs phenomenon; the filters are linear phase.
 */

#ifndef _FILTER_H
#define _FILTER_H

#define MAX_NUM_FILTER_TAPS 1000

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

enum filterType {LPF, HPF, BPF};

class Filter{
	private:
		filterType m_filt_t;
		int m_num_taps;
		int m_error_flag;
		double m_Fs;
		double m_Fx;
		double m_lambda;
		double *m_taps;
		double *m_sr;
		void designLPF();
		void designHPF();

		// Only needed for the bandpass filter case
		double m_Fu, m_phi;
		void designBPF();

	public:
		Filter(filterType filt_t, int num_taps, double Fs, double Fx);
		Filter(filterType filt_t, int num_taps, double Fs, double Fl, double Fu);
		~Filter( );
		void init();
		double do_sample(double data_sample);
		int get_error_flag(){return m_error_flag;};
		void get_taps( double *taps );
		int write_taps_to_file( char* filename );
		int write_freqres_to_file( char* filename );
};

#endif
