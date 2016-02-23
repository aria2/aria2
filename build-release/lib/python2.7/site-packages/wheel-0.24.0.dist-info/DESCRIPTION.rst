Wheel
=====

A built-package format for Python.

A wheel is a ZIP-format archive with a specially formatted filename
and the .whl extension. It is designed to contain all the files for a
PEP 376 compatible install in a way that is very close to the on-disk
format. Many packages will be properly installed with only the "Unpack"
step (simply extracting the file onto sys.path), and the unpacked archive
preserves enough information to "Spread" (copy data and scripts to their
final locations) at any later time.

The wheel project provides a `bdist_wheel` command for setuptools
(requires setuptools >= 0.8.0). Wheel files can be installed with a
newer `pip` from https://github.com/pypa/pip or with wheel's own command
line utility.

The wheel documentation is at http://wheel.rtfd.org/. The file format
is documented in PEP 427 (http://www.python.org/dev/peps/pep-0427/).

The reference implementation is at https://bitbucket.org/pypa/wheel

Why not egg?
------------

Python's egg format predates the packaging related standards we have
today, the most important being PEP 376 "Database of Installed Python
Distributions" which specifies the .dist-info directory (instead of
.egg-info) and PEP 426 "Metadata for Python Software Packages 2.0"
which specifies how to express dependencies (instead of requires.txt
in .egg-info).

Wheel implements these things. It also provides a richer file naming
convention that communicates the Python implementation and ABI as well
as simply the language version used in a particular package.

Unlike .egg, wheel will be a fully-documented standard at the binary
level that is truly easy to install even if you do not want to use the
reference implementation.



0.24.0
======
- The python tag used for pure-python packages is now .pyN (major version
  only). This change actually occurred in 0.23.0 when the --python-tag
  option was added, but was not explicitly mentioned in the changelog then.
- wininst2wheel and egg2wheel removed. Use "wheel convert [archive]"
  instead.
- Wheel now supports setuptools style conditional requirements via the
  extras_require={} syntax. Separate 'extra' names from conditions using
  the : character. Wheel's own setup.py does this. (The empty-string
  extra is the same as install_requires.) These conditional requirements
  should work the same whether the package is installed by wheel or
  by setup.py.

0.23.0
======
- Compatibiltiy tag flags added to the bdist_wheel command
- sdist should include files necessary for tests
- 'wheel convert' can now also convert unpacked eggs to wheel
- Rename pydist.json to metadata.json to avoid stepping on the PEP
- The --skip-scripts option has been removed, and not generating scripts is now
  the default. The option was a temporary approach until installers could
  generate scripts themselves. That is now the case with pip 1.5 and later.
  Note that using pip 1.4 to install a wheel without scripts will leave the
  installation without entry-point wrappers. The "wheel install-scripts"
  command can be used to generate the scripts in such cases.
- Thank you contributors

0.22.0
======
- Include entry_points.txt, scripts a.k.a. commands, in experimental
  pydist.json
- Improved test_requires parsing
- Python 2.6 fixes, "wheel version" command courtesy pombredanne

0.21.0
======
- Pregenerated scripts are the default again.
- "setup.py bdist_wheel --skip-scripts" turns them off.
- setuptools is no longer a listed requirement for the 'wheel'
  package. It is of course still required in order for bdist_wheel
  to work.
- "python -m wheel" avoids importing pkg_resources until it's necessary.

0.20.0
======
- No longer include console_scripts in wheels. Ordinary scripts (shell files,
  standalone Python files) are included as usual.
- Include new command "python -m wheel install-scripts [distribution
  [distribution ...]]" to install the console_scripts (setuptools-style
  scripts using pkg_resources) for a distribution.

0.19.0
======
- pymeta.json becomes pydist.json

0.18.0
======
- Python 3 Unicode improvements

0.17.0
======
- Support latest PEP-426 "pymeta.json" (json-format metadata)

0.16.0
======
- Python 2.6 compatibility bugfix (thanks John McFarlane)
- Non-prerelease version number

1.0.0a2
=======
- Bugfix for C-extension tags for CPython 3.3 (using SOABI)

1.0.0a1
=======
- Bugfix for bdist_wininst converter "wheel convert"
- Bugfix for dists where "is pure" is None instead of True or False

1.0.0a0
=======
- Update for version 1.0 of Wheel (PEP accepted).
- Python 3 fix for moving Unicode Description to metadata body
- Include rudimentary API documentation in Sphinx (thanks Kevin Horn)

0.15.0
======
- Various improvements

0.14.0
======
- Changed the signature format to better comply with the current JWS spec.
  Breaks all existing signatures.
- Include ``wheel unsign`` command to remove RECORD.jws from an archive.
- Put the description in the newly allowed payload section of PKG-INFO
  (METADATA) files.

0.13.0
======
- Use distutils instead of sysconfig to get installation paths; can install
  headers.
- Improve WheelFile() sort.
- Allow bootstrap installs without any pkg_resources.

0.12.0
======
- Unit test for wheel.tool.install

0.11.0
======
- API cleanup

0.10.3
======
- Scripts fixer fix

0.10.2
======
- Fix keygen

0.10.1
======
- Preserve attributes on install.

0.10.0
======
- Include a copy of pkg_resources. Wheel can now install into a virtualenv
  that does not have distribute (though most packages still require
  pkg_resources to actually work; wheel install distribute)
- Define a new setup.cfg section [wheel]. universal=1 will
  apply the py2.py3-none-any tag for pure python wheels.

0.9.7
=====
- Only import dirspec when needed. dirspec is only needed to find the
  configuration for keygen/signing operations.

0.9.6
=====
- requires-dist from setup.cfg overwrites any requirements from setup.py
  Care must be taken that the requirements are the same in both cases,
  or just always install from wheel.
- drop dirspec requirement on win32
- improved command line utility, adds 'wheel convert [egg or wininst]' to
  convert legacy binary formats to wheel

0.9.5
=====
- Wheel's own wheel file can be executed by Python, and can install itself:
  ``python wheel-0.9.5-py27-none-any/wheel install ...``
- Use argparse; basic ``wheel install`` command should run with only stdlib
  dependencies.
- Allow requires_dist in setup.cfg's [metadata] section. In addition to
  dependencies in setup.py, but will only be interpreted when installing
  from wheel, not from sdist. Can be qualified with environment markers.

0.9.4
=====
- Fix wheel.signatures in sdist

0.9.3
=====
- Integrated digital signatures support without C extensions.
- Integrated "wheel install" command (single package, no dependency
  resolution) including compatibility check.
- Support Python 3.3
- Use Metadata 1.3 (PEP 426)

0.9.2
=====
- Automatic signing if WHEEL_TOOL points to the wheel binary
- Even more Python 3 fixes

0.9.1
=====
- 'wheel sign' uses the keys generated by 'wheel keygen' (instead of generating
  a new key at random each time)
- Python 2/3 encoding/decoding fixes
- Run tests on Python 2.6 (without signature verification)

0.9
===
- Updated digital signatures scheme
- Python 3 support for digital signatures
- Always verify RECORD hashes on extract
- "wheel" command line tool to sign, verify, unpack wheel files

0.8
===
- none/any draft pep tags update
- improved wininst2wheel script
- doc changes and other improvements

0.7
===
- sort .dist-info at end of wheel archive
- Windows & Python 3 fixes from Paul Moore
- pep8
- scripts to convert wininst & egg to wheel

0.6
===
- require distribute >= 0.6.28
- stop using verlib

0.5
===
- working pretty well

0.4.2
=====
- hyphenated name fix

0.4
===
- improve test coverage
- improve Windows compatibility
- include tox.ini courtesy of Marc Abramowitz
- draft hmac sha-256 signing function

0.3
===
- prototype egg2wheel conversion script

0.2
===
- Python 3 compatibility

0.1
===
- Initial version


