// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "AffinitySupport/CpuSelectionWidget.hpp"
#include <QHeaderView>
#include <QVBoxLayout>
#include <algorithm>

static const int COL_MAX = 4;

CpuSelectionWidget::CpuSelectionWidget(const std::vector<Pothos::System::NumaInfo> &numaInfos, QWidget *parent):
    QWidget(parent),
    _table(new QTableWidget(this)),
    _label(new QLabel(this))
{
    auto layout = new QVBoxLayout(this);
    this->setLayout(layout);
    layout->addWidget(_table);
    layout->addWidget(_label);

    _table->verticalHeader()->hide();
    _table->horizontalHeader()->hide();
    _table->setColumnCount(COL_MAX+1);
    int row = 0;
    for (const auto &info : numaInfos)
    {
        int col = 0;
        auto nodeItem = new QTableWidgetItem(QString("Node %1").arg(info.nodeNumber));
        _itemToSelected[nodeItem] = false;
        _itemToNum[nodeItem] = info.nodeNumber;
        _nodeItems.insert(nodeItem);
        _table->setRowCount(row+1);
        _table->setItem(row, col++, nodeItem);

        for (auto  cpuIndex : info.cpus)
        {
            auto item = new QTableWidgetItem(QString("%1").arg(cpuIndex));
            _itemToSelected[item] = false;
            _itemToNum[item] = cpuIndex;
            _cpuItemToNumaNodeItem[item] = nodeItem;
            _cpuItems.insert(item);
            _table->setRowCount(row+1);
            _table->setItem(row, col++, item);
            if (col == COL_MAX+1)
            {
                col = 1;
                row++;
            }
        }
        if (col != 1) row++;
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

void CpuSelectionWidget::handleTableItemClicked(QTableWidgetItem *item)
{
    if (item == nullptr) return;
    _itemToSelected[item] = not _itemToSelected[item];
    emit this->selectionChanged();
    this->update();
}

void CpuSelectionWidget::update(void)
{
    //is a numa node selected?
    bool numaNodeSelected = false;
    for (const auto &nodeItem : _nodeItems)
    {
        if (_itemToSelected[nodeItem]) numaNodeSelected = true;
    }

    //if any numa node selected -- select all cpus based on numa node selection
    if (numaNodeSelected) for (const auto &nodeItem : _nodeItems)
    {
        for (const auto &cpuItem : _cpuItems)
        {
            if (_cpuItemToNumaNodeItem[cpuItem] == nodeItem)
            {
                _itemToSelected[cpuItem] = _itemToSelected[nodeItem];
            }
        }
    }

    //recolor and select every block
    for (const auto &pair : _itemToSelected)
    {
        if (numaNodeSelected and _cpuItems.count(pair.first) != 0) pair.first->setFlags(Qt::NoItemFlags);
        else pair.first->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        pair.first->setBackgroundColor((pair.second)?Qt::green:Qt::white);
        pair.first->setSelected(false);
    }

    //a helpful label
    QString t;
    for (const auto &cpuItem : _cpuItems)
    {
        if (not _itemToSelected[cpuItem]) continue;
        if (not t.isEmpty()) t += ", ";
        t += QString("%1").arg(_itemToNum[cpuItem]);
    }
    if (t.isEmpty()) t = tr("No selection");
    _label->setText(t);
}
