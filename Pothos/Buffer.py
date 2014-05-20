# Copyright (c) 2014 Josh Blum
# SPDX-License-Identifier: BSL-1.0

import numpy

def pointer_to_ndarray(addr, nitems, dtype=numpy.dtype(numpy.uint8), readonly=False):
    class array_like:
        __array_interface__ = {
            'data' : (addr, readonly),
            'typestr' : dtype.base.str,
            'descr' : dtype.base.descr,
            'shape' : (nitems,) + dtype.shape,
            'strides' : None,
            'version' : 3,
        }
    return numpy.asarray(array_like()).view(dtype.base)
