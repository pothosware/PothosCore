// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Spectrogram.hpp"
#include "MyPlotUtils.hpp"
#include <qwt_plot.h>
#include <QTimer>
#include <complex>

/***********************************************************************
 * conversion to complex double support
 **********************************************************************/
template <typename InType>
Complex toComplex(const InType &in)
{
    return Complex(in);
}

template <typename InType>
Complex toComplex(const std::complex<InType> &in)
{
    return Complex(in.real(), in.imag());
}

template <typename T>
void convertElementsToCArray(Pothos::InputPort *inPort, CArray &bins)
{
    auto buff = inPort->buffer().as<const T *>();
    for (size_t i = 0; i < bins.size(); i++) bins[i] = toComplex(buff[i]);
}

/***********************************************************************
 * initialization functions
 **********************************************************************/
void Spectrogram::activate(void)
{
    //reload num bins so we know inPort->setReserve is set
    this->setNumFFTBins(this->numFFTBins());

    auto inPort = this->input(0);
    #define doForThisType__(type) \
    else if (inPort->dtype() == Pothos::DType(typeid(type))) \
    { \
        _inputConverter = std::bind( \
            &convertElementsToCArray<type>, \
            std::placeholders::_1, \
            std::placeholders::_2); \
    }
    #define doForThisType(type) \
        doForThisType__(type) \
        doForThisType__(std::complex<type>)
    if (false){}
    doForThisType(double)
    doForThisType(float)
    doForThisType(signed long long)
    doForThisType(unsigned long long)
    doForThisType(signed long)
    doForThisType(unsigned long)
    doForThisType(signed int)
    doForThisType(unsigned int)
    doForThisType(signed short)
    doForThisType(unsigned short)
    doForThisType(signed char)
    doForThisType(unsigned char)
    doForThisType(char)
    else throw Pothos::InvalidArgumentException("Spectrogram::setupPlotterCurves("+inPort->dtype().toString()+")", "dtype not supported");

    _replotTimer->start();
}

void Spectrogram::deactivate(void)
{
    _replotTimer->stop();
}

/***********************************************************************
 * work functions
 **********************************************************************/
void Spectrogram::updateCurve(Pothos::InputPort *inPort)
{
    //create an array of complex doubles to transform with FFT
    CArray fftBins(std::min(inPort->elements(), this->numFFTBins()));
    _inputConverter(inPort, std::ref(fftBins));

    //power bins to points on the curve
    const auto powerBins = fftPowerSpectrum(fftBins);
    this->appendBins(powerBins);
}

void Spectrogram::work(void)
{
    auto updateRate = this->height()/_timeSpan;

    //should we update the plotter with these values?
    const auto timeBetweenUpdates = std::chrono::nanoseconds((long long)(1e9/updateRate));
    bool doUpdate = (std::chrono::high_resolution_clock::now() - _timeLastUpdate) > timeBetweenUpdates;

    //create a new entry in the raster data + consume
    auto inPort = this->input(0);
    if (doUpdate) this->updateCurve(inPort);
    inPort->consume(inPort->elements());

    //perform the plotter update
    if (doUpdate) _timeLastUpdate = std::chrono::high_resolution_clock::now();
}
