// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Framework.hpp>
#include <QWidget>

class QTableWidget;

class LogicAnalyzerDisplay : public QWidget, public Pothos::Block
{
    Q_OBJECT
public:

    LogicAnalyzerDisplay(void);

    ~LogicAnalyzerDisplay(void);

    void setNumInputs(const size_t numInputs);

    /*!
     * sample rate for the plotter
     * controls the time scaling display
     */
    void setSampleRate(const double sampleRate);

    size_t numInputs(void) const
    {
        return this->inputs().size();
    }

    double sampleRate(void) const
    {
        return _sampleRate;
    }

    QWidget *widget(void)
    {
        return this;
    }

    //allow for standard resize controls with the default size policy
    QSize minimumSizeHint(void) const
    {
        return QSize(300, 100);
    }
    QSize sizeHint(void) const
    {
        return this->minimumSizeHint();
    }

    void setXAxisMode(const std::string &mode)
    {
        _xAxisMode = mode;
        QMetaObject::invokeMethod(this, "handleReplot", Qt::QueuedConnection);
    }

    void setRateLabelId(const std::string &id)
    {
        _rateLabelId = id;
    }

    void setChannelLabel(const size_t ch, const QString &label)
    {
        if (_chLabel.size() <= ch) _chLabel.resize(ch+1);
        _chLabel[ch] = label;
        QMetaObject::invokeMethod(this, "handleReplot", Qt::QueuedConnection);
    }

    void setChannelBase(const size_t ch, const size_t base)
    {
        if (_chBase.size() <= ch) _chBase.resize(ch+1);
        _chBase[ch] = base;
        QMetaObject::invokeMethod(this, "handleReplot", Qt::QueuedConnection);
    }

    void work(void);

private slots:
    void updateData(const int channel, const Pothos::BufferChunk &);
    void handleReplot(void);

private:

    template <typename T>
    void populateChannel(const int channel, const Pothos::BufferChunk &);

    QTableWidget *_tableView;

    double _sampleRate;
    std::string _xAxisMode;
    std::string _rateLabelId;

    //per-channel settings
    std::vector<QString> _chLabel;
    std::vector<size_t> _chBase;
    std::vector<Pothos::BufferChunk> _chBuffers;
};

