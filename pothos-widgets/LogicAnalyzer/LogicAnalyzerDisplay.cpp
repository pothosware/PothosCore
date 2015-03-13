// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "LogicAnalyzerDisplay.hpp"
#include <QTableWidget>
#include <QTabWidget>
#include <QHBoxLayout>
#include <complex>

LogicAnalyzerDisplay::LogicAnalyzerDisplay(void):
    _viewTabs(new QTabWidget(this)),
    _waveView(new QTableWidget(this)),
    _listView(new QTableWidget(this))
{
    auto layout = new QHBoxLayout(this);
    layout->addWidget(_viewTabs);

    //setup block
    this->registerCall(this, POTHOS_FCN_TUPLE(LogicAnalyzerDisplay, widget));
    this->registerCall(this, POTHOS_FCN_TUPLE(LogicAnalyzerDisplay, setNumInputs));
    this->setupInput(0);

    //build gui
    _viewTabs->addTab(_waveView, tr("Wave View"));
    _viewTabs->addTab(_listView, tr("List View"));

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
    this->initTables();
}

void LogicAnalyzerDisplay::initTables(void)
{
    _waveView->clear();
    _listView->clear();

    const size_t numChans(this->inputs().size());

    _waveView->setRowCount(numChans);
    _listView->setColumnCount(numChans);

    for (size_t i = 0; i < numChans; i++)
    {
        _waveView->setVerticalHeaderItem(i, new QTableWidgetItem(tr("Ch%1").arg(i)));
        _listView->setHorizontalHeaderItem(i, new QTableWidgetItem(tr("Ch%1").arg(i)));
    }
}

template <typename T>
QString toStr(const std::complex<T> num)
{
    return QString("%1 + %2j").arg(num.real()).arg(num.imag());
}

template <typename T>
QString toStr(const T num)
{
    return QString::number(num);
}

template <typename T>
void LogicAnalyzerDisplay::populateChannel(const int channel, const Pothos::BufferChunk &buffer)
{
    const auto numericBuff = buffer.convert(typeid(T));
    for (size_t i = 0; i < buffer.elements(); i++)
    {
        const auto num = numericBuff.as<const T *>()[i];
        const auto s = toStr(num);
        _waveView->setItem(channel, i, new QTableWidgetItem(s));
        _listView->setItem(i, channel, new QTableWidgetItem(s));
    }
}

void LogicAnalyzerDisplay::updateData(const int channel, const Pothos::BufferChunk &buffer)
{
    _waveView->setColumnCount(buffer.elements());
    _listView->setRowCount(buffer.elements());

    if (buffer.dtype.isComplex()) this->populateChannel<std::complex<double>>(channel, buffer);
    else if (buffer.dtype.isFloat()) this->populateChannel<double>(channel, buffer);
    else if (buffer.dtype.isInteger()) this->populateChannel<long long>(channel, buffer);
}

void LogicAnalyzerDisplay::work(void)
{
    for (auto inPort : this->inputs())
    {
        if (not inPort->hasMessage()) continue;
        const auto msg = inPort->popMessage();

        //packet-based messages have payloads to display
        if (msg.type() == typeid(Pothos::Packet))
        {
            const auto &packet = msg.convert<Pothos::Packet>();
            QMetaObject::invokeMethod(this, "updateData", Qt::QueuedConnection,
                Q_ARG(int, inPort->index()), Q_ARG(Pothos::BufferChunk, packet.payload));
        }
    }
}
