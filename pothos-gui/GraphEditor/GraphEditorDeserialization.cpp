// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphEditor/GraphEditor.hpp"
#include "GraphEditor/GraphDraw.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include "GraphObjects/GraphBreaker.hpp"
#include "GraphObjects/GraphConnection.hpp"
#include "GraphObjects/GraphWidget.hpp"
#include <Pothos/Exception.hpp>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/Logger.h>
#include <cassert>

/***********************************************************************
 * Per-type creation routine
 **********************************************************************/
static void loadPages(GraphEditor *editor, Poco::JSON::Array::Ptr pages, const std::string &type)
{
    for (size_t pageNo = 0; pageNo < pages->size(); pageNo++)
    {
        auto pageObj = pages->getObject(pageNo);
        auto graphObjects = pageObj->getArray("graphObjects");
        auto parent = editor->widget(pageNo);

        for (size_t objIndex = 0; objIndex < graphObjects->size(); objIndex++)
        {
            GraphObject *obj = nullptr;
            POTHOS_EXCEPTION_TRY
            {
                const auto jGraphObj = graphObjects->getObject(objIndex);
                if (not jGraphObj) continue;
                const auto what = jGraphObj->getValue<std::string>("what");
                if (what != type) continue;
                if (type == "Block") obj = new GraphBlock(parent);
                if (type == "Breaker") obj = new GraphBreaker(parent);
                if (type == "Connection") obj = new GraphConnection(parent);
                if (type == "Widget") obj = new GraphWidget(parent);
                if (obj != nullptr) obj->deserialize(jGraphObj);
            }
            POTHOS_EXCEPTION_CATCH(const Pothos::Exception &ex)
            {
                poco_error(Poco::Logger::get("PothosGui.GraphEditor.loadState"), ex.displayText());
                delete obj;
            }
        }
    }
}

/***********************************************************************
 * Deserialization routine
 **********************************************************************/
void GraphEditor::loadState(std::istream &is)
{
    Poco::JSON::Parser p; p.parse(is);

    //extract topObj, old style is page array only
    Poco::JSON::Object::Ptr topObj;
    if (p.getHandler()->asVar().type() == typeid(Poco::JSON::Array::Ptr))
    {
        topObj = new Poco::JSON::Object();
        topObj->set("pages", p.getHandler()->asVar().extract<Poco::JSON::Array::Ptr>());
    }
    else
    {
        topObj = p.getHandler()->asVar().extract<Poco::JSON::Object::Ptr>();
    }

    //extract constants graph properties
    this->globals().clear();
    auto constants = topObj->getArray("constants");
    if (constants) for (size_t constNo = 0; constNo < constants->size(); constNo++)
    {
        const auto constantObj = constants->getObject(constNo);
        if (not constantObj->has("name")) continue;
        if (not constantObj->has("expr")) continue;
        this->globals().setGlobalExpression(
            QString::fromStdString(constantObj->getValue<std::string>("name")),
            QString::fromStdString(constantObj->getValue<std::string>("expr")));
    }

    //extract pages
    auto pages = topObj->getArray("pages");
    assert(pages);

    ////////////////////////////////////////////////////////////////////
    // clear existing stuff
    ////////////////////////////////////////////////////////////////////
    for (int pageNo = 0; pageNo < this->count(); pageNo++)
    {
        for (auto graphObj : this->getGraphDraw(pageNo)->getGraphObjects())
        {
            delete graphObj;
        }

        //delete page later so we dont mess up the tabs
        this->widget(pageNo)->deleteLater();
    }
    this->clear(); //removes all tabs from this widget

    ////////////////////////////////////////////////////////////////////
    // create pages
    ////////////////////////////////////////////////////////////////////
    for (size_t pageNo = 0; pageNo < pages->size(); pageNo++)
    {
        auto pageObj = pages->getObject(pageNo);
        auto pageName = pageObj->getValue<std::string>("pageName");
        auto graphObjects = pageObj->getArray("graphObjects");
        auto page = new GraphDraw(this);
        this->insertTab(int(pageNo), page, QString::fromStdString(pageName));
        if (pageObj->getValue<bool>("selected")) this->setCurrentIndex(pageNo);
    }

    ////////////////////////////////////////////////////////////////////
    // create graph objects
    ////////////////////////////////////////////////////////////////////
    loadPages(this, pages, "Block");
    loadPages(this, pages, "Breaker");
    loadPages(this, pages, "Connection");
    loadPages(this, pages, "Widget");
}
