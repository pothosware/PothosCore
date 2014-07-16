// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include "AffinitySupport/AffinityZoneEditor.hpp"
#include <Pothos/Remote.hpp>
#include <Pothos/Proxy.hpp>
#include <QFormLayout>

static const int ARBITRARY_MAX_THREADS = 4096;

AffinityZoneEditor::AffinityZoneEditor(const QString &zoneName, QWidget *parent):
    QWidget(parent),
    _zoneName(zoneName),
    _colorPicker(new QtColorPicker(this)),
    _remoteNodesBox(new QComboBox(this)),
    _processNameEdit(new QLineEdit(this)),
    _numThreadsSpin(new QSpinBox(this)),
    _prioritySpin(new QSpinBox(this)),
    _cpuSelection(nullptr),
    _cpuSelectionContainer(new QVBoxLayout()),
    _yieldModeBox(new QComboBox(this))
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
        _colorPicker->setCurrentColor(Qt::yellow);
        _colorPicker->setToolTip(tr("Select a color to associate affinities in the graph editor"));
        connect(_colorPicker, SIGNAL(colorChanged(const QColor &)), this, SLOT(somethingChanged(const QColor &)));
    }

    //remote nodes
    {
        formLayout->addRow(tr("Remote node"), _remoteNodesBox);
        _remoteNodesBox->addItems(getRemoteNodeUris());
        _remoteNodesBox->setToolTip(tr("Select the URI for a local or remote host"));
        connect(_remoteNodesBox, SIGNAL(currentIndexChanged(int)), this, SLOT(somethingChanged(int)));
    }

    //process id
    {
        formLayout->addRow(tr("Process name"), _processNameEdit);
        _processNameEdit->setPlaceholderText(tr("The string name of a process"));
        _processNameEdit->setToolTip(tr("An arbitrary name to identify a process on a node"));
        connect(_processNameEdit, SIGNAL(textChanged(const QString &)), this, SLOT(somethingChanged(const QString &)));
    }

    //num threads
    {
        formLayout->addRow(tr("Thread count"), _numThreadsSpin);
        _numThreadsSpin->setRange(0, ARBITRARY_MAX_THREADS);
        _numThreadsSpin->setToolTip(tr("Number of threads to allocate, 0 means automatic"));
        connect(_numThreadsSpin, SIGNAL(editingFinished(void)), this, SLOT(somethingChanged()));
    }

    //priority selection
    {
        formLayout->addRow(tr("Process priority %"), _prioritySpin);
        _prioritySpin->setRange(-100, +100);
        _prioritySpin->setToolTip(tr("A priority percentage between -100% and 100%"));
        connect(_prioritySpin, SIGNAL(editingFinished(void)), this, SLOT(somethingChanged()));
    }

    //cpu/node selection
    {
        formLayout->addRow(tr("CPU selection"), _cpuSelectionContainer);
    }

    //yield mode
    {
        formLayout->addRow(tr("Yield mode"), _yieldModeBox);
        _yieldModeBox->addItem(tr("Condition"), "CONDITION");
        _yieldModeBox->addItem(tr("Hybrid"), "HYBRID");
        _yieldModeBox->addItem(tr("Spin"), "SPIN");
        connect(_yieldModeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(somethingChanged(int)));
    }

    this->update();
}

void AffinityZoneEditor::update(void)
{
    //update the cpu selection widget
    auto uriStr = _remoteNodesBox->itemText(_remoteNodesBox->currentIndex());
    delete _cpuSelection;
    try
    {
        auto env = Pothos::RemoteClient(uriStr.toStdString()).makeEnvironment("managed");
        auto nodeInfos = env->findProxy("Pothos/System/NumaInfo").call<std::vector<Pothos::System::NumaInfo>>("get");
        _cpuSelection = new CpuSelectionWidget(nodeInfos, this);
        _cpuSelectionContainer->addWidget(_cpuSelection);
    }
    catch (const Pothos::Exception &ex)
    {
        //make a junk _cpuSelection?
        //TODO log this?
    }

    Pothos::ThreadPoolArgs threadPoolArgs;
    threadPoolArgs.numThreads = _numThreadsSpin->value();
    threadPoolArgs.priority = _prioritySpin->value()/100.0;
    threadPoolArgs.affinityMode = _cpuSelection->affinityMode();
    threadPoolArgs.affinity = _cpuSelection->affinity();
    threadPoolArgs.yieldMode = _yieldModeBox->itemData(_yieldModeBox->currentIndex()).toString().toStdString();
}
