add_custom_target(ossfuzz)
add_dependencies(ossfuzz polc_opt_ossfuzz polc_noopt_ossfuzz const_opt_ossfuzz)

#[[FuzzingEngine.a is provided by oss-fuzz's Dockerized build environment]]
add_executable(polc_opt_ossfuzz polc_opt_ossfuzz.cpp ../fuzzer_common.cpp)
target_link_libraries(polc_opt_ossfuzz PRIVATE libpolc svmasm FuzzingEngine.a)

add_executable(polc_noopt_ossfuzz polc_noopt_ossfuzz.cpp ../fuzzer_common.cpp)
target_link_libraries(polc_noopt_ossfuzz PRIVATE libpolc svmasm FuzzingEngine.a)

add_executable(const_opt_ossfuzz const_opt_ossfuzz.cpp ../fuzzer_common.cpp)
target_link_libraries(const_opt_ossfuzz PRIVATE libpolc svmasm FuzzingEngine.a)
