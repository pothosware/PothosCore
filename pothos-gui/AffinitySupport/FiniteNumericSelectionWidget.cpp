// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "AffinitySupport/FiniteNumericSelectionWidget.hpp"
#include <QHeaderView>
#include <QVBoxLayout>
#include <algorithm>

static const int COL_MAX = 4;

FiniteNumericSelectionWidget::FiniteNumericSelectionWidget(const size_t size, QWidget *parent):
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
    _table->setRowCount((size+COL_MAX-1)/COL_MAX);
    _table->setColumnCount(COL_MAX);
    for (size_t i = 0; i < size; i++)
    {
        auto item = new QTableWidgetItem(QString("%1").arg(i));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        _itemToSelected[item] = false;
        _itemToNum[item] = i;
        _table->setItem(i/COL_MAX, i%COL_MAX, item);
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

std::vector<size_t> FiniteNumericSelectionWidget::getSelection(void) const
{
    std::vector<size_t> selection;
    for (const auto &pair : _itemToSelected)
    {
        if (pair.second) selection.push_back(_itemToNum.at(pair.first));
    }
    std::sort(selection.begin(), selection.end());
    return selection;
}

void FiniteNumericSelectionWidget::handleTableItemClicked(QTableWidgetItem *item)
{
    if (item == nullptr) return;
    _itemToSelected[item] = not _itemToSelected[item];
    this->update();
    emit this->selectionChanged(this->getSelection());
}

void FiniteNumericSelectionWidget::update(void)
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
