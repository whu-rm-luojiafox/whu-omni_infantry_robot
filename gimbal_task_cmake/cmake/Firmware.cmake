include(${CMAKE_CURRENT_LIST_DIR}/Sources.cmake)

add_executable(${PROJECT_NAME}
    ${CORE_SOURCES}
    ${APPLICATION_SOURCES}
    ${UI_SOURCES}
    ${BSP_SOURCES}
    ${COMPONENT_SOURCES}
    ${HAL_SOURCES}
    ${FREERTOS_SOURCES}
    ${USB_SOURCES}
    platform/stm32f407/startup_stm32f407xx.s
)

#-------------------------------------------------------------------------------
# Include paths
#-------------------------------------------------------------------------------
target_include_directories(${PROJECT_NAME} PRIVATE
    platform/stm32f407/include
    third_party/stm32_hal/Inc
    third_party/stm32_hal/Inc/Legacy
    third_party/cmsis/Device/ST/STM32F4xx/Include
    third_party/cmsis/Include
    third_party/freertos/Source/include
    third_party/freertos/Source/CMSIS_RTOS
    third_party/freertos/Source/portable/GCC/ARM_CM4F
    third_party/stm32_usb_device/Core/Inc
    third_party/stm32_usb_device/Class/CDC/Inc
    src/application
    src/application/protocol
    src/bsp/boards
    src/components/algorithm
    third_party/cmsis/DSP/Include
    src/components/controller
    src/components/devices
    src/components/support
    src/components/ui
)

#-------------------------------------------------------------------------------
# Compiler / linker options
#-------------------------------------------------------------------------------
target_compile_definitions(${PROJECT_NAME} PRIVATE
    USE_HAL_DRIVER
    STM32F407xx
    __FPU_PRESENT=1U
    __FPU_USED=1U
    ARM_MATH_CM4
    ARM_MATH_MATRIX_CHECK
    ARM_MATH_ROUNDING
)

target_compile_options(${PROJECT_NAME} PRIVATE
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
    -T${CMAKE_CURRENT_SOURCE_DIR}/platform/stm32f407/STM32F407IGHX_FLASH.ld
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.map
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    -specs=nano.specs
    -specs=nosys.specs
)

# The gimbal uses QuaternionEKF instead of the prebuilt AHRS.lib.
# arm_cortexM4lf_math.lib is replaced by platform/stm32f407/src/cmsis_dsp_compat.c and
# the CMSIS-DSP matrix sources under third_party/cmsis/DSP/Source.
target_link_libraries(${PROJECT_NAME} PRIVATE m)

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
