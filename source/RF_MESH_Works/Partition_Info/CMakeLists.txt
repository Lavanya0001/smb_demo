# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Partition_Info C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Partition_Info main.c uf2_family_ids.c)

pico_set_program_name(Partition_Info "Partition_Info")
pico_set_program_version(Partition_Info "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Partition_Info 0)
pico_enable_stdio_usb(Partition_Info 1)

# Add the standard library to the build
target_link_libraries(Partition_Info
        pico_stdlib
        pico_stdlib
        pico_bootrom
        hardware_flash
        boot_uf2_headers)

# add a partition table
pico_embed_pt_in_binary(Partition_Info ${CMAKE_CURRENT_LIST_DIR}/pt.json)

# Add the standard include files to the build
target_include_directories(Partition_Info PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(Partition_Info)

