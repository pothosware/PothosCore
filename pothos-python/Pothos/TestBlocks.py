# Copyright (c) 2014 Josh Blum
# SPDX-License-Identifier: BSL-1.0

import Pothos

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

        if self.workInfo().minElements:
            print('minElements %s'%self.workInfo().minElements)
            print('minInElements %s'%self.workInfo().minInElements)
            print('minOutElements %s'%self.workInfo().minOutElements)

        #forward message
        if self.input(0).hasMessage():
            m = self.input(0).popMessage()
            print('msg %s'%m)
            self.output(0).postMessage(m)

        #forward labels
        while True:
            loop = False
            for l in self.input(0).labels():
                print(l.index)
                print(l.data)
                self.output(0).postLabel(l)
                self.input(0).removeLabel(l)
                loop = True
                break
            if not loop: break

        #forward buffer
        if self.input(0).elements():
            print(self.input(0).dtype())
            print(self.input(0).buffer())

            out0 = self.output(0).buffer()
            in0 = self.input(0).buffer()
            n = min(len(out0), len(in0))
            out0[:n] = in0[:n]
            self.input(0).consume(n)
            self.output(0).produce(n)

    def propagateLabels(self, input, labels):
        print('propagateLabels')
        print(self.input(0).totalElements())
        for l in labels:
            print(l.index)
            print(l.data)
            #self.output(0).postLabel(l)
