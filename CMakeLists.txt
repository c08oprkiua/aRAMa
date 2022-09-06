cmake_minimum_required(VERSION 3.18)

# Set the project name
set(PROJECT_NAME "tcpgecko")
project(${PROJECT_NAME})

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

# Add definitions for smarter code analysis
# add_definitions(-D__WIIU__)

# Add extra include directories (for use with a Linux distribution or WSL in CLion)
set(DEVKITPPC /mnt/c/devkitPro)
set(WUT ${DEVKITPPC}/mnt/c/devkitPro/wut/include)
set(POWERPCEABI ${DEVKITPPC}/devkitPPC/powerpc-eabi/include)
set(LIBOGC ${DEVKITPPC}/libogc/include)
set(PORTLIBS ${DEVKITPPC}/portlibs/ppc/include)
set(WUPS ${DEVKITPPC}/wups/include)
set(WUMS ${DEVKITPPC}/wums/include)
include_directories(${PORTLIBS})
include_directories(${POWERPCEABI})
include_directories(${LIBOGC})
include_directories(${WUT})
include_directories(${WUPS})
include_directories(${WUMS})

add_executable(${PROJECT_NAME} src/entry.c

        src/common/common.h
        src/common/fs_defs.h
        src/common/kernel_defs.h
        src/common/loader_defs.h
        src/common/os_defs.h
        src/common/retain_vars.c
        src/common/retain_vars.h
        src/common/types.h

        src/dynamic_libs/aoc_functions.c
        src/dynamic_libs/aoc_functions.h
        src/dynamic_libs/ax_functions.c
        src/dynamic_libs/ax_functions.h
        src/dynamic_libs/fs_functions.c
        src/dynamic_libs/fs_functions.h
        src/dynamic_libs/gx2_functions.c
        src/dynamic_libs/gx2_functions.h
        src/dynamic_libs/os_functions.c
        src/dynamic_libs/os_functions.h
        src/dynamic_libs/padscore_functions.c
        src/dynamic_libs/padscore_functions.h
        src/dynamic_libs/socket_functions.c
        src/dynamic_libs/socket_functions.h
        src/dynamic_libs/sys_functions.c
        src/dynamic_libs/sys_functions.h
        src/dynamic_libs/vpad_functions.c
        src/dynamic_libs/vpad_functions.h

        src/fs/CFile.cpp
        src/fs/CFile.hpp
        src/fs/DirList.h
        src/fs/DirList.cpp
        src/fs/fs_utils.h
        src/fs/fs_utils.c
        src/fs/sd_fat_devoptab.c
        src/fs/sd_fat_devoptab.h

        src/game/memory_area_table.h
        src/game/memory_area_table.c
        src/game/rpx_rpl_table.h
        src/game/rpx_rpl_table.c

        src/kernel/kernel_functions.h
        src/kernel/kernel_functions.c
        src/kernel/kernel_hooks.S
        src/kernel/syscalls.c
        src/kernel/syscalls.h
        src/kernel/syscalls_asm.S

        src/patcher/fs_logger.h
        src/patcher/fs_logger.c
        src/patcher/function_patcher_coreinit.h
        src/patcher/function_patcher_coreinit.c
        src/patcher/function_patcher_gx2.h
        src/patcher/function_patcher_gx2.c

        src/system/exception_handler.h
        src/system/exception_handler.cpp
        src/system/memory.c
        src/system/memory.h

        src/tcpgecko/main.h
        src/tcpgecko/main.cpp
        src/tcpgecko/address.h
        src/tcpgecko/address.cpp
        src/tcpgecko/assertions.h
        src/tcpgecko/cafe.h
        src/tcpgecko/code_handler.h
        src/tcpgecko/disassembler.h
        src/tcpgecko/disassembler.cpp
        src/tcpgecko/hardware_breakpoints.h
        src/tcpgecko/hardware_breakpoints.s
        src/tcpgecko/kernel.h
        src/tcpgecko/linked_list.h
        src/tcpgecko/linked_list.cpp
        src/tcpgecko/net.h
        src/tcpgecko/net.cpp
        src/tcpgecko/pause.h
        src/tcpgecko/raw_assembly_cheats.h
        src/tcpgecko/sd_cheats.h
        src/tcpgecko/sd_cheats.cpp
        src/tcpgecko/sd_ip_reader.h
        src/tcpgecko/sd_ip_reader.cpp
        src/tcpgecko/software_breakpoints.h
        src/tcpgecko/stack.h
        src/tcpgecko/stringify.h
        src/tcpgecko/tcp_gecko.h
        src/tcpgecko/tcp_gecko.cpp
        src/tcpgecko/texture.h
        src/tcpgecko/threads.h
        src/tcpgecko/threads.cpp
        src/tcpgecko/title.h
        src/tcpgecko/title.cpp
        src/tcpgecko/utilities.h

        src/utils/function_patcher.h
        src/utils/function_patcher.cpp
        src/utils/logger.h
        src/utils/logger.c
        src/utils/ppc_asm.h
        src/utils/StringTools.h
        src/utils/StringTools.cpp
        src/utils/utils.h)