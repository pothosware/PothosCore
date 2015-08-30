// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "BlockTree/BlockTreeDock.hpp"
#include "PothosGuiUtils.hpp" //getObjectMap
#include "BlockTree/BlockTreeWidget.hpp"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QDockWidget>
#include <cassert>
#include <QAction>

BlockTreeDock::BlockTreeDock(QWidget *parent):
    QDockWidget(parent),
    _searchBox(new QLineEdit(this))
{
    this->setObjectName("BlockTreeDock");
    this->setWindowTitle(tr("Block Tree"));
    this->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    this->setWidget(new QWidget(this));

    auto layout = new QVBoxLayout(this->widget());
    this->widget()->setLayout(layout);

    _searchBox->setPlaceholderText(tr("Search blocks"));
#if QT_VERSION > 0x050200
    _searchBox->setClearButtonEnabled(true);
#endif
    layout->addWidget(_searchBox);

    _blockTree = new BlockTreeWidget(this->widget());
    connect(getObjectMap()["blockCache"], SIGNAL(blockDescUpdate(const Poco::JSON::Array::Ptr &)),
        _blockTree, SLOT(handleBlockDescUpdate(const Poco::JSON::Array::Ptr &)));
    connect(_blockTree, SIGNAL(blockDescEvent(const Poco::JSON::Object::Ptr &, bool)),
        this, SLOT(handleBlockDescEvent(const Poco::JSON::Object::Ptr &, bool)));
    connect(_searchBox, SIGNAL(textChanged(const QString &)), _blockTree, SLOT(handleFilter(const QString &)));
    layout->addWidget(_blockTree);

    _addButton = new QPushButton(makeIconFromTheme("list-add"), "Add Block", this->widget());
    layout->addWidget(_addButton);
    connect(_addButton, SIGNAL(released(void)), this, SLOT(handleAdd(void)));
    _addButton->setEnabled(false); //default disabled
}

void BlockTreeDock::activateFind(void)
{
    this->show();
    this->raise();
    //on ctrl-f or edit:find, set focus on search window and select all text
    _searchBox->setFocus();
    _searchBox->selectAll();
    _addButton->setEnabled(false);
}

void BlockTreeDock::handleAdd(void)
{
    emit addBlockEvent(_blockDesc);
}

void BlockTreeDock::handleBlockDescEvent(const Poco::JSON::Object::Ptr &blockDesc, bool add)
{
    _blockDesc = blockDesc;
    _addButton->setEnabled(bool(blockDesc));
    if (add) emit addBlockEvent(_blockDesc);
}
