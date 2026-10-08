cmake_minimum_required(VERSION 3.16)
project(ppcvm VERSION 0.0.1 LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
add_library(ppcvm_core src/cpu.c src/cpu_bus.c src/memory.c src/bus.c src/pegasos2.c src/mmu.c)
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

add_executable(test_cpu_syscall tests/test_cpu_syscall.c)
target_link_libraries(test_cpu_syscall PRIVATE ppcvm_core)
add_test(NAME cpu_syscall COMMAND test_cpu_syscall)

add_executable(test_cpu_exception_entry tests/test_cpu_exception_entry.c)
target_link_libraries(test_cpu_exception_entry PRIVATE ppcvm_core)
add_test(NAME cpu_exception_entry COMMAND test_cpu_exception_entry)

add_executable(test_cpu_twi tests/test_cpu_twi.c)
target_link_libraries(test_cpu_twi PRIVATE ppcvm_core)
add_test(NAME cpu_twi COMMAND test_cpu_twi)

add_executable(test_cpu_tw tests/test_cpu_tw.c)
target_link_libraries(test_cpu_tw PRIVATE ppcvm_core)
add_test(NAME cpu_tw COMMAND test_cpu_tw)

add_executable(test_pegasos2_isi tests/test_pegasos2_isi.c)
target_link_libraries(test_pegasos2_isi PRIVATE ppcvm_core)
add_test(NAME pegasos2_isi COMMAND test_pegasos2_isi)

add_executable(test_cpu_dsi tests/test_cpu_dsi.c)
target_link_libraries(test_cpu_dsi PRIVATE ppcvm_core)
add_test(NAME cpu_dsi COMMAND test_cpu_dsi)

add_executable(test_pegasos2_exceptions tests/test_pegasos2_exceptions.c)
target_link_libraries(test_pegasos2_exceptions PRIVATE ppcvm_core)
add_test(NAME pegasos2_exceptions COMMAND test_pegasos2_exceptions)

add_executable(test_mmu tests/test_mmu.c)
target_link_libraries(test_mmu PRIVATE ppcvm_core)
add_test(NAME mmu_real_mode COMMAND test_mmu)

add_executable(test_mmu_bat tests/test_mmu_bat.c)
target_link_libraries(test_mmu_bat PRIVATE ppcvm_core)
add_test(NAME mmu_bat COMMAND test_mmu_bat)

add_executable(test_pegasos2_bat_fetch tests/test_pegasos2_bat_fetch.c)
target_link_libraries(test_pegasos2_bat_fetch PRIVATE ppcvm_core)
add_test(NAME pegasos2_bat_fetch COMMAND test_pegasos2_bat_fetch)

add_executable(test_pegasos2_bat_data tests/test_pegasos2_bat_data.c)
target_link_libraries(test_pegasos2_bat_data PRIVATE ppcvm_core)
add_test(NAME pegasos2_bat_data COMMAND test_pegasos2_bat_data)

add_executable(test_pegasos2_bat_spr tests/test_pegasos2_bat_spr.c)
target_link_libraries(test_pegasos2_bat_spr PRIVATE ppcvm_core)
add_test(NAME pegasos2_bat_spr COMMAND test_pegasos2_bat_spr)

add_executable(test_mmu_bat_pp tests/test_mmu_bat_pp.c)
target_link_libraries(test_mmu_bat_pp PRIVATE ppcvm_core)
add_test(NAME mmu_bat_pp COMMAND test_mmu_bat_pp)

add_executable(test_pegasos2_bat_protection tests/test_pegasos2_bat_protection.c)
target_link_libraries(test_pegasos2_bat_protection PRIVATE ppcvm_core)
add_test(NAME pegasos2_bat_protection COMMAND test_pegasos2_bat_protection)

add_executable(test_mmu_bat_validation tests/test_mmu_bat_validation.c)
target_link_libraries(test_mmu_bat_validation PRIVATE ppcvm_core)
add_test(NAME mmu_bat_validation COMMAND test_mmu_bat_validation)

add_executable(test_mmu_segments tests/test_mmu_segments.c)
target_link_libraries(test_mmu_segments PRIVATE ppcvm_core)
add_test(NAME mmu_segments COMMAND test_mmu_segments)

add_executable(test_pegasos2_segments tests/test_pegasos2_segments.c)
target_link_libraries(test_pegasos2_segments PRIVATE ppcvm_core)
add_test(NAME pegasos2_segments COMMAND test_pegasos2_segments)

add_executable(test_pegasos2_sdr1 tests/test_pegasos2_sdr1.c)
target_link_libraries(test_pegasos2_sdr1 PRIVATE ppcvm_core)
add_test(NAME pegasos2_sdr1 COMMAND test_pegasos2_sdr1)
