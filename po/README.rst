GNU gettext translation files
=============================

We use `launchpad
<https://translations.launchpad.net/aria2/trunk/+pots/aria2>`_ to
translate gettext translation files (PO files).

The PO files are not stored in the repository. They are exported from
launchpad and imported to this directory when creating the
distribution archive.

Exporting PO files from launchpad
---------------------------------

Visit `launchpad <https://translations.launchpad.net/aria2/trunk/+pots/aria2>`_
and follow the link ``download``.

Importing PO files from launchpad-export.tar.gz
-----------------------------------------------

The downloaded file is named as launchpad-export.tar.gz at the time of
this writing.  It includes all PO files translated at launchpad.  To
import those files, use ``import-po`` script in the top directory. It
will inflate the tar.gz file and rename PO files and move them to po
directory and run ``make update-po``.
