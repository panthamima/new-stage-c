project(C_P2P_Chat C)

cmake_minimum_required(VERSION 3.9)
set(CMAKE_C_STANDARD 11)

# Подключаем ncursesw
set(CURSES_NEED_NCURSES TRUE)
set(CURSES_NEED_WIDE TRUE)
find_package(Curses)
include(FindCurses)

set(SOURCE_FILES
        src/main.c
        src/chat.c
        src/clients.c
        src/network/socket.c
        src/network/packet.c
        src/utils/utils.c
        src/utils/interface.c)

add_executable(C_P2P_Chat ${SOURCE_FILES})
target_link_libraries(C_P2P_Chat ${CURSES_LIBRARIES})