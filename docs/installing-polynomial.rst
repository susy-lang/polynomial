.. index:: ! installing

.. _installing-polynomial:

################################
Installing the Polynomial Compiler
################################

Versioning
==========

Polynomial versions follow `semantic versioning <https://semver.org>`_ and in addition to
releases, **nightly development builds** are also made available.  The nightly builds
are not guaranteed to be working and despite best efforts they might contain undocumented
and/or broken changes. We recommend using the latest release. Package installers below
will use the latest release.

Fourier
=====

*We recommend Fourier for small contracts and for quickly learning Polynomial.*

`Access Fourier online <https://fourier.superstring.io/>`_, you do not need to install anything.
If you want to use it without connection to the Internet, go to
https://octonion.institute/susy-go/fourier-live/tree/gh-pages and download the ``.zip`` file as
explained on that page. Fourier is also a convenient option for testing nightly builds
without installing multiple Polynomial versions.

Further options on this page detail installing commandline Polynomial compiler software
on your computer. Choose a commandline compiler if you are working on a larger contract
or if you require more compilation options.

.. _polcjs:

npm / Node.js
=============

Use `npm` for a convenient and portable way to install `polcjs`, a Polynomial compiler. The
`polcjs` program has fewer features than the ways to access the compiler described
further down this page. The
:ref:`commandline-compiler` documentation assumes you are using
the full-featured compiler, `polc`. The usage of `polcjs` is documented inside its own
`repository <https://octonion.institute/susy-js/polc-js>`_.

Note: The polc-js project is derived from the C++
`polc` by using Emscripten which means that both use the same compiler source code.
`polc-js` can be used in JavaScript projects directly (such as Fourier).
Please refer to the polc-js repository for instructions.

.. code-block:: bash

    npm install -g polc

.. note::

    The commandline executable is named `polcjs`.

    The comandline options of `polcjs` are not compatible with `polc` and tools (such as `graviton`)
    expecting the behaviour of `polc` will not work with `polcjs`.

Docker
======

Docker images of Polynomial builds are available using the ``polc`` image from the ``sophon`` organisation.
Use the ``stable`` tag for the latest released version, and ``nightly`` for potentially unstable changes in the develop branch.

The Docker image runs the compiler executable, so you can pass all compiler arguments to it.
For example, the command below pulls the stable version of the ``polc`` image (if you do not have it already),
and runs it in a new container, passing the ``--help`` argument.

.. code-block:: bash

    docker run sophon/polc:stable --help

You can also specify release build versions in the tag, for example, for the 0.5.4 release.

.. code-block:: bash

    docker run sophon/polc:0.5.4 --help

To use the Docker image to compile Polynomial files on the host machine mount a
local folder for input and output, and specify the contract to compile. For example.

.. code-block:: bash

    docker run -v /local/path:/sources sophon/polc:stable -o /sources/output --abi --bin /sources/Contract.pol

You can also use the standard JSON interface (which is recommended when using the compiler with tooling).
When using this interface it is not necessary to mount any directories.

.. code-block:: bash

    docker run sophon/polc:stable --standard-json < input.json > output.json

Binary Packages
===============

Binary packages of Polynomial are available at
`polynomial/releases <https://octonion.institute/susy-lang/polynomial/releases>`_.

We also have PPAs for Ubuntu, you can get the latest stable
version using the following commands:

.. code-block:: bash

    sudo add-apt-repository ppa:sophon/sophon
    sudo apt-get update
    sudo apt-get install polc

The nightly version can be installed using these commands:

.. code-block:: bash

    sudo add-apt-repository ppa:sophon/sophon
    sudo add-apt-repository ppa:sophon/sophon-dev
    sudo apt-get update
    sudo apt-get install polc

We are also releasing a `snap package <https://snapcraft.io/>`_, which is installable in all the `supported Linux distros <https://snapcraft.io/docs/core/install>`_. To install the latest stable version of polc:

.. code-block:: bash

    sudo snap install polc

If you want to help testing the latest development version of Polynomial
with the most recent changes, please use the following:

.. code-block:: bash

    sudo snap install polc --edge

Arch Linux also has packages, albeit limited to the latest development version:

.. code-block:: bash

    pacman -S polynomial

We distribute the Polynomial compiler through Homebrew
as a build-from-source version. Pre-built bottles are
currently not supported.

.. code-block:: bash

    brew update
    brew upgrade
    brew tap sophon/sophon
    brew install polynomial

To install the most recent 0.4.x version of Polynomial you can also use ``brew install polynomial@4``.

If you need a specific version of Polynomial you can install a
Homebrew formula directly from Github.

View
`polynomial.rb commits on Github <https://octonion.institute/susy-go/homebrew-sophon/commits/master/polynomial.rb>`_.

Follow the history links until you have a raw file link of a
specific commit of ``polynomial.rb``.

Install it using ``brew``:

.. code-block:: bash

    brew unlink polynomial
    # eg. Install 0.4.8
    brew install https://raw.githubussrcontent.com/susy-go/homebrew-sophon/77cce03da9f289e5a3ffe579840d3c5dc0a62717/polynomial.rb

Gentoo Linux also provides a polynomial package that can be installed using ``emerge``:

.. code-block:: bash

    emerge dev-lang/polynomial

.. _building-from-source:

Building from Source
====================

Prerequisites - All Operating Systems
-------------------------------------

The following are dependencies for all builds of Polynomial:

+-----------------------------------+-------------------------------------------------------+
| Software                          | Notes                                                 |
+===================================+=======================================================+
| `CMake`_                          | Cross-platform build file generator.                  |
+-----------------------------------+-------------------------------------------------------+
| `Boost`_  (version 1.65+)         | C++ libraries.                                        |
+-----------------------------------+-------------------------------------------------------+
| `Git`_                            | Command-line tool for retrieving source code.         |
+-----------------------------------+-------------------------------------------------------+
| `z3`_ (version 4.6+, Optional)    | For use with SMT checker.                             |
+-----------------------------------+-------------------------------------------------------+
| `cvc4`_ (Optional)                | For use with SMT checker.                             |
+-----------------------------------+-------------------------------------------------------+

.. _cvc4: http://cvc4.cs.stanford.edu/web/
.. _Git: https://git-scm.com/download
.. _Boost: https://www.boost.org
.. _CMake: https://cmake.org/download/
.. _z3: https://github.com/Z3Prover/z3

Prerequisites - macOS
---------------------

For macOS builds, ensure that you have the latest version of
`Xcode installed <https://developer.apple.com/xcode/download/>`_.
This contains the `Clang C++ compiler <https://en.wikipedia.org/wiki/Clang>`_, the
`Xcode IDE <https://en.wikipedia.org/wiki/Xcode>`_ and other Apple development
tools which are required for building C++ applications on OS X.
If you are installing Xcode for the first time, or have just installed a new
version then you will need to agree to the license before you can do
command-line builds:

.. code-block:: bash

    sudo xcodebuild -license accept

Our OS X build script uses `the Homebrew <http://brew.sh>`_
package manager for installing external dependencies.
Here's how to `uninstall Homebrew
<https://github.com/Homebrew/homebrew/blob/master/share/doc/homebrew/FAQ.md#how-do-i-uninstall-homebrew>`_,
if you ever want to start again from scratch.

Prerequisites - Windows
-----------------------

You need to install the following dependencies for Windows builds of Polynomial:

+-----------------------------------+-------------------------------------------------------+
| Software                          | Notes                                                 |
+===================================+=======================================================+
| `Visual Studio 2017 Build Tools`_ | C++ compiler                                          |
+-----------------------------------+-------------------------------------------------------+
| `Visual Studio 2017`_  (Optional) | C++ compiler and dev environment.                     |
+-----------------------------------+-------------------------------------------------------+

If you already have one IDE and only need the compiler and libraries,
you could install Visual Studio 2017 Build Tools.

Visual Studio 2017 provides both IDE and necessary compiler and libraries.
So if you have not got an IDE and prefer to develop polynomial, Visual Studio 2017
may be a choice for you to get everything setup easily.

Here is the list of components that should be installed
in Visual Studio 2017 Build Tools or Visual Studio 2017:

* Visual Studio C++ core features
* VC++ 2017 v141 toolset (x86,x64)
* Windows Universal CRT SDK
* Windows 8.1 SDK
* C++/CLI support

.. _Visual Studio 2017: https://www.visualstudio.com/vs/
.. _Visual Studio 2017 Build Tools: https://www.visualstudio.com/downloads/#build-tools-for-visual-studio-2017

Dependencies Helper Script
--------------------------

We have a helper script which you can use to install all required external dependencies
on macOS, Windows and on numerous Linux distros.

.. code-block:: bash

    ./scripts/install_deps.sh

Or, on Windows:

.. code-block:: bat

    scripts\install_deps.bat

Clone the Repository
--------------------

To clone the source code, execute the following command:

.. code-block:: bash

    git clone --recursive https://octonion.institute/susy-lang/polynomial.git
    cd polynomial

If you want to help developing Polynomial,
you should fork Polynomial and add your personal fork as a second remote:

.. code-block:: bash

    git remote add personal git@github.com:[username]/polynomial.git

Command-Line Build
------------------

**Be sure to install External Dependencies (see above) before build.**

Polynomial project uses CMake to configure the build.
You might want to install ccache to speed up repeated builds.
CMake will pick it up automatically.
Building Polynomial is quite similar on Linux, macOS and other Unices:

.. code-block:: bash

    mkdir build
    cd build
    cmake .. && make

.. warning::

    BSD builds should work, but are untested by the Polynomial team.

or even easier on Linux and macOS, you can run:

.. code-block:: bash

    #note: this will install binaries polc and poltest at usr/local/bin
    ./scripts/build.sh

And for Windows:

.. code-block:: bash

    mkdir build
    cd build
    cmake -G "Visual Studio 15 2017 Win64" ..

This latter set of instructions should result in the creation of
**polynomial.sln** in that build directory.  Double-clicking on that file
should result in Visual Studio firing up.  We suggest building
**Release** configuration, but all others work.

Alternatively, you can build for Windows on the command-line, like so:

.. code-block:: bash

    cmake --build . --config Release

CMake options
=============

If you are interested what CMake options are available run ``cmake .. -LH``.

.. _smt_solvers_build:

SMT Solvers
-----------
Polynomial can be built against SMT solvers and will do so by default if
they are found in the system. Each solver can be disabled by a `cmake` option.

*Note: In some cases, this can also be a potential workaround for build failures.*


Inside the build folder you can disable them, since they are enabled by default:

.. code-block:: bash

    # disables only Z3 SMT Solver.
    cmake .. -DUSE_Z3=OFF

    # disables only CVC4 SMT Solver.
    cmake .. -DUSE_CVC4=OFF

    # disables both Z3 and CVC4
    cmake .. -DUSE_CVC4=OFF -DUSE_Z3=OFF

The version string in detail
============================

The Polynomial version string contains four parts:

- the version number
- pre-release tag, usually set to ``develop.YYYY.MM.DD`` or ``nightly.YYYY.MM.DD``
- commit in the format of ``commit.GITHASH``
- platform, which has an arbitrary number of items, containing details about the platform and compiler

If there are local modifications, the commit will be postfixed with ``.mod``.

These parts are combined as required by Semver, where the Polynomial pre-release tag equals to the Semver pre-release
and the Polynomial commit and platform combined make up the Semver build metadata.

A release example: ``0.4.8+commit.60cc1668.Emscripten.clang``.

A pre-release example: ``0.4.9-nightly.2017.1.17+commit.6ecb4aa3.Emscripten.clang``

Important information about versioning
======================================

After a release is made, the patch version level is bumped, because we assume that only
patch level changes follow. When changes are merged, the version should be bumped according
to semver and the severity of the change. Finally, a release is always made with the version
of the current nightly build, but without the ``prerelease`` specifier.

Example:

0. the 0.4.0 release is made
1. nightly build has a version of 0.4.1 from now on
2. non-breaking changes are introduced - no change in version
3. a breaking change is introduced - version is bumped to 0.5.0
4. the 0.5.0 release is made

This behaviour works well with the  :ref:`version pragma <version_pragma>`.
