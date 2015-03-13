// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Framework.hpp>
#include <QWidget>

class QTableWidget;
class QTabWidget;

class LogicAnalyzerDisplay : public QWidget, public Pothos::Block
{
    Q_OBJECT
public:

    LogicAnalyzerDisplay(void);

    ~LogicAnalyzerDisplay(void);

    void setNumInputs(const size_t numInputs);

    QWidget *widget(void)
    {
        return this;
    }

    //allow for standard resize controls with the default size policy
    QSize minimumSizeHint(void) const
    {
        return QSize(300, 150);
    }
    QSize sizeHint(void) const
    {
        return this->minimumSizeHint();
    }

    void work(void);

private slots:
    void updateData(const int channel, const Pothos::BufferChunk &);

private:

    template <typename T>
    void populateChannel(const int channel, const Pothos::BufferChunk &);

    void initTables(void);

    QTabWidget *_viewTabs;
    QTableWidget *_waveView;
    QTableWidget *_listView;
};

