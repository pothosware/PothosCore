# Copyright (c) 2014 Josh Blum
# SPDX-License-Identifier: BSL-1.0

import Pothos

"""/*
|PothosDoc Forwarder (python)

The Python forwarder block forwards all data
from input port 0 to the output port 0.
This block is mainly used for testing purposes.

|category /Misc
|keywords forwarder

|param dtype[Data Type] The input and output data type.
|default "float32"
|widget StringEntry()

|factory /python/forwarder(dtype)
*/"""
class Forwarder(Pothos.Block):
    def __init__(self, dtype):
        Pothos.Block.__init__(self)
        self.setupInput("0", dtype)
        self.setupOutput("0", dtype)

    def activate(self):
        print('activate called')

    def deactivate(self):
        print('deactivate called')

    def work(self):

        #forward message
        if self.input(0).hasMessage():
            m = self.input(0).popMessage()
            #print('msg %s'%m)
            self.output(0).postMessage(m)

        #for testing purposes:
        #forward and remove the first label,
        #use propagateLabels for the rest
        for l in self.input(0).labels():
            #print(l.index)
            #print(l.data)
            self.output(0).postLabel(l)
            self.input(0).removeLabel(l)
            break

        #forward buffer
        if self.input(0).elements():
            #print(self.input(0).dtype())
            #print(self.input(0).buffer())

            out0 = self.output(0).buffer()
            in0 = self.input(0).buffer()
            n = min(len(out0), len(in0))
            out0[:n] = in0[:n]
            self.input(0).consume(n)
            self.output(0).produce(n)

    def propagateLabels(self, input):
        #print('propagateLabels')
        print(self.input(0).totalElements())
        for l in input.labels():
            #print(l.index)
            #print(l.data)
            self.output(0).postLabel(l)
