# generates BuildInfo.h
# 
# this module expects
# SOF_SOURCE_DIR - main CMAKE_SOURCE_DIR
# SOF_DST_DIR - main CMAKE_BINARY_DIR
# SOF_BUILD_TYPE
# SOF_BUILD_PLATFORM
# SOF_BUILD_NUMBER
# SOF_VERSION_SUFFIX
#
# example usage:
# cmake -DSOF_SOURCE_DIR=. -DSOF_DST_DIR=build -DSOF_BUILD_TYPE=Debug -DSOF_BUILD_PLATFORM=Darwin/appleclang -P scripts/buildinfo.cmake

if (NOT SOF_BUILD_TYPE)
	set(SOF_BUILD_TYPE "unknown")
endif()

if (NOT SOF_BUILD_PLATFORM)
	set(SOF_BUILD_PLATFORM "unknown")
endif()

execute_process(
	COMMAND git --git-dir=${SOF_SOURCE_DIR}/.git --work-tree=${SOF_SOURCE_DIR} rev-parse HEAD
	OUTPUT_VARIABLE SOF_COMMIT_HASH OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
) 

if (NOT SOF_COMMIT_HASH)
	set(SOF_COMMIT_HASH 0)
endif()

execute_process(
	COMMAND git --git-dir=${SOF_SOURCE_DIR}/.git --work-tree=${SOF_SOURCE_DIR} diff HEAD --shortstat
	OUTPUT_VARIABLE SOF_LOCAL_CHANGES OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
)

if (SOF_LOCAL_CHANGES)
	set(SOF_CLEAN_REPO 0)
else()
	set(SOF_CLEAN_REPO 1)
endif()

set(TMPFILE "${SOF_DST_DIR}/BuildInfo.h.tmp")
set(OUTFILE "${SOF_DST_DIR}/BuildInfo.h")

configure_file("${SOF_BUILDINFO_IN}" "${TMPFILE}")

include("${SOF_CMAKE_DIR}/SofUtils.cmake")
replace_if_different("${TMPFILE}" "${OUTFILE}" CREATE)

