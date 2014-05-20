# Copyright (c) 2014-2014 Josh Blum
# SPDX-License-Identifier: BSL-1.0

from . PothosModule import *

class Label(object):
    def __init__(self, data, index):
        self.data = data
        self.index = index

class LabelProxy(object):
    """
    LabelProxy is a wrapper for a ProxyObject holding a Label
    that provides the familiar field access for data and index.
    """

    def __init__(self, label):
        self._label = label

    def __getattr__(self, name):
        """
        WorkInfo in C++ has only fields.
        This call emulates field access.
        """
        return self._label.call(name)

    def proxy(self):
        """
        Get the underlying label proxy.
        """
        return self._label

class LabelIteratorRange(object):
    def __init__(self, labelIter):
        self._labelIter = labelIter

    def __getattr__(self, name):
        return lambda *args: self._labelIter.call(name, *args)

    def __iter__(self):
        index = 0
        while True:
            i = self._labelIter.at(index)
            if i == self._labelIter.end(): break
            yield LabelProxy(i.deref())
            index += 1
