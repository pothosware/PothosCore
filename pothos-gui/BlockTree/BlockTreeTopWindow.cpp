// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include "BlockTree/BlockTreeTopWindow.hpp"
#include "BlockTree/BlockTreeWidget.hpp"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QDockWidget>
#include <QAction>
#include <cassert>

 BlockTreeTopWindow::BlockTreeTopWindow(QWidget *parent):
    QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    this->setLayout(layout);

    auto search = new QLineEdit(this);
    search->setPlaceholderText(tr("Search blocks"));
#if QT_VERSION > 0x050200
    search->setClearButtonEnabled(true);
#endif
    layout->addWidget(search);

    _blockTree = new BlockTreeWidget(this);
    connect(getObjectMap()["blockCache"], SIGNAL(blockDescUpdate(const Poco::JSON::Array::Ptr &)),
        _blockTree, SLOT(handleBlockDescUpdate(const Poco::JSON::Array::Ptr &)));
    connect(_blockTree, SIGNAL(blockDescEvent(const Poco::JSON::Object::Ptr &, bool)),
        this, SLOT(handleBlockDescEvent(const Poco::JSON::Object::Ptr &, bool)));
    connect(search, SIGNAL(textChanged(const QString &)), _blockTree, SLOT(handleFilter(const QString &)));
    layout->addWidget(_blockTree);

    _addButton = new QPushButton(makeIconFromTheme("list-add"), "Add Block", this);
    layout->addWidget(_addButton);
    connect(_addButton, SIGNAL(released(void)), this, SLOT(handleAdd(void)));
    _addButton->setEnabled(false); //default disabled

    //on ctrl-f or edit:find, set focus on search window and select all text
    connect(getActionMap()["find"], SIGNAL(triggered(void)), search, SLOT(setFocus(void)));
    connect(getActionMap()["find"], SIGNAL(triggered(void)), search, SLOT(selectAll(void)));
    auto dock = dynamic_cast<QDockWidget *>(parent);
    assert(dock != nullptr);
    connect(getActionMap()["find"], SIGNAL(triggered(void)), dock, SLOT(show(void)));
    connect(getActionMap()["find"], SIGNAL(triggered(void)), dock, SLOT(raise(void)));
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

QWidget *makeBlockTree(QWidget *parent)
{
    return new BlockTreeTopWindow(parent);
}
