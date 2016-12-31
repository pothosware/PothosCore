# Pothos framework library and build environment

This is the project for the Pothos framework library.
It also contains submodules for various dependencies
and Pothos toolkits to make it easy to install
the entire Pothos software suite in one build.

##Documentation

* https://github.com/pothosware/pothos/wiki

##Build Status

- Travis: [![Travis Build Status](https://travis-ci.org/pothosware/pothos.svg?branch=master)](https://travis-ci.org/pothosware/pothos)
- AppVeyor: [![AppVeyor Build status](https://ci.appveyor.com/api/projects/status/7owo8qb9oldw8iq8)](https://ci.appveyor.com/project/guruofquality/pothos)

## Dependencies

The poco, muparserx, and spuce libraries are available
as git submodules to simplify building this project from source.
The user may install these dependencies separately prior to building,
or the build will install them automatically from submodule if missing.

* poco C++ utility library (framework+toolkit dependency)
* muparserx expression parser (framework dependency)
* spuce filter design library (toolkit dependency)

## Toolkits

Although all toolkits can be built as standalone cmake projects,
user may wish to build these toolkits alongside the framework
library to simplfy the effort involved building from source.
The following toolkits are available as git submodules:

* pothos-audio
* pothos-blocks
* pothos-comms
* pothos-gui
* pothos-plotters
* pothos-python
* pothos-sdr
* pothos-widgets

## Building

configure, build, and install with CMake

## Layout

* include/ - public API C++11 header files
* lib/ - C++ sources for the Pothos library
* docs/ - doxygen configuration and generation
* apps/ - the source for the PothosUtil binary
* cmake/ - cmake utilities for building projects

## Licensing library

Use, modification and distribution is subject to the Boost Software
License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)

## Licensing other

This repository is a meta-project for developing, building, and installing
the main Pothos framework, core dependency libraries, and official toolkits.
License files and source code are available in each project sub-directory.

The core Pothos library, Poco C++ libraries,
and official toolkits found in this repository
are available under the Boost Software License - Version 1.0.

Please note that Pothos toolkits that are NOT found in this repository,
both official and unofficial, may have different licensing terms.

Several external libraries have been included into this meta-project
that are not licensed under the Boost Software License - Version 1.0,
however are available under a permissive compatible software license.

### kissfft

* Description: C and C++ Fast Fourier Transform library
* Homepage: https://github.com/itdaniher/kissfft/blob/master/README
* License: https://github.com/itdaniher/kissfft/blob/master/COPYING
* Usage: Pothos comms FFT transform blocks
* Source: comms/fft/ (embedded copy)

### muparserX

* Description: C++ mathematical expression evaluator library
* Homepage: http://articles.beltoforion.de/article.php?a=muparserx
* License: http://opensource.org/licenses/BSD-3-Clause
* Usage: Pothos util tookit expression parser support
* Source: muparserx/ (top level submodule)

### Qwt

* Description: C++ plotting library based around Qt widgets
* Homepage: http://qwt.sourceforge.net/
* License: http://qwt.sourceforge.net/qwtlicense.html
* Usage: Pothos plotters toolkit support library
* Source: plotters/qwt/ (nested submodule)

### qtcolorpicker

* Description: C++ drop-down color selection Qt widget
* License: GNU Lesser General Public License Usage
* Usage: Pothos GUI designer affinity color selection
* Source: gui/qtcolorpicker/ (embedded copy)
