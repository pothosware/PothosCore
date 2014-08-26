// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Periodogram.hpp"
#include "MyPlotUtils.hpp"
#include <qwt_plot_curve.h>
#include <qwt_plot.h>
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
void Periodogram::activate(void)
{
    //reload num bins so we know inPort->setReserve is set
    this->setNumFFTBins(this->numFFTBins());
    this->setupPlotterCurves();
}

void Periodogram::setupPlotterCurves(void)
{
    //clear old curves
    _curves.clear();
    _inputConverters.clear();
    for (auto inPort : this->inputs())
    {
        #define doForThisType__(type) \
        else if (inPort->dtype() == Pothos::DType(typeid(type))) \
        { \
            _curves[inPort->index()].reset(new QwtPlotCurve(QString("Ch%1").arg(inPort->index()))); \
            _inputConverters[inPort->index()] = std::bind( \
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
        else throw Pothos::InvalidArgumentException("Periodogram::setupPlotterCurves("+inPort->dtype().toString()+")", "dtype not supported");
    }

    //continued setup for the curves
    size_t whichCurve = 0;
    for (const auto &pair : _curves)
    {
        auto &curve = pair.second;
        {
            curve->attach(_mainPlot);
            curve->setPen(pastelize(getDefaultCurveColor(whichCurve)));
            whichCurve++;
        }
    }

    //install legend for multiple channels
    if (whichCurve > 1) QMetaObject::invokeMethod(this, "installLegend", Qt::QueuedConnection);
}

/***********************************************************************
 * work functions
 **********************************************************************/
void Periodogram::updateCurve(Pothos::InputPort *inPort)
{
    //create an array of complex doubles to transform with FFT
    CArray fftBins(std::min(inPort->elements(), this->numFFTBins()));
    _inputConverters.at(inPort->index())(inPort, std::ref(fftBins));

    //power bins to points on the curve
    const auto powerBins = fftPowerSpectrum(fftBins);
    QMetaObject::invokeMethod(this, "handlePowerBins", Qt::QueuedConnection, Q_ARG(int, inPort->index()), Q_ARG(std::valarray<float>, powerBins));
}

void Periodogram::handlePowerBins(const int index, const std::valarray<float> &powerBins)
{
    QVector<QPointF> points(powerBins.size());
    for (size_t i = 0; i < powerBins.size(); i++)
    {
        auto freq = (_sampleRateWoAxisUnits*i)/(powerBins.size()-1) - _sampleRateWoAxisUnits/2;
        points[i] = QPointF(freq, powerBins[i]);
    }
    _curves.at(index)->setSamples(points);
}

void Periodogram::work(void)
{
    //should we update the plotter with these values?
    const auto timeBetweenUpdates = std::chrono::nanoseconds((long long)(1e9/_displayRate));
    bool doUpdate = (std::chrono::high_resolution_clock::now() - _timeLastUpdate) > timeBetweenUpdates;

    //reload the curves with new data -- also consume all input
    const size_t nsamps = this->workInfo().minElements;
    for (auto inPort : this->inputs())
    {
        if (doUpdate) this->updateCurve(inPort);
        inPort->consume(nsamps);
    }

    //perform the plotter update
    if (doUpdate)
    {
        QMetaObject::invokeMethod(_mainPlot, "replot", Qt::QueuedConnection);
        _timeLastUpdate = std::chrono::high_resolution_clock::now();
    }
}
