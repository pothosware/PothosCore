// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphEditor/GraphEditor.hpp"
#include "GraphEditor/GraphDraw.hpp"

/***********************************************************************
 * Serialization routine
 **********************************************************************/
void GraphEditor::dumpState(std::ostream &os) const
{
    Poco::JSON::Object topObj;

    //store global variables
    Poco::JSON::Array globals;
    for (const auto &name : this->listGlobals())
    {
        const auto &value = this->getGlobalExpression(name);
        Poco::JSON::Object globalObj;
        globalObj.set("name", name.toStdString());
        globalObj.set("value", value.toStdString());
        globals.add(globalObj);
    }
    topObj.set("globals", globals);

    //store pages
    Poco::JSON::Array pages;
    for (int pageNo = 0; pageNo < this->count(); pageNo++)
    {
        Poco::JSON::Object page;
        Poco::JSON::Array graphObjects;
        page.set("pageName", this->tabText(pageNo).toStdString());
        page.set("selected", this->currentIndex() == pageNo);

        for (auto graphObj : this->getGraphDraw(pageNo)->getGraphObjects())
        {
            graphObjects.add(graphObj->serialize());
        }

        page.set("graphObjects", graphObjects);
        pages.add(page);
    }
    topObj.set("pages", pages);

    topObj.stringify(os, 4/*indent*/);
}
