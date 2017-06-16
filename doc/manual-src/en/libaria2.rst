.. default-domain:: cpp
.. highlight:: cpp

libaria2: C++ library interface to aria2
========================================

.. Warning::

  The API has not been frozen yet. It will be changed on the course of
  the development.

The libaria2 is a C++ library and offers the core functionality of
aria2. The library takes care of all networking and downloading stuff,
so its usage is very straight forward right now. See the following
Tutorial section to see how to use API.

Tutorial
--------

This section is a step by step guide to create a program to download
files using libaria2. The complete source is located at
*libaria2ex.cc* in *examples* directory.

The *libaria2ex* program takes one or more URIs and downloads each of
them in parallel. The usage is::

    Usage: libaria2ex URI [URI...]

      Download given URIs in parallel in the current directory.

The source code uses C++11 features, so C++11 enabled compiler is
required. GCC 4.7 works well here.

OK, let's look into the source code. First, include aria2.h header
file::

    #include <aria2/aria2.h>

Skip to the ``main()`` function. After checking command-line
arguments, we initialize libaria2::

    aria2::libraryInit();

And create aria2 session object::

    aria2::Session* session;
    // Create default configuration. The libaria2 takes care of signal
    // handling.
    aria2::SessionConfig config;
    // Add event callback
    config.downloadEventCallback = downloadEventCallback;
    session = aria2::sessionNew(aria2::KeyVals(), config);

:type:`Session` ``session`` is an aria2 session object. You need this
object through out the download process. Please keep in mind that only
one :type:`Session` object can be allowed per process due to the heavy
use of static objects in aria2 code base.  :type:`Session` object is
not safe for concurrent accesses from multiple threads.  It must be
used from one thread at a time.  In general, libaria2 is not entirely
thread-safe.  :type:`SessionConfig` ``config`` holds configuration for
the session object. The constructor initializes it with the default
values. In this setup, :member:`SessionConfig::keepRunning` is
``false`` which means :func:`run()` returns when all downloads are
processed, just like aria2c utility without RPC enabled.  And
:member:`SessionConfig::useSignalHandler` is ``true``, which means
libaria2 will setup signal handlers and catches certain signals to
halt download process gracefully. We also setup event handler callback
function ``downloadEventCallback``.  It will be called when an event
occurred such as download is started, completed, etc. In this example
program, we handle 2 events: download completion and error. For each
event, we print the GID of the download and several other
information::

  int downloadEventCallback(aria2::Session* session, aria2::DownloadEvent event,
			    const aria2::A2Gid& gid, void* userData)
  {
    switch(event) {
    case aria2::EVENT_ON_DOWNLOAD_COMPLETE:
      std::cerr << "COMPLETE";
      break;
    case aria2::EVENT_ON_DOWNLOAD_ERROR:
      std::cerr << "ERROR";
      break;
    default:
      return 0;
    }
    std::cerr << " [" << aria2::gidToHex(gid) << "] ";
    ...
  }

The ``userData`` object is specified by
:member:`SessionConfig::userData`. In this example, we don't specify
it, so it is ``nullptr``.

The first argument to :func:`sessionNew()` is ``aria2::KeyVals()``.
This type is used in API to specify vector of key/value pairs, mostly
representing aria2 options. For example, specify an option
``file-allocation`` to ``none``::

    aria2::KeyVals options;
    options.push_back(std::pair<std::string, std::string> ("file-allocation", "none"));

The first argument of :func:`sessionNew()` is analogous to the
command-line argument to aria2c program. In the example program, we
provide no options, so just pass empty vector.

After the creation of session object, let's add downloads given in the
command-line::

    // Add download item to session
    for(int i = 1; i < argc; ++i) {
      std::vector<std::string> uris = {argv[i]};
      aria2::KeyVals options;
      rv = aria2::addUri(session, nullptr, uris, options);
      if(rv < 0) {
	std::cerr << "Failed to add download " << uris[0] << std::endl;
      }
    }

We iterate command-line arguments and add each of them as a separate
download. :func:`addUri()` can take one or more URIs to download
several sources, just like aria2c does, but in this example, we just
give just one URI. We provide no particular option for the download,
so pass the empty vector as options. The second argument of
:func:`addUri()` takes a pointer to :type:`A2Gid`. If it is not
``NULL``, the function assigns the GID of the new download to it.  In
this example code, we have no interest for it, so just pass
``nullptr``.

We have set up everything at this stage. So let's start download.  To
perform the download, call :func:`run()` repeatedly until it returns
the value other than ``1``::

    for(;;) {
      rv = aria2::run(session, aria2::RUN_ONCE);
      if(rv != 1) {
	break;
      }
      ...
    }

Here, we call :func:`run()` with :c:macro:`RUN_ONCE`. It means
:func:`run()` returns after one event polling and its action handling
or polling timeout (which is approximately 1 second). If :func:`run()`
returns ``1``, it means the download is in progress and the
application must call it again. If it returns ``0``, then no download
is left (or it is stopped by signal handler or :func:`shutdown()`).
If the function catches error, it returns ``-1``.  The good point of
using :c:macro:`RUN_ONCE` is that the application can use libaria2 API
when :func:`run()` returns. In the example program, we print the
progress of the download in every no less than 500 millisecond::

    // Print progress information once per 500ms
    if(count >= 500) {
      start = now;
      aria2::GlobalStat gstat = aria2::getGlobalStat(session);
      std::cerr << "Overall #Active:" << gstat.numActive
                << " #waiting:" << gstat.numWaiting
                << " D:" << gstat.downloadSpeed/1024 << "KiB/s"
                << " U:"<< gstat.uploadSpeed/1024 << "KiB/s " << std::endl;
      std::vector<aria2::A2Gid> gids = aria2::getActiveDownload(session);
      for(const auto& gid : gids) {
        aria2::DownloadHandle* dh = aria2::getDownloadHandle(session, gid);
        if(dh) {
          std::cerr << "    [" << aria2::gidToHex(gid) << "] "
                    << dh->getCompletedLength() << "/"
                    << dh->getTotalLength() << "("
                    << (dh->getTotalLength() > 0 ?
                        (100*dh->getCompletedLength()/dh->getTotalLength())
                        : 0) << "%)"
                    << " D:"
                    << dh->getDownloadSpeed()/1024 << "KiB/s, U:"
                    << dh->getUploadSpeed()/1024 << "KiB/s"
                    << std::endl;
          aria2::deleteDownloadHandle(dh);
        }
      }
    }

We first call :func:`getGlobalStat()` function to get global
statistics of the downloads. Then, call :func:`getActiveDownload()`
function to get the vector of active download's GID. For each GID, we
retrieve :class:`DownloadHandle` object using
:func:`getDownloadHandle` function and get detailed information.
Please don't forget to delete :class:`DownloadHandle` after the use
and before the next call of :func:`run()`.  Keep in mind that the life
time of :class:`DownloadHandle` object is before the next call of
:func:`run()` function.

After the loop, finalize download calling :func:`sessionFinal()`
function and call :func:`libraryDeinit()` to release resources for the
library::

    rv = aria2::sessionFinal(session);
    aria2::libraryDeinit();
    return rv;

Calling :func:`sessionFinal()` is important because it performs
post-download action, including saving sessions and destroys session
object. So failing to call this function will lead to lose the
download progress and memory leak. The :func:`sessionFinal()` returns
the code defined in :ref:`exit-status`. aria2c program also returns
the same value as exist status, so do the same in this tiny example
program.

See also *libaria2wx.cc* which uses wx GUI component as UI and use
background thread to run download.

API Reference
-------------

To use the API function, include ``aria2/aria2.h``::

    #include <aria2/aria2.h>

All enums, types and functions are under ``aria2`` namespace. To link
with libaria2, use linker flag ``-laria2``.


.. include:: libaria2api
