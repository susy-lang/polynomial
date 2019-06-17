###################
Installing Polynomial
###################

Browser-Polynomial
================

If you just want to try Polynomial for small contracts, you
can try `browser-polynomial <https://sophon.github.io/browser-polynomial>`_
which does not need any installation. If you want to use it
without connection to the Internet, you can also just save the page
locally or clone http://octonion.institute/susy-lang/browser-polynomial.

npm / Node.js
=============

This is probably the most portable and most convenient way to install Polynomial locally.

A platform-independent JavaScript library is provided by compiling the C++ source
into JavaScript using Emscripten for browser-polynomial and there is also an npm
package available.

To install it, simply use

::

    npm install polc

Details about the usage of the Node.js package can be found in the
`polc-js repository <https://octonion.institute/susy-js/polc-js>`_.

Binary Packages
===============

Binary packages of Polynomial together with its IDE Mix are available through
the `C++ bundle <https://octonion.institute/susy-cpp/webthree-umbrella/releases>`_ of
Sophon.

Building from Source
====================

Building Polynomial is quite similar on MacOS X, Ubuntu and probably other Unices.
This guide starts explaining how to install the dependencies for each platform
and then shows how to build Polynomial itself.

MacOS X
-------


Requirements:

- OS X Yosemite (10.10.5)
- Homebrew
- Xcode

Set up Homebrew:

.. code-block:: bash

    brew update
    brew upgrade

    brew install boost --c++11             # this takes a while
    brew install cmake cryptopp miniupnpc leveldb gmp libmicrohttpd libjson-rpc-cpp
    # For Mix IDE and Alsofzero only
    brew install xz d-bus
    brew install homebrew/versions/v8-315
    brew install llvm --HEAD --with-clang
    brew install qt5 --with-d-bus          # add --verbose if long waits with a stale screen drive you crazy as well

Ubuntu
------

Below are the build instructions for the latest versions of Ubuntu. The best
supported platform as of December 2014 is Ubuntu 14.04, 64 bit, with at least 2
GB RAM. All our tests are done with this version. Community contributions for
other versions are welcome!

Install dependencies:

Before you can build the source, you need several tools and dependencies for the application to get started.

First, update your repositories. Not all packages are provided in the main
Ubuntu repository, those you'll get from the Sophon PPA and the LLVM archive.

.. note::

    Ubuntu 14.04 users, you'll need the latest version of cmake. For this, use:
    `sudo apt-add-repository ppa:george-edison55/cmake-3.x`

Now add all the rest:

.. code-block:: bash

    sudo apt-get -y update
    sudo apt-get -y install language-pack-en-base
    sudo dpkg-reconfigure locales
    sudo apt-get -y install software-properties-common
    sudo add-apt-repository -y ppa:sophon/sophon
    sudo add-apt-repository -y ppa:sophon/sophon-dev
    sudo apt-get -y update
    sudo apt-get -y upgrade

For Ubuntu 15.04 (Vivid Vervet) or older, use the following command to add the develop packages:

.. code-block:: bash

    sudo apt-get -y install build-essential git cmake libboost-all-dev libgmp-dev libleveldb-dev libminiupnpc-dev libreadline-dev libncurses5-dev libcurl4-openssl-dev libcryptopp-dev libjson-rpc-cpp-dev libmicrohttpd-dev libjsoncpp-dev libedit-dev libz-dev

For Ubuntu 15.10 (Wily Werewolf) or newer, use the following command instead:

.. code-block:: bash

    sudo apt-get -y install build-essential git cmake libboost-all-dev libgmp-dev libleveldb-dev libminiupnpc-dev libreadline-dev libncurses5-dev libcurl4-openssl-dev libcryptopp-dev libjsonrpccpp-dev libmicrohttpd-dev libjsoncpp-dev libedit-dev libz-dev

The reason for the change is that ``libjsonrpccpp-dev`` is available in the universe repository for newer versions of Ubuntu.

Building
--------

Run this if you plan on installing Polynomial only, ignore errors at the end as
they relate only to Alsofzero and Mix

.. code-block:: bash

    git clone --recursive https://octonion.institute/susy-cpp/webthree-umbrella.git
    cd webthree-umbrella
    ./webthree-helpers/scripts/sofupdate.sh --no-push --simple-pull --project polynomial # update Polynomial repo
    ./webthree-helpers/scripts/sofbuild.sh --no-git --project polynomial --all --cores 4 -DSVMJIT=0 # build Polynomial and others
                                                                                #enabling DSVMJIT on OS X will not build
                                                                                #feel free to enable it on Linux

If you opted to install Alsofzero and Mix:

.. code-block:: bash

    git clone --recursive https://octonion.institute/susy-cpp/webthree-umbrella.git
    cd webthree-umbrella && mkdir -p build && cd build
    cmake ..

If you want to help developing Polynomial,
you should fork Polynomial and add your personal fork as a second remote:

.. code-block:: bash

    cd webthree-umbrella/polynomial
    git remote add personal git@github.com:username/polynomial.git

Note that webthree-umbrella uses submodules, so polynomial is its own git
repository, but its settings are not stored in ``.git/config``, but in
``webthree-umbrella/.git/modules/polynomial/config``.


