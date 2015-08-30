// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QWidget>
#include <QString>
#include <Pothos/System.hpp>
#include <cstddef>
#include <vector>
#include <set>
#include <map>

class QLabel;
class QTableWidget;
class QTableWidgetItem;

/*!
 * A table-based display to select a finite number of integers.
 */
class CpuSelectionWidget : public QWidget
{
    Q_OBJECT
public:
    CpuSelectionWidget(const std::vector<Pothos::System::NumaInfo> &numaInfos, QWidget *parent);

    void setup(const std::string &mode, const std::vector<size_t> &selection)
    {
        for (auto &pair : _itemToSelected) pair.second = false; //unselect all
        if (mode == "NUMA")
        {
            for (const auto &item : _nodeItems)
            {
                _itemToSelected[item] = std::find(selection.begin(), selection.end(), _itemToNum[item]) != selection.end();
            }
        }
        if (mode == "CPU")
        {
            for (const auto &item : _cpuItems)
            {
                _itemToSelected[item] = std::find(selection.begin(), selection.end(), _itemToNum[item]) != selection.end();
            }
        }
        this->update();
    }

    //! get affinity mode for thread pool args
    std::string mode(void) const
    {
        for (const auto &item : _nodeItems)
        {
            if (_itemToSelected.at(item)) return "NUMA";
        }
        for (const auto &item : _cpuItems)
        {
            if (_itemToSelected.at(item)) return "CPU";
        }
        return "ALL";
    }

    //! get affinity selection for thread pool args
    std::vector<size_t> selection(void) const
    {
        std::vector<size_t> nums;
        if (this->mode() == "NUMA")
        {
            for (const auto &item : _nodeItems)
            {
                if (_itemToSelected.at(item)) nums.push_back(_itemToNum.at(item));
            }
        }
        if (this->mode() == "CPU")
        {
            for (const auto &item : _cpuItems)
            {
                if (_itemToSelected.at(item)) nums.push_back(_itemToNum.at(item));
            }
        }
        return nums;
    }

signals:
    void selectionChanged(void);

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
