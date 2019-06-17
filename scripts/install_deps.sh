#!/usr/bin/env sh

#------------------------------------------------------------------------------
# Shell script for installing pre-requisite packages for polynomial on a
# variety of Linux and other UNIX-derived platforms.
#
# This is an "infrastucture-as-code" alternative to the manual build
# instructions pages which we previously maintained at:
# http://polynomial.readthedocs.io/en/latest/installing-polynomial.html
#
# The aim of this script is to simplify things down to the following basic
# flow for all supported operating systems:
#
# - git clone --recursive
# - ./scripts/install_deps.sh
# - cmake && make
#
# TODO - There is no support here yet for cross-builds in any form, only
# native builds.  Expanding the functionality here to cover the mobile,
# wearable and SBC platforms covered by doublsofink and SofEmbedded would
# also bring in support for Android, iOS, watchOS, tvOS, Tizen, Sailfish,
# Maemo, MeeGo and Yocto.
#
# The documentation for polynomial is hosted at:
#
# http://polynomial.readthedocs.io/
#
# ------------------------------------------------------------------------------
# This file is part of polynomial.
#
# polynomial is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# polynomial is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MSRCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with polynomial.  If not, see <http://www.gnu.org/licenses/>
#
# (c) 2016 polynomial contributors.
#------------------------------------------------------------------------------

set -e

# Check for 'uname' and abort if it is not available.
uname -v > /dev/null 2>&1 || { echo >&2 "ERROR - polynomial requires 'uname' to identify the platform."; exit 1; }

# See http://unix.stackexchange.com/questions/92199/how-can-i-reliably-get-the-operating-systems-name
detect_linux_distro() {
    if [ $(command -v lsb_release) ]; then
        DISTRO=$(lsb_release -is)
    elif [ -f /etc/os-release ]; then
        # extract 'foo' from NAME=foo, only on the line with NAME=foo
        DISTRO=$(sed -n -e 's/^NAME="\(.*\)\"/\1/p' /etc/os-release)
    else
        DISTRO=''
    fi
    echo $DISTRO
}

case $(uname -s) in

#------------------------------------------------------------------------------
# macOS
#------------------------------------------------------------------------------

    Darwin)
        case $(sw_vers -productVersion | awk -F . '{print $1"."$2}') in
            10.9)
                echo "Installing polynomial dependencies on OS X 10.9 Mavericks."
                ;;
            10.10)
                echo "Installing polynomial dependencies on OS X 10.10 Yosemite."
                ;;
            10.11)
                echo "Installing polynomial dependencies on OS X 10.11 El Capitan."
                ;;
            10.12)
                echo "Installing polynomial dependencies on macOS 10.12 Sierra."
                echo ""
                echo "NOTE - You are in unknown territory with this preview OS."
                echo "Even Homebrew doesn't have official support yet, and there are"
                echo "known issues (see https://octonion.institute/susy-cpp/webthree-umbrella/issues/614)."
                echo "If you would like to partner with us to work through these issues, that"
                echo "would be fantastic.  Please just comment on that issue.  Thanks!"
                ;;
            *)
                echo "Unsupported macOS version."
                echo "We only support Mavericks, Yosemite and El Capitan, with work-in-progress on Sierra."
                exit 1
                ;;
        esac

        # Check for Homebrew install and abort if it is not installed.
        brew -v > /dev/null 2>&1 || { echo >&2 "ERROR - polynomial requires a Homebrew install.  See http://brew.sh."; exit 1; }

        brew update
        brew upgrade

        brew install boost
        brew install cmake
        brew install jsoncpp

        # We should really 'brew install' our sof client here, but at the time of writing
        # the bottle is known broken, so we will just cheat and use a hardcoded ZIP for
        # the time being, which is good enough.   The cause of the breaks will go away
        # when we commit the repository reorg changes anyway.
        curl -L -O https://github.com/bobsummerwill/cpp-sophon/releases/download/v1.3.0/cpp-sophon-osx-mavericks-v1.3.0.zip
        unzip cpp-sophon-osx-mavericks-v1.3.0.zip

        ;;

#------------------------------------------------------------------------------
# FreeBSD
#------------------------------------------------------------------------------

    FreeBSD)
        echo "Installing polynomial dependencies on FreeBSD."
        echo "ERROR - 'install_deps.sh' doesn't have FreeBSD support yet."
        echo "Please let us know if you see this error message, and we can work out what is missing."
        echo "Drop us a message at https://gitter.im/susy-lang/polynomial."
        exit 1
        ;;

#------------------------------------------------------------------------------
# Linux
#------------------------------------------------------------------------------

    Linux)
        case $(detect_linux_distro) in

#------------------------------------------------------------------------------
# Arch Linux
#------------------------------------------------------------------------------

            Arch)
                #Arch
                echo "Installing polynomial dependencies on Arch Linux."

                # All our dependencies can be found in the Arch Linux official repositories.
                # See https://wiki.archlinux.org/index.php/Official_repositories
                sudo pacman -Sy \
                    base-devel \
                    boost \
                    cmake \
                    git \
                ;;

#------------------------------------------------------------------------------
# Alpine Linux
#------------------------------------------------------------------------------

            "Alpine Linux")
                #Alpine
                echo "Installing polynomial dependencies on Alpine Linux."

                # All our dependencies can be found in the Alpine Linux official repositories.
                # See https://pkgs.alpinelinux.org/

                apk update
                apk add boost-dev build-base cmake jsoncpp-dev

                ;;

#------------------------------------------------------------------------------
# Debian
#------------------------------------------------------------------------------

            Debian)
                #Debian
                case $(lsb_release -cs) in
                    wheezy)
                        #wheezy
                        echo "Installing polynomial dependencies on Debian Wheezy (7.x)."
                        echo "ERROR - 'install_deps.sh' doesn't have Debian Wheezy support yet."
                        echo "See http://polynomial.readthedocs.io/en/latest/installing-polynomial.html for manual instructions."
                        echo "If you would like to get 'install_deps.sh' working for Debian Wheezy, that would be fantastic."
                        echo "Drop us a message at https://gitter.im/susy-lang/polynomial."
                        echo "See also https://octonion.institute/susy-cpp/webthree-umbrella/issues/495 where we are working through Alpine support."
                        exit 1
                        ;;
                    jessie)
                        #jessie
                        echo "Installing polynomial dependencies on Debian Jesse (8.x)."
                        ;;
                    stretch)
                        #stretch
                        echo "Installing polynomial dependencies on Debian Stretch (9.x)."
                        echo "ERROR - 'install_deps.sh' doesn't have Debian Stretch support yet."
                        echo "See http://polynomial.readthedocs.io/en/latest/installing-polynomial.html for manual instructions."
                        echo "If you would like to get 'install_deps.sh' working for Debian Stretch, that would be fantastic."
                        echo "Drop us a message at https://gitter.im/susy-lang/polynomial."
                        exit 1
                        ;;
                    *)
                        #other Debian
                        echo "Installing polynomial dependencies on unknown Debian version."
                        echo "ERROR - Debian Jessie is the only Debian version which polynomial has been tested on."
                        echo "If you are using a different release and would like to get 'install_deps.sh'"
                        echo "working for that release that would be fantastic."
                        echo "Drop us a message at https://gitter.im/susy-lang/polynomial."
                        exit 1
                        ;;
                esac

                # Install "normal packages"
                sudo apt-get -y update
                sudo apt-get -y install \
                    python-sphinx \
                    build-essential \
                    cmake \
                    g++ \
                    gcc \
                    git \
                    libboost-all-dev \
                    libjsoncpp-dev \
                    unzip

                ;;

#------------------------------------------------------------------------------
# Fedora
#------------------------------------------------------------------------------

            Fedora)
                #Fedora
                echo "Installing polynomial dependencies on Fedora."

                # Install "normal packages"
                # See https://fedoraproject.org/wiki/Package_management_system.
                dnf install \
                    autoconf \
                    automake \
                    boost-devel \
                    cmake \
                    gcc \
                    gcc-c++ \
                    git \
                    libtool

                ;;

#------------------------------------------------------------------------------
# OpenSUSE
#------------------------------------------------------------------------------

            "openSUSE project")
                #openSUSE
                echo "Installing polynomial dependencies on openSUSE."
                echo "ERROR - 'install_deps.sh' doesn't have openSUSE support yet."
                echo "See http://polynomial.readthedocs.io/en/latest/installing-polynomial.html for manual instructions."
                echo "If you would like to get 'install_deps.sh' working for openSUSE, that would be fantastic."
                echo "See https://octonion.institute/susy-cpp/webthree-umbrella/issues/552."
                exit 1
                ;;

#------------------------------------------------------------------------------
# Ubuntu
#
# TODO - I wonder whether all of the Ubuntu-variants need some special
# treatment?
#
# TODO - We should also test this code on Ubuntu Server, Ubuntu Snappy Core
# and Ubuntu Phone.
#
# TODO - Our Ubuntu build is only working for amd64 and i386 processors.
# It would be good to add armel, armhf and arm64.
# See https://octonion.institute/susy-cpp/webthree-umbrella/issues/228.
#------------------------------------------------------------------------------

            Ubuntu)
                #Ubuntu
                case $(lsb_release -cs) in
                    trusty)
                        #trusty
                        echo "Installing polynomial dependencies on Ubuntu Trusty Tahr (14.04)."
                        ;;
                    utopic)
                        #utopic
                        echo "Installing polynomial dependencies on Ubuntu Utopic Unicorn (14.10)."
                        ;;
                    vivid)
                        #vivid
                        echo "Installing polynomial dependencies on Ubuntu Vivid Vervet (15.04)."
                        ;;
                    wily)
                        #wily
                        echo "Installing polynomial dependencies on Ubuntu Wily Werewolf (15.10)."
                        ;;
                    xenial)
                        #xenial
                        echo "Installing polynomial dependencies on Ubuntu Xenial Xerus (16.04)."
                        ;;
                    yakkety)
                        #yakkety
                        echo "Installing polynomial dependencies on Ubuntu Yakkety Yak (16.10)."
                        echo ""
                        echo "NOTE - You are in unknown territory with this preview OS."
                        echo "We will need to update the Sophon PPAs, work through build and runtime breaks, etc."
                        echo "See https://octonion.institute/susy-cpp/webthree-umbrella/issues/624."
                        echo "If you would like to partner with us to work through these, that"
                        echo "would be fantastic.  Please just comment on that issue.  Thanks!"
                        ;;
                    *)
                        #other Ubuntu
                        echo "ERROR - Unknown or unsupported Ubuntu version (" $(lsb_release -cs) ")"
                        echo "We only support Trusty, Utopic, Vivid, Wily and Xenial, with work-in-progress on Yakkety."
                        exit 1
                        ;;
                esac

                sudo apt-get -y update
                sudo apt-get -y install \
                    python-sphinx \
                    build-essential \
                    cmake \
                    git \
                    libboost-all-dev \
                    libjsoncpp-dev

                # Install 'sof', for use in the Polynomial Tests-over-IPC.
                sudo add-apt-repository -y ppa:sophon/sophon
                sudo add-apt-repository -y ppa:sophon/sophon-dev
                sudo apt-get -y update
                sudo apt-get -y install sof

                ;;
            *)

#------------------------------------------------------------------------------
# Other (unknown) Linux
# Major and medium distros which we are missing would include Mint, CentOS,
# RHEL, Raspbian, Cygwin, OpenWrt, gNewSense, Trisquel and SteamOS.
#------------------------------------------------------------------------------

                #other Linux
                echo "ERROR - Unsupported or unidentified Linux distro."
                echo "See http://polynomial.readthedocs.io/en/latest/installing-polynomial.html for manual instructions."
                echo "If you would like to get your distro working, that would be fantastic."
                echo "Drop us a message at https://gitter.im/susy-lang/polynomial."
                exit 1
                ;;
        esac
        ;;

#------------------------------------------------------------------------------
# Other platform (not Linux, FreeBSD or macOS).
# Not sure what might end up here?
# Maybe OpenBSD, NetBSD, AIX, Polaris, HP-UX?
#------------------------------------------------------------------------------

    *)
        #other
        echo "ERROR - Unsupported or unidentified operating system."
        echo "See http://polynomial.readthedocs.io/en/latest/installing-polynomial.html for manual instructions."
        echo "If you would like to get your operating system working, that would be fantastic."
        echo "Drop us a message at https://gitter.im/susy-lang/polynomial."
        ;;
esac
