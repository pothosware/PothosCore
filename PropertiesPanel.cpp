// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include "GraphObjects/GraphObject.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>

class PropertiesPanelBlock : public QWidget
{
    Q_OBJECT
public:
    PropertiesPanelBlock(GraphBlock *block, QWidget *parent):
        QWidget(parent),
        _block(block)
    {
        auto layout = new QVBoxLayout();
        this->setLayout(layout);

        //title
        {
            auto label = new QLabel(QString("<h1>%1</h1>")
                .arg(_block->getTitle().toHtmlEscaped()), this);
            label->setAlignment(Qt::AlignCenter);
            layout->addWidget(label);
        }

        //properties
        for (const auto &prop : _block->getProperties())
        {
            auto propLayout = new QHBoxLayout();
            layout->addLayout(propLayout);

            auto label = new QLabel(QString("<b>%1</b>")
                .arg(prop.getName()), this);
            propLayout->addWidget(label);

            const auto value = _block->getPropertyValue(prop.getKey());
            auto edit = new QLineEdit(this);
            edit->setText(value);
            propLayout->addWidget(edit);
        }

    }

private:
    QPointer<GraphBlock> _block;
};

class PropertiesPanelTopWidget : public QStackedWidget
{
    Q_OBJECT
public:
    PropertiesPanelTopWidget(QWidget *parent):
        QStackedWidget(parent),
        _propertiesPanel(nullptr),
        _blockTree(makeBlockTree(this))
    {
        getObjectMap()["blockTree"] = _blockTree;
        this->addWidget(_blockTree);
    }

private slots:

    void handleGraphModifyProperties(GraphObject *obj)
    {
        auto block = dynamic_cast<GraphBlock *>(obj);
        if (block == nullptr) this->setCurrentWidget(_blockTree);
        else
        {
            //TODO connect panel delete to block delete
            delete _propertiesPanel;
            _propertiesPanel = new PropertiesPanelBlock(block, this);
            this->addWidget(_propertiesPanel);
            this->setCurrentWidget(_propertiesPanel);
        }
    }

private:
    QWidget *_propertiesPanel;
    QWidget *_blockTree;
};

QWidget *makePropertiesPanel(QWidget *parent)
{
    return new PropertiesPanelTopWidget(parent);
}

#include "PropertiesPanel.moc"
