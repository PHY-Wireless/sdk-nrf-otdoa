#
# Copyright (c) 2020 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(otdoa)

# get settings from sysbuild
zephyr_get(otdoa_FS_EXT_STORAGE    SYSBUILD GLOBAL)
message("otdoa_FS_EXT_STORAGE : ${otdoa_FS_EXT_STORAGE}")
message("CONFIG_OTDOA_DEBUG_LOGS : ${CONFIG_OTDOA_DEBUG_LOGS}")

# include littlefs if specified, disable otherwise
add_compile_definitions(OTDOA_LITTLE_FS=1)
add_compile_definitions(ZEPHYR_FS_API)
# determine whether File System mounted on internal/external flash
if ("${otdoa_FS_EXT_STORAGE}" STREQUAL "ON")
  add_compile_definitions(FS_EXT_STORAGE=1)
else()
  add_compile_definitions(FS_EXT_STORAGE=0)
endif()

target_compile_options(app PRIVATE -flto -fdata-sections -ffunction-sections)
target_link_options(app PRIVATE --gc-sections)

# NORDIC SDK APP START

add_library(otdoa STATIC IMPORTED)
if (CONFIG_OTDOA_DEBUG_LOGS)
  # Use library with extended logging
  message("Linking with debug logging version of OTDOA library")
  set_property(TARGET otdoa PROPERTY IMPORTED_LOCATION "${NRF_DIR}/ext/lib/bin/otdoa/libotdoa_dbg.a")
else()
  set_property(TARGET otdoa PROPERTY IMPORTED_LOCATION "${NRF_DIR}/ext/lib/bin/otdoa/libotdoa.a")
endif()

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/otdoa_shell.c)
target_sources(app PRIVATE src/otdoa_sample_app.c)
target_sources(app PRIVATE src/otdoa_gpio.c)
target_sources(app PRIVATE src/otdoa_little_fs.c)

target_include_directories(app PRIVATE ./include)

target_link_libraries(app PUBLIC otdoa)
target_link_libraries(app PUBLIC m)

# GCC 11 by default emits DWARF version 5 which cannot be parsed by
# pyelftools. Can be removed once pyelftools supports v5.
add_link_options(-gdwarf-4)

# NORDIC SDK APP END
