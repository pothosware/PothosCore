# Python bindings for the Pothos framework

## Dependencies

* Pothos library
* Python development libraries and headers (2.7 or 3.*)
* Numpy is a runtime dependency for the Pothos Python module

## Building

configure, build, and install with CMake

# Layout

* The root directory contains a Pothos::ProxyEnvironment overload that can call into the Python C API.
* The Pothos/ directory contains a python module that:
  * Provides access to a Pothos::ProxyEnvironment through Python
  * Pythonic API wrapper for Pothos::Block class
