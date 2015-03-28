// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //action map
#include "EvalEngine/EvalEngine.hpp"
#include "GraphEditor/GraphActionsDock.hpp"
#include "GraphEditor/GraphEditor.hpp"
#include "GraphEditor/GraphDraw.hpp"
#include "GraphEditor/Constants.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include "GraphObjects/GraphBreaker.hpp"
#include "GraphObjects/GraphConnection.hpp"
#include "GraphObjects/GraphWidget.hpp"
#include <Poco/Logger.h>
#include <QTabBar>
#include <QInputDialog>
#include <QAction>
#include <QMenu>
#include <QSignalMapper>
#include <QDockWidget>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QRegExp>
#include <fstream>
#include <iostream>
#include <cassert>
#include <set>
#include <sstream>
#include <Poco/Path.h>
#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>
#include <Poco/JSON/Parser.h>
#include <Pothos/Exception.hpp>

GraphEditor::GraphEditor(QWidget *parent):
    QTabWidget(parent),
    _parentTabWidget(dynamic_cast<QTabWidget *>(parent)),
    _moveGraphObjectsMapper(new QSignalMapper(this)),
    _insertGraphWidgetsMapper(new QSignalMapper(this)),
    _stateManager(new GraphStateManager(this)),
    _evalEngine(new EvalEngine(this)),
    _isTopologyActive(false)
{
    this->setMovable(true);
    this->setUsesScrollButtons(true);
    this->setTabPosition(QTabWidget::West);
    this->makeDefaultPage();

    this->tabBar()->setStyleSheet("font-size:8pt;");

    //connect handlers that work at the page-level of control
    connect(QApplication::clipboard(), SIGNAL(dataChanged(void)), this, SLOT(handleClipboardDataChange(void)));
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(handleCurrentChanged(int)));
    connect(_stateManager, SIGNAL(newStateSelected(int)), this, SLOT(handleResetState(int)));
    connect(getActionMap()["createGraphPage"], SIGNAL(triggered(void)), this, SLOT(handleCreateGraphPage(void)));
    connect(getActionMap()["renameGraphPage"], SIGNAL(triggered(void)), this, SLOT(handleRenameGraphPage(void)));
    connect(getActionMap()["deleteGraphPage"], SIGNAL(triggered(void)), this, SLOT(handleDeleteGraphPage(void)));
    connect(getActionMap()["inputBreaker"], SIGNAL(triggered(void)), this, SLOT(handleCreateInputBreaker(void)));
    connect(getActionMap()["outputBreaker"], SIGNAL(triggered(void)), this, SLOT(handleCreateOutputBreaker(void)));
    connect(getActionMap()["cut"], SIGNAL(triggered(void)), this, SLOT(handleCut(void)));
    connect(getActionMap()["copy"], SIGNAL(triggered(void)), this, SLOT(handleCopy(void)));
    connect(getActionMap()["paste"], SIGNAL(triggered(void)), this, SLOT(handlePaste(void)));
    connect(getObjectMap()["blockTreeDock"], SIGNAL(addBlockEvent(const Poco::JSON::Object::Ptr &)), this, SLOT(handleAddBlock(const Poco::JSON::Object::Ptr &)));
    connect(getActionMap()["selectAll"], SIGNAL(triggered(void)), this, SLOT(handleSelectAll(void)));
    connect(getActionMap()["delete"], SIGNAL(triggered(void)), this, SLOT(handleDelete(void)));
    connect(getActionMap()["rotateLeft"], SIGNAL(triggered(void)), this, SLOT(handleRotateLeft(void)));
    connect(getActionMap()["rotateRight"], SIGNAL(triggered(void)), this, SLOT(handleRotateRight(void)));
    connect(getActionMap()["objectProperties"], SIGNAL(triggered(void)), this, SLOT(handleObjectProperties(void)));
    connect(getActionMap()["graphProperties"], SIGNAL(triggered(void)), this, SLOT(handleGraphProperties(void)));
    connect(getActionMap()["zoomIn"], SIGNAL(triggered(void)), this, SLOT(handleZoomIn(void)));
    connect(getActionMap()["zoomOut"], SIGNAL(triggered(void)), this, SLOT(handleZoomOut(void)));
    connect(getActionMap()["zoomOriginal"], SIGNAL(triggered(void)), this, SLOT(handleZoomOriginal(void)));
    connect(getActionMap()["undo"], SIGNAL(triggered(void)), this, SLOT(handleUndo(void)));
    connect(getActionMap()["redo"], SIGNAL(triggered(void)), this, SLOT(handleRedo(void)));
    connect(getActionMap()["enable"], SIGNAL(triggered(void)), this, SLOT(handleEnable(void)));
    connect(getActionMap()["disable"], SIGNAL(triggered(void)), this, SLOT(handleDisable(void)));
    connect(getActionMap()["reeval"], SIGNAL(triggered(void)), this, SLOT(handleReeval(void)));
    connect(getMenuMap()["setAffinityZone"], SIGNAL(zoneClicked(const QString &)), this, SLOT(handleAffinityZoneClicked(const QString &)));
    connect(getObjectMap()["affinityZonesDock"], SIGNAL(zoneChanged(const QString &)), this, SLOT(handleAffinityZoneChanged(const QString &)));
    connect(getActionMap()["showRenderedGraphView"], SIGNAL(triggered(void)), this, SLOT(handleShowRenderedGraphDialog(void)));
    connect(getActionMap()["activateTopology"], SIGNAL(toggled(bool)), this, SLOT(handleToggleActivateTopology(bool)));
    connect(getActionMap()["showPortNames"], SIGNAL(changed(void)), this, SLOT(handleShowPortNames(void)));
    connect(getActionMap()["increment"], SIGNAL(triggered(void)), this, SLOT(handleBlockIncrement(void)));
    connect(getActionMap()["decrement"], SIGNAL(triggered(void)), this, SLOT(handleBlockDecrement(void)));
    connect(_moveGraphObjectsMapper, SIGNAL(mapped(int)), this, SLOT(handleMoveGraphObjects(int)));
    connect(_insertGraphWidgetsMapper, SIGNAL(mapped(QObject *)), this, SLOT(handleInsertGraphWidget(QObject *)));
    connect(_evalEngine, SIGNAL(deactivateDesign(void)), this, SLOT(handleEvalEngineDeactivate(void)));
    connect(this, SIGNAL(newTitleSubtext(const QString &)), getObjectMap()["mainWindow"], SLOT(handleNewTitleSubtext(const QString &)));
}

GraphEditor::~GraphEditor(void)
{
    //the actions dock owns state manager for display purposes,
    //so delete it here when the graph editor is actually done with it
    delete _stateManager;
}

QString GraphEditor::newId(const QString &hint) const
{
    std::set<QString> allIds;
    for (auto obj : this->getGraphObjects()) allIds.insert(obj->getId());

    //either use the hint or UUID if blank
    QString idBase = hint;
    if (idBase.isEmpty())
    {
        Poco::UUIDGenerator &generator = Poco::UUIDGenerator::defaultGenerator();
        idBase = QString::fromStdString(generator.createRandom().toString());
    }

    //find a reasonable name and index
    size_t index = 0;
    QRegExp rx("(.*)(\\d+)"); rx.indexIn(idBase);
    if (rx.captureCount() == 2 and not rx.cap(1).isEmpty() and not rx.cap(2).isEmpty())
    {
        idBase = rx.cap(1);
        index = rx.cap(2).toInt();
    }

    //loop for a unique ID name
    QString possibleId;
    do
    {
        possibleId = QString("%1%2").arg(idBase).arg(index++);
    } while (allIds.find(possibleId) != allIds.end());

    return possibleId;
}

void GraphEditor::showEvent(QShowEvent *event)
{
    //load our state monitor into the actions dock
    auto actionsDock = dynamic_cast<GraphActionsDock *>(getObjectMap()["graphActionsDock"]);
    assert(actionsDock != nullptr);
    actionsDock->setActiveWidget(_stateManager);

    this->updateGraphEditorMenus();
    this->updateEnabledActions();
    QWidget::showEvent(event);
}

void GraphEditor::updateEnabledActions(void)
{
    if (not this->isVisible()) return;

    getActionMap()["undo"]->setEnabled(_stateManager->isPreviousAvailable());
    getActionMap()["redo"]->setEnabled(_stateManager->isSubsequentAvailable());
    getActionMap()["save"]->setEnabled(not _stateManager->isCurrentSaved());
    getActionMap()["reload"]->setEnabled(not this->getCurrentFilePath().isEmpty());
    getActionMap()["activateTopology"]->setChecked(_isTopologyActive);

    //can we paste something from the clipboard?
    auto mimeData = QApplication::clipboard()->mimeData();
    const bool canPaste = mimeData->hasFormat("text/json/pothos_object_array") and
                      not mimeData->data("text/json/pothos_object_array").isEmpty();
    getActionMap()["paste"]->setEnabled(canPaste);

    //update window title
    QString subtext = this->getCurrentFilePath();
    if (subtext.isEmpty()) subtext = tr("untitled");
    emit this->newTitleSubtext(tr("Editing ") + subtext);
}

void GraphEditor::handleCurrentChanged(int)
{
    if (not this->isVisible()) return;
    this->updateGraphEditorMenus();
}

void GraphEditor::handleCreateGraphPage(void)
{
    if (not this->isVisible()) return;
    const QString newName = QInputDialog::getText(this, tr("Create page"),
        tr("New page name"), QLineEdit::Normal, tr("untitled"));
    if (newName.isEmpty()) return;
    this->addTab(new GraphDraw(this), newName);
    this->updateGraphEditorMenus();

    handleStateChange(GraphState("document-new", tr("Create graph page ") + newName));
}

void GraphEditor::handleRenameGraphPage(void)
{
    if (not this->isVisible()) return;
    const auto oldName = this->tabText(this->currentIndex());
    const QString newName = QInputDialog::getText(this, tr("Rename page"),
        tr("New page name"), QLineEdit::Normal, oldName);
    if (newName.isEmpty()) return;
    this->setTabText(this->currentIndex(), newName);
    this->updateGraphEditorMenus();

    handleStateChange(GraphState("edit-rename", tr("Rename graph page ") + oldName + " -> " + newName));
}

void GraphEditor::handleDeleteGraphPage(void)
{
    if (not this->isVisible()) return;
    const auto oldName = this->tabText(this->currentIndex());
    this->removeTab(this->currentIndex());
    if (this->count() == 0) this->makeDefaultPage();
    this->updateGraphEditorMenus();

    handleStateChange(GraphState("edit-delete", tr("Delete graph page ") + oldName));
}

GraphConnection *GraphEditor::makeConnection(const GraphConnectionEndpoint &ep0, const GraphConnectionEndpoint &ep1)
{
    //direction check
    if (ep0.getConnectableAttrs().direction == ep1.getConnectableAttrs().direction or
        (ep0.getConnectableAttrs().direction == GRAPH_CONN_INPUT and ep1.getConnectableAttrs().direction == GRAPH_CONN_SLOT) or
        (ep0.getConnectableAttrs().direction == GRAPH_CONN_OUTPUT and ep1.getConnectableAttrs().direction == GRAPH_CONN_SIGNAL) or
        (ep0.getConnectableAttrs().direction == GRAPH_CONN_SLOT and ep1.getConnectableAttrs().direction == GRAPH_CONN_INPUT) or
        (ep0.getConnectableAttrs().direction == GRAPH_CONN_SIGNAL and ep1.getConnectableAttrs().direction == GRAPH_CONN_OUTPUT))
    {
        throw Pothos::Exception("GraphEditor::makeConnection()", "cant connect endpoints of the same direction");
    }

    //duplicate check
    for (auto obj : this->getGraphObjects(GRAPH_CONNECTION))
    {
        auto conn = dynamic_cast<GraphConnection *>(obj);
        assert(conn != nullptr);
        if (
            (conn->getOutputEndpoint() == ep0 and conn->getInputEndpoint() == ep1) or
            (conn->getOutputEndpoint() == ep1 and conn->getInputEndpoint() == ep0)
        ) throw Pothos::Exception("GraphEditor::makeConnection()", "connection already exists");
    }

    auto conn = new GraphConnection(ep0.getObj()->draw());
    conn->setupEndpoint(ep0);
    conn->setupEndpoint(ep1);

    const auto idHint = QString("Connection_%1%2_%3%4").arg(
        conn->getOutputEndpoint().getObj()->getId(),
        conn->getOutputEndpoint().getKey().id,
        conn->getInputEndpoint().getObj()->getId(),
        conn->getInputEndpoint().getKey().id
    );
    conn->setId(this->newId(idHint));
    assert(conn->getInputEndpoint().isValid());
    assert(conn->getOutputEndpoint().isValid());

    return conn;
}

//TODO traverse breakers and find one in the node mass that already exists

static GraphBreaker *findInputBreaker(GraphEditor *editor, const GraphConnectionEndpoint &ep)
{
    for (auto obj : editor->getGraphObjects(GRAPH_CONNECTION))
    {
        auto conn = dynamic_cast<GraphConnection *>(obj);
        assert(conn != nullptr);
        if (not (conn->getOutputEndpoint().getObj()->scene() == conn->getInputEndpoint().getObj()->scene())) continue;
        if (not (conn->getOutputEndpoint() == ep)) continue;
        auto breaker = dynamic_cast<GraphBreaker *>(conn->getInputEndpoint().getObj().data());
        if (breaker == nullptr) continue;
        return breaker;
    }
    return nullptr;
}

void GraphEditor::handleMoveGraphObjects(const int index)
{
    if (not this->isVisible()) return;
    if (index >= this->count()) return;
    auto draw = this->getCurrentGraphDraw();
    auto desc = tr("Move %1 to %2").arg(draw->getSelectionDescription(~GRAPH_CONNECTION), this->tabText(index));

    //move all selected objects
    for (auto obj : draw->getObjectsSelected())
    {
        obj->setSelected(false);
        this->getGraphDraw(index)->scene()->addItem(obj);
    }

    //reparent all connections based on endpoints:
    std::vector<GraphConnection *> boundaryConnections;
    for (auto obj : this->getGraphObjects(GRAPH_CONNECTION))
    {
        auto conn = dynamic_cast<GraphConnection *>(obj);
        assert(conn != nullptr);

        //Connection has the same endpoints, so make sure that the parent is corrected to the endpoint
        if (conn->getOutputEndpoint().getObj()->scene() == conn->getInputEndpoint().getObj()->scene())
        {
            if (conn->getOutputEndpoint().getObj()->scene() != conn->scene())
            {
                conn->getInputEndpoint().getObj()->scene()->addItem(conn);
            }
        }

        //otherwise stash it for more processing
        else
        {
            boundaryConnections.push_back(conn);
        }
    }

    //create breakers for output endpoints that have to cross
    for (auto conn : boundaryConnections)
    {
        const auto &epOut = conn->getOutputEndpoint();
        const auto &epIn = conn->getInputEndpoint();

        auto sigSlotPairs = conn->getSigSlotPairs();
        if (sigSlotPairs.empty()) sigSlotPairs.resize(1); //add empty
        for (const auto &sigSlotPair : sigSlotPairs)
        {
            auto breaker = findInputBreaker(this, epOut);
            if (breaker != nullptr) continue;

            breaker = new GraphBreaker(epOut.getObj()->draw());
            breaker->setInput(true);
            const auto name = QString("%1[%2]").arg(epOut.getObj()->getId(), epOut.getKey().id);
            breaker->setId(this->newId(name));
            breaker->setNodeName(breaker->getId()); //the first of its name
            breaker->setRotation(epIn.getObj()->rotation());
            breaker->setPos(epIn.getObj()->pos());

            auto outConn = this->makeConnection(epOut, GraphConnectionEndpoint(breaker, breaker->getConnectableKeys().at(0)));
            if (not sigSlotPair.first.isEmpty()) outConn->addSigSlotPair(std::make_pair(sigSlotPair.first, breaker->getConnectableKeys().at(0).id));
            if (outConn->scene() != breaker->scene()) breaker->scene()->addItem(outConn); //use desired parent
        }
    }

    //create breakers for input endpoints that have to cross
    for (auto conn : boundaryConnections)
    {
        const auto &epOut = conn->getOutputEndpoint();
        const auto &epIn = conn->getInputEndpoint();

        auto sigSlotPairs = conn->getSigSlotPairs();
        if (sigSlotPairs.empty()) sigSlotPairs.resize(1); //add empty
        for (const auto &sigSlotPair : sigSlotPairs)
        {
            //find the existing breaker or make a new one
            const auto name = findInputBreaker(this, epOut)->getNodeName();
            GraphBreaker *breaker = nullptr;
            for (auto obj : this->getGraphObjects(GRAPH_BREAKER))
            {
                if (obj->draw() != epIn.getObj()->draw()) continue;
                auto outBreaker = dynamic_cast<GraphBreaker *>(obj);
                assert(outBreaker != nullptr);
                if (outBreaker->isInput()) continue;
                if (outBreaker->getNodeName() != name) continue;
                breaker = outBreaker;
                break;
            }

            //make a new output breaker
            if (breaker == nullptr)
            {
                breaker = new GraphBreaker(epIn.getObj()->draw());
                breaker->setInput(false);
                breaker->setId(this->newId(name));
                breaker->setNodeName(name);
                breaker->setRotation(epOut.getObj()->rotation());
                breaker->setPos(epOut.getObj()->pos());
            }

            //connect to this breaker
            auto inConn = this->makeConnection(epIn, GraphConnectionEndpoint(breaker, breaker->getConnectableKeys().at(0)));
            if (not sigSlotPair.second.isEmpty()) inConn->addSigSlotPair(std::make_pair(breaker->getConnectableKeys().at(0).id, sigSlotPair.second));
            if (inConn->scene() != breaker->scene()) breaker->scene()->addItem(inConn); //use desired parent
        }

        delete conn;
    }

    handleStateChange(GraphState("transform-move", desc));
}

void GraphEditor::handleAddBlock(const Poco::JSON::Object::Ptr &blockDesc)
{
    if (not this->isVisible()) return;
    QPointF where(std::rand()%100, std::rand()%100);

    //determine where, a nice point on the visible drawing area sort of upper left
    auto view = dynamic_cast<QGraphicsView *>(this->currentWidget());
    where += view->mapToScene(this->size().width()/4, this->size().height()/4);

    this->handleAddBlock(blockDesc, where);
}

void GraphEditor::handleAddBlock(const Poco::JSON::Object::Ptr &blockDesc, const QPointF &where)
{
    if (not blockDesc) return;
    auto draw = this->getCurrentGraphDraw();
    auto block = new GraphBlock(draw);
    block->setBlockDesc(blockDesc);

    QString hint;
    const auto title = block->getTitle();
    for (int i = 0; i < title.length(); i++)
    {
        if (i == 0 and title.at(i).isNumber()) hint.append(QChar('_'));
        if (title.at(i).isLetterOrNumber() or title.at(i).toLatin1() == '_')
        {
            hint.append(title.at(i));
        }
    }
    block->setId(this->newId(hint));

    //set highest z-index on new block
    block->setZValue(draw->getMaxZValue()+1);

    block->setPos(where);
    block->setRotation(0);
    handleStateChange(GraphState("list-add", tr("Create block %1").arg(title)));
}

void GraphEditor::handleCreateBreaker(const bool isInput)
{
    if (not this->isVisible()) return;

    const auto dirName = isInput?tr("input"):tr("output");
    const auto newName = QInputDialog::getText(this, tr("Create %1 breaker").arg(dirName),
        tr("New breaker node name"), QLineEdit::Normal, tr("untitled"));
    if (newName.isEmpty()) return;

    auto draw = this->getCurrentGraphDraw();
    auto breaker = new GraphBreaker(draw);
    breaker->setInput(isInput);
    breaker->setNodeName(newName);
    breaker->setId(this->newId(newName));
    breaker->setPos(draw->getLastContextMenuPos());

    handleStateChange(GraphState("document-new", tr("Create %1 breaker %2").arg(dirName, newName)));
}

void GraphEditor::handleCreateInputBreaker(void)
{
    this->handleCreateBreaker(true);
}

void GraphEditor::handleCreateOutputBreaker(void)
{
    this->handleCreateBreaker(false);
}

void GraphEditor::handleInsertGraphWidget(QObject *obj)
{
    auto block = dynamic_cast<GraphBlock *>(obj);
    assert(block != nullptr);
    assert(block->isGraphWidget());

    auto draw = this->getCurrentGraphDraw();
    auto display = new GraphWidget(draw);
    display->setGraphBlock(block);
    display->setId(this->newId("Widget"+block->getId()));
    display->setZValue(draw->getMaxZValue()+1);
    display->setPos(draw->getLastContextMenuPos());
    display->setRotation(0);

    handleStateChange(GraphState("insert-image", tr("Insert widget %1").arg(block->getId())));
}

void GraphEditor::handleCut(void)
{
    auto draw = this->getCurrentGraphDraw();
    auto desc = tr("Cut %1").arg(draw->getSelectionDescription());

    //load up the clipboard
    this->handleCopy();

    //delete all selected graph objects
    for (auto obj : draw->getObjectsSelected())
    {
        delete obj;
    }

    this->deleteFlagged();

    handleStateChange(GraphState("edit-cut", desc));
}

void GraphEditor::handleCopy(void)
{
    if (not this->isVisible()) return;
    auto draw = this->getCurrentGraphDraw();

    Poco::JSON::Array jsonObjs;
    for (auto obj : draw->getObjectsSelected())
    {
        jsonObjs.add(obj->serialize());
    }

    //to byte array
    std::ostringstream oss;
    jsonObjs.stringify(oss);
    QByteArray byteArray(oss.str().data(), oss.str().size());

    //load the clipboard
    auto mimeData = new QMimeData();
    mimeData->setData("text/json/pothos_object_array", byteArray);
    QApplication::clipboard()->setMimeData(mimeData);
}

/*!
 * paste only one object type so handlePaste can control the order of creation
 */
static GraphObjectList handlePasteType(GraphDraw *draw, const Poco::JSON::Array::Ptr &graphObjects, const std::string &type)
{
    GraphObjectList newObjects;
    for (size_t objIndex = 0; objIndex < graphObjects->size(); objIndex++)
    {
        const auto jGraphObj = graphObjects->getObject(objIndex);
        const auto what = jGraphObj->getValue<std::string>("what");
        GraphObject *obj = nullptr;
        if (what != type) continue;
        if (what == "Block") obj = new GraphBlock(draw);
        if (what == "Breaker") obj = new GraphBreaker(draw);
        if (what == "Connection") obj = new GraphConnection(draw);
        if (what == "Widget") obj = new GraphWidget(draw);
        if (obj == nullptr) continue;
        try {obj->deserialize(jGraphObj);}
        catch (const Pothos::NotFoundException &)
        {
            delete obj;
            continue;
        }
        obj->setSelected(true);
        newObjects.push_back(obj);
    }
    return newObjects;
}

void GraphEditor::handlePaste(void)
{
    if (not this->isVisible()) return;
    auto draw = this->getCurrentGraphDraw();

    auto mimeData = QApplication::clipboard()->mimeData();
    const bool canPaste = mimeData->hasFormat("text/json/pothos_object_array") and
                      not mimeData->data("text/json/pothos_object_array").isEmpty();
    if (not canPaste) return;

    //extract object array
    const auto data = mimeData->data("text/json/pothos_object_array");
    const std::string dataStr(data.constData(), data.size());
    std::istringstream iss(dataStr);
    Poco::JSON::Parser p; p.parse(iss);
    auto graphObjects = p.getHandler()->asVar().extract<Poco::JSON::Array::Ptr>();
    assert(graphObjects);

    //rewrite ids
    std::map<std::string, std::string> oldIdToNew;
    for (size_t objIndex = 0; objIndex < graphObjects->size(); objIndex++)
    {
        const auto jGraphObj = graphObjects->getObject(objIndex);
        auto oldId = jGraphObj->getValue<std::string>("id");
        oldIdToNew[oldId] = this->newId(QString::fromStdString(oldId)).toStdString();
    }
    for (size_t objIndex = 0; objIndex < graphObjects->size();)
    {
        for (auto &pair : *graphObjects->getObject(objIndex))
        {
            if (QString::fromStdString(pair.first).endsWith("id", Qt::CaseInsensitive))
            {
                //if not in oldIdToNew, remove from list
                if (oldIdToNew.count(pair.second) == 0)
                {
                    graphObjects->remove(objIndex);
                    goto nextObj;
                }
                pair.second = oldIdToNew[pair.second];
            }
        }
        objIndex++;
        nextObj: continue;
    }

    //unselect all objects
    draw->deselectAllObjs();

    //create objects
    GraphObjectList objsToMove;
    objsToMove.append(handlePasteType(draw, graphObjects, "Block"));
    objsToMove.append(handlePasteType(draw, graphObjects, "Breaker"));
    handlePasteType(draw, graphObjects, "Connection"); //dont append, connection position doesnt matter
    objsToMove.append(handlePasteType(draw, graphObjects, "Widget"));

    //deal with initial positions of pasted objects
    QPointF cornerest(1e6, 1e6);
    for (auto obj : objsToMove)
    {
        cornerest.setX(std::min(cornerest.x(), obj->pos().x()));
        cornerest.setY(std::min(cornerest.y(), obj->pos().y()));
    }

    //determine an acceptable position to center the paste
    auto view = dynamic_cast<QGraphicsView *>(this->currentWidget());
    auto pastePos = view->mapToScene(view->mapFromGlobal(QCursor::pos()));
    if (not view->sceneRect().contains(pastePos))
    {
        pastePos = view->mapToScene(this->size().width()/2, this->size().height()/2);
    }

    //move objects into position
    for (auto obj : objsToMove) obj->setPos(obj->pos()-cornerest+pastePos);

    handleStateChange(GraphState("edit-paste", tr("Paste %1").arg(draw->getSelectionDescription())));
}

void GraphEditor::handleClipboardDataChange(void)
{
    if (not this->isVisible()) return;
    this->updateEnabledActions();
}

void GraphEditor::handleSelectAll(void)
{
    if (not this->isVisible()) return;
    auto draw = this->getCurrentGraphDraw();
    for (auto obj : draw->getGraphObjects())
    {
        obj->setSelected(true);
    }
    this->render();
}

void GraphEditor::deleteFlagged(void)
{
    //delete all objects flagged for deletion
    while (true)
    {
        bool deletionOccured = false;
        for (auto obj : this->getGraphObjects())
        {
            if (obj->isFlaggedForDelete())
            {
                delete obj;
                deletionOccured = true;
            }
        }
        if (not deletionOccured) break;
    }
}

void GraphEditor::handleDelete(void)
{
    if (not this->isVisible()) return;
    auto draw = this->getCurrentGraphDraw();
    auto desc = tr("Delete %1").arg(draw->getSelectionDescription());

    //delete all selected graph objects
    for (auto obj : draw->getObjectsSelected())
    {
        delete obj;
    }

    this->deleteFlagged();

    handleStateChange(GraphState("edit-delete", desc));
}

void GraphEditor::handleRotateLeft(void)
{
    if (not this->isVisible()) return;
    auto draw = this->getCurrentGraphDraw();
    //TODO rotate group of objects around central point
    for (auto obj : draw->getObjectsSelected(~GRAPH_CONNECTION))
    {
        obj->rotateLeft();
    }
    handleStateChange(GraphState("object-rotate-left", tr("Rotate %1 left").arg(draw->getSelectionDescription(~GRAPH_CONNECTION))));
}

void GraphEditor::handleRotateRight(void)
{
    if (not this->isVisible()) return;
    auto draw = this->getCurrentGraphDraw();
    //TODO rotate group of objects around central point
    for (auto obj : draw->getObjectsSelected(~GRAPH_CONNECTION))
    {
        obj->rotateRight();
    }
    handleStateChange(GraphState("object-rotate-right", tr("Rotate %1 right").arg(draw->getSelectionDescription(~GRAPH_CONNECTION))));
}

void GraphEditor::handleObjectProperties(void)
{
    if (not this->isVisible()) return;
    auto draw = this->getCurrentGraphDraw();
    const auto objs = draw->getObjectsSelected();
    if (not objs.isEmpty()) emit draw->modifyProperties(objs.at(0));
}

void GraphEditor::handleGraphProperties(void)
{
    if (not this->isVisible()) return;
    emit this->getCurrentGraphDraw()->modifyProperties(this);
}

void GraphEditor::handleZoomIn(void)
{
    if (not this->isVisible()) return;
    auto draw = this->getCurrentGraphDraw();
    if (draw->zoomScale() >= GraphDrawZoomMax) return;
    draw->setZoomScale(draw->zoomScale() + GraphDrawZoomStep);
}

void GraphEditor::handleZoomOut(void)
{
    if (not this->isVisible()) return;
    auto draw = this->getCurrentGraphDraw();
    if (draw->zoomScale() <= GraphDrawZoomMin) return;
    draw->setZoomScale(draw->zoomScale() - GraphDrawZoomStep);
}

void GraphEditor::handleZoomOriginal(void)
{
    if (not this->isVisible()) return;
    auto draw = this->getCurrentGraphDraw();
    draw->setZoomScale(1.0);
}

void GraphEditor::handleUndo(void)
{
    if (not this->isVisible()) return;
    assert(_stateManager->isPreviousAvailable());
    this->handleResetState(_stateManager->getCurrentIndex()-1);
}

void GraphEditor::handleRedo(void)
{
    if (not this->isVisible()) return;
    assert(_stateManager->isSubsequentAvailable());
    this->handleResetState(_stateManager->getCurrentIndex()+1);
}

void GraphEditor::handleEnable(void)
{
    return this->handleSetEnabled(true);
}

void GraphEditor::handleDisable(void)
{
    return this->handleSetEnabled(false);
}

void GraphEditor::handleSetEnabled(const bool enb)
{
    if (not this->isVisible()) return;
    auto draw = this->getCurrentGraphDraw();

    auto objs = draw->getObjectsSelected();
    if (objs.isEmpty()) return;

    for (auto obj : objs)
    {
        obj->setEnabled(enb);
    }

    if (enb) handleStateChange(GraphState("document-import", tr("Enable %1").arg(draw->getSelectionDescription())));
    else handleStateChange(GraphState("document-export", tr("Disable %1").arg(draw->getSelectionDescription())));
}

void GraphEditor::handleReeval(void)
{
    if (not this->isVisible()) return;
    auto draw = this->getCurrentGraphDraw();
    _evalEngine->submitReeval(draw->getObjectsSelected(GRAPH_BLOCK));
}

void GraphEditor::handleResetState(int stateNo)
{
    if (not this->isVisible()) return;

    //Resets the state of whoever is modding the properties:
    //Do this before loading the state, otherwise a potential
    //call to handleCancel() afterwards can overwrite settings.
    auto draw = this->getCurrentGraphDraw();
    emit draw->modifyProperties(nullptr);

    _stateManager->resetTo(stateNo);
    const auto dump = _stateManager->current().dump;
    std::istringstream iss(std::string(dump.constData(), dump.size()));
    this->loadState(iss);
    this->updateGraphEditorMenus();
    this->render();

    this->updateExecutionEngine();
}

void GraphEditor::handleAffinityZoneClicked(const QString &zone)
{
    if (not this->isVisible()) return;
    auto draw = this->getCurrentGraphDraw();

    for (auto obj : draw->getObjectsSelected(GRAPH_BLOCK))
    {
        auto block = dynamic_cast<GraphBlock *>(obj);
        assert(block != nullptr);
        block->setAffinityZone(zone);
    }
    handleStateChange(GraphState("document-export", tr("Set %1 affinity zone").arg(draw->getSelectionDescription(GRAPH_BLOCK))));
}

void GraphEditor::handleAffinityZoneChanged(const QString &zone)
{
    for (auto obj : this->getGraphObjects(GRAPH_BLOCK))
    {
        auto block = dynamic_cast<GraphBlock *>(obj);
        assert(block != nullptr);
        if (block->getAffinityZone() == zone) block->changed();
    }

    this->render();
    this->updateExecutionEngine();
}

void GraphEditor::handleStateChange(const GraphState &state)
{
    //empty states tell us to simply reset to the current known point
    if (state.iconName.isEmpty() and state.description.isEmpty())
    {
        return this->handleResetState(_stateManager->getCurrentIndex());
    }

    //serialize the graph into the state manager
    std::ostringstream oss;
    this->dumpState(oss);
    GraphState stateWithDump = state;
    stateWithDump.dump = QByteArray(oss.str().data(), oss.str().size());
    _stateManager->post(stateWithDump);
    this->render();

    this->updateGraphEditorMenus();
    this->updateExecutionEngine();
}

void GraphEditor::handleToggleActivateTopology(const bool enable)
{
    if (not this->isVisible()) return;
    _evalEngine->submitActivateTopology(enable);
    _isTopologyActive = enable;
}

void GraphEditor::handleShowPortNames(void)
{
    for (auto obj : this->getGraphObjects(GRAPH_BLOCK))
    {
        auto block = dynamic_cast<GraphBlock *>(obj);
        assert(block != nullptr);
        block->changed();
    }
    if (this->isVisible()) this->render();
}

void GraphEditor::handleBlockIncrement(void)
{
    this->handleBlockXcrement(+1);
}

void GraphEditor::handleBlockDecrement(void)
{
    this->handleBlockXcrement(-1);
}

void GraphEditor::handleBlockXcrement(const int adj)
{
    if (not this->isVisible()) return;
    auto draw = this->getCurrentGraphDraw();
    GraphObjectList changedObjects;
    for (auto obj : draw->getObjectsSelected(GRAPH_BLOCK))
    {
        auto block = dynamic_cast<GraphBlock *>(obj);
        assert(block != nullptr);
        for (const auto &propKey : block->getProperties())
        {
            auto paramDesc = block->getParamDesc(propKey);
            if (paramDesc->has("widgetType") and paramDesc->getValue<std::string>("widgetType") == "SpinBox")
            {
                const auto newValue = block->getPropertyValue(propKey).toInt() + adj;
                block->setPropertyValue(propKey, QString("%1").arg(newValue));
                changedObjects.push_back(block);
                break;
            }
        }
    }

    if (changedObjects.empty()) return;
    const auto desc = (changedObjects.size() == 1)? changedObjects.front()->getId() : tr("selected");
    if (adj > 0) handleStateChange(GraphState("list-add", tr("Increment %1").arg(desc)));
    if (adj < 0) handleStateChange(GraphState("list-remove", tr("Decrement %1").arg(desc)));
}

void GraphEditor::updateExecutionEngine(void)
{
    _evalEngine->submitTopology(this->getGraphObjects());
}

void GraphEditor::handleEvalEngineDeactivate(void)
{
    getActionMap()["activateTopology"]->setChecked(false);
    _isTopologyActive = false;
}

void GraphEditor::save(void)
{
    assert(not this->getCurrentFilePath().isEmpty());

    auto fileName = this->getCurrentFilePath().toStdString();
    try
    {
        std::ofstream outFile(fileName.c_str());
        this->dumpState(outFile);
    }
    catch (const std::exception &ex)
    {
        poco_error_f2(Poco::Logger::get("PothosGui.GraphEditor.save"), "Error saving %s: %s", fileName, std::string(ex.what()));
    }

    _stateManager->saveCurrent();
    this->render();
}

void GraphEditor::load(void)
{
    auto fileName = this->getCurrentFilePath().toStdString();

    if (fileName.empty())
    {
        _stateManager->resetToDefault();
        handleStateChange(GraphState("document-new", tr("Create new topology")));
        _stateManager->saveCurrent();
        this->render();
        return;
    }

    try
    {
        poco_information_f1(Poco::Logger::get("PothosGui.GraphEditor.load"), "Loading %s from file", fileName);
        std::ifstream inFile(fileName.c_str());
        this->loadState(inFile);
    }
    catch (const std::exception &ex)
    {
        poco_error_f2(Poco::Logger::get("PothosGui.GraphEditor.load"), "Error loading %s: %s", fileName, std::string(ex.what()));
    }

    _stateManager->resetToDefault();
    handleStateChange(GraphState("document-new", tr("Load topology from file")));
    _stateManager->saveCurrent();
    this->updateGraphEditorMenus();
    this->render();
}

void GraphEditor::render(void)
{
    //generate a title
    QString title = tr("untitled");
    if (not this->getCurrentFilePath().isEmpty())
    {
        auto name = Poco::Path(this->getCurrentFilePath().toStdString()).getBaseName();
        title = QString::fromStdString(name);
    }
    if (this->hasUnsavedChanges()) title += "*";

    //set the tab text
    for (int i = 0; i < _parentTabWidget->count(); i++)
    {
        if (_parentTabWidget->widget(i) != this) continue;
        _parentTabWidget->setTabText(i, title);
    }

    this->getCurrentGraphDraw()->render();
    this->updateEnabledActions();
}

void GraphEditor::updateGraphEditorMenus(void)
{
    if (not this->isVisible()) return;

    auto menu = getMenuMap()["moveGraphObjects"];
    menu->clear();
    for (int i = 0; i < this->count(); i++)
    {
        if (i == this->currentIndex()) continue;
        auto action = menu->addAction(QString("%1 (%2)").arg(this->tabText(i)).arg(i));
        connect(action, SIGNAL(triggered(void)), _moveGraphObjectsMapper, SLOT(map(void)));
        _moveGraphObjectsMapper->setMapping(action, i);
    }

    menu = getMenuMap()["insertGraphWidgets"];
    menu->clear();
    for (auto obj : this->getGraphObjects(GRAPH_BLOCK))
    {
        auto block = dynamic_cast<GraphBlock *>(obj);
        assert(block != nullptr);
        if (not block->isGraphWidget()) continue;

        //does block have an active graph display?
        for (auto subObj : this->getGraphObjects(GRAPH_WIDGET))
        {
            auto display = dynamic_cast<GraphWidget *>(subObj);
            assert(display != nullptr);
            if (display->getGraphBlock() == block) goto next_block;
        }

        //block is a display widget with no active displays:
        {
            auto action = menu->addAction(QString("%1 (%2)").arg(block->getTitle()).arg(block->getId()));
            connect(action, SIGNAL(triggered(void)), _insertGraphWidgetsMapper, SLOT(map(void)));
            _insertGraphWidgetsMapper->setMapping(action, block);
        }

        next_block: continue;
    }
}

GraphDraw *GraphEditor::getGraphDraw(const int index) const
{
    auto draw = dynamic_cast<GraphDraw *>(this->widget(index));
    assert(draw != nullptr);
    return draw;
}

GraphDraw *GraphEditor::getCurrentGraphDraw(void) const
{
    return this->getGraphDraw(this->currentIndex());
}

GraphObjectList GraphEditor::getGraphObjects(const int selectionFlags) const
{
    GraphObjectList all;
    for (int i = 0; i < this->count(); i++)
    {
        for (auto obj : this->getGraphDraw(i)->getGraphObjects(selectionFlags))
        {
            all.push_back(obj);
        }
    }
    return all;
}

GraphObject *GraphEditor::getObjectById(const QString &id, const int selectionFlags)
{
    for (auto obj : this->getGraphObjects(selectionFlags))
    {
        if (obj->getId() == id) return obj;
    }
    return nullptr;
}

void GraphEditor::makeDefaultPage(void)
{
    this->insertTab(0, new GraphDraw(this), tr("Main"));
}


void GraphEditor::clearGlobals(void)
{
    _globalNames.clear();
    _globalExprs.clear();
}

void GraphEditor::setGlobalExpression(const QString &name, const QString &expression)
{
    if (_globalExprs.count(name) == 0) _globalNames.push_back(name);
    _globalExprs[name] = expression;
}

const QString &GraphEditor::getGlobalExpression(const QString &name) const
{
    return _globalExprs.at(name);
}

const QStringList &GraphEditor::listGlobals(void) const
{
    return _globalNames;
}

void GraphEditor::commitGlobalsChanges(void)
{
    this->updateExecutionEngine();
}
