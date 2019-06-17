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

Ubuntu Trusty (14.04)
---------------------

Below are the instructions to install the minimal dependencies required
to compile Polynomial on Ubuntu 14.04 (Trusty Tahr).

.. note::

    These dependencies are not enough to compile the GUIs (Alsofzero and Mix).

.. code-block:: bash

    sudo apt-get -y install build-essential git cmake libgmp-dev libboost-all-dev \
        libjsoncpp-dev libleveldb-dev libcurl4-openssl-dev libminiupnpc-dev \
        libmicrohttpd-dev
    
    sudo add-apt-repository -y ppa:sophon/sophon
    sudo add-apt-repository -y ppa:sophon/sophon-dev
    sudo apt-get -y update
    sudo apt-get -y upgrade # this will update cmake to version 3.x
    sudo apt-get -y install libcryptopp-dev libjson-rpc-cpp-dev

Ubuntu Xenial (16.04)
---------------------

Below are the instructions to install the minimal dependencies required
to compile Polynomial on Ubuntu 16.04 (Xenial Xerus).

One of the dependencies (Crypto++ Library, with version >= 5.6.2) can be
installed either by adding the Sophon PPA (Option 1) or by backporting
``libcrypto++`` from Ubuntu Development to Ubuntu Xenial (Option 2).

.. note::

    These dependencies are not enough to compile the GUIs (Alsofzero and Mix).

.. code-block:: bash

    sudo apt-get -y install build-essential git cmake libgmp-dev libboost-all-dev \
        libjsoncpp-dev libleveldb-dev libcurl4-openssl-dev libminiupnpc-dev \
        libjsonrpccpp-dev libmicrohttpd-dev
    
    # (Option 1) For those willing to add the Sophon PPA:
    sudo add-apt-repository -y ppa:sophon/sophon
    sudo add-apt-repository -y ppa:sophon/sophon-dev
    sudo apt-get -y update
    sudo apt-get -y upgrade
    sudo apt-get -y install libcryptopp-dev
    
    ## (Option 2) For those willing to backport libcrypto++:
    #sudo apt-get -y install ubuntu-dev-tools
    #sudo pbuilder create
    #mkdir ubuntu
    #cd ubuntu
    #backportpackage --workdir=. --build --dont-sign libcrypto++
    #sudo dpkg -i buildresult/libcrypto++6_*.deb buildresult/libcrypto++-dev_*.deb
    #cd ..

Building
--------

Run this if you plan on installing Polynomial only:

.. code-block:: bash

    git clone --recursive https://octonion.institute/susy-cpp/webthree-umbrella.git
    cd webthree-umbrella
    ./webthree-helpers/scripts/sofupdate.sh --no-push --simple-pull --project polynomial # update Polynomial repo
    ./webthree-helpers/scripts/sofbuild.sh --no-git --project polynomial --cores 4 -DSVMJIT=0 -DSOFASHCL=0 # build Polynomial only

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


