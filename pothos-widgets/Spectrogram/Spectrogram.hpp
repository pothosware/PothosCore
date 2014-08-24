// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Framework.hpp>
#include <QWidget>
#include <memory>
#include <chrono>
#include <map>
#include <vector>
#include <functional>
#include "MyFFTUtils.hpp"

class QwtPlot;
class QwtColorMap;
class QwtPlotSpectrogram;
class QwtMatrixRasterData;

/***********************************************************************
 * |PothosDoc Spectrogram
 *
 * The spectrogram plot displays a live plot of the spectrum vs time.
 *
 * |category /Widgets
 * |keywords frequency plot fft dft spectrum spectral
 *
 * |param dtype[Data Type] The data type of the input elements.
 * |option [Complex128] "complex128"
 * |option [Float64] "float64"
 * |option [Complex64] "complex64"
 * |option [Float32] "float32"
 * |option [Complex Int64] "complex_int64"
 * |option [Int64] "int64"
 * |option [Complex Int32] "complex_int32"
 * |option [Int32] "int32"
 * |option [Complex Int16] "complex_int16"
 * |option [Int16] "int16"
 * |option [Complex Int8] "complex_int8"
 * |option [Int8] "int8"
 * |default "float32"
 * |preview disable
 *
 * |param title The title of the plot
 * |default "Spectrogram"
 *
 * |param displayRate[Display Rate] How often the plotter updates.
 * |default 2.0
 * |units updates/sec
 *
 * |param sampleRate[Sample Rate] The rate of the input elements.
 * |default 1e6
 * |units samples/sec
 *
 * |param numBins[Num FFT Bins] The number of bins per fourier transform.
 * |default 1024
 *
 * |param overlap[STFT Overlap] The sample overlap per subsequent fourier transform.
 * |default 512
 *
 * |param timeSpan[Time Span] How many seconds of data to display in the plot.
 * |default 10.0
 * |units seconds
 *
 * |param enableXAxis[Enable X-Axis] Show or hide the horizontal axis markers.
 * |option [Show] true
 * |option [Hide] false
 * |default true
 * |preview disable
 *
 * |param enableYAxis[Enable Y-Axis] Show or hide the vertical axis markers.
 * |option [Show] true
 * |option [Hide] false
 * |default true
 * |preview disable
 *
 * |mode graphWidget
 * |factory /widgets/spectrogram(dtype)
 * |setter setTitle(title)
 * |setter setDisplayRate(displayRate)
 * |setter setSampleRate(sampleRate)
 * |setter setNumFFTBins(numBins)
 * |setter setSTFTOverlap(overlap)
 * |setter setTimeSpan(timeSpan)
 * |setter enableXAxis(enableXAxis)
 * |setter enableYAxis(enableYAxis)
 **********************************************************************/
class Spectrogram : public QWidget, public Pothos::Block
{
    Q_OBJECT
public:

    static Block *make(const Pothos::DType &dtype)
    {
        return new Spectrogram(dtype);
    }

    Spectrogram(const Pothos::DType &dtype);

    ~Spectrogram(void);

    QWidget *widget(void)
    {
        return this;
    }

    //! set the plotter's title
    void setTitle(const QString &title);

    /*!
     * update rate for the plotter
     * how often to update the display
     */
    void setDisplayRate(const double displayRate);

    /*!
     * sample rate for the plotter
     * controls the frequency scaling display
     */
    void setSampleRate(const double sampleRate);

    void setNumFFTBins(const size_t numBins);
    void setSTFTOverlap(const size_t overlap);
    void setTimeSpan(const double timeSpan);

    QString title(void) const;

    double displayRate(void) const
    {
        return _displayRate;
    }

    double sampleRate(void) const
    {
        return _sampleRate;
    }

    size_t numFFTBins(void) const
    {
        return _numBins;
    }

    size_t stftOverlap(void) const
    {
        return _overlap;
    }

    double timeSpan(void) const
    {
        return _timeSpan;
    }

    void enableXAxis(const bool enb);
    void enableYAxis(const bool enb);

    void activate(void);
    void work(void);
    void updateCurve(Pothos::InputPort *inPort);

    //allow for standard resize controls with the default size policy
    QSize minimumSizeHint(void) const
    {
        return QSize(300, 150);
    }
    QSize sizeHint(void) const
    {
        return this->minimumSizeHint();
    }

private slots:
    void handlePickerSelected(const QPointF &);

private:
    QwtPlot *_mainPlot;
    std::shared_ptr<QwtPlotSpectrogram> _plotSpect;
    QwtMatrixRasterData *_plotMatrix;
    double _displayRate;
    double _sampleRate;
    double _sampleRateWoAxisUnits;
    size_t _numBins;
    size_t _overlap;
    double _timeSpan;
    std::chrono::high_resolution_clock::time_point _timeLastUpdate;

    std::function<void(Pothos::InputPort *, CArray &)> _inputConverter;
    void updateMatrixDimensions(void);
    QwtColorMap *makeColorMap(void) const;
};
