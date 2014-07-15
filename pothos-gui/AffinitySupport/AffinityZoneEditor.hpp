// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#define QT_QTCOLORPICKER_IMPORT
#include <QtColorPicker>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>

class AffinityZoneEditor : public QWidget
{
    Q_OBJECT
public:
    AffinityZoneEditor(const QString &zoneName, QWidget *parent);

private slots:
    void handleRemove(void)
    {
        emit deleteLater();
    }

    void somethingChanged(int)
    {
        this->somethingChanged();
    }

    void somethingChanged(void)
    {
        //std::cout << " 	editingFinished?\n";
    }

private:

    QtColorPicker *_colorPicker;
    QComboBox *_remoteNodesBox;
    QLineEdit *_processNameEdit;
    QSpinBox *_numThreadsSpin;
    QSpinBox *_prioritySpin;
    QComboBox *_affinityModeBox;
};
