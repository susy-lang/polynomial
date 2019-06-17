aux_source_directory(. SRC_LIST)

set(EXECUTABLE lllc)

file(GLOB HEADERS "*.h")
include_directories(BEFORE ..)
add_executable(${EXECUTABLE} ${SRC_LIST} ${HEADERS})

sof_use(${EXECUTABLE} REQUIRED Polynomial::lll Dev::buildinfo)

install( TARGETS ${EXECUTABLE} DESTINATION bin )

