# The Pothos-Serialization C++ library

The Pothos Serialization library is a direct copy of the Boost Serialization library;
however its completely separated from Boost, and with namespace and MACROs renamed.
The Boost-Serialization library is desirable because of its dynamic type registry,
however pulling in Boost as a core dependency of Pothos was too burdensome.
This extraction of the serialization library builds and installs as a stand-alone project.

## Building and installing

This is a stand-alone CMake project with no external dependencies.
Installation of the Pothos-Serialization library will not interfere with an installation of Boost.

## Licensing information

Use, modification and distribution is subject to the Boost Software
License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)

## More information

This source was generated and extracted from a checkout of the Boost library using the included extract.py python script.
The source contains only a handful of C++ source files to compile.
However, the serialization component pulls in literally thousands of Boost framework headers included along with the project.
