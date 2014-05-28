# Copyright (c) 2014-2014 Josh Blum
# SPDX-License-Identifier: BSL-1.0

from . PothosModule import *
from . InputPort import InputPort
from . OutputPort import OutputPort
from . Label import Label, LabelIteratorRange
import weakref

class WorkInfoProxy(object):
    """
    WorkInfoProxy is a wrapper for a ProxyObject holding a WorkInfo
    that provides the familiar field access min/max number of elements.
    """

    def __init__(self, info):
        self._info = info

    def __getattr__(self, name):
        """
        WorkInfo in C++ has only fields.
        This call emulates field access.
        """
        return self._info.call(name)

class Block(object):
    def __init__(self):
        env = ProxyEnvironment("managed")
        reg = env.findProxy("Pothos/BlockRegistry")
        self._block = reg.callProxy("/blocks/python/python_block")
        self._block._setPyBlock(Proxy(Proxy(weakref.proxy(self))))

    def __getattr__(self, name):
        return lambda *args: self._block.call(name, *args)

    def workInfo(self):
        return WorkInfoProxy(self._block.workInfo())

    def setupInput(self, name, dtype="byte"):
        self._block.setupInput(name, dtype)

    def setupOutput(self, name, dtype="byte"):
        self._block.setupOutput(name, dtype)

    def inputs(self):
        ports = self._block.inputs()
        return [InputPort(ports.at(i)) for i in range(ports.size())]

    def allInputs(self):
        ports = self._block.allInputs()
        return dict([(key, InputPort(ports.at(key))) for key in ports.keys()])

    def input(self, name):
        return InputPort(self._block.input(name))

    def outputs(self):
        ports = self._block.outputs()
        return [OutputPort(ports.at(i)) for i in range(ports.size())]

    def allOutputs(self):
        ports = self._block.allOutputs()
        return dict([(key, OutputPort(ports.at(key))) for key in ports.keys()])

    def output(self, name):
        return OutputPort(self._block.output(name))

    def activate(self): pass

    def deactivate(self): pass

    def work(self): pass

    def propagateLabels(self, input, labels): pass

    def propagateLabelsAdaptor(self, input, labels):
        self.propagateLabels(InputPort(input), LabelIteratorRange(labels))

    def emitSignal(self, name, *args):
        self._block._emitSignalArgs(name, list(args))
