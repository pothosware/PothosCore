// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include "GraphEditor/GraphEditorTabs.hpp"
#include "GraphEditor/GraphEditor.hpp"
#include <QTabWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QAction>
#include <QStandardPaths>
#include <Poco/Logger.h>
#include <iostream>
#include <cassert>

GraphEditorTabs::GraphEditorTabs(QWidget *parent):
    QTabWidget(parent)
{
    this->setTabsClosable(true);
    this->setMovable(true);
    this->setUsesScrollButtons(true);
    this->setTabPosition(QTabWidget::North);
    this->setStyleSheet(
        QString("QTabBar::close-button {image: url(%1);}").arg(makeIconPath("standardbutton-closetab-16.png"))+
        QString("QTabBar::close-button:hover {image: url(%1);}").arg(makeIconPath("standardbutton-closetab-hover-16.png"))+
        QString("QTabBar::close-button:pressed {image: url(%1);}").arg(makeIconPath("standardbutton-closetab-down-16.png")));

    connect(getObjectMap()["mainWindow"], SIGNAL(initDone(void)), this, SLOT(handleInit(void)));
    connect(getObjectMap()["mainWindow"], SIGNAL(exitBegin(QCloseEvent *)), this, SLOT(handleExit(QCloseEvent *)));
    connect(getActionMap()["new"], SIGNAL(triggered(void)), this, SLOT(handleNew(void)));
    connect(getActionMap()["open"], SIGNAL(triggered(void)), this, SLOT(handleOpen(void)));
    connect(getActionMap()["save"], SIGNAL(triggered(void)), this, SLOT(handleSave(void)));
    connect(getActionMap()["saveAs"], SIGNAL(triggered(void)), this, SLOT(handleSaveAs(void)));
    connect(getActionMap()["saveAll"], SIGNAL(triggered(void)), this, SLOT(handleSaveAll(void)));
    connect(getActionMap()["reload"], SIGNAL(triggered(void)), this, SLOT(handleReload(void)));
    connect(getActionMap()["close"], SIGNAL(triggered(void)), this, SLOT(handleClose(void)));
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(handleClose(int)));
}

void GraphEditorTabs::handleInit(void)
{
    this->loadState();
}

void GraphEditorTabs::handleNew(void)
{
    auto editor = makeGraphEditor(this);
    this->addTab(editor, "");
    this->setCurrentWidget(editor);
    dynamic_cast<GraphEditor *>(editor)->load();
    this->saveState();
}

void GraphEditorTabs::doReloadDialog(GraphEditor *editor)
{
    if (not editor->hasUnsavedChanges()) return;

    //yes/no dialog if we have unsaved changes
    const auto reply = QMessageBox::question(this,
        tr("Reload: unsaved changes!"),
        tr("Unsaved changes %1!\nAre you sure that you want to reload?").arg(editor->getCurrentFilePath()),
        QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) editor->load();
}

void GraphEditorTabs::handleOpen(void)
{
    auto lastPath = getSettings().value("GraphEditorTabs/lastFile").toString();
    if(lastPath.isEmpty()) {
        lastPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }
    assert(not lastPath.isEmpty());
    auto filePaths = QFileDialog::getOpenFileNames(this,
                        "Open Files",
                        lastPath,
                        "Pothos Topologies (*.pth)");

    for (const auto &filePath : filePaths)
    {
        getSettings().setValue("GraphEditorTabs/lastFile", filePath);
        this->handleOpen(filePath);
    }
}

void GraphEditorTabs::handleOpen(const QString &filePath)
{
    //filter out files that are already open
    for (int j = 0; j < this->count(); j++)
    {
        auto editor = dynamic_cast<GraphEditor *>(this->widget(j));
        assert(editor != nullptr);
        if (editor->getCurrentFilePath() == filePath)
        {
            return this->doReloadDialog(editor);
        }
    }

    //open a new editor with the specified file
    auto editor = dynamic_cast<GraphEditor *>(makeGraphEditor(this));
    editor->setCurrentFilePath(filePath);
    this->addTab(editor, "");
    editor->load();
    this->setCurrentWidget(editor);

    this->saveState();
}

void GraphEditorTabs::handleSave(void)
{
    auto editor = dynamic_cast<GraphEditor *>(this->currentWidget());
    assert(editor != nullptr);

    //no file path? redirect to save as
    if (editor->getCurrentFilePath().isEmpty()) this->handleSaveAs();

    //otherwise, just save the topology
    else editor->save();
}

void GraphEditorTabs::handleSaveAs(void)
{
    auto editor = dynamic_cast<GraphEditor *>(this->currentWidget());
    assert(editor != nullptr);

    QString lastPath;
    if(editor->getCurrentFilePath().isEmpty())
    {
        auto defaultPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/untitled.pth";
        assert(!defaultPath.isEmpty());
        lastPath = defaultPath;
    }
    else
    {
        lastPath = editor->getCurrentFilePath();
    }
    auto filePath = QFileDialog::getSaveFileName(this,
                        tr("Save As"),
                        lastPath,
                        tr("Pothos Topologies (*.pth)"));
    if(filePath.isEmpty()) return;
    getSettings().setValue("GraphEditorTabs/lastFile", filePath);
    editor->setCurrentFilePath(filePath);
    editor->save();
    this->saveState();
}

void GraphEditorTabs::handleReload(void)
{
    auto editor = dynamic_cast<GraphEditor *>(this->currentWidget());
    assert(editor != nullptr);
    this->doReloadDialog(editor);
}

void GraphEditorTabs::handleSaveAll(void)
{
    for (int i = 0; i < this->count(); i++)
    {
        auto editor = dynamic_cast<GraphEditor *>(this->widget(i));
        assert(editor != nullptr);
    }
    this->saveState();
}

void GraphEditorTabs::handleClose(void)
{
    auto editor = dynamic_cast<GraphEditor *>(this->currentWidget());
    assert(editor != nullptr);
    this->handleClose(editor);
}

void GraphEditorTabs::handleClose(int index)
{
    auto editor = dynamic_cast<GraphEditor *>(this->widget(index));
    assert(editor != nullptr);
    this->handleClose(editor);
}

void GraphEditorTabs::handleClose(GraphEditor *editor)
{
    if (editor->hasUnsavedChanges())
    {
        //yes/no dialog if we have unsaved changes
        const auto reply = QMessageBox::question(this,
            tr("Close: unsaved changes!"),
            tr("Unsaved changes %1!\nWould you like to save changes?").arg(editor->getCurrentFilePath()),
            QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);

        if (reply == QMessageBox::Cancel) return;
        if (reply == QMessageBox::Yes) this->handleSave();
    }
    delete editor;
    this->ensureOneEditor();
    this->saveState();
}

void GraphEditorTabs::handleExit(QCloseEvent *event)
{
    //save the currently selected editor tab
    getSettings().setValue("GraphEditorTabs/activeIndex", this->currentIndex());

    //exit logic -- save changes dialogs
    for (int i = 0; i < this->count(); i++)
    {
        auto editor = dynamic_cast<GraphEditor *>(this->widget(i));
        assert(editor != nullptr);
        if (not editor->hasUnsavedChanges()) continue;

        this->setCurrentIndex(i); //select this editor

        //yes/no dialog if we have unsaved changes
        const auto reply = QMessageBox::question(this,
            tr("Exit: unsaved changes!"),
            tr("Unsaved changes %1!\nWould you like to save changes?").arg(editor->getCurrentFilePath()),
            QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);

        if (reply == QMessageBox::Cancel)
        {
            event->ignore();
            return;
        }
        if (reply == QMessageBox::Yes) this->handleSave();
    }

    event->accept();
}

void GraphEditorTabs::loadState(void)
{
    //load option topologies from file list
    auto files = getSettings().value("GraphEditorTabs/files").toStringList();
    for (int i = 0; i < files.size(); i++)
    {
        if (files.at(i).isEmpty()) continue; //skip empty files
        if (not QFile::exists(files.at(i)))
        {
            poco_error_f1(Poco::Logger::get("PothosGui.GraphEditorTabs.loadState"), "File %s does not exist", files.at(i).toStdString());
            continue;
        }
        auto editor = dynamic_cast<GraphEditor *>(makeGraphEditor(this));
        editor->setCurrentFilePath(files.at(i));
        this->addTab(editor, "");
        editor->load();
    }

    //Nothing? make sure we have at least one editor
    this->ensureOneEditor();

    //restore the active index setting
    this->setCurrentIndex(getSettings().value("GraphEditorTabs/activeIndex").toInt());
}

void GraphEditorTabs::ensureOneEditor(void)
{
    if (this->count() > 0) return;
    this->handleNew();
    this->saveState();
}

void GraphEditorTabs::saveState(void)
{
    //save the file paths for the editors
    QStringList files;
    for (int i = 0; i < this->count(); i++)
    {
        auto editor = dynamic_cast<GraphEditor *>(this->widget(i));
        assert(editor != nullptr);
        files.push_back(editor->getCurrentFilePath());
    }
    getSettings().setValue("GraphEditorTabs/files", files);
}

QWidget *makeGraphEditorTabs(QWidget *parent)
{
    return new GraphEditorTabs(parent);
}
