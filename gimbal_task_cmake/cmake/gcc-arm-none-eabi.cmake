#-------------------------------------------------------------------------------
# ARM GNU Toolchain for STM32F407IGHx
#-------------------------------------------------------------------------------
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(TOOLCHAIN_PREFIX arm-none-eabi-)

find_program(ARM_TOOLCHAIN_GCC   NAMES ${TOOLCHAIN_PREFIX}gcc   REQUIRED)
find_program(ARM_TOOLCHAIN_GXX   NAMES ${TOOLCHAIN_PREFIX}g++   REQUIRED)
find_program(ARM_TOOLCHAIN_AR    NAMES ${TOOLCHAIN_PREFIX}ar    REQUIRED)
find_program(ARM_TOOLCHAIN_OBJCOPY NAMES ${TOOLCHAIN_PREFIX}objcopy REQUIRED)
find_program(ARM_TOOLCHAIN_OBJDUMP NAMES ${TOOLCHAIN_PREFIX}objdump REQUIRED)
find_program(ARM_TOOLCHAIN_SIZE  NAMES ${TOOLCHAIN_PREFIX}size  REQUIRED)
find_program(ARM_TOOLCHAIN_RANLIB NAMES ${TOOLCHAIN_PREFIX}ranlib REQUIRED)

set(CMAKE_C_COMPILER   "${ARM_TOOLCHAIN_GCC}")
set(CMAKE_ASM_COMPILER "${ARM_TOOLCHAIN_GCC}")
set(CMAKE_CXX_COMPILER "${ARM_TOOLCHAIN_GXX}")
set(CMAKE_AR           "${ARM_TOOLCHAIN_AR}")
set(CMAKE_OBJCOPY      "${ARM_TOOLCHAIN_OBJCOPY}")
set(CMAKE_OBJDUMP      "${ARM_TOOLCHAIN_OBJDUMP}")
set(CMAKE_SIZE         "${ARM_TOOLCHAIN_SIZE}")
set(CMAKE_RANLIB       "${ARM_TOOLCHAIN_RANLIB}")

# Cross compilers cannot link a hosted executable for the build machine.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(MCU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

set(CMAKE_C_FLAGS_INIT   "${MCU_FLAGS}")
set(CMAKE_ASM_FLAGS_INIT "${MCU_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${MCU_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${MCU_FLAGS}")

