// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "LogicAnalyzerDisplay.hpp"
#include <QTableWidget>
#include <QHBoxLayout>
#include <complex>

/***********************************************************************
 * Number format overloads
 **********************************************************************/
static QString toStr(const std::complex<qreal> num, const int)
{
    if (num.real() == 0.0 and num.imag() == 0.0) return "0";
    if (num.real() == 0.0) return QString::number(num.imag())+"j";
    if (num.imag() == 0.0) return QString::number(num.real());
    if (num.imag() < 0.0) return QString("%1%2j").arg(num.real()).arg(num.imag());
    return QString("%1+%2j").arg(num.real()).arg(num.imag());
}

static QString toStr(const qreal num, const int)
{
    return QString::number(num);
}

static QString toStr(const qlonglong num, const int base)
{
    return QString::number(num, base);
}

/***********************************************************************
 * Logic analyzer GUI implementation
 **********************************************************************/
LogicAnalyzerDisplay::LogicAnalyzerDisplay(void):
    _tableView(new QTableWidget(this)),
    _sampleRate(1.0),
    _xAxisMode("INDEX"),
    _rateLabelId("rxRate")
{
    auto layout = new QHBoxLayout(this);
    layout->addWidget(_tableView);

    //setup block
    this->registerCall(this, POTHOS_FCN_TUPLE(LogicAnalyzerDisplay, widget));
    this->registerCall(this, POTHOS_FCN_TUPLE(LogicAnalyzerDisplay, setNumInputs));
    this->registerCall(this, POTHOS_FCN_TUPLE(LogicAnalyzerDisplay, numInputs));
    this->registerCall(this, POTHOS_FCN_TUPLE(LogicAnalyzerDisplay, setSampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(LogicAnalyzerDisplay, sampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(LogicAnalyzerDisplay, setChannelLabel));
    this->registerCall(this, POTHOS_FCN_TUPLE(LogicAnalyzerDisplay, setChannelBase));
    this->registerCall(this, POTHOS_FCN_TUPLE(LogicAnalyzerDisplay, setXAxisMode));
    this->registerCall(this, POTHOS_FCN_TUPLE(LogicAnalyzerDisplay, setRateLabelId));
    this->setupInput(0);

    //register types passed to gui thread from work
    qRegisterMetaType<Pothos::BufferChunk>("Pothos::BufferChunk");
}

LogicAnalyzerDisplay::~LogicAnalyzerDisplay(void)
{
    return;
}

void LogicAnalyzerDisplay::setNumInputs(const size_t numInputs)
{
    for (size_t i = this->inputs().size(); i < numInputs; i++)
    {
        this->setupInput(i, this->input(0)->dtype());
    }

    _chLabel.resize(numInputs);
    _chBase.resize(numInputs, 10);
    _chBuffers.resize(numInputs);
    _tableView->setRowCount(numInputs);
}

void LogicAnalyzerDisplay::setSampleRate(const double sampleRate)
{
    _sampleRate = sampleRate;
    QMetaObject::invokeMethod(this, "handleReplot", Qt::QueuedConnection);
}

template <typename T>
void LogicAnalyzerDisplay::populateChannel(const int channel, const Pothos::BufferChunk &buffer)
{
    const auto numericBuff = buffer.convert(typeid(T));
    _chBuffers[channel] = numericBuff;
    for (size_t i = 0; i < buffer.elements(); i++)
    {
        const auto num = numericBuff.as<const T *>()[i];
        const auto s = toStr(num, _chBase.at(channel));
        auto item = new QTableWidgetItem(s);
        auto flags = item->flags();
        flags &= ~Qt::ItemIsEditable;
        item->setFlags(flags);
        _tableView->setItem(channel, i, item);
    }
}

void LogicAnalyzerDisplay::updateData(const int channel, const Pothos::BufferChunk &buffer)
{
    //column count changed? new labels
    if (_tableView->columnCount() != int(buffer.elements()))
    {
        double factor = 1.0;
        QString units("s");
        double timeSpan = buffer.elements()/_sampleRate;
        if (timeSpan <= 100e-9)
        {
            factor = 1e9;
            units = "ns";
        }
        else if (timeSpan <= 100e-6)
        {
            factor = 1e6;
            units = "us";
        }
        else if (timeSpan <= 100e-3)
        {
            factor = 1e3;
            units = "ms";
        }

        _tableView->setColumnCount(buffer.elements());
        if (_xAxisMode == "INDEX")
        {
            for (size_t i = 0; i < buffer.elements(); i++)
            {
                _tableView->setHorizontalHeaderItem(i, new QTableWidgetItem(QString::number(i)));
            }
        }
        if (_xAxisMode == "TIME")
        {
            for (size_t i = 0; i < buffer.elements(); i++)
            {
                double t = i*_sampleRate/factor;
                _tableView->setHorizontalHeaderItem(i, new QTableWidgetItem(QString::number(t)+units));
            }
        }
    }

    if (buffer.dtype.isComplex()) this->populateChannel<std::complex<qreal>>(channel, buffer);
    else if (buffer.dtype.isFloat()) this->populateChannel<qreal>(channel, buffer);
    else if (buffer.dtype.isInteger()) this->populateChannel<qlonglong>(channel, buffer);
}

void LogicAnalyzerDisplay::handleReplot(void)
{
    _tableView->clear();
    _tableView->setColumnCount(0);

    const int numChans(this->inputs().size());

    _tableView->setRowCount(numChans);

    for (int ch = 0; ch < numChans; ch++)
    {
        auto label = _chLabel.at(ch);
        if (label.isEmpty()) label = tr("Ch%1").arg(ch);
        _tableView->setVerticalHeaderItem(ch, new QTableWidgetItem(label));
        this->updateData(ch, _chBuffers.at(ch));
    }

    _tableView->resizeColumnsToContents();
}

void LogicAnalyzerDisplay::work(void)
{
    for (auto inPort : this->inputs())
    {
        if (not inPort->hasMessage()) continue;
        const auto msg = inPort->popMessage();

        //label-based messages have in-line commands
        if (msg.type() == typeid(Pothos::Label))
        {
            const auto &label = msg.convert<Pothos::Label>();
            if (label.id == _rateLabelId and label.data.canConvert(typeid(double)))
            {
                this->setSampleRate(label.data.convert<double>());
            }
        }

        //packet-based messages have payloads to display
        if (msg.type() == typeid(Pothos::Packet))
        {
            const auto &packet = msg.convert<Pothos::Packet>();
            QMetaObject::invokeMethod(this, "updateData", Qt::QueuedConnection,
                Q_ARG(int, inPort->index()), Q_ARG(Pothos::BufferChunk, packet.payload));
        }
    }
}
