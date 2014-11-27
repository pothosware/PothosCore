// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <qwt_raster_data.h>
#include <QList>
#include <valarray>
#include <mutex>

class MySpectrogramRasterData : public QwtRasterData
{
public:
    MySpectrogramRasterData(void):
        _numCols(1)
    {
        this->setNumRows(1);
    }

    //! translate a plot coordinate into a raster value
    double value(double x, double y) const
    {
        const auto time = size_t(_yScale*(y-_yOff));
        const auto bin = size_t(_xScale*(x-_xOff));
        return _data[time][bin];
    }

    //! append a new power spectrum bin array
    void appendBins(const std::valarray<float> &bins)
    {
        std::unique_lock<std::mutex> lock(_rasterMutex);
        _data.push_front(bins);
        _data.pop_back();
    }

    //! A raster operation has begun
    void initRaster(const QRectF &, const QSize &raster)
    {
        _rasterMutex.lock();

        this->setNumRows(raster.height());

        _yOff = this->interval(Qt::YAxis).minValue();
        _yScale = (_data.size()-1)/this->interval(Qt::YAxis).width();
        _xOff = this->interval(Qt::XAxis).minValue();
        _xScale = (_numCols-1)/this->interval(Qt::XAxis).width();

    }

    //! A raster operation has ended
    void discardRaster(void)
    {
        _rasterMutex.unlock();
    }

    //! Change the number of bins per power spectrum
    void setNumColumns(const size_t numCols)
    {
        std::unique_lock<std::mutex> lock(_rasterMutex);
        if (numCols == _numCols) return;
        _numCols = numCols;
        for (auto &row : _data)
        {
            std::valarray<float> newRow(_numCols);
            for (size_t i = 0; i < newRow.size(); i++)
                newRow[i] = row[size_t((double(i)*(row.size()-1))/(newRow.size()-1))];
            row = newRow;
        }
    }

private:
    void setNumRows(const int num)
    {
        if (_data.isEmpty()) _data.push_front(std::valarray<float>(-1000, _numCols));
        while (_data.size() > num) _data.pop_back();
        while (_data.size() < num) _data.push_front(_data.front());
    }

    //raster scale+adjustment factors
    float _yOff, _yScale, _xOff, _xScale;

    //raw data for the entire raster
    QList<std::valarray<float>> _data;

    //thread-safe access mutex
    std::mutex _rasterMutex;

    size_t _numCols;
};
