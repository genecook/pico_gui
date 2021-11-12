cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

#set(PICO_SDK_PATH ~/Desktop/pico-sdk)
set(PICO_LCD_LIB /home/genec/Desktop/pico/Pico-ResTouch-LCD-X_X_Code/c)

project(my_gui)

#set(CMAKE_C_STANDARD 11)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFOR_PICO")

pico_sdk_init()

add_subdirectory(${PICO_LCD_LIB}/lib/config lib/config)
add_subdirectory(${PICO_LCD_LIB}/lib/lcd lib/lcd)
add_subdirectory(${PICO_LCD_LIB}/lib/font lib/font)
add_subdirectory(${PICO_LCD_LIB}/examples examples)
add_subdirectory(${PICO_LCD_LIB}/lib/fatfs lib/fatfs)
add_subdirectory(${PICO_LCD_LIB}/lib/sdcard lib/sdcard)

include_directories(${PICO_LCD_LIB}/examples)
include_directories(${PICO_LCD_LIB}/lib/config)
include_directories(${PICO_LCD_LIB}/lib/lcd)
include_directories(${PICO_LCD_LIB}/lib/font)
include_directories(${PICO_LCD_LIB}/lib/fatfs)
include_directories(${PICO_LCD_LIB}/lib/sdcard)

add_executable(my_gui main.c)

# enable usb output, disable uart output
pico_enable_stdio_usb(my_gui 1)
pico_enable_stdio_uart(my_gui 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(my_gui)

target_link_libraries(my_gui examples lcd font config pico_stdlib hardware_spi fatfs)


