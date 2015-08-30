// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "LogicAnalyzerDisplay.hpp"
#include <QTableWidget>
#include <QHBoxLayout>
#include <complex>
#include <cassert>

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
    qRegisterMetaType<Pothos::Packet>("Pothos::Packet");
}

LogicAnalyzerDisplay::~LogicAnalyzerDisplay(void)
{
    return;
}

void LogicAnalyzerDisplay::setNumInputs(const size_t numInputs)
{
    _chLabel.resize(numInputs);
    _chBase.resize(numInputs, 10);
    _chData.resize(numInputs);
    _tableView->setRowCount(numInputs);
}

void LogicAnalyzerDisplay::setSampleRate(const double sampleRate)
{
    _sampleRate = sampleRate;
    QMetaObject::invokeMethod(this, "handleReplot", Qt::QueuedConnection);
}

template <typename T>
void LogicAnalyzerDisplay::populateChannel(const int channel, const Pothos::Packet &packet)
{
    //convert buffer (does not convert when type matches)
    const auto numericBuff = packet.payload.convert(typeid(T));
    assert(_chData.size() > channel);
    _chData[channel] = packet;
    _chData[channel].payload = numericBuff;

    //load element data into table
    for (size_t i = 0; i < numericBuff.elements(); i++)
    {
        const auto num = numericBuff.as<const T *>()[i];
        const auto s = toStr(num, _chBase.at(channel));
        auto item = new QTableWidgetItem(s);
        auto flags = item->flags();
        flags &= ~Qt::ItemIsEditable;
        item->setFlags(flags);
        item->setTextAlignment(Qt::AlignRight);
        _tableView->setItem(channel, i, item);
    }

    //inspect labels to decorate table
    for (const auto &label : packet.labels)
    {
        const int column = label.index;
        assert(column < _tableView->columnCount());
        auto item = _tableView->item(channel, column);

        //highlight and display label id
        item->setBackground(Qt::yellow);
        item->setText(QString("%1\n%2")
            .arg(item->text())
            .arg(QString::fromStdString(label.id)));
        _tableView->resizeColumnToContents(column);
    }
}

void LogicAnalyzerDisplay::updateData(const Pothos::Packet &packet)
{
    const auto indexIt = packet.metadata.find("index");
    const auto channel = (indexIt == packet.metadata.end())?0:indexIt->second.convert<int>();

    //column count changed? new labels
    const size_t numElems = packet.payload.elements();
    const bool changed = _tableView->columnCount() != int(numElems);
    _tableView->setColumnCount(numElems);
    if (changed) this->updateHeaders();

    const auto dtype = packet.payload.dtype;
    if (dtype.isComplex()) this->populateChannel<std::complex<qreal>>(channel, packet);
    else if (dtype.isFloat()) this->populateChannel<qreal>(channel, packet);
    else if (dtype.isInteger()) this->populateChannel<qlonglong>(channel, packet);

    if (changed) _tableView->resizeColumnsToContents();
}

void LogicAnalyzerDisplay::updateHeaders(void)
{
    const size_t numElems = _tableView->columnCount();

    double factor = 1.0;
    QString units("s");
    double timeSpan = numElems/_sampleRate;
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

    if (_xAxisMode == "INDEX") for (size_t i = 0; i < numElems; i++)
    {
        _tableView->setHorizontalHeaderItem(i, new QTableWidgetItem(QString::number(i)));
    }

    if (_xAxisMode == "TIME") for (size_t i = 0; i < numElems; i++)
    {
        double t = i*_sampleRate/factor;
        _tableView->setHorizontalHeaderItem(i, new QTableWidgetItem(QString::number(t)+units));
    }
}

void LogicAnalyzerDisplay::handleReplot(void)
{
    _tableView->clear();
    this->updateHeaders();

    for (size_t ch = 0; ch < this->numInputs(); ch++)
    {
        auto label = _chLabel.at(ch);
        if (label.isEmpty()) label = tr("Ch%1").arg(ch);
        _tableView->setVerticalHeaderItem(ch, new QTableWidgetItem(label));
        this->updateData(_chData.at(ch));
    }

    _tableView->resizeColumnsToContents();
}

void LogicAnalyzerDisplay::work(void)
{
    auto inPort = this->input(0);

    if (not inPort->hasMessage()) return;
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
        QMetaObject::invokeMethod(this, "updateData", Qt::QueuedConnection, Q_ARG(Pothos::Packet, packet));
    }
}
