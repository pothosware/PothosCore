// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Managed.hpp>
#include <Pothos/Proxy.hpp>

class PythonBlock : Pothos::Block
{
public:
    PythonBlock(void)
    {
        this->registerCall(POTHOS_FCN_TUPLE(PythonBlock, _setPyBlock));
    }

    static Block *make(void)
    {
        return new PythonBlock();
    }

    void _setPyBlock(const Pothos::Proxy &block)
    {
        _block = block;
    }

    void work(void)
    {
        _block.callProxy("work");
    }

    void activate(void)
    {
        _block.callProxy("activate");
    }

    void deactivate(void)
    {
        _block.callProxy("deactivate");
    }

    void propagateLabels(const Pothos::InputPort *input, const Pothos::LabelIteratorRange &labels)
    {
        _block.callProxy("propagateLabelsAdaptor", input, labels);
    }

    Pothos::Object opaqueCallHandler(const std::string &name, const Pothos::Object *inputArgs, const size_t numArgs)
    {
        if (name == "_setPyBlock") return Pothos::Block::opaqueCallHandler(name, inputArgs, numArgs);
        if (not _block) throw name;
        auto env = _block.getEnvironment();
        Pothos::ProxyVector args(numArgs);
        for (size_t i = 0; i < numArgs; i++)
        {
            args[i] = env->convertObjectToProxy(inputArgs[i]);
        }
        auto result = _block.getHandle()->call(name, args.data(), args.size());
        return env->convertProxyToObject(result);
    }

    Pothos::Proxy _block;
};

static Pothos::BlockRegistry registerSocketSink(
    "/blocks/python_block", &PythonBlock::make);
