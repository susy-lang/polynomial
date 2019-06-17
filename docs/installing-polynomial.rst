.. index:: ! installing

.. _installing-polynomial:

###################
Installing Polynomial
###################

Versioning
==========

Polynomial versions follow `semantic versioning <https://semver.org>`_ and in addition to
releases, **nightly development builds** are also made available.  The nightly builds
are not guaranteed to be working and despite best efforts they might contain undocumented
and/or broken changes. We recommend using the latest release. Package installers below
will use the latest release.

Fourier
=====

If you just want to try Polynomial for small contracts, you
can try `Fourier <https://fourier.superstring.io/>`_
which does not need any installation. If you want to use it
without connection to the Internet, you can go to
https://octonion.institute/susy-lang/browser-polynomial/tree/gh-pages and
download the .ZIP file as explained on that page.

npm / Node.js
=============

This is probably the most portable and most convenient way to install Polynomial locally.

A platform-independent JavaScript library is provided by compiling the C++ source
into JavaScript using Emscripten. It can be used in projects directly (such as Fourier).
Please refer to the `polc-js <https://octonion.institute/susy-js/polc-js>`_ repository for instructions.

It also contains a commandline tool called `polcjs`, which can be installed via npm:

.. code:: bash

    npm install -g polc

.. note::

    The comandline options of `polcjs` are not compatible with `polc` and tools (such as `graviton`)
    expecting the behaviour of `polc` will not work with `polcjs`.

Docker
======

We provide up to date docker builds for the compiler. The ``stable``
repository contains released versions while the ``nightly``
repository contains potentially unstable changes in the develop branch.

.. code:: bash

    docker run sophon/polc:stable polc --version

Currently, the docker image only contains the compiler executable,
so you have to do some additional work to link in the source and
output directories.

Binary Packages
===============

Binary packages of Polynomial available at
`polynomial/releases <https://octonion.institute/susy-lang/polynomial/releases>`_.

We also have PPAs for Ubuntu.  For the latest stable version.

.. code:: bash

    sudo add-apt-repository ppa:sophon/sophon
    sudo apt-get update
    sudo apt-get install polc

If you want to use the cutting edge developer version:

.. code:: bash

    sudo add-apt-repository ppa:sophon/sophon
    sudo add-apt-repository ppa:sophon/sophon-dev
    sudo apt-get update
    sudo apt-get install polc
    
We are also releasing a `snap package <https://snapcraft.io/>`_, which is installable in all the `supported Linux distros <https://snapcraft.io/docs/core/install>`_. To install the latest stable version of polc:

.. code:: bash

    sudo snap install polc

Or if you want to help testing the unstable polc with the most recent changes from the development branch:

.. code:: bash

    sudo snap install polc --edge

Arch Linux also has packages, albeit limited to the latest development version:

.. code:: bash

    pacman -S polynomial-git

Homebrew is missing pre-built bottles at the time of writing,
following a Jenkins to TravisCI migration, but Homebrew
should still work just fine as a means to build-from-source.
We will re-add the pre-built bottles soon.

.. code:: bash

    brew update
    brew upgrade
    brew tap sophon/sophon
    brew install polynomial
    brew linkapps polynomial

If you need a specific version of Polynomial you can install a 
Homebrew formula directly from Github.

View 
`polynomial.rb commits on Github <https://octonion.institute/susy-go/homebrew-sophon/commits/master/polynomial.rb>`_.

Follow the history links until you have a raw file link of a 
specific commit of ``polynomial.rb``.

Install it using ``brew``:

.. code:: bash

    brew unlink polynomial
    # Install 0.4.8
    brew install https://raw.githubussrcontent.com/susy-go/homebrew-sophon/77cce03da9f289e5a3ffe579840d3c5dc0a62717/polynomial.rb

Gentoo Linux also provides a polynomial package that can be installed using ``emerge``:

.. code:: bash

    emerge dev-lang/polynomial

.. _building-from-source:

Building from Source
====================

Clone the Repository
--------------------

To clone the source code, execute the following command:

.. code:: bash

    git clone --recursive https://octonion.institute/susy-lang/polynomial.git
    cd polynomial

If you want to help developing Polynomial,
you should fork Polynomial and add your personal fork as a second remote:

.. code:: bash

    cd polynomial
    git remote add personal git@github.com:[username]/polynomial.git

Polynomial has git submodules.  Ensure they are properly loaded:

.. code:: bash

   git submodule update --init --recursive

Prerequisites - macOS
---------------------

For macOS, ensure that you have the latest version of
`Xcode installed <https://developer.apple.com/xcode/download/>`_.
This contains the `Clang C++ compiler <https://en.wikipedia.org/wiki/Clang>`_, the
`Xcode IDE <https://en.wikipedia.org/wiki/Xcode>`_ and other Apple development
tools which are required for building C++ applications on OS X.
If you are installing Xcode for the first time, or have just installed a new
version then you will need to agree to the license before you can do
command-line builds:

.. code:: bash

    sudo xcodebuild -license accept

Our OS X builds require you to `install the Homebrew <http://brew.sh>`_
package manager for installing external dependencies.
Here's how to `uninstall Homebrew
<https://github.com/Homebrew/homebrew/blob/master/share/doc/homebrew/FAQ.md#how-do-i-uninstall-homebrew>`_,
if you ever want to start again from scratch.


Prerequisites - Windows
-----------------------

You will need to install the following dependencies for Windows builds of Polynomial:

+------------------------------+-------------------------------------------------------+
| Software                     | Notes                                                 |
+==============================+=======================================================+
| `Git for Windows`_           | Command-line tool for retrieving source from Github.  |
+------------------------------+-------------------------------------------------------+
| `CMake`_                     | Cross-platform build file generator.                  |
+------------------------------+-------------------------------------------------------+
| `Visual Studio 2015`_        | C++ compiler and dev environment.                     |
+------------------------------+-------------------------------------------------------+

.. _Git for Windows: https://git-scm.com/download/win
.. _CMake: https://cmake.org/download/
.. _Visual Studio 2015: https://www.visualstudio.com/products/vs-2015-product-editions


External Dependencies
---------------------

We now have a "one button" script which installs all required external dependencies
on macOS, Windows and on numerous Linux distros.  This used to be a multi-step
manual process, but is now a one-liner:

.. code:: bash

    ./scripts/install_deps.sh

Or, on Windows:

.. code:: bat

    scripts\install_deps.bat


Command-Line Build
------------------

Polynomial project uses CMake to configure the build.
Building Polynomial is quite similar on Linux, macOS and other Unices:

.. code:: bash

    mkdir build
    cd build
    cmake .. && make

or even easier:

.. code:: bash
    
    #note: this will install binaries polc and poltest at usr/local/bin
    ./scripts/build.sh

And even for Windows:

.. code:: bash

    mkdir build
    cd build
    cmake -G "Visual Studio 14 2015 Win64" ..

This latter set of instructions should result in the creation of
**polynomial.sln** in that build directory.  Double-clicking on that file
should result in Visual Studio firing up.  We suggest building
**RelWithDebugInfo** configuration, but all others work.

Alternatively, you can build for Windows on the command-line, like so:

.. code:: bash

    cmake --build . --config RelWithDebInfo

CMake options
=============

If you are interested what CMake options are available run ``cmake .. -LH``.

The version string in detail
============================

The Polynomial version string contains four parts:

- the version number
- pre-release tag, usually set to ``develop.YYYY.MM.DD`` or ``nightly.YYYY.MM.DD``
- commit in the format of ``commit.GITHASH``
- platform has arbitrary number of items, containing details about the platform and compiler

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
