// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphEditor/GraphEditor.hpp"
#include "GraphEditor/GraphDraw.hpp"

/***********************************************************************
 * Serialization routine
 **********************************************************************/
void GraphEditor::dumpState(std::ostream &os) const
{
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

    pages.stringify(os, 4/*indent*/);
}
