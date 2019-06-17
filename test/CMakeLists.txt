file(GLOB sources "*.cpp")
file(GLOB headers "*.h")

file(GLOB contracts_sources "contracts/*.cpp")
file(GLOB contracts_headers "contracts/*.h")
file(GLOB libdevcore_sources "libdevcore/*.cpp")
file(GLOB libdevcore_headers "libdevcore/*.h")
file(GLOB libsvmasm_sources "libsvmasm/*.cpp")
file(GLOB libsvmasm_headers "libsvmasm/*.h")
file(GLOB libyul_sources "libyul/*.cpp")
file(GLOB libyul_headers "libyul/*.h")
file(GLOB liblll_sources "liblll/*.cpp")
file(GLOB liblll_headers "liblll/*.h")
file(GLOB libpolynomial_sources "libpolynomial/*.cpp")
file(GLOB libpolynomial_headers "libpolynomial/*.h")

add_executable(poltest ${sources} ${headers}
    ${contracts_sources} ${contracts_headers}
    ${libdevcore_sources} ${libdevcore_headers}
    ${libsvmasm_sources} ${libsvmasm_headers}
    ${libyul_sources} ${libyul_headers}
    ${liblll_sources} ${liblll_headers}
    ${libpolynomial_sources} ${libpolynomial_headers}
)
target_link_libraries(poltest PRIVATE libpolc polynomial lll svmasm devcore ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})

if (NOT Boost_USE_STATIC_LIBS)
    target_compile_definitions(poltest PUBLIC -DBOOST_TEST_DYN_LINK)
endif()

add_subdirectory(tools)
