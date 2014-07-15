// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include "AffinitySupport/AffinityZoneEditor.hpp"
#include "AffinitySupport/FiniteNumericSelectionWidget.hpp"
#include <QFormLayout>

static const int ARBITRARY_MAX_THREADS = 4096;

AffinityZoneEditor::AffinityZoneEditor(const QString &zoneName, QWidget *parent):
    QWidget(parent),
    _colorPicker(new QtColorPicker(this)),
    _remoteNodesBox(new QComboBox(this)),
    _processNameEdit(new QLineEdit(this)),
    _numThreadsSpin(new QSpinBox(this)),
    _prioritySpin(new QSpinBox(this)),
    _affinityModeBox(new QComboBox(this))
{
    //bold title
    this->setStyleSheet("QGroupBox{font-weight: bold;}");

    //setup layout
    auto formLayout = new QFormLayout(this);
    this->setLayout(formLayout);

    //color picker
    {
        formLayout->addRow(tr("Affinity color"), _colorPicker);
        _colorPicker->setStandardColors();
    }

    //remote nodes
    {
        formLayout->addRow(tr("Remote node"), _remoteNodesBox);
        _remoteNodesBox->addItems(getRemoteNodeUris());
        connect(_remoteNodesBox, SIGNAL(currentIndexChanged(int)), this, SLOT(somethingChanged(int)));
    }

    //process id
    {
        formLayout->addRow(tr("Process name"), _processNameEdit);
        _processNameEdit->setPlaceholderText(tr("The string name of a process"));
        _processNameEdit->setToolTip(tr("Refer to the name of a process"));
    }

    //num threads
    {
        formLayout->addRow(tr("Thread count"), _numThreadsSpin);
        _numThreadsSpin->setRange(1, ARBITRARY_MAX_THREADS);
        connect(_numThreadsSpin, SIGNAL(editingFinished(void)), this, SLOT(somethingChanged()));
    }

    //priority selection
    {
        formLayout->addRow(tr("Process priority %"), _prioritySpin);
        _prioritySpin->setRange(-100, +100);
        _prioritySpin->setToolTip(tr("A priority percentage between -100% and 100%"));
        connect(_prioritySpin, SIGNAL(editingFinished(void)), this, SLOT(somethingChanged()));
    }

    //affinity mode
    {
        formLayout->addRow(tr("Affinity mode"), _affinityModeBox);
        _affinityModeBox->addItem(tr("Use all resources"), "ALL");
        _affinityModeBox->addItem(tr("Specify individual CPUs"), "CPU");
        _affinityModeBox->addItem(tr("Specify NUMA nodes"), "NUMA");
        connect(_affinityModeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(somethingChanged(int)));
    }

    //cpu/node selection
    {
        auto _cpuTable = new FiniteNumericSelectionWidget(8, this);
        formLayout->addRow(tr("CPU Selection"), _cpuTable);
    }
}
