# Copyright (c) 2014-2014 Josh Blum
# SPDX-License-Identifier: BSL-1.0

from . PothosModule import *
from . Label import Label, LabelIteratorRange
from . Buffer import pointer_to_ndarray
from . Buffer import dtype_to_numpy
import numpy

class InputPort(object):
    def __init__(self, port):
        self._port = port

    def __getattr__(self, name):
        return lambda *args: self._port.call(name, *args)

    def labels(self):
        return LabelIteratorRange(self._port.labels())

    def dtype(self):
        return dtype_to_numpy(self._port.dtype())

    def buffer(self):
        addr = self._port.buffer().address
        nitems = self._port.elements()
        dtype = self.dtype()
        return pointer_to_ndarray(addr, nitems, dtype, readonly=True)

    def removeLabel(self, label):
        if isinstance(label, Proxy):# and label.getClassName() == "Pothos::Label":
            return self._port.removeLabel(label)
        raise Exception('InputPort.removeLabel - label must come from a label iterator')
