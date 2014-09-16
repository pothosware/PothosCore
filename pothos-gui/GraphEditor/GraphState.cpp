// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //make icon theme
#include "GraphEditor/GraphState.hpp"
#include <QListWidgetItem>
#include <QLabel>
#include <iostream>

GraphState::GraphState(void)
{
    return;
}

GraphState::GraphState(const QString &iconName, const QString &description, const QByteArray &dump):
    iconName(iconName),
    description(description),
    dump(dump)
{
    return;
}

GraphStateManager::GraphStateManager(QWidget *parent):
    QListWidget(parent)
{
    connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(handleItemDoubleClicked(QListWidgetItem *)));
}

GraphStateManager::~GraphStateManager(void)
{
    return;
}


void GraphStateManager::handleItemDoubleClicked(QListWidgetItem *item)
{
    emit this->newStateSelected(_itemToIndex[item]);
}

void GraphStateManager::change(void)
{
    this->clear();
    _itemToIndex.clear();
    for (int i = this->numStates()-1; i >= 0; i--)
    {
        const auto &state = this->getStateAt(i);
        auto item = new QListWidgetItem(this);
        item->setIcon(makeIconFromTheme(state.iconName));
        this->addItem(item);
        auto desc = state.description;
        if (size_t(i) == this->getCurrentIndex()) desc = QString("<b>%1</b>").arg(desc.toHtmlEscaped());
        if (size_t(i) == this->getSavedIndex()) desc = QString("<i>%1</i>").arg(desc.toHtmlEscaped());
        this->setItemWidget(item, new QLabel(desc));
        _itemToIndex[item] = i;
    }
}
