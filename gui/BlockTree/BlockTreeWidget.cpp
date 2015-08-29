// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //get object map
#include "BlockTree/BlockTreeWidget.hpp"
#include "BlockTree/BlockTreeWidgetItem.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include "GraphEditor/Constants.hpp"
#include "GraphEditor/GraphEditorTabs.hpp"
#include "GraphEditor/GraphEditor.hpp"
#include "GraphEditor/GraphDraw.hpp"
#include <QTreeWidgetItem>
#include <QApplication>
#include <QDrag>
#include <QMouseEvent>
#include <QMimeData>
#include <QTimer>
#include <QPainter>
#include <Poco/Logger.h>
#include <Poco/String.h>
#include <memory>

static const long UPDATE_TIMER_MS = 500;

BlockTreeWidget::BlockTreeWidget(QWidget *parent):
    QTreeWidget(parent),
    _filttimer(new QTimer(this))
{
    QStringList columnNames;
    columnNames.push_back(tr("Available Blocks"));
    this->setColumnCount(columnNames.size());
    this->setHeaderLabels(columnNames);

    _filttimer->setSingleShot(true);
    _filttimer->setInterval(UPDATE_TIMER_MS);

    connect(this, SIGNAL(itemSelectionChanged(void)), this, SLOT(handleSelectionChange(void)));
    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(handleItemDoubleClicked(QTreeWidgetItem *, int)));
    connect(_filttimer, SIGNAL(timeout()), this, SLOT(handleFilterTimerExpired(void)));
}

void BlockTreeWidget::mousePressEvent(QMouseEvent *event)
{
    this->setFocus();
    //if the item under the mouse is the bottom of the tree (a block, not category)
    //then we set a dragstartpos
    if(!itemAt(event->pos()))
    {
        QTreeWidget::mousePressEvent(event);
        return;
    }
    if(itemAt(event->pos())->childCount() == 0 and event->button() == Qt::LeftButton) {
        _dragStartPos = event->pos();
    }
    //pass the event along
    QTreeWidget::mousePressEvent(event);
}

void BlockTreeWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(!(event->buttons() & Qt::LeftButton))
    {
        QTreeWidget::mouseMoveEvent(event);
        return;
    }
    if((event->pos() - _dragStartPos).manhattanLength() < QApplication::startDragDistance())
    {
        QTreeWidget::mouseMoveEvent(event);
        return;
    }

    //get the block data
    auto blockItem = dynamic_cast<BlockTreeWidgetItem *>(itemAt(_dragStartPos));
    if (not blockItem->getBlockDesc()) return;

    //create a block object to render the image
    auto draw = dynamic_cast<GraphEditorTabs *>(getObjectMap()["editorTabs"])->getCurrentGraphEditor()->getCurrentGraphDraw();
    std::shared_ptr<GraphBlock> renderBlock(new GraphBlock(draw));
    renderBlock->setBlockDesc(blockItem->getBlockDesc());
    renderBlock->prerender(); //precalculate so we can get bounds
    const auto bounds = renderBlock->boundingRect();

    //draw the block's preview onto a mini pixmap
    QPixmap pixmap(bounds.size().toSize()+QSize(2,2));
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.translate(-bounds.topLeft()+QPoint(1,1));
    //painter.scale(zoomScale, zoomScale); //TODO get zoomscale from draw
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    renderBlock->render(painter);
    renderBlock.reset();
    painter.end();

    //create the drag object
    auto mimeData = new QMimeData();
    std::ostringstream oss; blockItem->getBlockDesc()->stringify(oss);
    QByteArray byteArray(oss.str().c_str(), oss.str().size());
    mimeData->setData("text/json/pothos_block", byteArray);
    auto drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->setHotSpot(-bounds.topLeft().toPoint());
    drag->exec(Qt::CopyAction | Qt::MoveAction);
}

void BlockTreeWidget::handleBlockDescUpdate(const Poco::JSON::Array::Ptr &blockDescs)
{
    _blockDescs = blockDescs;
    this->populate();
    this->resizeColumnToContents(0);
}

void BlockTreeWidget::handleFilterTimerExpired(void)
{
    this->clear();
    _rootNodes.clear();
    this->populate();
    for (auto item : this->findItems("", Qt::MatchContains, 0)) item->setExpanded(not _filter.isEmpty());
}

void BlockTreeWidget::handleFilter(const QString &filter)
{
    _filter = filter;
    //use the timer if search gets expensive.
    //otherwise just call handleFilterTimerExpired here.
    //_filttimer->start(UPDATE_TIMER_MS);
    handleFilterTimerExpired();
}

void BlockTreeWidget::handleSelectionChange(void)
{
    for (auto item : this->selectedItems())
    {
        auto b = dynamic_cast<BlockTreeWidgetItem *>(item);
        if (b != nullptr) emit blockDescEvent(b->getBlockDesc(), false);
    }
}

void BlockTreeWidget::handleItemDoubleClicked(QTreeWidgetItem *item, int)
{
    auto b = dynamic_cast<BlockTreeWidgetItem *>(item);
    if (b != nullptr) emit blockDescEvent(b->getBlockDesc(), true);
}

void BlockTreeWidget::populate(void)
{
    for (const auto &blockDescObj : *_blockDescs)
    {
        try
        {
            const auto blockDesc = blockDescObj.extract<Poco::JSON::Object::Ptr>();
            if (not this->blockDescMatchesFilter(blockDesc)) continue;
            const auto path = blockDesc->get("path").extract<std::string>();
            const auto name = blockDesc->get("name").extract<std::string>();
            if (blockDesc->isArray("categories")) for (auto categoryObj : *blockDesc->getArray("categories"))
            {
                const auto category = categoryObj.extract<std::string>().substr(1);
                const auto key = category.substr(0, category.find("/"));
                if (_rootNodes.find(key) == _rootNodes.end()) _rootNodes[key] = new BlockTreeWidgetItem(this, key);
                _rootNodes[key]->load(blockDesc, category + "/" + name);
            }
        }
        catch (const Poco::Exception &ex)
        {
            poco_error_f1(Poco::Logger::get("PothosGui.BlockTree"), "Failed JSON Doc parse %s", ex.displayText());
        }
    }

    //sort the columns alphabetically
    this->sortByColumn(0, Qt::AscendingOrder);

    emit this->blockDescEvent(Poco::JSON::Object::Ptr(), false); //unselect
}

bool BlockTreeWidget::blockDescMatchesFilter(const Poco::JSON::Object::Ptr &blockDesc)
{
    if (_filter.isEmpty()) return true;

    const auto path = blockDesc->get("path").extract<std::string>();
    const auto name = blockDesc->get("name").extract<std::string>();

    //construct a candidate string from path, name, categories, and keywords.
    std::string candidate = path+name;
    if (blockDesc->isArray("categories")) for (auto categoryObj : *blockDesc->getArray("categories"))
    {
        candidate += categoryObj.extract<std::string>();
    }
    if(blockDesc->isArray("keywords"))
    {
        const auto keywords = blockDesc->getArray("keywords");
        for(auto keyword : *keywords) candidate += keyword.extract<std::string>();
    }

    //reject if filter string not found in candidate
    candidate = Poco::toLower(candidate);
    const auto searchToken = Poco::toLower(_filter.toStdString());
    return (candidate.find(searchToken) != std::string::npos);
}

QMimeData *BlockTreeWidget::mimeData(const QList<QTreeWidgetItem *> items) const
{
    for (auto item : items)
    {
        auto b = dynamic_cast<BlockTreeWidgetItem *>(item);
        if (b == nullptr) continue;
        auto mimeData = new QMimeData();
        std::ostringstream oss; b->getBlockDesc()->stringify(oss);
        QByteArray byteArray(oss.str().c_str(), oss.str().size());
        mimeData->setData("text/json/pothos_block", byteArray);
        return mimeData;
    }
    return QTreeWidget::mimeData(items);
}
