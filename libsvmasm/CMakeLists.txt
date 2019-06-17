file(GLOB sources "*.cpp")
file(GLOB headers "*.h")

add_library(svmasm ${sources} ${headers})
target_link_libraries(svmasm PUBLIC devcore jsoncpp)
