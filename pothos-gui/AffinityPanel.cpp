// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QToolTip>
#include <QLabel>
#include <QSlider>
#include <QGroupBox>
#include <QScrollArea>

class AffinityZoneEditor : public QGroupBox
{
    Q_OBJECT
public:
    AffinityZoneEditor(const QString &zoneName, QWidget *parent):
        QGroupBox(zoneName, parent),
        _remoteNodesBox(new QComboBox(this)),
        _prioritySlider(new QSlider(Qt::Horizontal, this)),
        _priorityEdit(new QLineEdit(this)),
        _affinityModeBox(new QComboBox(this)),
        _removeButton(new QPushButton(makeIconFromTheme("list-remove"), tr("Remove"), this))
    {
        auto formLayout = new QFormLayout(this);
        this->setLayout(formLayout);

        //remote nodes
        {
            formLayout->addRow(tr("Remote Node"), _remoteNodesBox);
            _remoteNodesBox->addItems(getRemoteNodeUris());
        }

        //priority selection
        {
            auto hbox = new QHBoxLayout();
            hbox->addWidget(_prioritySlider);
            hbox->addWidget(_priorityEdit);
            formLayout->addRow(tr("Process Priority"), hbox);
            connect(_prioritySlider, SIGNAL(sliderMoved(int)), this, SLOT(handlePrioritySlider(int)));
            connect(_priorityEdit, SIGNAL(returnPressed(void)), this, SLOT(handlePriorityEdit(void)));
        }

        //affinity mode
        {
            formLayout->addRow(tr("Affinity Mode"), _affinityModeBox);
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

    void handlePrioritySlider(int value)
    {
        
    }

    void handlePriorityEdit(void)
    {
        
    }

private:
    QComboBox *_remoteNodesBox;
    QSlider *_prioritySlider;
    QLineEdit *_priorityEdit;
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

    void initAffinityZoneEditors(void)
    {
        
    }

    void saveAffinityZoneEditorsState(void)
    {
        
    }

    void handleAffinityZoneEditor(QObject *)
    {
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
