// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include <QTreeWidget>
#include "GraphObjects/GraphBlock.hpp"
#include "GraphEditor/Constants.hpp"
#include <QMimeData>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMouseEvent>
#include <QApplication>
#include <QTimer>
#include <QDrag>
#include <QPainter>
#include <QLineEdit>
#include <QAction>
#include <QDockWidget>
#include <Poco/Logger.h>
#include <iostream>
#include <sstream>
#include <map>

static const long UPDATE_TIMER_MS = 500;

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

    void load(const Poco::JSON::Object::Ptr &blockDesc, const std::string &category, const size_t depth = 0)
    {
        const auto slashIndex = category.find("/");
        const auto catName = category.substr(0, slashIndex);
        if (slashIndex == std::string::npos)
        {
            _blockDesc = blockDesc;
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
            _subNodes[key]->load(blockDesc, catRest, depth+1);
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
        const auto doc = extractDocString(_blockDesc);
        if (doc.isEmpty()) return;
        this->setToolTip(0, doc);
    }

    Poco::JSON::Object::Ptr getBlockDesc(void) const
    {
        return _blockDesc;
    }

    static QString extractDocString(Poco::JSON::Object::Ptr blockDesc)
    {
        if (not blockDesc or not blockDesc->isArray("docs")) return "";
        QString output;
        output += "<b>" + QString::fromStdString(blockDesc->get("name").convert<std::string>()) + "</b>";
        output += "<p>";
        for (const auto &lineObj : *blockDesc->getArray("docs"))
        {
            const auto line = lineObj.extract<std::string>();
            if (line.empty()) output += "<p /><p>";
            else output += QString::fromStdString(line)+"\n";
        }
        output += "</p>";
        return "<div>" + output + "</div>";
    }

private:
    std::map<std::string, BlockTreeWidgetItem *> _subNodes;
    Poco::JSON::Object::Ptr _blockDesc;
};

/***********************************************************************
 * Block Tree widget builder
 **********************************************************************/
class BlockTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:

    BlockTreeWidget(QWidget *parent):
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

    void mousePressEvent(QMouseEvent *event)
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

    void mouseMoveEvent(QMouseEvent *event)
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
        std::shared_ptr<GraphBlock> renderBlock(new GraphBlock(nullptr));
        renderBlock->setBlockDesc(blockItem->getBlockDesc());
        renderBlock->prerender(); //precalculate so we can get bounds
        const auto bounds = renderBlock->getBoundingRect();

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

signals:
    void blockDescEvent(const Poco::JSON::Object::Ptr &, bool);

public slots:
    void handleBlockDescUpdate(const Poco::JSON::Array::Ptr &blockDescs)
    {
        _blockDescs = blockDescs;
        this->populate();
        this->resizeColumnToContents(0);
    }

private slots:
    void handleFilterTimerExpired(void)
    {
        this->clear();
        _rootNodes.clear();
        this->populate();
    }

    void handleFilter(const QString &filter)
    {
        _filter = filter;
        //use the timer if search gets expensive.
        //otherwise just call handleFilterTimerExpired here.
        //_filttimer->start(UPDATE_TIMER_MS);
        handleFilterTimerExpired();
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

    void populate(void)
    {
        for (const auto &blockDescObj : *_blockDescs)
        {
            try
            {
                const auto blockDesc = blockDescObj.extract<Poco::JSON::Object::Ptr>();
                if (not this->blockDescMatchesFilter(blockDesc)) continue;
                const auto path = blockDesc->get("path").extract<std::string>();
                const auto name = blockDesc->get("name").extract<std::string>();
                const auto categories = blockDesc->getArray("categories");
                if (categories) for (auto categoryObj : *categories)
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
    }

    bool blockDescMatchesFilter(const Poco::JSON::Object::Ptr &blockDesc)
    {
        if (_filter.isEmpty()) return true;

        const auto path = blockDesc->get("path").extract<std::string>();
        const auto name = blockDesc->get("name").extract<std::string>();
        const auto categories = blockDesc->getArray("categories");

        //construct a candidate string from path, name, categories, and keywords.
        std::string candidate = path+name;
        if (categories) for(auto category : *categories)
        {
            candidate += category.extract<std::string>();
        }
        if(blockDesc->isArray("keywords"))
        {
            const auto keywords = blockDesc->getArray("keywords");
            for(auto keyword : *keywords) candidate += keyword.extract<std::string>();
        }

        //reject if filter string not found in candidate
        return (candidate.find(_filter.toStdString()) != std::string::npos);
    }

    QMimeData *mimeData(const QList<QTreeWidgetItem *> items) const
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

    QString _filter;
    QTimer *_filttimer;
    QPoint _dragStartPos;
    Poco::JSON::Array::Ptr _blockDescs;
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

        auto search = new QLineEdit(this);
        search->setPlaceholderText(tr("Search blocks"));
#if QT_VERSION > 0x050200
        search->setClearButtonEnabled(true);
#endif
        layout->addWidget(search);

        _blockTree = new BlockTreeWidget(this);
        connect(getObjectMap()["blockCache"], SIGNAL(blockDescUpdate(const Poco::JSON::Array::Ptr &)),
            _blockTree, SLOT(handleBlockDescUpdate(const Poco::JSON::Array::Ptr &)));
        connect(_blockTree, SIGNAL(blockDescEvent(const Poco::JSON::Object::Ptr &, bool)),
            this, SLOT(handleBlockDescEvent(const Poco::JSON::Object::Ptr &, bool)));
        connect(search, SIGNAL(textChanged(const QString &)), _blockTree, SLOT(handleFilter(const QString &)));
        layout->addWidget(_blockTree);

        _addButton = new QPushButton(makeIconFromTheme("list-add"), "Add Block", this);
        layout->addWidget(_addButton);
        connect(_addButton, SIGNAL(released(void)), this, SLOT(handleAdd(void)));
        _addButton->setEnabled(false); //default disabled

        //on ctrl-f or edit:find, set focus on search window and select all text
        connect(getActionMap()["find"], SIGNAL(triggered(void)), search, SLOT(setFocus(void)));
        connect(getActionMap()["find"], SIGNAL(triggered(void)), search, SLOT(selectAll(void)));
        auto dock = dynamic_cast<QDockWidget *>(parent);
        assert(dock != nullptr);
        connect(getActionMap()["find"], SIGNAL(triggered(void)), dock, SLOT(show(void)));
        connect(getActionMap()["find"], SIGNAL(triggered(void)), dock, SLOT(raise(void)));
    }

signals:
    void addBlockEvent(const Poco::JSON::Object::Ptr &);

private slots:
    void handleAdd(void)
    {
        emit addBlockEvent(_blockDesc);
    }

    void handleBlockDescEvent(const Poco::JSON::Object::Ptr &blockDesc, bool add)
    {
        _blockDesc = blockDesc;
        _addButton->setEnabled(bool(blockDesc));
        if (add) emit addBlockEvent(_blockDesc);
    }

private:
    QPushButton *_addButton;
    Poco::JSON::Object::Ptr _blockDesc;
    BlockTreeWidget *_blockTree;
};

QWidget *makeBlockTree(QWidget *parent)
{
    return new BlockTreeTopWindow(parent);
}

#include "BlockTree.moc"
