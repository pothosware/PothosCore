// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MainWindow.hpp"
#include "PothosGuiUtils.hpp" //object and action maps, settings
#include <Pothos/System.hpp>
#include "BlockTree/BlockCache.hpp"
#include "BlockTree/BlockTreeDock.hpp"
#include "PropertiesPanel/PropertiesPanelDock.hpp"
#include "GraphEditor/GraphEditorTabs.hpp"
#include "GraphEditor/GraphActionsDock.hpp"
#include "HostExplorer/HostExplorerDock.hpp"
#include "AffinitySupport/AffinityZonesDock.hpp"
#include "MessageWindow/MessageWindowDock.hpp"
#include "ColorUtils/ColorsDialog.hpp"
#include <QMainWindow>
#include <QGridLayout>
#include <QSettings>
#include <QDockWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QTabWidget>
#include <QMessageBox>
#include <QMap>
#include <Poco/Logger.h>
#include <iostream>

PothosGuiMainWindow::PothosGuiMainWindow(QWidget *parent):
    QMainWindow(parent),
    _actionMap(getActionMap()),
    _menuMap(getMenuMap())
{
    getObjectMap()["mainWindow"] = this;

    this->setMinimumSize(800, 600);
    this->setWindowTitle("Pothos GUI");

    //initialize actions and action buttons
    this->createActions();
    this->createMainToolBar();

    //create message window dock
    _messageWindowDock = new MessageWindowDock(this);
    this->addDockWidget(Qt::BottomDockWidgetArea, _messageWindowDock);
    getObjectMap()["messageWindowDock"] = _messageWindowDock;

    //create graph actions dock
    _graphActionsDock = new GraphActionsDock(this);
    this->addDockWidget(Qt::BottomDockWidgetArea, _graphActionsDock);
    getObjectMap()["graphActionsDock"] = _graphActionsDock;

    //create host explorer dock
    _hostExplorerDock = new HostExplorerDock(this);
    getObjectMap()["hostExplorerDock"] = _hostExplorerDock;
    this->addDockWidget(Qt::RightDockWidgetArea, _hostExplorerDock);

    //create affinity panel
    _affinityZonesDock = new AffinityZonesDock(this);
    getObjectMap()["affinityZonesDock"] = _affinityZonesDock;
    this->tabifyDockWidget(_hostExplorerDock, _affinityZonesDock);

    //block cache (make before block tree)
    auto blockCache = new BlockCache(this);
    getObjectMap()["blockCache"] = blockCache;
    connect(this, SIGNAL(initDone(void)), blockCache, SLOT(handleUpdate(void)));

    //create topology editor tabbed widget
    auto editorTabs = new GraphEditorTabs(this);
    this->setCentralWidget(editorTabs);
    getObjectMap()["editorTabs"] = editorTabs;

    //create block tree (after the block cache)
    _blockTreeDock = new BlockTreeDock(this);
    connect(getActionMap()["find"], SIGNAL(triggered(void)), _blockTreeDock, SLOT(activateFind(void)));
    getObjectMap()["blockTreeDock"] = _blockTreeDock;
    this->tabifyDockWidget(_affinityZonesDock, _blockTreeDock);

    //create properties panel (make after block cache)
    _propertiesPanelDock = new PropertiesPanelDock(this);
    getObjectMap()["propertiesPanel"] = _propertiesPanelDock;
    this->tabifyDockWidget(_blockTreeDock, _propertiesPanelDock);

    //restore main window settings from file
    this->restoreGeometry(getSettings().value("MainWindow/geometry").toByteArray());
    this->restoreState(getSettings().value("MainWindow/state").toByteArray());
    _propertiesPanelDock->hide(); //hidden until used
    _showPortNamesAction->setChecked(getSettings().value("MainWindow/showPortNames", true).toBool());

    //create menus after docks and tool bars (view menu calls their toggleViewAction())
    this->createMenus();

    //we do this last so all of the connections and logging is setup
    poco_information(Poco::Logger::get("PothosGui.MainWindow"), "Initialization complete");
    emit this->initDone();
}

PothosGuiMainWindow::~PothosGuiMainWindow(void)
{
    getSettings().setValue("MainWindow/geometry", saveGeometry());
    getSettings().setValue("MainWindow/state", saveState());
    getSettings().setValue("MainWindow/showPortNames", _showPortNamesAction->isChecked());
}

void PothosGuiMainWindow::handleNewTitleSubtext(const QString &s)
{
    this->setWindowTitle("Pothos GUI - " + s);
}

void PothosGuiMainWindow::handleShowAbout(void)
{
    QMessageBox::about(this, "About Pothos", QString::fromStdString(Pothos::System::getApiVersion()));
}

void PothosGuiMainWindow::handleShowAboutQt(void)
{
    QMessageBox::aboutQt(this);
}

void PothosGuiMainWindow::handleColorsDialogAction(void)
{
    auto dialog = new ColorsDialog(this);
    dialog->exec();
    delete dialog;
}

void PothosGuiMainWindow::closeEvent(QCloseEvent *event)
{
    emit this->exitBegin(event);
}

void PothosGuiMainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
}

void PothosGuiMainWindow::createActions(void)
{
    _newAction = new QAction(makeIconFromTheme("document-new"), tr("&New"), this);
    _newAction->setShortcut(QKeySequence::New);
    _actionMap["new"] = _newAction;

    _openAction = new QAction(makeIconFromTheme("document-open"), tr("&Open"), this);
    _openAction->setShortcut(QKeySequence::Open);
    _actionMap["open"] = _openAction;

    _saveAction = new QAction(makeIconFromTheme("document-save"), tr("&Save"), this);
    _saveAction->setShortcut(QKeySequence::Save);
    _actionMap["save"] = _saveAction;

    _saveAsAction = new QAction(makeIconFromTheme("document-save-as"), tr("Save &As"), this);
    _saveAsAction->setShortcut(QKeySequence::SaveAs);
    _actionMap["saveAs"] = _saveAsAction;

    _saveAllAction = new QAction(makeIconFromTheme("document-save-all"), tr("Save A&ll"), this);
    _saveAllAction->setShortcut(QKeySequence("CTRL+SHIFT+A"));
    _actionMap["saveAll"] = _saveAllAction;

    _reloadAction = new QAction(makeIconFromTheme("view-refresh"), tr("&Reload"), this);
    QList<QKeySequence> reloadShortcuts;
    reloadShortcuts.push_back(QKeySequence::Refresh);
    reloadShortcuts.push_back(QKeySequence("CTRL+R"));
    if (reloadShortcuts.front().matches(reloadShortcuts.back()) == QKeySequence::ExactMatch) reloadShortcuts.pop_back();
    _reloadAction->setShortcuts(reloadShortcuts);
    _actionMap["reload"] = _reloadAction;

    _closeAction = new QAction(makeIconFromTheme("document-close"), tr("&Close"), this);
    _closeAction->setShortcut(QKeySequence::Close);
    _actionMap["close"] = _closeAction;

    _exitAction = new QAction(makeIconFromTheme("application-exit"), tr("&Exit Pothos GUI"), this);
    _exitAction->setShortcut(QKeySequence::Quit);
    connect(_exitAction, SIGNAL(triggered(void)), this, SLOT(close(void)));
    _actionMap["exit"] = _exitAction;

    _undoAction = new QAction(makeIconFromTheme("edit-undo"), tr("&Undo"), this);
    _undoAction->setShortcut(QKeySequence::Undo);
    _actionMap["undo"] = _undoAction;

    _redoAction = new QAction(makeIconFromTheme("edit-redo"), tr("&Redo"), this);
    QList<QKeySequence> redoShortcuts;
    redoShortcuts.push_back(QKeySequence::Redo);
    redoShortcuts.push_back(QKeySequence("CTRL+Y"));
    if (redoShortcuts.front().matches(redoShortcuts.back()) == QKeySequence::ExactMatch) redoShortcuts.pop_back();
    _redoAction->setShortcuts(redoShortcuts);
    _actionMap["redo"] = _redoAction;

    _cutAction = new QAction(makeIconFromTheme("edit-cut"), tr("Cu&t"), this);
    _cutAction->setShortcut(QKeySequence::Cut);
    _actionMap["cut"] = _cutAction;

    _copyAction = new QAction(makeIconFromTheme("edit-copy"), tr("&Copy"), this);
    _copyAction->setShortcut(QKeySequence::Copy);
    _actionMap["copy"] = _copyAction;

    _pasteAction = new QAction(makeIconFromTheme("edit-paste"), tr("&Paste"), this);
    _pasteAction->setShortcut(QKeySequence::Paste);
    _actionMap["paste"] = _pasteAction;

    _deleteAction = new QAction(makeIconFromTheme("edit-delete"), tr("&Delete"), this);
    _deleteAction->setShortcut(QKeySequence::Delete);
    _actionMap["delete"] = _deleteAction;

    _selectAllAction = new QAction(makeIconFromTheme("edit-select-all"), tr("Select &All"), this);
    _selectAllAction->setShortcut(QKeySequence::SelectAll);
    _actionMap["selectAll"] = _selectAllAction;

    _propertiesAction = new QAction(makeIconFromTheme("document-properties"), tr("Properti&es"), this);
    _selectAllAction->setShortcut(QKeySequence("Enter"));
    _actionMap["properties"] = _propertiesAction;

    _createGraphPageAction = new QAction(makeIconFromTheme("document-new"), tr("Create new graph page"), this);
    _actionMap["createGraphPage"] = _createGraphPageAction;

    _renameGraphPageAction = new QAction(makeIconFromTheme("edit-rename"), tr("Rename this graph page"), this);
    _actionMap["renameGraphPage"] = _renameGraphPageAction;

    _deleteGraphPageAction = new QAction(makeIconFromTheme("edit-delete"), tr("Delete this graph page"), this);
    _actionMap["deleteGraphPage"] = _deleteGraphPageAction;

    _createInputBreakerAction = new QAction(makeIconFromTheme("document-new"), tr("Create input breaker"), this);
    _actionMap["createInputBreaker"] = _createInputBreakerAction;

    _createOutputBreakerAction = new QAction(makeIconFromTheme("document-new"), tr("Create output breaker"), this);
    _actionMap["createOutputBreaker"] = _createOutputBreakerAction;

    _rotateLeftAction = new QAction(makeIconFromTheme("object-rotate-left"), tr("Rotate Left"), this);
    _rotateLeftAction->setShortcut(QKeySequence("Left"));
    _actionMap["rotateLeft"] = _rotateLeftAction;

    _rotateRightAction = new QAction(makeIconFromTheme("object-rotate-right"), tr("Rotate Right"), this);
    _rotateRightAction->setShortcut(QKeySequence("Right"));
    _actionMap["rotateRight"] = _rotateRightAction;

    _zoomInAction = new QAction(makeIconFromTheme("zoom-in"), tr("Zoom in"), this);
    _zoomInAction->setShortcut(QKeySequence::ZoomIn);
    _actionMap["zoomIn"] = _zoomInAction;

    _zoomOutAction = new QAction(makeIconFromTheme("zoom-out"), tr("Zoom out"), this);
    _zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    _actionMap["zoomOut"] = _zoomOutAction;

    _zoomOriginalAction = new QAction(makeIconFromTheme("zoom-original"), tr("Normal size"), this);
    _zoomOriginalAction->setShortcut(QKeySequence("CTRL+0"));
    _actionMap["zoomOriginal"] = _zoomOriginalAction;

    _findAction = new QAction(makeIconFromTheme("edit-find"), tr("&Find"), this);
    _findAction->setShortcut(QKeySequence::Find);
    _actionMap["find"] = _findAction;

    _showGraphConnectionPointsAction = new QAction(tr("Show graph &connection points"), this);
    _showGraphConnectionPointsAction->setCheckable(true);
    _actionMap["showGraphConnectionPoints"] = _showGraphConnectionPointsAction;

    _showGraphBoundingBoxesAction = new QAction(tr("Show graph &bounding boxes"), this);
    _showGraphBoundingBoxesAction->setCheckable(true);
    _actionMap["showGraphBoundingBoxes"] = _showGraphBoundingBoxesAction;

    _showGraphFlattenedViewAction = new QAction(tr("Show graph &flattened view"), this);
    _actionMap["showGraphFlattenedView"] = _showGraphFlattenedViewAction;

    _activateTopologyAction = new QAction(makeIconFromTheme("run-build"), tr("&Activate topology"), this);
    _activateTopologyAction->setCheckable(true);
    _actionMap["activateTopology"] = _activateTopologyAction;

    _showPortNamesAction = new QAction(tr("Show &port names"), this);
    _showPortNamesAction->setCheckable(true);
    _actionMap["showPortNamesAction"] = _showPortNamesAction;

    _showAboutAction = new QAction(makeIconFromTheme("help-about"), tr("&About Pothos"), this);
    _showAboutAction->setStatusTip(tr("Information about this version of Pothos"));
    connect(_showAboutAction, SIGNAL(triggered(void)), this, SLOT(handleShowAbout(void)));

    _showAboutQtAction = new QAction(makeIconFromTheme("help-about"), tr("About &Qt"), this);
    _showAboutQtAction->setStatusTip(tr("Information about this version of QT"));
    connect(_showAboutQtAction, SIGNAL(triggered(void)), this, SLOT(handleShowAboutQt(void)));

    _showColorsDialogAction = new QAction(makeIconFromTheme("color-picker"), tr("&Colors Map"), this);
    _showColorsDialogAction->setStatusTip(tr("Data type colors used for block proprties and ports"));
    connect(_showColorsDialogAction, SIGNAL(triggered(void)), this, SLOT(handleColorsDialogAction(void)));
}

void PothosGuiMainWindow::createMenus(void)
{
    _fileMenu = menuBar()->addMenu(tr("&File"));
    _menuMap["file"] = _fileMenu;
    _fileMenu->addAction(_newAction);
    _fileMenu->addAction(_openAction);
    _fileMenu->addAction(_saveAction);
    _fileMenu->addAction(_saveAsAction);
    _fileMenu->addAction(_saveAllAction);
    _fileMenu->addAction(_reloadAction);
    _fileMenu->addAction(_closeAction);
    _fileMenu->addSeparator();
    _fileMenu->addAction(_exitAction);

    _editMenu = menuBar()->addMenu(tr("&Edit"));
    _menuMap["edit"] = _editMenu;
    _editMenu->addAction(_undoAction);
    _editMenu->addAction(_redoAction);
    _editMenu->addSeparator();
    _editMenu->addAction(_cutAction);
    _editMenu->addAction(_copyAction);
    _editMenu->addAction(_pasteAction);
    _editMenu->addAction(_deleteAction);
    _editMenu->addSeparator();
    _editMenu->addAction(_selectAllAction);
    _editMenu->addSeparator();
    _editMenu->addAction(_findAction);
    _editMenu->addSeparator();
    _editMenu->addAction(_createGraphPageAction);
    _editMenu->addAction(_renameGraphPageAction);
    _editMenu->addAction(_deleteGraphPageAction);
    _menuMap["moveGraphObjects"] = _editMenu->addMenu(makeIconFromTheme("transform-move"), tr("Move selected graph objects..."));
    _menuMap["setAffinityZone"] = dynamic_cast<AffinityZonesDock *>(_affinityZonesDock)->makeMenu(_editMenu);
    _editMenu->addMenu(_menuMap["setAffinityZone"]);
    _editMenu->addSeparator();
    _editMenu->addAction(_createInputBreakerAction);
    _editMenu->addAction(_createOutputBreakerAction);
    _editMenu->addSeparator();
    _editMenu->addAction(_rotateLeftAction);
    _editMenu->addAction(_rotateRightAction);
    _editMenu->addAction(_propertiesAction);

    _executeMenu = menuBar()->addMenu(tr("&Execute"));
    _executeMenu->addSeparator();
    _executeMenu->addAction(_activateTopologyAction);
    _executeMenu->addAction(_showGraphFlattenedViewAction);

    _viewMenu = menuBar()->addMenu(tr("&View"));
    _menuMap["view"] = _viewMenu;
    _viewMenu->addAction(_hostExplorerDock->toggleViewAction());
    _viewMenu->addAction(_messageWindowDock->toggleViewAction());
    _viewMenu->addAction(_graphActionsDock->toggleViewAction());
    _viewMenu->addAction(_blockTreeDock->toggleViewAction());
    _viewMenu->addAction(_affinityZonesDock->toggleViewAction());
    _viewMenu->addAction(_mainToolBar->toggleViewAction());
    _viewMenu->addSeparator();
    _viewMenu->addAction(_zoomInAction);
    _viewMenu->addAction(_zoomOutAction);
    _viewMenu->addAction(_zoomOriginalAction);
    _viewMenu->addSeparator();
    _viewMenu->addAction(_showPortNamesAction);
    _viewMenu->addSeparator();

    _debugMenu = _viewMenu->addMenu(tr("&Debug"));
    _debugMenu->addAction(_showGraphConnectionPointsAction);
    _debugMenu->addAction(_showGraphBoundingBoxesAction);

    _helpMenu = menuBar()->addMenu(tr("&Help"));
    _menuMap["help"] = _helpMenu;
    _helpMenu->addAction(_showAboutAction);
    _helpMenu->addAction(_showAboutQtAction);
    _helpMenu->addAction(_showColorsDialogAction);
}

void PothosGuiMainWindow::createMainToolBar(void)
{
    _mainToolBar = new QToolBar(tr("Main Tool Bar"), this);
    _mainToolBar->setObjectName("MainToolBar");
    this->addToolBar(_mainToolBar);

    _mainToolBar->addAction(_newAction);
    _mainToolBar->addAction(_openAction);
    _mainToolBar->addAction(_saveAction);
    _mainToolBar->addAction(_saveAsAction);
    _mainToolBar->addAction(_saveAllAction);
    _mainToolBar->addAction(_reloadAction);
    _mainToolBar->addAction(_closeAction);
    _mainToolBar->addSeparator();

    _mainToolBar->addAction(_zoomInAction);
    _mainToolBar->addAction(_zoomOutAction);
    _mainToolBar->addAction(_zoomOriginalAction);
    _mainToolBar->addSeparator();

    _mainToolBar->addAction(_undoAction);
    _mainToolBar->addAction(_redoAction);
    _mainToolBar->addSeparator();

    _mainToolBar->addAction(_activateTopologyAction);
    _mainToolBar->addSeparator();

    _mainToolBar->addAction(_cutAction);
    _mainToolBar->addAction(_copyAction);
    _mainToolBar->addAction(_pasteAction);
    _mainToolBar->addAction(_deleteAction);
    _mainToolBar->addSeparator();

    _mainToolBar->addAction(_selectAllAction);
    _mainToolBar->addSeparator();

    _mainToolBar->addAction(_findAction);
    _mainToolBar->addSeparator();

    _mainToolBar->addAction(_rotateLeftAction);
    _mainToolBar->addAction(_rotateRightAction);
    _mainToolBar->addAction(_propertiesAction);
}
