// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //getObjectMap
#include "BlockTree/BlockTreeTopWindow.hpp"
#include "BlockTree/BlockTreeWidget.hpp"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QDockWidget>
#include <cassert>

 BlockTreeTopWindow::BlockTreeTopWindow(QWidget *parent):
    QWidget(parent),
    _searchBox(new QLineEdit(this))
{
    auto layout = new QVBoxLayout(this);
    this->setLayout(layout);

    _searchBox->setPlaceholderText(tr("Search blocks"));
#if QT_VERSION > 0x050200
    _searchBox->setClearButtonEnabled(true);
#endif
    layout->addWidget(_searchBox);

    _blockTree = new BlockTreeWidget(this);
    connect(getObjectMap()["blockCache"], SIGNAL(blockDescUpdate(const Poco::JSON::Array::Ptr &)),
        _blockTree, SLOT(handleBlockDescUpdate(const Poco::JSON::Array::Ptr &)));
    connect(_blockTree, SIGNAL(blockDescEvent(const Poco::JSON::Object::Ptr &, bool)),
        this, SLOT(handleBlockDescEvent(const Poco::JSON::Object::Ptr &, bool)));
    connect(_searchBox, SIGNAL(textChanged(const QString &)), _blockTree, SLOT(handleFilter(const QString &)));
    layout->addWidget(_blockTree);

    _addButton = new QPushButton(makeIconFromTheme("list-add"), "Add Block", this);
    layout->addWidget(_addButton);
    connect(_addButton, SIGNAL(released(void)), this, SLOT(handleAdd(void)));
    _addButton->setEnabled(false); //default disabled
}

void BlockTreeTopWindow::activateFind(void)
{
    //on ctrl-f or edit:find, set focus on search window and select all text
    _searchBox->setFocus();
    _searchBox->selectAll();
}

void BlockTreeTopWindow::handleAdd(void)
{
    emit addBlockEvent(_blockDesc);
}

void BlockTreeTopWindow::handleBlockDescEvent(const Poco::JSON::Object::Ptr &blockDesc, bool add)
{
    _blockDesc = blockDesc;
    _addButton->setEnabled(bool(blockDesc));
    if (add) emit addBlockEvent(_blockDesc);
}
