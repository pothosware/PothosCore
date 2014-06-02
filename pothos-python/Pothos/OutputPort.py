# Copyright (c) 2014-2014 Josh Blum
# SPDX-License-Identifier: BSL-1.0

from . PothosModule import *
from . Buffer import pointer_to_ndarray
from . Label import Label
import numpy

class OutputPort(object):
    def __init__(self, port):
        self._port = port

    def __getattr__(self, name):
        return lambda *args: self._port.call(name, *args)

    def dtype(self):
        dtype = self._port.dtype()
        return numpy.dtype((dtype.name(), tuple(dtype.shape())))

    def buffer(self):
        addr = self._port.buffer().address
        nitems = self._port.elements()
        dtype = self.dtype()
        return pointer_to_ndarray(addr, nitems, dtype, readonly=False)

    def postBuffer(self, buffer):
        raise NotImplementedError("postBuffer not implemented")

    def postLabel(self, label):
        if isinstance(label, Proxy) and label.getClassName() == "Pothos::Label":
            self._port.postLabel(label)
        elif isinstance(label, Label):
            cls = self._port.getEnvironment().findProxy("Pothos/Label")
            label = cls.new(label.data, label.index)
            self._port.postLabel(label)
        else:
            raise Exception('OutputPort.postLabel - unknown type %s'%type(label))

    def postMessage(self, message):
        self._port.postMessage(message)
