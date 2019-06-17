set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

aux_source_directory(. SRC_LIST)

set(EXECUTABLE polsvmasm)

file(GLOB HEADERS "*.h")

include_directories(BEFORE ..)
add_library(${EXECUTABLE} ${SRC_LIST} ${HEADERS})
sof_use(${EXECUTABLE} REQUIRED Dev::poldevcore)
target_link_libraries(${EXECUTABLE} jsoncpp)

install( TARGETS ${EXECUTABLE} RUNTIME DESTINATION bin ARCHIVE DESTINATION lib LIBRARY DESTINATION lib )
