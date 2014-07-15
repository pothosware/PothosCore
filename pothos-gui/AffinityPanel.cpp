// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QToolTip>
#include <QGroupBox>
#include <QScrollArea>
#include <iostream>

static const int ARBITRARY_MAX_THREADS = 4096;

class AffinityZoneEditor : public QGroupBox
{
    Q_OBJECT
public:
    AffinityZoneEditor(const QString &zoneName, QWidget *parent):
        QGroupBox(zoneName, parent),
        _remoteNodesBox(new QComboBox(this)),
        _numThreadsSpin(new QSpinBox(this)),
        _prioritySpin(new QSpinBox(this)),
        _affinityModeBox(new QComboBox(this)),
        _removeButton(new QPushButton(makeIconFromTheme("list-remove"), tr("Remove"), this))
    {
        //bold title
        this->setStyleSheet("QGroupBox{font-weight: bold;}");

        //setup layout
        auto formLayout = new QFormLayout(this);
        this->setLayout(formLayout);

        //remote nodes
        {
            formLayout->addRow(tr("Remote Node"), _remoteNodesBox);
            _remoteNodesBox->addItems(getRemoteNodeUris());
            connect(_remoteNodesBox, SIGNAL(currentIndexChanged(int)), this, SLOT(somethingChanged(int)));
        }

        //num threads
        {
            formLayout->addRow(tr("Num Threads"), _numThreadsSpin);
            _numThreadsSpin->setRange(1, ARBITRARY_MAX_THREADS);
            connect(_numThreadsSpin, SIGNAL(editingFinished(void)), this, SLOT(somethingChanged()));
        }

        //priority selection
        {
            formLayout->addRow(tr("Process Priority %"), _prioritySpin);
            _prioritySpin->setRange(-100, +100);
            connect(_prioritySpin, SIGNAL(editingFinished(void)), this, SLOT(somethingChanged()));
        }

        //affinity mode
        {
            formLayout->addRow(tr("Affinity Mode"), _affinityModeBox);
            _affinityModeBox->addItem(tr("Use all resources"), "ALL");
            _affinityModeBox->addItem(tr("Specify individual CPUs"), "CPU");
            _affinityModeBox->addItem(tr("Specify NUMA nodes"), "NUMA");
            connect(_affinityModeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(somethingChanged(int)));
        }

        //remove button
        {
            formLayout->addRow(_removeButton);
            connect(_removeButton, SIGNAL(pressed(void)), this, SLOT(handleRemove(void)));
        }
    }

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
        std::cout << " 	editingFinished?\n";
    }

private:

    QComboBox *_remoteNodesBox;
    QSpinBox *_numThreadsSpin;
    QSpinBox *_prioritySpin;
    QComboBox *_affinityModeBox;
    QPushButton *_removeButton;
};

class AffinityPanel : public QWidget
{
    Q_OBJECT
public:
    AffinityPanel(QWidget *parent):
        QWidget(parent),
        _zoneEntry(new QLineEdit(this)),
        _createButton(new QPushButton(makeIconFromTheme("list-add"), tr("Create"), this)),
        _editorsScroll(new QScrollArea(this)),
        _editorsLayout(new QVBoxLayout(_editorsScroll))
    {
        //layout setup
        auto mainLayout = new QVBoxLayout();
        this->setLayout(mainLayout);

        //zone creation area
        {
            auto formLayout = new QFormLayout();
            mainLayout->addLayout(formLayout);
            auto hbox = new QHBoxLayout();
            hbox->addWidget(_zoneEntry);
            hbox->addWidget(_createButton);
            _zoneEntry->setPlaceholderText(tr("The name of a new affinity zone"));
            _createButton->setToolTip(tr("Create a new affinity zone"));
            formLayout->addRow(tr("Zone name: "), hbox);
            connect(_zoneEntry, SIGNAL(returnPressed(void)), this, SLOT(handleCreateZone(void)));
            connect(_createButton, SIGNAL(pressed(void)), this, SLOT(handleCreateZone(void)));
        }

        //editors area
        {
            mainLayout->addWidget(_editorsScroll);
            _editorsScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            _editorsScroll->setWidgetResizable(true);
            _editorsScroll->setWidget(new QWidget(_editorsScroll));
            _editorsScroll->widget()->setLayout(_editorsLayout);
        }

        this->initAffinityZoneEditors();
    }

private slots:
    void handleCreateZone(void)
    {
        auto zoneName = _zoneEntry->text();
        _zoneEntry->setText("");
        if (zoneName.isEmpty()) return;
        //TODO is in existing list?
        this->handleErrorMessage(zoneName);
        _editorsLayout->addWidget(new AffinityZoneEditor(zoneName, this));
        this->saveAffinityZoneEditorsState();
    }

private:

    void ensureDefault(void)
    {
        //FIXME not this way
        _editorsLayout->addWidget(new AffinityZoneEditor("default", this));
    }

    void initAffinityZoneEditors(void)
    {
        //TODO restore from save
        this->ensureDefault();
    }

    void saveAffinityZoneEditorsState(void)
    {
        //TODO save it
        this->ensureDefault();
    }

    void handleAffinityZoneEditor(QObject *)
    {
        //TODO maybe something other than destroyed signal to record this
        //objects are always destroyed
        this->saveAffinityZoneEditorsState();
    }

    void handleErrorMessage(const QString &errMsg)
    {
        QToolTip::showText(_zoneEntry->mapToGlobal(QPoint()), "<font color=\"red\">"+errMsg+"</font>");
    }

    QLineEdit *_zoneEntry;
    QPushButton *_createButton;
    QScrollArea *_editorsScroll;
    QVBoxLayout *_editorsLayout;
};

QWidget *makeAffinityPanel(QWidget *parent)
{
    return new AffinityPanel(parent);
}

#include "AffinityPanel.moc"
