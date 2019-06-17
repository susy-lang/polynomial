# generates BuildInfo.h
# 
# this module expects
# SOF_SOURCE_DIR - main CMAKE_SOURCE_DIR
# SOF_DST_DIR - main CMAKE_BINARY_DIR
# SOF_BUILD_TYPE
# SOF_BUILD_PLATFORM
#
# example usage:
# cmake -DSOF_SOURCE_DIR=. -DSOF_DST_DIR=build -DSOF_BUILD_TYPE=Debug -DSOF_BUILD_PLATFORM=Darwin.appleclang -P scripts/buildinfo.cmake
#
# Its main output variables are POL_VERSION_BUILDINFO and POL_VERSION_PRERELEASE

if (NOT SOF_BUILD_TYPE)
	set(SOF_BUILD_TYPE "unknown")
endif()

if (NOT SOF_BUILD_PLATFORM)
	set(SOF_BUILD_PLATFORM "unknown")
endif()

# Logic here: If prereleases.txt exists but is empty, it is a non-pre release.
# If it does not exist, create our own prerelease string
if (EXISTS ${SOF_SOURCE_DIR}/prerelease.txt)
	file(READ ${SOF_SOURCE_DIR}/prerelease.txt POL_VERSION_PRERELEASE)
	string(STRIP "${POL_VERSION_PRERELEASE}" POL_VERSION_PRERELEASE)
else()
	string(TIMESTAMP POL_VERSION_PRERELEASE "develop.%Y.%m.%d" UTC)
	string(REPLACE .0 . POL_VERSION_PRERELEASE "${POL_VERSION_PRERELEASE}")
endif()

if (EXISTS ${SOF_SOURCE_DIR}/commit_hash.txt)
	file(READ ${SOF_SOURCE_DIR}/commit_hash.txt POL_COMMIT_HASH)
	string(STRIP ${POL_COMMIT_HASH} POL_COMMIT_HASH)
else()
	execute_process(
		COMMAND git --git-dir=${SOF_SOURCE_DIR}/.git --work-tree=${SOF_SOURCE_DIR} rev-parse --short=8 HEAD
		OUTPUT_VARIABLE POL_COMMIT_HASH OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
	)
	execute_process(
		COMMAND git --git-dir=${SOF_SOURCE_DIR}/.git --work-tree=${SOF_SOURCE_DIR} diff HEAD --shortstat
		OUTPUT_VARIABLE POL_LOCAL_CHANGES OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
	)
endif()

if (POL_COMMIT_HASH)
	string(STRIP ${POL_COMMIT_HASH} POL_COMMIT_HASH)
	string(SUBSTRING ${POL_COMMIT_HASH} 0 8 POL_COMMIT_HASH)
endif()

if (NOT POL_COMMIT_HASH)
	message(FATAL_ERROR "Unable to determine commit hash. Either compile from within git repository or "
		"supply a file called commit_hash.txt")
endif()
if (NOT POL_COMMIT_HASH MATCHES [a-f0-9][a-f0-9][a-f0-9][a-f0-9][a-f0-9][a-f0-9][a-f0-9][a-f0-9])
    message(FATAL_ERROR "Malformed commit hash \"${POL_COMMIT_HASH}\". It has to consist of exactly 8 hex digits.")
endif()

if (POL_COMMIT_HASH AND POL_LOCAL_CHANGES)
	set(POL_COMMIT_HASH "${POL_COMMIT_HASH}.mod")
endif()

set(POL_VERSION_BUILDINFO "commit.${POL_COMMIT_HASH}.${SOF_BUILD_PLATFORM}")

set(TMPFILE "${SOF_DST_DIR}/BuildInfo.h.tmp")
set(OUTFILE "${SOF_DST_DIR}/BuildInfo.h")

configure_file("${SOF_BUILDINFO_IN}" "${TMPFILE}")

include("${SOF_CMAKE_DIR}/SofUtils.cmake")
replace_if_different("${TMPFILE}" "${OUTFILE}" CREATE)

