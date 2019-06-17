set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

aux_source_directory(. SRC_LIST)

set(EXECUTABLE poldevcore)

file(GLOB HEADERS "*.h")

include_directories(..)
add_library(${EXECUTABLE} ${SRC_LIST} ${HEADERS})

sof_use(${EXECUTABLE} REQUIRED Dev::base)

install( TARGETS ${EXECUTABLE} RUNTIME DESTINATION bin ARCHIVE DESTINATION lib LIBRARY DESTINATION lib )
