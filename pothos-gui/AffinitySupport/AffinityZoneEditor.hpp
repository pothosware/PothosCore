// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <QWidget>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Pothos/System/NumaInfo.hpp>
#include <vector>
#include <map>

class CpuSelectionWidget;
class QtColorPicker;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QComboBox;
class QVBoxLayout;

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

    void handleUriChanged(int)
    {
        this->updateCpuSelection();
        emit this->settingsChanged();
    }

    void handleColorChanged(const QColor &)
    {
        emit this->settingsChanged();
    }

    void handleProcessNameChanged(const QString &)
    {
        emit this->settingsChanged();
    }

    void handleComboChanged(int)
    {
        emit this->settingsChanged();
    }

    void handleSpinSelChanged(void)
    {
        emit this->settingsChanged();
    }

private:

    void updateCpuSelection(void);

    QtColorPicker *_colorPicker;
    QComboBox *_hostsBox;
    QLineEdit *_processNameEdit;
    QSpinBox *_numThreadsSpin;
    QSpinBox *_prioritySpin;
    CpuSelectionWidget *_cpuSelection;
    QVBoxLayout *_cpuSelectionContainer;
    QComboBox *_yieldModeBox;

    std::map<QString, std::vector<Pothos::System::NumaInfo>> _uriToNumaInfo;
};
