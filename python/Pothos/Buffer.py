# Copyright (c) 2014 Josh Blum
# SPDX-License-Identifier: BSL-1.0

import numpy

def dtype_to_numpy(dtype):
    name = dtype.name()
    shape = [dtype.dimension()]

    #support numpy float-complex types
    if name == 'complex_float32': name = "complex64"
    elif name == 'complex_float64': name = "complex128"

    #no integer complex types, make tuple
    elif name.startswith('complex_'):
        name = name.split('_', 1)[1]
        shape = [2] + list(shape)

    return numpy.dtype((name, tuple(shape)))

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
