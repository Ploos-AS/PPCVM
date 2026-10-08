cmake_minimum_required(VERSION 3.16)
project(ppcvm VERSION 0.0.1 LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
add_library(ppcvm_core src/cpu.c src/cpu_bus.c src/memory.c src/bus.c src/pegasos2.c)
target_include_directories(ppcvm_core PUBLIC include)
if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(ppcvm_core PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(ppcvm src/main.c)
target_link_libraries(ppcvm PRIVATE ppcvm_core)
enable_testing()
add_executable(test_cpu tests/test_cpu.c)
target_link_libraries(test_cpu PRIVATE ppcvm_core)
add_test(NAME cpu_basics COMMAND test_cpu)

add_executable(test_memory tests/test_memory.c)
target_link_libraries(test_memory PRIVATE ppcvm_core)
add_test(NAME memory_basics COMMAND test_memory)

add_executable(test_load_store tests/test_load_store.c)
target_link_libraries(test_load_store PRIVATE ppcvm_core)
add_test(NAME load_store COMMAND test_load_store)

add_executable(test_bus tests/test_bus.c)
target_link_libraries(test_bus PRIVATE ppcvm_core)
add_test(NAME bus_regions COMMAND test_bus)

add_executable(test_cpu_bus tests/test_cpu_bus.c)
target_link_libraries(test_cpu_bus PRIVATE ppcvm_core)
add_test(NAME cpu_bus COMMAND test_cpu_bus)

add_executable(test_mmio32 tests/test_mmio32.c)
target_link_libraries(test_mmio32 PRIVATE ppcvm_core)
add_test(NAME mmio32 COMMAND test_mmio32)

add_executable(test_pegasos2 tests/test_pegasos2.c)
target_link_libraries(test_pegasos2 PRIVATE ppcvm_core)
add_test(NAME pegasos2_skeleton COMMAND test_pegasos2)

add_executable(test_pegasos2_program tests/test_pegasos2_program.c)
target_link_libraries(test_pegasos2_program PRIVATE ppcvm_core)
add_test(NAME pegasos2_program COMMAND test_pegasos2_program)

add_executable(test_pegasos2_run tests/test_pegasos2_run.c)
target_link_libraries(test_pegasos2_run PRIVATE ppcvm_core)
add_test(NAME pegasos2_run COMMAND test_pegasos2_run)

add_executable(test_cpu_branch tests/test_cpu_branch.c)
target_link_libraries(test_cpu_branch PRIVATE ppcvm_core)
add_test(NAME cpu_branch COMMAND test_cpu_branch)

add_executable(test_cpu_spr tests/test_cpu_spr.c)
target_link_libraries(test_cpu_spr PRIVATE ppcvm_core)
add_test(NAME cpu_spr COMMAND test_cpu_spr)

add_executable(test_cpu_bcctr tests/test_cpu_bcctr.c)
target_link_libraries(test_cpu_bcctr PRIVATE ppcvm_core)
add_test(NAME cpu_bcctr COMMAND test_cpu_bcctr)

add_executable(test_cpu_rlwinm tests/test_cpu_rlwinm.c)
target_link_libraries(test_cpu_rlwinm PRIVATE ppcvm_core)
add_test(NAME cpu_rlwinm COMMAND test_cpu_rlwinm)

add_executable(test_cpu_immediate_logic tests/test_cpu_immediate_logic.c)
target_link_libraries(test_cpu_immediate_logic PRIVATE ppcvm_core)
add_test(NAME cpu_immediate_logic COMMAND test_cpu_immediate_logic)

add_executable(test_cpu_register_ops tests/test_cpu_register_ops.c)
target_link_libraries(test_cpu_register_ops PRIVATE ppcvm_core)
add_test(NAME cpu_register_ops COMMAND test_cpu_register_ops)

add_executable(test_pegasos2_integration tests/test_pegasos2_integration.c)
target_link_libraries(test_pegasos2_integration PRIVATE ppcvm_core)
add_test(NAME pegasos2_integration COMMAND test_pegasos2_integration)

add_executable(test_cpu_rfi tests/test_cpu_rfi.c)
target_link_libraries(test_cpu_rfi PRIVATE ppcvm_core)
add_test(NAME cpu_rfi COMMAND test_cpu_rfi)
