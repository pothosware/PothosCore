// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include "GraphObjects/GraphObject.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QScrollArea>
#include <QLabel>

class PropertiesPanelBlock : public QScrollArea
{
    Q_OBJECT
public:
    PropertiesPanelBlock(GraphBlock *block, QWidget *parent):
        QScrollArea(parent),
        _block(block)
    {
        this->setWidgetResizable(true);
        this->setWidget(new QWidget(this));
        auto blockDesc = block->getBlockDesc();
        auto layout = new QVBoxLayout();
        this->widget()->setLayout(layout);

        //title
        {
            auto label = new QLabel(QString("<h1>%1</h1>")
                .arg(_block->getTitle().toHtmlEscaped()), this);
            label->setAlignment(Qt::AlignCenter);
            layout->addWidget(label);
        }

        //path
        {
            auto path = blockDesc->get("path").convert<std::string>();
            auto label = new QLabel(QString("<p>(%1)</p>")
                .arg(QString::fromStdString(path).toHtmlEscaped()), this);
            label->setAlignment(Qt::AlignCenter);
            layout->addWidget(label);
        }

        //id
        {
            auto propLayout = new QHBoxLayout();
            layout->addLayout(propLayout);

            auto label = new QLabel(QString("<b>%1</b>")
                .arg(tr("ID")), this);
            propLayout->addWidget(label);

            auto edit = new QLineEdit(this);
            edit->setText(block->getId());
            propLayout->addWidget(edit);
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

        //block level description
        if (blockDesc->isArray("docs"))
        {
            const auto docsArray = blockDesc->getArray("docs");
            auto text = new QTextEdit(this);
            layout->addWidget(text);

            QString output;
            output += QString("<p><b>%1</b></p>").arg(QString::fromStdString(blockDesc->get("name").convert<std::string>()));
            output += "<p>";
            for (size_t i = 0; i < docsArray->size(); i++)
            {
                const auto line = docsArray->get(i).convert<std::string>();
                if (line.empty()) output += "<p /><p>";
                else output += QString::fromStdString(line);
                output += "\n";
            }
            output += "</p>";

            //enumerate slots
            output += QString("<p><b>%1</b></p>").arg(tr("Available Slots"));
            output += "<ul>";
            for (const auto &port : block->getSlotPorts())
            {
                output += QString("<li>%1(...)</li>").arg(port.getName());
            }
            if (block->getSlotPorts().empty())
            {
                output += QString("<li>%1</li>").arg(tr("None"));
            }
            output += "</ul>";

            //enumerate signals
            output += QString("<p><b>%1</b></p>").arg(tr("Available Signals"));
            output += "<ul>";
            for (const auto &port : block->getSignalPorts())
            {
                output += QString("<li>%1(...)</li>").arg(port.getName());
            }
            if (block->getSignalPorts().empty())
            {
                output += QString("<li>%1</li>").arg(tr("None"));
            }
            output += "</ul>";

            text->insertHtml(output);
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
