// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QTabWidget>

class GraphEditor;

class GraphEditorTabs : public QTabWidget
{
    Q_OBJECT
public:
    GraphEditorTabs(QWidget *parent);

    void loadState(void);
    void saveState(void);

private slots:
    void handleInit(void);
    void handleNew(void);
    void handleOpen(void);
    void handleOpen(const QString &filePath);
    void handleSave(void);
    void handleSaveAs(void);
    void handleSaveAll(void);
    void handleReload(void);
    void handleClose(void);
    void handleClose(int);
    void handleClose(GraphEditor *editor);
    void handleExit(QCloseEvent *event);

private:
    void doReloadDialog(GraphEditor *editor);
    void ensureOneEditor(void);
};
