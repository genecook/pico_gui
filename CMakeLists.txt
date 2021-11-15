cmake_minimum_required(VERSION 3.13)

# PICO_SDK_PATH shell variable must be set before running cmake:
# export PICO_SDK_PATH=~/Desktop/pico-sdk

include(pico_sdk_import.cmake)

# use shell variable and/or update this path to reflect where you
# have unpacked the Waveshare LCD code:

set(PICO_LCD_LIB $ENV{HOME}/Desktop/pico/Pico-ResTouch-LCD-X_X_Code/c)

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

# build line_sweeps executable...

add_executable(line_sweeps line_sweeps.c)
# enable usb output, disable uart output
pico_enable_stdio_usb(line_sweeps 1)
pico_enable_stdio_uart(line_sweeps 0)
# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(line_sweeps)
target_link_libraries(line_sweeps examples lcd font config pico_stdlib hardware_spi fatfs)

# build display_bmp executable...

add_executable(display_bmp display_bmp.c)
# enable usb output, disable uart output
pico_enable_stdio_usb(display_bmp 1)
pico_enable_stdio_uart(display_bmp 0)
# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(display_bmp)
target_link_libraries(display_bmp examples lcd font config pico_stdlib hardware_spi fatfs)

# build track_touch executable...

add_executable(track_touch track_touch.c)
# enable usb output, disable uart output
pico_enable_stdio_usb(track_touch 1)
pico_enable_stdio_uart(track_touch 0)
# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(track_touch)
target_link_libraries(track_touch examples lcd font config pico_stdlib hardware_spi fatfs)




