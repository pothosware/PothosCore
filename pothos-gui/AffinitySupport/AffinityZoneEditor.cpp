// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include "AffinitySupport/AffinityZoneEditor.hpp"
#include <Pothos/Remote.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/Logger.h>
#include <QFormLayout>

static const int ARBITRARY_MAX_THREADS = 4096;

AffinityZoneEditor::AffinityZoneEditor(QWidget *parent):
    QWidget(parent),
    _colorPicker(new QtColorPicker(this)),
    _nodesBox(new QComboBox(this)),
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
        connect(_colorPicker, SIGNAL(colorChanged(const QColor &)), this, SLOT(handleColorChanged(const QColor &)));
    }

    //node selection
    {
        formLayout->addRow(tr("Remote node"), _nodesBox);
        _nodesBox->addItems(getRemoteNodeUris());
        _nodesBox->setToolTip(tr("Select the URI for a local or remote host"));
        connect(_nodesBox, SIGNAL(currentIndexChanged(int)), this, SLOT(handleUriChanged(int)));
    }

    //process id
    {
        formLayout->addRow(tr("Process name"), _processNameEdit);
        _processNameEdit->setPlaceholderText(tr("The string name of a process"));
        _processNameEdit->setToolTip(tr("An arbitrary name to identify a process on a node"));
        connect(_processNameEdit, SIGNAL(textChanged(const QString &)), this, SLOT(handleProcessNameChanged(const QString &)));
    }

    //num threads
    {
        formLayout->addRow(tr("Thread count"), _numThreadsSpin);
        _numThreadsSpin->setRange(0, ARBITRARY_MAX_THREADS);
        _numThreadsSpin->setToolTip(tr("Number of threads to allocate, 0 means automatic"));
        connect(_numThreadsSpin, SIGNAL(editingFinished(void)), this, SLOT(handleSpinSelChanged()));
    }

    //priority selection
    {
        formLayout->addRow(tr("Process priority %"), _prioritySpin);
        _prioritySpin->setRange(-100, +100);
        _prioritySpin->setToolTip(tr("A priority percentage between -100% and 100%"));
        connect(_prioritySpin, SIGNAL(editingFinished(void)), this, SLOT(handleSpinSelChanged()));
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
        _yieldModeBox->setToolTip(tr("Yield mode specifies the internal threading mechanisms"));
        connect(_yieldModeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(handleComboChanged(int)));
    }

    this->updateCpuSelection();
}


void AffinityZoneEditor::loadFromConfig(const Poco::JSON::Object::Ptr &config)
{
    if (config->has("color"))
    {
        auto color = QString::fromStdString(config->getValue<std::string>("color"));
        _colorPicker->blockSignals(true);
        _colorPicker->setCurrentColor(QColor(color));
        _colorPicker->blockSignals(false);
    }
    if (config->has("nodeUri"))
    {
        auto uri = QString::fromStdString(config->getValue<std::string>("nodeUri"));
        for (int i = 0; i < _nodesBox->count(); i++)
        {
            if (_nodesBox->itemText(i) == uri) _nodesBox->setCurrentIndex(i);
        }
    }
    if (config->has("processName"))
    {
        auto name = QString::fromStdString(config->getValue<std::string>("processName"));
        _processNameEdit->setText(name);
    }
    if (config->has("numThreads"))
    {
        _numThreadsSpin->setValue(config->getValue<int>("numThreads"));
    }
    if (config->has("priority"))
    {
        _prioritySpin->setValue(int(config->getValue<double>("priority")*100));
    }
    if (config->has("affinityMode") and config->has("affinityMask"))
    {
        auto mode = config->getValue<std::string>("affinityMode");
        auto mask = config->getArray("affinityMask");
        std::vector<size_t> selection;
        for (size_t i = 0; i < mask->size(); i++) selection.push_back(mask->getElement<int>(i));
        _cpuSelection->setup(mode, selection);
    }
    if (config->has("yieldMode"))
    {
        auto mode = QString::fromStdString(config->getValue<std::string>("yieldMode"));
        for (int i = 0; i < _yieldModeBox->count(); i++)
        {
            if (_yieldModeBox->itemData(i).toString() == mode) _yieldModeBox->setCurrentIndex(i);
        }
    }
}

Poco::JSON::Object::Ptr AffinityZoneEditor::getCurrentConfig(void) const
{
    Poco::JSON::Object::Ptr config = new Poco::JSON::Object();
    config->set("color", _colorPicker->currentColor().name().toStdString());
    config->set("nodeUri", _nodesBox->itemText(_nodesBox->currentIndex()).toStdString());
    config->set("processName", _processNameEdit->text().toStdString());
    config->set("numThreads", _numThreadsSpin->value());
    config->set("priority", _prioritySpin->value()/100.0);
    config->set("affinityMode", _cpuSelection->mode());
    Poco::JSON::Array::Ptr affinityMask = new Poco::JSON::Array();
    for (auto num : _cpuSelection->selection()) affinityMask->add(num);
    config->set("affinityMask", affinityMask);
    config->set("yieldMode", _yieldModeBox->itemData(_yieldModeBox->currentIndex()).toString().toStdString());
    return config;
}

void AffinityZoneEditor::updateCpuSelection(void)
{
    //get node info and cache it
    auto uriStr = _nodesBox->itemText(_nodesBox->currentIndex());
    if (_uriToNumaInfo.count(uriStr) == 0) try
    {
        auto env = Pothos::RemoteClient(uriStr.toStdString()).makeEnvironment("managed");
        auto nodeInfos = env->findProxy("Pothos/System/NumaInfo").call<std::vector<Pothos::System::NumaInfo>>("get");
        _uriToNumaInfo[uriStr] = nodeInfos;
    }
    catch (const Pothos::Exception &ex)
    {
        poco_error(Poco::Logger::get("PothosGui.AffinityZoneEditor"), ex.displayText());
        _uriToNumaInfo[uriStr] = std::vector<Pothos::System::NumaInfo>(); //empty
    }

    delete _cpuSelection;
    _cpuSelection = new CpuSelectionWidget(_uriToNumaInfo[uriStr], this);
    connect(_cpuSelection, SIGNAL(selectionChanged(void)), this, SLOT(handleSpinSelChanged(void)));
    _cpuSelectionContainer->addWidget(_cpuSelection);
}
