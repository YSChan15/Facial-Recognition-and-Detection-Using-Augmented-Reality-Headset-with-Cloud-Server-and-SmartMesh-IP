cmake_minimum_required(VERSION 3.15.3)

# Optional: print out extra messages to see what is going on. Comment it to have less verbose messages
set(CMAKE_VERBOSE_MAKEFILE OFF)

# Path to toolchain file. This one has to be before 'project()' below
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/arm-none-eabi-gcc.cmake)

# Setup project, output and linker file
project(Senior-Design)
set(EXECUTABLE ${PROJECT_NAME}.elf)
set(LINKER_FILE ${CMAKE_SOURCE_DIR}/device/saml21g18b_flash.ld)

 # Set the build type to Debug use set(CMAKE_BUILD_TYPE Release) to 
set(CMAKE_BUILD_TYPE Debug)

enable_language(CXX C ASM)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Optional: issue a message to be sure it uses the correct toolchain file.
message(STATUS "CMAKE_TOOLCHAIN_FILE is: ${CMAKE_TOOLCHAIN_FILE}")

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "CMSIS/*.h")

# List of source files
set(SRC_FILES
        src/main.cpp
        src/Drivers/LTE.cpp
        src/Drivers/RTCClass.cpp
        src/Drivers/SERIALCOM.cpp
        src/Drivers/DMA.cpp
        src/System/system_setup.cpp
        src/System/API.cpp
        src/System/bluetooth.cpp
        src/System/send_task.cpp
        src/System/lte_parse.cpp
        src/System/sm_parse.cpp
        src/Common/common.cpp

        device/startup_saml21.c
        device/system_saml21.c
        
        FreeRTOS/Source/tasks.c
        FreeRTOS/Source/event_groups.c
        FreeRTOS/Source/croutine.c
        FreeRTOS/Source/stream_buffer.c
        FreeRTOS/Source/portable/port.c
        FreeRTOS/Source/portable/MemMang/heap_4.c
        FreeRTOS/Source/list.c
        FreeRTOS/Source/queue.c
        FreeRTOS/Source/timers.c
        )

# Build the executable based on the source files
add_executable(${EXECUTABLE} ${SRC_FILES} ${SOURCES})

# List of compiler defines, prefix with -D compiler option
target_compile_definitions(${EXECUTABLE} PRIVATE
        -D__SAML21J18B__ 
        -DDONT_USE_CMSIS_INIT 
        -DF_CPU=8000000
        )

# List of include directories
target_include_directories(${EXECUTABLE} PRIVATE
        src 
        src/Drivers/Inc
        src/System/Inc
        src/Common/Inc
        device

        CMSIS
        CMSIS/include
        CMSIS/include/component
        CMSIS/include/pio
        CMSIS/include/instance

        FreeRTOS/Source/include
        FreeRTOS/Source/portable
        )

# Set compile options for the target executable
target_compile_options(${EXECUTABLE} PRIVATE
        # Set the CPU architecture to Cortex-M0+
        -mcpu=cortex-m0plus
        # Set the instruction set to Thumb
        -mthumb
        # Set the target architecture to ARMv6-M
        -march=armv6s-m
        # Set the byte order to little-endian
        -mlittle-endian

        # Place each variable or function in its own data or function section
        -fdata-sections
        -ffunction-sections

        # Enable all compiler warnings
        -Wall
        # Disable optimization
        -O2
        # Include debugging information in the generated object files for use with GDB
        -ggdb

        # Ignore these warnings
        -Wno-parentheses 
        #-w
        )

# Linker options
target_link_options(${EXECUTABLE} PRIVATE
        -T${LINKER_FILE}
        -L../${ARM_TOOLCHAIN_DIR}/lib/gcc/arm-none-eabi/10.2.1/thumb/v6-m/
        -L../${ARM_TOOLCHAIN_DIR}/arm-none-eabi/lib/thumb/v6-m/
        -L../${ARM_TOOLCHAIN_DIR}/arm-none-eabi/lib/
        -L../${ARM_TOOLCHAIN_DIR}/lib/
        -L../${ARM_TOOLCHAIN_DIR}/lib/gcc/arm-none-eabi/10.2.1/
        -static
        -mcpu=cortex-m0plus
        -march=armv6-m
        -mthumb

        -specs=nano.specs
        -specs=nosys.specs
        -falign-functions=4
        
        -lc
        -lm
        -lnosys
        -Wl,-Map=${PROJECT_NAME}.map,--cref
        -Wl,--gc-sections
        -Xlinker -print-memory-usage -Xlinker
        )

# Optional: Print executable size as part of the post build process
add_custom_command(TARGET ${EXECUTABLE}
        POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE})

# Optional: Create hex, bin and S-Record files after the build
add_custom_command(TARGET ${EXECUTABLE}
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O srec --srec-len=64 ${EXECUTABLE} ${PROJECT_NAME}.s19
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE} ${PROJECT_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE} ${PROJECT_NAME}.bin)