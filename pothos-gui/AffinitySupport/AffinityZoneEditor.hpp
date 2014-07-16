// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include "AffinitySupport/CpuSelectionWidget.hpp"
#define QT_QTCOLORPICKER_IMPORT
#include <QtColorPicker>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <Pothos/Framework/ThreadPool.hpp>

class AffinityZoneEditor : public QWidget
{
    Q_OBJECT
public:
    AffinityZoneEditor(const QString &zoneName, QWidget *parent);

private slots:

    void somethingChanged(const QColor &)
    {
        this->somethingChanged();
    }

    void somethingChanged(const QString &)
    {
        this->somethingChanged();
    }

    void somethingChanged(int)
    {
        this->somethingChanged();
    }

    void somethingChanged(void)
    {
        this->update();
        //TODO save changes
    }

private:

    void update(void);

    const QString _zoneName;
    QtColorPicker *_colorPicker;
    QComboBox *_remoteNodesBox;
    QLineEdit *_processNameEdit;
    QSpinBox *_numThreadsSpin;
    QSpinBox *_prioritySpin;
    CpuSelectionWidget *_cpuSelection;
    QVBoxLayout *_cpuSelectionContainer;
    QComboBox *_yieldModeBox;
};
