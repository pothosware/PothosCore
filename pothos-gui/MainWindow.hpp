// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QMainWindow>
#include <QString>
#include <QMap>

class QToolBar;
class QDockWidget;
class QAction;
class QCloseEvent;
class QShowEvent;

class PothosGuiMainWindow : public QMainWindow
{
    Q_OBJECT
public:

    PothosGuiMainWindow(QWidget *parent);

    ~PothosGuiMainWindow(void);

signals:
    void initDone(void);
    void exitBegin(QCloseEvent *);

private slots:

    void handleNewTitleSubtext(const QString &s);
    void handleShowAbout(void);
    void handleShowAboutQt(void);
    void handleColorsDialogAction(void);
    void handleFullScreenViewAction(const bool);

protected:
    void closeEvent(QCloseEvent *event);

    void showEvent(QShowEvent *event);

private:

    void createActions(void);
    QAction *_newAction;
    QAction *_openAction;
    QAction *_saveAction;
    QAction *_saveAsAction;
    QAction *_saveAllAction;
    QAction *_reloadAction;
    QAction *_closeAction;
    QAction *_exitAction;
    QAction *_undoAction;
    QAction *_redoAction;
    QAction *_enableAction;
    QAction *_disableAction;
    QAction *_reevalAction;
    QAction *_cutAction;
    QAction *_copyAction;
    QAction *_pasteAction;
    QAction *_deleteAction;
    QAction *_selectAllAction;
    QAction *_objectPropertiesAction;
    QAction *_graphPropertiesAction;
    QAction *_createGraphPageAction;
    QAction *_renameGraphPageAction;
    QAction *_deleteGraphPageAction;
    QAction *_inputBreakerAction;
    QAction *_outputBreakerAction;
    QAction *_rotateLeftAction;
    QAction *_rotateRightAction;
    QAction *_zoomInAction;
    QAction *_zoomOutAction;
    QAction *_zoomOriginalAction;
    QAction *_showAboutAction;
    QAction *_showAboutQtAction;
    QAction *_findAction;
    QAction *_showGraphConnectionPointsAction;
    QAction *_showGraphBoundingBoxesAction;
    QAction *_showRenderedGraphViewAction;
    QAction *_activateTopologyAction;
    QAction *_showPortNamesAction;
    QAction *_showColorsDialogAction;
    QAction *_incrementAction;
    QAction *_decrementAction;
    QAction *_fullScreenViewAction;
    QMap<QString, QAction *> &_actionMap;

    void createMenus(void);
    QMenu *_fileMenu;
    QMenu *_editMenu;
    QMenu *_executeMenu;
    QMenu *_viewMenu;
    QMenu *_debugMenu;
    QMenu *_helpMenu;
    QMap<QString, QMenu *> &_menuMap;

    void createMainToolBar(void);
    QToolBar *_mainToolBar;
    QDockWidget *_hostExplorerDock;
    QDockWidget *_messageWindowDock;
    QDockWidget *_graphActionsDock;
    QDockWidget *_blockTreeDock;
    QDockWidget *_propertiesPanelDock;
    QDockWidget *_affinityZonesDock;

    //restoring from full screen
    std::map<QWidget *, bool> _widgetToOldVisibility;
};
