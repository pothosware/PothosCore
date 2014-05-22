// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include <QTreeWidget>
#include <Pothos/Remote.hpp>
#include <Pothos/Proxy.hpp>
#include "GraphObjects/GraphBlock.hpp"
#include "GraphEditor/Constants.hpp"
#include <QMimeData>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <QMouseEvent>
#include <QApplication>
#include <QDrag>
#include <QPainter>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/Logger.h>
#include <iostream>
#include <sstream>
#include <map>

/***********************************************************************
 * Custom tree widget item
 **********************************************************************/
class BlockTreeWidgetItem : public QTreeWidgetItem
{
public:
    template <typename ParentType>
    BlockTreeWidgetItem(ParentType *parent, const std::string &name):
        QTreeWidgetItem(parent, QStringList(QString::fromStdString(name)))
    {
        return;
    }

    void load(const std::string &nodeKey, const Poco::JSON::Object::Ptr &blockDesc, const std::string &category, const size_t depth = 0)
    {
        const auto slashIndex = category.find("/");
        const auto catName = category.substr(0, slashIndex);
        if (slashIndex == std::string::npos)
        {
            _nodeKeyToBlockDesc[nodeKey] = blockDesc;
        }
        else
        {
            const auto catRest = category.substr(slashIndex+1);
            const auto key = catRest.substr(0, catRest.find("/"));
            if (_subNodes.find(key) == _subNodes.end())
            {
                _subNodes[key] = new BlockTreeWidgetItem(this, key);
                _subNodes[key]->setExpanded(depth < 2);
            }
            _subNodes[key]->load(nodeKey, blockDesc, catRest, depth+1);
        }
    }

    //this sets a tool tip -- but only when requested
    QVariant data(int column, int role) const
    {
        if (role == Qt::ToolTipRole)
        {
            //sorry about this cast
            const_cast<BlockTreeWidgetItem *>(this)->setToolTipOnRequest();
        }
        return QTreeWidgetItem::data(column, role);
    }

    void setToolTipOnRequest(void)
    {
        for (const auto &elem : _nodeKeyToBlockDesc)
        {
            if (not elem.second) continue;
            const auto doc = extractDocString(elem.second);
            if (not doc.isEmpty()) {this->setToolTip(0, doc); return;}
        }
    }

    QByteArray getBlockDesc(void) const
    {
        for (const auto &elem : _nodeKeyToBlockDesc)
        {
            if (not elem.second) continue;
            std::ostringstream oss; elem.second->stringify(oss);
            return QByteArray(oss.str().c_str(), oss.str().size());
        }
        return QByteArray();
    }

    static QString extractDocString(Poco::JSON::Object::Ptr blocDesc)
    {
        if (not blocDesc or not blocDesc->isArray("docs")) return "";
        const auto docsArray = blocDesc->getArray("docs");
        QString output;
        output += "<b>" + QString::fromStdString(blocDesc->get("name").convert<std::string>()) + "</b><br />";
        for (size_t i = 0; i < docsArray->size(); i++)
        {
            const auto line = docsArray->get(i).convert<std::string>();
            if (line.empty()) output += "<br />";
            else output += QString::fromStdString(line);
        }
        return "<div>" + output + "</div>";
    }

private:
    std::map<std::string, BlockTreeWidgetItem *> _subNodes;
    std::map<std::string, Poco::JSON::Object::Ptr> _nodeKeyToBlockDesc;
};

/***********************************************************************
 * Query JSON docs from node
 **********************************************************************/
static std::string queryJSONDocs(const std::string &nodeKey)
{
    try
    {
        auto node = Pothos::RemoteNode::fromKey(nodeKey);
        auto env = node.makeClient("json").makeEnvironment("managed");
        return env->findProxy("Pothos/Gui/DocUtils").call<std::string>("dumpJson");
    }
    catch (const Pothos::Exception &ex)
    {
        const auto uri = Pothos::RemoteNode::fromKey(nodeKey).getUri();
        poco_error_f2(Poco::Logger::get("PothosGui.BlockTree"), "Failed to query JSON Docs from %s - %s", uri, ex.displayText());
    }

    return "[]"; //empty JSON array
}

/***********************************************************************
 * Block Tree widget builder
 **********************************************************************/
class BlockTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:

    BlockTreeWidget(QWidget *parent):
        QTreeWidget(parent),
        _watcher(new QFutureWatcher<std::string>(this))
    {
        QStringList columnNames;
        columnNames.push_back(tr("Available Blocks"));
        this->setColumnCount(columnNames.size());
        this->setHeaderLabels(columnNames);

        connect(_watcher, SIGNAL(resultReadyAt(int)), this, SLOT(handleWatcherDone(int)));
        connect(_watcher, SIGNAL(finished()), this, SLOT(handleWatcherFinished()));
        connect(this, SIGNAL(itemSelectionChanged(void)), this, SLOT(handleSelectionChange(void)));
        connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(handleItemDoubleClicked(QTreeWidgetItem *, int)));

        //nodeKeys cannot be a temporary because QtConcurrent will reference them
        _allNodeKeys = Pothos::RemoteNode::listRegistryKeys();
        _watcher->setFuture(QtConcurrent::mapped(_allNodeKeys, &queryJSONDocs));
    }

    void mousePressEvent(QMouseEvent *event)
    {
        this->setFocus();
        //if the item under the mouse is the bottom of the tree (a block, not category)
        //then we set a dragstartpos
        if(itemAt(event->pos())->childCount() == 0 and event->button() == Qt::LeftButton) {
            _dragStartPos = event->pos();
        }
        //pass the event along
        QTreeWidget::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent *event)
    {
        if(!(event->buttons() & Qt::LeftButton)) QTreeWidget::mouseMoveEvent(event);
        if((event->pos() - _dragStartPos).manhattanLength() < QApplication::startDragDistance()) QTreeWidget::mouseMoveEvent(event);
        auto drag = new QDrag(this);
        //get the block data
        auto blockItem = dynamic_cast<BlockTreeWidgetItem *>(itemAt(_dragStartPos));
        if(blockItem->getBlockDesc().isEmpty()) return;
        auto mimeData = new QMimeData();
        mimeData->setData("text/json/pothos_block", blockItem->getBlockDesc());
        drag->setMimeData(mimeData);
        auto renderBlock = new GraphBlock(this);
        renderBlock->setBlockDesc(blockItem->getBlockDesc());
        auto pixmap = new QPixmap(200,200); //TODO this doesn't account for the size of the block
        pixmap->fill(Qt::transparent);
        auto painter = new QPainter(pixmap);
        renderBlock->setPosition(QPointF(pixmap->width()/2, pixmap->height()/2));
        renderBlock->render(*painter);
        painter->end();
        drag->setPixmap(*pixmap);
        drag->setHotSpot(QPoint(pixmap->width()/2, pixmap->height()/2));
        drag->exec(Qt::CopyAction | Qt::MoveAction);
    }

signals:
    void blockDescEvent(const QByteArray &, bool);

private slots:

    void handleWatcherFinished(void)
    {
        this->resizeColumnToContents(0);
    }

    void handleWatcherDone(const int which)
    {
        auto json = _watcher->resultAt(which);
        Poco::JSON::Parser p; p.parse(json);
        auto array = p.getHandler()->asVar().extract<Poco::JSON::Array::Ptr>();
        for (size_t i = 0; i < array->size(); i++)
        {
            try
            {
                const auto blockDesc = array->getObject(i);
                if (not blockDesc) continue;
                const auto path = blockDesc->get("path").extract<std::string>();
                const auto name = blockDesc->get("name").extract<std::string>();
                const auto categories = blockDesc->getArray("categories");
                if (not categories) continue;
                for (size_t ci = 0; ci < categories->size(); ci++)
                {
                    const auto category = categories->get(ci).extract<std::string>().substr(1);
                    const auto key = category.substr(0, category.find("/"));
                    if (_rootNodes.find(key) == _rootNodes.end()) _rootNodes[key] = new BlockTreeWidgetItem(this, key);
                    _rootNodes[key]->load(_allNodeKeys[which], blockDesc, category + "/" + name);
                }
            }
            catch (const Poco::Exception &ex)
            {
                poco_error_f1(Poco::Logger::get("PothosGui.BlockTree"), "Failed JSON Doc parse %s", ex.displayText());
            }
        }
    }

    void handleSelectionChange(void)
    {
        for (auto item : this->selectedItems())
        {
            auto b = dynamic_cast<BlockTreeWidgetItem *>(item);
            if (b != nullptr) emit blockDescEvent(b->getBlockDesc(), false);
        }
    }

    void handleItemDoubleClicked(QTreeWidgetItem *item, int)
    {
        auto b = dynamic_cast<BlockTreeWidgetItem *>(item);
        if (b != nullptr) emit blockDescEvent(b->getBlockDesc(), true);
    }

private:

    QMimeData *mimeData(const QList<QTreeWidgetItem *> items) const
    {
        for (auto item : items)
        {
            auto b = dynamic_cast<BlockTreeWidgetItem *>(item);
            if (b == nullptr) continue;
            auto mimeData = new QMimeData();
            mimeData->setData("text/json/pothos_block", b->getBlockDesc());
            return mimeData;
        }
        return QTreeWidget::mimeData(items);
    }

    QPoint _dragStartPos;
    std::vector<std::string> _allNodeKeys;
    QFutureWatcher<std::string> *_watcher;
    std::map<std::string, BlockTreeWidgetItem *> _rootNodes;
};

/***********************************************************************
 * Top level block tree widget
 **********************************************************************/
class BlockTreeTopWindow : public QWidget
{
    Q_OBJECT
public:
    BlockTreeTopWindow(QWidget *parent):
        QWidget(parent)
    {
        auto layout = new QVBoxLayout(this);
        this->setLayout(layout);

        auto tree = new BlockTreeWidget(this);
        connect(tree, SIGNAL(blockDescEvent(const QByteArray &, bool)),
            this, SLOT(handleBlockDescEvent(const QByteArray &, bool)));
        layout->addWidget(tree);

        _addButton = new QPushButton(makeIconFromTheme("list-add"), "Add Block", this);
        layout->addWidget(_addButton);
        connect(_addButton, SIGNAL(released(void)), this, SLOT(handleAdd(void)));
        _addButton->setEnabled(false); //default disabled
    }

signals:
    void addBlockEvent(const QByteArray &);

private slots:
    void handleAdd(void)
    {
        emit addBlockEvent(_blockDesc);
    }

    void handleBlockDescEvent(const QByteArray &blockDesc, bool add)
    {
        _blockDesc = blockDesc;
        _addButton->setEnabled(not blockDesc.isEmpty());
        if (add) emit addBlockEvent(_blockDesc);
    }

private:
    QPushButton *_addButton;
    QByteArray _blockDesc;
};

QWidget *makeBlockTree(QWidget *parent)
{
    return new BlockTreeTopWindow(parent);
}

#include "BlockTree.moc"
