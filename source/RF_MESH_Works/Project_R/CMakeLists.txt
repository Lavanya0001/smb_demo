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
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Project_R C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_LIST_DIR}/Lib/freeRTOS/FreeRTOS-Kernel)
include(${CMAKE_CURRENT_LIST_DIR}/Lib/freeRTOS/FreeRTOS_Kernel_import.cmake)
# Add executable. Default name is the project name, version 0.1

add_executable(Project_R 
        Src/main.cpp
        Lib/API/PIO.cpp
        Lib/API/EEPROM.cpp
        Lib/Flash/Flash_Inc.cpp
)

pico_set_program_name(Project_R "Project_R")
pico_set_program_version(Project_R "0.1")

# Modify the below lines to enable/disSrcable output over UART/USB
pico_enable_stdio_uart(Project_R 0)
pico_enable_stdio_usb(Project_R 1)

#To define it's not the Arduino
add_definitions(-DRADIOLIB_STATIC_ONLY)
add_subdirectory(Lib/RadioLib)

# Add the standard library to the build
target_link_libraries(Project_R
        pico_stdlib
        pico_bootrom

        FreeRTOS-Kernel
        pico_multicore
        pico_async_context_freertos
        FreeRTOS-Kernel-Heap4
        
        hardware_i2c
        hardware_adc
        hardware_spi
        hardware_pwm

        RadioLib
)

#Linking the partition JSON file to the file
# pico_embed_pt_in_binary(Project_R ${CMAKE_CURRENT_LIST_DIR}/Lib/Flash/pt.json)

# Add the standard include files to the build
target_include_directories(Project_R PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/Lib/freeRTOS
        ${CMAKE_CURRENT_LIST_DIR}/Lib/API
        ${CMAKE_CURRENT_LIST_DIR}/Lib/freeRTOS/FreeRTOS-Kernel
        ${CMAKE_CURRENT_LIST_DIR}/Lib/RadioLib/src
        ${CMAKE_CURRENT_LIST_DIR}/Lib/RadioLib/src/protocols/PhysicalLayer
        ${CMAKE_CURRENT_LIST_DIR}/Lib/RadioLib/src/modules/SX127x
        ${CMAKE_CURRENT_LIST_DIR}/Lib/RadioLib/src/modules/SX126x
        ${CMAKE_CURRENT_LIST_DIR}/Lib/RadioLib/src/hal/RPiPico
)

target_sources(Project_R PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Lib/RadioLib/src/hal/RPiPico/PicoHal.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Lib/API/Application_Layer.cpp

)
target_compile_definitions(Project_R PRIVATE ARDUINO=0 RADIOLIB_BUILD_RPI_PICO)
pico_add_extra_outputs(Project_R)

