// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include "GraphObjects/GraphObject.hpp"
#include "GraphEditor/GraphState.hpp"
#include <Poco/JSON/Object.h>
#include <QTabWidget>
#include <QPointer>
#include <ios>

class GraphConnection;
class GraphDraw;
class QSignalMapper;
class QTabWidget;
class EvalEngine;

class GraphEditor : public QTabWidget
{
    Q_OBJECT
public:
    GraphEditor(QWidget *parent);
    ~GraphEditor(void);

    void dumpState(std::ostream &os) const;

    void loadState(std::istream &os);

    QString newId(const QString &hint = "") const;

    //! Serializes the editor and saves to file.
    void save(void);

    //! Deserializes the editor from the file.
    void load(void);

    const QString &getCurrentFilePath(void) const
    {
        return _currentFilePath;
    }

    void setCurrentFilePath(const QString &path)
    {
        _currentFilePath = path;
    }

    bool hasUnsavedChanges(void) const
    {
        return not _stateManager->isCurrentSaved();
    }

    void handleAddBlock(const Poco::JSON::Object::Ptr &, const QPointF &);

    //! force a re-rendering of the graph page
    void render(void);

    GraphDraw *getGraphDraw(const int index) const;

    GraphDraw *getCurrentGraphDraw(void) const;

    //! Get a list of all graph objects in all pages
    GraphObjectList getGraphObjects(const int selectionFlags = ~0) const;

    //! Get the graph object with the specified ID or nullptr
    GraphObject *getObjectById(const QString &id, const int selectionFlags = ~0);

    //! Make a connection between two endpoints
    GraphConnection *makeConnection(const GraphConnectionEndpoint &ep0, const GraphConnectionEndpoint &ep1);

    //! clear all globals
    void clearGlobals(void);

    //! Set a global variable and its expression -- overwrite existing
    void setGlobalExpression(const QString &name, const QString &expression);

    //! Get a global variable's expression
    const QString &getGlobalExpression(const QString &name) const;

    //! Get a list of all globals by name (order they were added)
    const QStringList &listGlobals(void) const;

    //! Reorder globals based on a new name list
    void reorderGlobals(const QStringList &names);

    //! Tell the evaluator that globals have been modified
    void commitGlobalsChanges(void);

signals:
    void newTitleSubtext(const QString &);

protected:
    //this widget is visible, populate menu with its tabs
    void showEvent(QShowEvent *event);

public slots:
    void handleStateChange(const GraphState &state);

private slots:
    void handleCurrentChanged(int);
    void handleCreateGraphPage(void);
    void handleRenameGraphPage(void);
    void handleDeleteGraphPage(void);
    void handleMoveGraphObjects(const int index);
    void handleAddBlock(const Poco::JSON::Object::Ptr &);
    void handleCreateBreaker(const bool isInput);
    void handleCreateInputBreaker(void);
    void handleCreateOutputBreaker(void);
    void handleInsertGraphWidget(QObject *);
    void handleCut(void);
    void handleCopy(void);
    void handlePaste(void);
    void handleClipboardDataChange(void);
    void handleSelectAll(void);
    void handleDelete(void);
    void handleRotateLeft(void);
    void handleRotateRight(void);
    void handleObjectProperties(void);
    void handleGraphProperties(void);
    void handleZoomIn(void);
    void handleZoomOut(void);
    void handleZoomOriginal(void);
    void handleUndo(void);
    void handleRedo(void);
    void handleEnable(void);
    void handleSetEnabled(const bool enb);
    void handleDisable(void);
    void handleReeval(void);
    void handleResetState(int);
    void handleAffinityZoneClicked(const QString &zone);
    void handleAffinityZoneChanged(const QString &zone);
    void handleShowRenderedGraphDialog(void);
    void handleShowTopologyStatsDialog(void);
    void handleToggleActivateTopology(bool);
    void handleShowPortNames(void);
    void handleBlockIncrement(void);
    void handleBlockDecrement(void);
    void handleBlockXcrement(const int adj);
    void handleEvalEngineDeactivate(void);

private:
    QTabWidget *_parentTabWidget;

    void updateGraphEditorMenus(void);

    void makeDefaultPage(void);

    void deleteFlagged(void);

    QSignalMapper *_moveGraphObjectsMapper;
    QSignalMapper *_insertGraphWidgetsMapper;

    QString _currentFilePath;
    QPointer<GraphStateManager> _stateManager;
    std::map<size_t, int> _stateToLastTabIndex;

    //! update enabled actions based on state - after a change or when editor becomes visible
    void updateEnabledActions(void);

    //! called after state changes
    void updateExecutionEngine(void);

    EvalEngine *_evalEngine;
    bool _isTopologyActive;

    //graph globals/constant expressions
    QStringList _globalNames;
    std::map<QString, QString> _globalExprs;
};
