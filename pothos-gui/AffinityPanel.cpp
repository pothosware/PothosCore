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
#define QT_QTCOLORPICKER_IMPORT
#include <QtColorPicker>
#include <QTableWidget>
#include <QHeaderView>
#include <algorithm>
#include <iostream>

static const int ARBITRARY_MAX_THREADS = 4096;

class FiniteNumericSelectionWidget : public QGroupBox
{
    Q_OBJECT
public:
    FiniteNumericSelectionWidget(const size_t size, const QString &name, QWidget *parent):
        QGroupBox(name, parent),
        _table(new QTableWidget(this)),
        _label(new QLabel(this))
    {
        auto layout = new QVBoxLayout(this);
        this->setLayout(layout);
        layout->addWidget(_table);
        layout->addWidget(_label);

        _table->verticalHeader()->hide();
        _table->horizontalHeader()->hide();
        _table->setRowCount(1);
        _table->setColumnCount(size);
        for (size_t i = 0; i < size; i++)
        {
            auto item = new QTableWidgetItem(QString("%1").arg(i));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            _itemToSelected[item] = false;
            _itemToNum[item] = i;
            _table->setItem(0, i, item);
        }
        _table->resizeColumnsToContents();
        _table->resizeRowsToContents();
        connect(_table, SIGNAL(itemPressed(QTableWidgetItem *)), this, SLOT(handleTableItemClicked(QTableWidgetItem *)));
        this->update();

        //table resize to exact fit
        _table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        _table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        int w = 0, h = 0;
        for (int i = 0; i < _table->columnCount(); i++) w += _table->columnWidth(i);
        for (int i = 0; i < _table->rowCount(); i++) h += _table->rowHeight(i);
        _table->setMaximumSize(w, h);
    }

    void setSelection(const std::vector<size_t> &selection)
    {
        for (auto &pair : _itemToSelected)
        {
            pair.second = std::find(selection.begin(), selection.end(), _itemToNum.at(pair.first)) != selection.end();
        }
        this->update();
    }

    std::vector<size_t> getSelection(void) const
    {
        std::vector<size_t> selection;
        for (const auto &pair : _itemToSelected)
        {
            if (pair.second) selection.push_back(_itemToNum.at(pair.first));
        }
        std::sort(selection.begin(), selection.end());
        return selection;
    }

signals:
    void selectionChanged(const std::vector<size_t> &);

private slots:
    void handleTableItemClicked(QTableWidgetItem *item)
    {
        _itemToSelected[item] = not _itemToSelected[item];
        this->update();
        emit this->selectionChanged(this->getSelection());
    }

private:

    void update(void)
    {
        for (const auto &pair : _itemToSelected)
        {
            pair.first->setBackgroundColor((pair.second)?Qt::green:Qt::white);
            pair.first->setSelected(false);
        }
        QString t;
        for (auto num : this->getSelection())
        {
            if (not t.isEmpty()) t += ", ";
            t += QString("%1").arg(num);
        }
        _label->setText(t);
    }

    std::map<QTableWidgetItem *, size_t> _itemToNum;
    std::map<QTableWidgetItem *, bool> _itemToSelected;
    QTableWidget *_table;
    QLabel *_label;
};

class AffinityZoneEditor : public QGroupBox
{
    Q_OBJECT
public:
    AffinityZoneEditor(const QString &zoneName, QWidget *parent):
        QGroupBox(zoneName, parent),
        _colorPicker(new QtColorPicker(this)),
        _remoteNodesBox(new QComboBox(this)),
        _processNameEdit(new QLineEdit(this)),
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
            auto _cpuTable = new FiniteNumericSelectionWidget(8, tr("CPU Selection"), this);
            formLayout->addRow(_cpuTable);
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

    QtColorPicker *_colorPicker;
    QComboBox *_remoteNodesBox;
    QLineEdit *_processNameEdit;
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

        //editors area
        {
            mainLayout->addWidget(_editorsScroll);
            _editorsScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            _editorsScroll->setWidgetResizable(true);
            _editorsScroll->setWidget(new QWidget(_editorsScroll));
            _editorsScroll->widget()->setLayout(_editorsLayout);
        }

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
        auto names = getSettings().value("AffinityZones/zoneNames").toStringList();
        for (const auto &name : names)
        {
            _editorsLayout->addWidget(new AffinityZoneEditor(name, this));
        }
        if (names.isEmpty()) this->ensureDefault();
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
