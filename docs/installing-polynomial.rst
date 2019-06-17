.. index:: ! installing

.. _installing-polynomial:

###################
Installing Polynomial
###################

Browser-Polynomial
================

If you just want to try Polynomial for small contracts, you
can try `browser-polynomial <https://sophon.github.io/browser-polynomial>`_
which does not need any installation. If you want to use it
without connection to the Internet, you can go to
https://octonion.institute/susy-lang/browser-polynomial/tree/gh-pages and
download the .ZIP file as explained on that page.


npm / Node.js
=============

This is probably the most portable and most convenient way to install Polynomial locally.

A platform-independent JavaScript library is provided by compiling the C++ source
into JavaScript using Emscripten for browser-polynomial and there is also an npm
package available.

To install it, simply use

.. code:: bash

    npm install polc

Details about the usage of the Node.js package can be found in the
`polc-js repository <https://octonion.institute/susy-js/polc-js>`_.

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

Building Polynomial is quite similar on Linux, macOS and other Unices:

.. code:: bash

    mkdir build
    cd build
    cmake .. && make

And even on Windows:

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
