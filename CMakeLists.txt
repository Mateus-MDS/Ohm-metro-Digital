# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Projeto_2 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Projeto_2 Projeto_2.c lib/ssd1306.c)

target_compile_definitions(${PROJECT_NAME} PRIVATE 
        PICO_PRINTF_SUPPORT_FLOAT=1 
        PICO_STDIO_ENABLE_PRINTF=1
    )

pico_set_program_name(Projeto_2 "Projeto_2")
pico_set_program_version(Projeto_2 "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Projeto_2 0)
pico_enable_stdio_usb(Projeto_2 1)

pico_generate_pio_header(Projeto_2 ${CMAKE_CURRENT_LIST_DIR}/animacoes_led.pio)

# Add the standard library to the build
target_link_libraries(Projeto_2
        pico_stdlib
        hardware_i2c
        hardware_adc
        hardware_pio
        pico_stdio_usb)

# Add the standard include files to the build
target_include_directories(Projeto_2 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(Projeto_2 
        
        )

pico_add_extra_outputs(Projeto_2)

