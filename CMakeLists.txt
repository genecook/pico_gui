cmake_minimum_required(VERSION 3.13)

# PICO_SDK_PATH shell variable must be set before running cmake:
#   export PICO_SDK_PATH=~/Desktop/pico-sdk
#   export PICO_EXAMPLES=~/Desktop/pico-examples
#
#   export PICO_CHESS=~/Desktop/pico_chess

set(CMAKE_MODULE_PATH $ENV{PICO_SDK_PATH} $ENV{PICO_EXAMPLES})

include(pico_sdk_import)

# use shell variable and/or update this path to reflect where you
# have unpacked the Waveshare LCD code:

set(PICO_LCD_LIB $ENV{HOME}/Desktop/pico/Pico-ResTouch-LCD-X_X_Code/c)
set(PICO_CHESS $ENV{HOME}/Desktop/pico_chess)

project(my_gui C CXX ASM)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -DFOR_PICO -DREDIRECT_XBOARD_CALLS")

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
pico_enable_stdio_usb(line_sweeps 0)
pico_enable_stdio_uart(line_sweeps 1)
# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(line_sweeps)
target_link_libraries(line_sweeps examples lcd font config pico_stdlib hardware_spi fatfs)

# build display_bmp executable...

#add_executable(display_bmp display_bmp.C chess_game_gui.c my_fatfs_storage.c)
add_subdirectory(${PICO_CHESS}/include pico_chess/include)
add_subdirectory(${PICO_CHESS}/src pico_chess/src)

include_directories(${PICO_CHESS}/include)

add_executable(display_bmp display_bmp.C chess_game_gui.c stream_player_calls.C
               my_fatfs_storage.c my_track_touch.c
               ${PICO_CHESS}/src/pico_stream_player.C
               ${PICO_CHESS}/src/board.C ${PICO_CHESS}/src/move.C ${PICO_CHESS}/src/pieces.C
               ${PICO_CHESS}/src/bishop.C ${PICO_CHESS}/src/king.C ${PICO_CHESS}/src/knight.C
	       ${PICO_CHESS}/src/queen.C ${PICO_CHESS}/src/rook.C ${PICO_CHESS}/src/pawn.C
               ${PICO_CHESS}/src/moves_tree.C ${PICO_CHESS}/src/moves_tree_minimax.C
	       ${PICO_CHESS}/src/eval_move.C ${PICO_CHESS}/src/engine.C
	       )

target_link_libraries(display_bmp examples lcd font config pico_stdlib hardware_spi fatfs)
	    
# enable usb output, disable uart output
pico_enable_stdio_usb(display_bmp 0)
pico_enable_stdio_uart(display_bmp 1)
# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(display_bmp)

# build track_touch executable...

add_executable(track_touch track_touch.c my_track_touch.c)

target_link_libraries(track_touch examples lcd font config pico_stdlib hardware_spi fatfs)

# enable usb output, disable uart output
pico_enable_stdio_usb(track_touch 0)
pico_enable_stdio_uart(track_touch 1)
# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(track_touch)

# build pico_chess executable...

#add_subdirectory(${PICO_CHESS}/include pico_chess/include)
#add_subdirectory(${PICO_CHESS}/src pico_chess/src)

#include_directories(${PICO_CHESS}/include)

add_executable(pico_chess chess.C chess_game_gui.c stream_player_calls.C
               my_fatfs_storage.c my_track_touch.c
               ${PICO_CHESS}/src/pico_stream_player.C
               ${PICO_CHESS}/src/board.C ${PICO_CHESS}/src/move.C ${PICO_CHESS}/src/pieces.C
               ${PICO_CHESS}/src/bishop.C ${PICO_CHESS}/src/king.C ${PICO_CHESS}/src/knight.C
	       ${PICO_CHESS}/src/queen.C ${PICO_CHESS}/src/rook.C ${PICO_CHESS}/src/pawn.C
               ${PICO_CHESS}/src/moves_tree.C ${PICO_CHESS}/src/moves_tree_minimax.C
	       ${PICO_CHESS}/src/eval_move.C ${PICO_CHESS}/src/engine.C
               )

target_link_libraries(pico_chess examples lcd font config pico_stdlib hardware_spi fatfs)

# enable uart output, disable usb output
pico_enable_stdio_usb(pico_chess 0)
pico_enable_stdio_uart(pico_chess 1)
# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(pico_chess)






