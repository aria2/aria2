.. default-domain:: cpp
.. highlight:: cpp

libaria2: C++ library interface to aria2
========================================

.. Warning::

  The API has not been frozen yet. It will be changed on the course of
  the development.

The libaria2 is a C++ library and offers the core functionality of
aria2. The library takes care of all networking and downloading stuff,
so its usage is very straight forward right now. See *libaria2ex.cc*
in *examples* directory to see how to use API.

API Reference
-------------

To use the API function, include ``aria2/aria2.h``::

    #include <aria2/aria2.h>

All enums, types and functions are under ``aria2`` namespace. To link
with libaria2, use linker flag ``-laria2``.


.. include:: libaria2api
