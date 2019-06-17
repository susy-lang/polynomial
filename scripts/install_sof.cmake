#------------------------------------------------------------------------------
# Cmake script for installing pre-requisite package sof for polynomial.
#
# The aim of this script is to simply download and unpack sof binaries to the deps folder.
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

function(download URL DST_FILE STATUS)
    set(TMP_FILE "${DST_FILE}.part")

    get_filename_component(FILE_NAME ${DST_FILE} NAME)
    if (NOT EXISTS ${DST_FILE})
        message("Downloading ${FILE_NAME}")
        file(DOWNLOAD ${URL} ${TMP_FILE} SHOW_PROGRESS STATUS DOWNLOAD_STATUS)
        list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
        if (STATUS_CODE EQUAL 0)
            file(RENAME ${TMP_FILE} ${DST_FILE})
        else()
            file(REMOVE ${TMP_FILE})
            list(GET DOWNLOAD_STATUS 1 ERROR_MSG)

            message("ERROR! Downloading '${FILE_NAME}' failed.")
            message(STATUS "URL:   ${URL}")
            message(STATUS "Error: ${STATUS_CODE} ${ERROR_MSG}")
            set(STATUS FALSE PARENT_SCOPE)
            return()
        endif()
    else()
        message("Using cached ${FILE_NAME}")
    endif()
    set(STATUS TRUE PARENT_SCOPE)
endfunction(download)

function(download_and_unpack PACKAGE_URL DST_DIR)
    get_filename_component(FILE_NAME ${PACKAGE_URL} NAME)

    set(DST_FILE "${CACHE_DIR}/${FILE_NAME}")
    set(TMP_FILE "${DST_FILE}.part")

    file(MAKE_DIRECTORY ${CACHE_DIR})
    file(MAKE_DIRECTORY ${DST_DIR})

    download(${PACKAGE_URL} ${DST_FILE} STATUS)

    if (STATUS)
        message("Unpacking ${FILE_NAME} to ${DST_DIR}")
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf ${DST_FILE}
                        WORKING_DIRECTORY ${DST_DIR})
    endif()
endfunction(download_and_unpack)

get_filename_component(ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(CACHE_DIR "${ROOT_DIR}/deps/cache")
set(INSTALL_DIR "${ROOT_DIR}/deps/install/x64/sof")
download_and_unpack("https://github.com/bobsummerwill/cpp-sophon/releases/download/develop-v1.3.0.401/cpp-sophon-develop-windows.zip" ${INSTALL_DIR})
