Wslay - The WebSocket library
=============================

Project Web: https://github.com/tatsuhiro-t/wslay

Wslay is a WebSocket library written in C.
It implements the protocol version 13 described in
`RFC 6455 <http://tools.ietf.org/html/rfc6455>`_.
This library offers 2 levels of API:
event-based API and frame-based low-level API. For event-based API, it
is suitable for non-blocking reactor pattern style. You can set
callbacks in various events. For frame-based API, you can send
WebSocket frame directly. Wslay only supports data transfer part of
WebSocket protocol and does not perform opening handshake in HTTP.

Wslay supports:

* Text/Binary messages.
* Automatic ping reply.
* Callback interface.
* External event loop.

Wslay does not perform any I/O operations for its own. Instead, it
offers callbacks for them. This makes Wslay independent on any I/O
frameworks, SSL, sockets, etc.  This makes Wslay portable across
various platforms and the application authors can choose freely I/O
frameworks.

See Autobahn test reports:
`server <http://wslay.sourceforge.net/autobahn/reports/servers/index.html>`_
and
`client <http://wslay.sourceforge.net/autobahn/reports/clients/index.html>`_.

Requirements
------------

`Sphinx <http://www.sphinx-doc.org/en/stable/>`_ is used to generate
man pages.

To build and run the unit test programs, the following packages are
needed:

* cunit >= 2.1

To build and run the example programs, the following packages are
needed:

* nettle >= 2.4


Build from git
--------------

Building from git is easy, but please be sure that at least autoconf 2.68 is
used.::

    $ autoreconf -i
    $ automake
    $ autoconf
    $ ./configure
    $ make
