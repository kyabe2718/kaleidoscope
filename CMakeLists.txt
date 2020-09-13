cmake_minimum_required(VERSION 3.16)
project(kaleidoscope)

set(CMAKE_CXX_STANDARD 20)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_library(llvm INTERFACE)
target_include_directories(llvm INTERFACE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(llvm INTERFACE ${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm-libs support core irreader codegen orcerror orcjit native nativecodegen)
target_link_libraries(llvm INTERFACE ${llvm-libs})


file(GLOB SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
add_library(libkaleidoscope STATIC ${SOURCES})
target_link_libraries(libkaleidoscope llvm)
target_include_directories(libkaleidoscope PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

add_executable(kaleidoscope main.cpp)
target_link_libraries(kaleidoscope libkaleidoscope)

add_executable(test_lex EXCLUDE_FROM_ALL test/lex.cpp)
target_link_libraries(test_lex libkaleidoscope)
add_custom_target(do_test_lex
        COMMAND cat ${CMAKE_CURRENT_LIST_DIR}/test/fib.ks | ./test_lex
        DEPENDS test_lex)

add_executable(test_parse EXCLUDE_FROM_ALL test/parse.cpp)
target_link_libraries(test_parse libkaleidoscope)

add_executable(test_cogen EXCLUDE_FROM_ALL test/cogen.cpp)
target_link_libraries(test_cogen libkaleidoscope)

add_executable(test_jit EXCLUDE_FROM_ALL test/jit.cpp)
target_link_libraries(test_jit libkaleidoscope)
