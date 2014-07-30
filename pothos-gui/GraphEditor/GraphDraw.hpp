// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include "GraphObjects/GraphObject.hpp"
#include "GraphEditor/GraphState.hpp"

class GraphEditor;

class GraphDraw : public QWidget
{
    Q_OBJECT
public:
    GraphDraw(QWidget *parent);

    GraphObjectList getObjectsAtPos(const QPointF &pos, const int selectionFlags = ~0);

    GraphObjectList getObjectsSelected(const int selectionFlags = ~0);

    GraphObjectList getGraphObjects(const int selectionFlags = ~0);

    /*!
     * Describe the selected objects in words.
     * This will be used with the event reporting.
     * A single object selected? use the ID.
     * Otherwise, just report "selections".
     */
    QString getSelectionDescription(const int selectionFlags = ~0);

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

    int getMaxZIndex(void);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void showEvent(QShowEvent *event);

private slots:
    void handleCustomContextMenuRequested(const QPoint &);
    void handleGraphDebugViewChange(void);

signals:
    void stateChanged(const GraphState &);
    void selectionChanged(const GraphObjectList &);
    void modifyProperties(GraphObject *);

private:

    void setupCanvas(void);

    void deselectAllObjs(void);

    void doClickSelection(const QPointF &point);

    void updateEnabledActions(void);

    GraphEditor *_graphEditor;
    qreal _zoomScale;
    QImage _image;
    bool _mouseLeftDown;
    QPointF _mouseLeftDownFirstPoint;
    QPointF _mouseLeftDownLastPoint;
    QString _selectionState;
    QPointF _lastContextMenuPos;

    GraphConnectionEndpoint _lastClickSelectEp;

    bool _showGraphConnectionPoints;
    bool _showGraphBoundingBoxes;
};
