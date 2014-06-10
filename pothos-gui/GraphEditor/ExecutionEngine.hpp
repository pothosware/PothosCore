// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <GraphObjects/GraphObject.hpp>
#include <QImage>

class ExecutionEngine
{
public:
    virtual ~ExecutionEngine(void);

    static ExecutionEngine *make(void);

    virtual void update(const GraphObjectList &graphObjects) = 0;

    virtual void activate(void) = 0;

    virtual void deactivate(void) = 0;

    virtual std::string toDotMarkup(void) = 0;
};
