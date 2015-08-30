# Support for OpenCL-based processing blocks

## Purpose

This component provides support for using OpenCL in the Pothos framework.
The OpenClKernel block allows the execution of array-based OpenCl kernels inside a Pothos Topology.
The block uses the Pothos DMA API to integrate OpenCL allocated buffers with the processing topology.

In addition, this component provides a device info plugin so the PothosGui
and others can query information about OpenCl on a particular system.

## Dependencies

* Pothos library
* OpenCl development libraries and headers

## Building

configure, build, and install with CMake

## Licensing information

Use, modification and distribution is subject to the Boost Software
License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
