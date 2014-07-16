// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <QString>
#include <QLabel>
#include <QTableWidget>
#include <Pothos/System.hpp>
#include <cstddef>
#include <vector>
#include <set>
#include <map>

/*!
 * A table-based display to select a finite number of integers.
 */
class CpuSelectionWidget : public QWidget
{
    Q_OBJECT
public:
    CpuSelectionWidget(const std::vector<Pothos::System::NumaInfo> &numaInfos, QWidget *parent);

private slots:
    void handleTableItemClicked(QTableWidgetItem *item);

private:

    void update(void);

    std::map<QTableWidgetItem *, QTableWidgetItem *> _cpuItemToNumaNodeItem;
    std::map<QTableWidgetItem *, size_t> _itemToNum;
    std::map<QTableWidgetItem *, bool> _itemToSelected;
    std::set<QTableWidgetItem *> _cpuItems, _nodeItems;
    QTableWidget *_table;
    QLabel *_label;
};
