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
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico-els C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(pico-els main.cpp )

pico_set_program_name(pico-els "pico-els")
pico_set_program_version(pico-els "0.1")

# Generate PIO headers
pico_generate_pio_header(pico-els ${CMAKE_CURRENT_LIST_DIR}/blink.pio)
pico_generate_pio_header(pico-els ${CMAKE_CURRENT_LIST_DIR}/stepper.pio)
pico_generate_pio_header(pico-els ${CMAKE_CURRENT_LIST_DIR}/quadrature_encoder.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(pico-els 1)
pico_enable_stdio_usb(pico-els 1)

# Add the standard library to the build
target_link_libraries(pico-els pico_stdlib)

# Add the standard include files to the build
target_include_directories(pico-els PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

add_library(pico_encoder INTERFACE)
target_sources(pico_encoder INTERFACE ${CMAKE_CURRENT_LIST_DIR}/Encoder.cpp)
add_library(pico_spibus INTERFACE)
target_sources(pico_spibus INTERFACE ${CMAKE_CURRENT_LIST_DIR}/SPIBus.cpp)
add_library(pico_controlpanel INTERFACE)
target_sources(pico_controlpanel INTERFACE ${CMAKE_CURRENT_LIST_DIR}/ControlPanel.cpp)
add_library(pico_stepperdrive INTERFACE)
target_sources(pico_stepperdrive INTERFACE ${CMAKE_CURRENT_LIST_DIR}/StepperDrive.cpp)
add_library(feedtable INTERFACE)
target_sources(feedtable INTERFACE ${CMAKE_CURRENT_LIST_DIR}/Tables.cpp)
add_library(core INTERFACE)
target_sources(core INTERFACE ${CMAKE_CURRENT_LIST_DIR}/Core.cpp)
add_library(ui INTERFACE)
target_sources(ui INTERFACE ${CMAKE_CURRENT_LIST_DIR}/UserInterface.cpp)
add_library(core_proxy INTERFACE)
target_sources(core_proxy INTERFACE ${CMAKE_CURRENT_LIST_DIR}/CoreProxy.cpp)
add_library(multicore_core INTERFACE)
target_sources(multicore_core INTERFACE ${CMAKE_CURRENT_LIST_DIR}/MulticoreCore.cpp)
add_library(cross_core_messaging INTERFACE)
target_sources(cross_core_messaging INTERFACE ${CMAKE_CURRENT_LIST_DIR}/CrossCoreMessaging.cpp)
add_library(gearbox INTERFACE)
target_sources(gearbox INTERFACE ${CMAKE_CURRENT_LIST_DIR}/Gearbox.cpp)

# Add any user requested libraries
target_link_libraries(pico-els 
        pico_multicore
        hardware_spi
        hardware_i2c
        hardware_pio
        hardware_timer
        hardware_clocks
        pico_encoder
        pico_spibus
        pico_controlpanel
        pico_stepperdrive
        feedtable
        core_proxy
        multicore_core
        cross_core_messaging
        core
        ui
        gearbox)

pico_add_extra_outputs(pico-els)
