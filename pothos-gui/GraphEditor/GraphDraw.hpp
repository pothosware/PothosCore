// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include "GraphObjects/GraphObject.hpp"
#include <QGraphicsView>
#include <memory>

class GraphEditor;
class QGraphicsItem;
class QGraphicsPixmapItem;

class GraphDraw : public QGraphicsView
{
    Q_OBJECT
public:
    GraphDraw(QWidget *parent);

    GraphObjectList getObjectsSelected(const int selectionFlags = ~0);

    GraphObjectList getGraphObjects(const int selectionFlags = ~0);

    /*!
     * Describe the selected objects in words.
     * This will be used with the event reporting.
     * A single object selected? use the ID.
     * Otherwise, just report "selections".
     */
    QString getSelectionDescription(const int selectionFlags = ~0);

    //! Get the graph object with the specified ID or nullptr
    GraphObject *getObjectById(const QString &id, const int selectionFlags = ~0);

    GraphEditor *getGraphEditor(void) const
    {
        return _graphEditor;
    }

    void render(void);

    qreal zoomScale(void) const
    {
        return _zoomScale;
    }

    void setZoomScale(const qreal zoom);

    QPointF getLastContextMenuPos(void) const
    {
        return _lastContextMenuPos;
    }

    //! get the largest z value of all objects
    qreal getMaxZValue(void);

    //! unselect all selected objects
    void deselectAllObjs(void);

    /*!
     * The GraphDraw maintains a selection state to detect drag events.
     * Calling this method will clear that selection state to stop a move.
     */
    void clearSelectionState(void);

    //! Get a list of graph objects at the given point
    GraphObjectList getObjectsAtPos(const QPoint &pos);

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void showEvent(QShowEvent *event);
    void keyPressEvent(QKeyEvent *event);

private slots:
    void handleCustomContextMenuRequested(const QPoint &);
    void handleGraphDebugViewChange(void);
    void updateEnabledActions(void);

signals:
    void modifyProperties(GraphObject *);

private:

    void doClickSelection(const QPointF &point);

    GraphEditor *_graphEditor;
    qreal _zoomScale;
    int _selectionState;
    QPointF _lastContextMenuPos;

    GraphConnectionEndpoint _lastClickSelectEp;

    std::shared_ptr<QGraphicsPixmapItem> _graphConnectionPoints;
    std::shared_ptr<QGraphicsPixmapItem> _graphBoundingBoxes;
};
