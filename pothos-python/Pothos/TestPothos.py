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
        self.assertFalse(p0)

        #the proxy for empty object (is non null)
        p1 = self.env.convertObjectToProxy(None)
        self.assertTrue(p1)

        #make a proxy that wraps a python obj
        p2 = Pothos.Proxy(True)
        self.assertTrue(p2)
        self.assertTrue(p2.convert())

        #check that the same env can be retrieved
        self.assertTrue(p1.getEnvironment() == self.env)

    def test_compare_to(self):
        #create integer proxies
        oneProxy = self.env.convertObjectToProxy(1)
        self.assertTrue(oneProxy)
        twoProxy = self.env.convertObjectToProxy(2)
        self.assertTrue(twoProxy)

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
        self.assertTrue(reg)
        print(reg.callProxy("/blocks/feeder_source", "int"))
        print(hash(reg))

def main():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestPothosModule)
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    if not result.wasSuccessful(): raise Exception("unittest FAIL")

if __name__ == '__main__':
    main()
