# Copyright (c) 2014 Josh Blum
# SPDX-License-Identifier: BSL-1.0

import Pothos

class SimpleSignalEmitter(Pothos.Block):
    def __init__(self):
        Pothos.Block.__init__(self)
        self.registerSignal("activateCalled")

    def activate(self):
        self.activateCalled("hello")

class SimpleSlotAcceptor(Pothos.Block):
    def __init__(self):
        Pothos.Block.__init__(self)
        self.registerSlot("activateHandler")

    def activateHandler(self, word):
        self._lastWord = word

    def getLastWord(self):
        return self._lastWord
