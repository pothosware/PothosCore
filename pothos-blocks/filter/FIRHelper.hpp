// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0
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

#pragma once
#include <Pothos/Config.hpp>
#include <Poco/Logger.h>
#include <vector>
#include <complex>
#include <cmath>
#include <cfloat>
#include <cstdlib>
#include <iostream>

//! Low pass: sample rate, cutoff frequency, window
static inline std::vector<double> designLPF(const size_t numTaps, const double Fs, const double Fx, const std::vector<double> &w)
{
    std::vector<double> taps(numTaps);
    const auto lambda = M_PI * Fx / (Fs/2);

    for (size_t n = 0; n < numTaps; n++)
    {
        const double mm = n - (numTaps - 1.0) / 2.0;
        if( mm == 0.0 ) taps[n] = lambda / M_PI;
        else taps[n] = sin( mm * lambda ) / (mm * M_PI);
    }

    for (size_t n = 0; n < w.size(); n++) taps[n] *= w[n];
    return taps;
}

//! High pass: sample rate, cutoff frequency, window
static inline std::vector<double> designHPF(const size_t numTaps, const double Fs, const double Fx, const std::vector<double> &w)
{
    std::vector<double> taps(numTaps);
    const auto lambda = M_PI * Fx / (Fs/2);

    for (size_t n = 0; n < numTaps; n++)
    {
        const double mm = n - (numTaps - 1.0) / 2.0;
        if( mm == 0.0 ) taps[n] = 1.0 - lambda / M_PI;
        else taps[n] = -sin( mm * lambda ) / (mm * M_PI);
    }

    for (size_t n = 0; n < w.size(); n++) taps[n] *= w[n];
    return taps;
}

//! Band pass: sample rate, lower frequency, upper frequency, window
static inline std::vector<double> designBPF(const size_t numTaps, const double Fs, const double Fl, const double Fu, const std::vector<double> &w)
{
    std::vector<double> taps(numTaps);
    const auto lambda = M_PI * Fl / (Fs/2);
    const auto phi = M_PI * Fu / (Fs/2);

    for (size_t n = 0; n < numTaps; n++)
    {
        const double mm = n - (numTaps - 1.0) / 2.0;
        if( mm == 0.0 ) taps[n] = (phi - lambda) / M_PI;
        else taps[n] = (   sin( mm * phi ) -
                             sin( mm * lambda )   ) / (mm * M_PI);
    }

    for (size_t n = 0; n < w.size(); n++) taps[n] *= w[n];
    return taps;
}

//! Band stop: sample rate, lower frequency, upper frequency, window
static inline std::vector<double> designBSF(const size_t numTaps, const double Fs, const double Fl, const double Fu, const std::vector<double> &w)
{
    std::vector<double> taps(numTaps);
    const auto lambda = M_PI * Fl / (Fs/2);
    const auto phi = M_PI * Fu / (Fs/2);

    for (size_t n = 0; n < numTaps; n++)
    {
        const double mm = n - (numTaps - 1.0) / 2.0;
        if( mm == 0.0 ) taps[n] = 1.0 - (phi - lambda) / M_PI;
        else taps[n] = -(   sin( mm * phi ) -
                             sin( mm * lambda )   ) / (mm * M_PI);
    }

    for (size_t n = 0; n < w.size(); n++) taps[n] *= w[n];
    return taps;
}

//! Helper function to shift the taps around the center frequency
static inline std::vector<std::complex<double>> _toComplexTaps(const std::vector<double> &taps, const double Fs, const double Fl, const double Fu)
{
    std::vector<std::complex<double>> complexTaps(taps.size());
    const auto center = (Fl+Fu)/2;
    const auto lambda = M_PI * center / (Fs/2);

    for (size_t n = 0; n < taps.size(); n++)
    {
        complexTaps[n] = std::polar(taps[n], n*lambda);
    }

    return complexTaps;
}

//! Complex band pass: sample rate, lower frequency, upper frequency, window
static inline std::vector<std::complex<double>> designCBPF(const size_t numTaps, const double Fs, const double Fl, const double Fu, const std::vector<double> &w)
{
    return _toComplexTaps(designLPF(numTaps, Fs, (Fu-Fl)/2, w), Fs, Fl, Fu);
}

//! Complex band stop: sample rate, lower frequency, upper frequency, window
static inline std::vector<std::complex<double>> designCBSF(const size_t numTaps, const double Fs, const double Fl, const double Fu, const std::vector<double> &w)
{
    return _toComplexTaps(designHPF(numTaps, Fs, (Fu-Fl)/2, w), Fs, Fl, Fu);
}

static inline std::vector<double> designRRC(const size_t numTaps, const double Fs, const double Fl, double beta)
{
    double sps = Fs / Fl;
    double sum = 0;
    std::vector<double> taps(numTaps);
    if(numTaps % 2 == 0)
    {
        poco_warning(Poco::Logger::get("designRRC"), "RRC filter should have odd number of taps");
    }

    if(beta <= 0)
    {
        poco_warning(Poco::Logger::get("designRRC"), "RRC filter beta should be >0");
        beta = FLT_MIN;
    }

    double nt = numTaps-1;
    double spstwo = 2*sps;
    for(size_t n = 0; n < numTaps; n++)
    {
        double t = (-nt+2*nt*(n/nt))/spstwo;
        if(std::abs(t)<=DBL_MIN) taps[n] = -1.0/(M_PI*spstwo)*(M_PI*(beta-1)-4.0*beta);
        else if(std::abs(std::abs(4.0*beta*t)-1.0) < sqrt(DBL_MIN))
            taps[n] = 1 / (2.0*M_PI*spstwo)
                    * (M_PI*(beta+1)*sin(M_PI*(beta+1)/(4.0*beta))
                    - 4.0*beta*sin(M_PI*(beta-1)/(4.0*beta))
                    + M_PI*(beta-1)*cos(M_PI*(beta-1)/(4.0*beta)));
        else
            taps[n] = -4.0*beta/spstwo
                     * (cos((1+beta)*M_PI*t) + sin((1-beta)*M_PI*t) / (4.0*beta*t))
                     / (M_PI * (pow(4.0*beta*t,2) - 1));
        sum += taps[n]*taps[n];
    }
    for(size_t n = 0; n < numTaps; n++)
    {
        taps[n] /= sqrt(sum);
    }

    return taps;
}

static inline std::vector<double> designGaussian(const size_t numTaps, const double Fs, const double Fl, const double bt)
{
    std::vector<double> taps(numTaps);
    double sum=0;
    double sps = Fs/Fl;
    double dt = 1/sps;
    double nt = numTaps-1;
    double alpha = sqrt(log(2)/2)/bt;
    double t = -nt/(sps*2);
    double sc = std::sqrt(M_PI)/alpha;
    for(size_t n=0; n<numTaps; n++)
    {
        taps[n] = sc * std::exp(-std::pow(t*M_PI/alpha,2));
        t += dt;
        sum += taps[n];
    }
    for(size_t n = 0; n < numTaps; n++)
    {
        taps[n] /= sum;
    }
    return taps;
}
