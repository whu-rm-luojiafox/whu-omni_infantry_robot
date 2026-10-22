# User-owned build customisation, kept outside CubeMX-generated CMake.
include(${CMAKE_CURRENT_LIST_DIR}/UserSources.cmake)

target_sources(${PROJECT_NAME} PRIVATE
    ${APPLICATION_SOURCES}
    ${UI_SOURCES}
    ${BSP_SOURCES}
    ${COMPONENT_SOURCES}
    ${COMPAT_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    src/application
    src/bsp/boards
    src/components/algorithm
    Drivers/CMSIS/DSP/Include
    src/components/controller
    src/components/devices
    src/components/UI
)

#-------------------------------------------------------------------------------
# Compiler / linker options
#-------------------------------------------------------------------------------
target_compile_definitions(stm32cubemx INTERFACE
    USE_HAL_DRIVER
    STM32F407xx
    __FPU_PRESENT=1U
    __FPU_USED=1U
    ARM_MATH_CM4
    ARM_MATH_MATRIX_CHECK
    ARM_MATH_ROUNDING
)

target_compile_options(stm32cubemx INTERFACE
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
    -Wall
    -fdata-sections
    -ffunction-sections
    $<$<CONFIG:Debug>:-Og>
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:RelWithDebInfo>:-O2>
    $<$<CONFIG:RelWithDebInfo>:-g3>
)

target_link_options(${PROJECT_NAME} PRIVATE
    -T${CMAKE_CURRENT_SOURCE_DIR}/STM32F407XX_FLASH.ld
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.map
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    -specs=nano.specs
    -specs=nosys.specs
)

# AHRS.lib is an ARM Compiler (Keil) archive. GNU ld can link its AAPCS
# objects directly; arm_cortexM4lf_math.lib is replaced by
# src/bsp/compat/cmsis_dsp_compat.c.
target_link_libraries(${PROJECT_NAME}
    stm32cubemx
    "${CMAKE_CURRENT_SOURCE_DIR}/src/components/algorithm/AHRS.lib"
    m
)

#-------------------------------------------------------------------------------
# Post-build artifacts
#-------------------------------------------------------------------------------
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}>
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}>
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMENT "Generating HEX/BIN for ${PROJECT_NAME}"
)
