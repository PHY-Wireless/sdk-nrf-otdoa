#
# Copyright (c) 2020 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.13.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(otdoa)

# get settings from sysbuild
zephyr_get(OTDOA_DEV_VERSION SYSBUILD GLOBAL)
zephyr_get(OTDOA_DBG_VERSION SYSBUILD GLOBAL)
zephyr_get(OTDOA_LITTLE_FS   SYSBUILD GLOBAL)
zephyr_get(FS_EXT_STORAGE    SYSBUILD GLOBAL)
zephyr_get(OBFUSCATE         SYSBUILD GLOBAL)
zephyr_get(OTDOA_LIB_PATH    SYSBUILD GLOBAL)

# Optionally disable encrypted uBSA downloads and revert to v2 getuBSA
if("${OTDOA_DEV_VERSION}" STREQUAL "ON")
  add_compile_definitions(OTDOA_CONFIG_TARGET_NRF_DEV OTDOA_MIN_LOG=2)
elseif("${OTDOA_DBG_VERSION}" STREQUAL "ON")
  add_compile_definitions(OTDOA_CONFIG_TARGET_NRF_PROD OTDOA_MIN_LOG=2)
else()
  add_compile_definitions(OTDOA_CONFIG_TARGET_NRF_PROD OTDOA_MIN_LOG=1)
endif()

# include littlefs if specified, disable otherwise
add_compile_definitions(OTDOA_LITTLE_FS=1)
add_compile_definitions(ZEPHYR_FS_API)
# determine whether File System mounted on internal/external flash
if (FS_EXT_STORAGE)
  add_compile_definitions(FS_EXT_STORAGE=1)
else()
  add_compile_definitions(FS_EXT_STORAGE=0)
endif()

target_compile_options(app PRIVATE -flto -fdata-sections -ffunction-sections)
target_link_options(app PRIVATE --gc-sections)

# NORDIC SDK APP START

add_library(otdoa STATIC IMPORTED)
set_property(TARGET otdoa PROPERTY IMPORTED_LOCATION ${OTDOA_LIB_PATH})

option(ENABLE_CAPTURE "Enable capture support" OFF)

# Only the thingy91x board supports periodic position estimates
if("${CONFIG_BOARD}" STREQUAL "thingy91x")
    target_compile_definitions(app PRIVATE CONFIG_ENABLE_PERIODIC_POS_EST=1)
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
