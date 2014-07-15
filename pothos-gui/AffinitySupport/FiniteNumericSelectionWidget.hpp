// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <QString>
#include <QLabel>
#include <QTableWidget>
#include <cstddef>
#include <vector>
#include <map>

/*!
 * A table-based display to select a finite number of integers.
 */
class FiniteNumericSelectionWidget : public QWidget
{
    Q_OBJECT
public:
    FiniteNumericSelectionWidget(const size_t size, QWidget *parent);

    void setSelection(const std::vector<size_t> &selection)
    {
        for (auto &pair : _itemToSelected)
        {
            pair.second = std::find(selection.begin(), selection.end(), _itemToNum.at(pair.first)) != selection.end();
        }
        this->update();
    }

    std::vector<size_t> getSelection(void) const;

signals:
    void selectionChanged(const std::vector<size_t> &);

private slots:
    void handleTableItemClicked(QTableWidgetItem *item);

private:

    void update(void);

    std::map<QTableWidgetItem *, size_t> _itemToNum;
    std::map<QTableWidgetItem *, bool> _itemToSelected;
    QTableWidget *_table;
    QLabel *_label;
};
