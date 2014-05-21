# Copyright (c) 2014-2014 Josh Blum
# SPDX-License-Identifier: BSL-1.0

import Pothos
import unittest

class TestPothosModule(unittest.TestCase):

    def setUp(self):
        self.env = Pothos.ProxyEnvironment("managed")
        self.assertEqual(self.env.getName(), "managed")

    def test_basic(self):
        #create null/empty proxy
        p0 = Pothos.Proxy()
        self.assertTrue(p0.null())

        #the proxy for empty object (is non null)
        p1 = self.env.convertObjectToProxy(None)
        self.assertFalse(p1.null())

        #make a proxy that wraps a python obj
        p2 = Pothos.Proxy(True)
        self.assertTrue(not p2.null())
        self.assertTrue(p2.convert())

        #check that the same env can be retrieved
        self.assertTrue(p1.getEnvironment() == self.env)

    def test_compare_to(self):
        #create integer proxies
        oneProxy = self.env.convertObjectToProxy(1)
        self.assertFalse(oneProxy.null())
        twoProxy = self.env.convertObjectToProxy(2)
        self.assertFalse(twoProxy.null())

        #exercise rich-comparisons
        self.assertLess(oneProxy, twoProxy)
        self.assertLessEqual(oneProxy, twoProxy)
        self.assertLessEqual(oneProxy, oneProxy)
        self.assertGreater(twoProxy, oneProxy)
        self.assertGreaterEqual(twoProxy, oneProxy)
        self.assertGreaterEqual(twoProxy, twoProxy)
        self.assertNotEqual(oneProxy, twoProxy)
        self.assertEqual(twoProxy, twoProxy)

        #mix python objects with non-proxies
        self.assertLess(oneProxy, 2)
        self.assertLess(1, twoProxy)

    def test_block(self):
        reg = self.env.findProxy("Pothos/BlockRegistry")
        self.assertFalse(reg.null())
        print(reg.callProxy("/blocks/sources/feeder_source", "int"))
        print(hash(reg))

def main():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestPothosModule)
    unittest.TextTestRunner(verbosity=2).run(suite)

if __name__ == '__main__':
    main()
