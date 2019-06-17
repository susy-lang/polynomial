set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

aux_source_directory(analysis SRC_LIST)
aux_source_directory(ast SRC_LIST)
aux_source_directory(codegen SRC_LIST)
aux_source_directory(formal SRC_LIST)
aux_source_directory(interface SRC_LIST)
aux_source_directory(parsing SRC_LIST)

set(EXECUTABLE polynomial)

file(GLOB HEADERS "*/*.h") 

include_directories(BEFORE ..)
add_library(${EXECUTABLE} ${SRC_LIST} ${HEADERS})

sof_use(${EXECUTABLE} REQUIRED Dev::devcore Sof::svmcore Sof::svmasm)

install( TARGETS ${EXECUTABLE} RUNTIME DESTINATION bin ARCHIVE DESTINATION lib LIBRARY DESTINATION lib )
install( FILES ${HEADERS} DESTINATION include/${EXECUTABLE} )

