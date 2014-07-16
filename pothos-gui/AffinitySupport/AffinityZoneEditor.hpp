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
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>

class AffinityZoneEditor : public QWidget
{
    Q_OBJECT
public:
    AffinityZoneEditor(QWidget *parent);

    void loadFromConfig(const Poco::JSON::Object::Ptr &config);

    Poco::JSON::Object::Ptr getCurrentConfig(void) const;

signals:
    void settingsChanged(void);

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
        emit this->settingsChanged();
    }

private:

    void update(void);

    QtColorPicker *_colorPicker;
    QComboBox *_nodesBox;
    QLineEdit *_processNameEdit;
    QSpinBox *_numThreadsSpin;
    QSpinBox *_prioritySpin;
    CpuSelectionWidget *_cpuSelection;
    QVBoxLayout *_cpuSelectionContainer;
    QComboBox *_yieldModeBox;
};
