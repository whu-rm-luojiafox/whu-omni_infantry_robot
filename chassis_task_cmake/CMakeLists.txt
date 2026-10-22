cmake_minimum_required(VERSION 3.22)

# Use the bundled GCC toolchain unless the caller supplied one.
if(NOT CMAKE_TOOLCHAIN_FILE OR CMAKE_TOOLCHAIN_FILE STREQUAL "")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/gcc-arm-none-eabi.cmake")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (Debug/Release)" FORCE)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(chassis_task C ASM)

set(CMAKE_EXECUTABLE_SUFFIX ".elf")

#-------------------------------------------------------------------------------
# Sources
#-------------------------------------------------------------------------------
set(CORE_SOURCES
    Core/Src/main.c
    Core/Src/freertos.c
    Core/Src/adc.c
    Core/Src/can.c
    Core/Src/dma.c
    Core/Src/gpio.c
    Core/Src/i2c.c
    Core/Src/rtc.c
    Core/Src/spi.c
    Core/Src/tim.c
    Core/Src/usart.c
    Core/Src/stm32f4xx_hal_msp.c
    Core/Src/stm32f4xx_it.c
    Core/Src/system_stm32f4xx.c
    Core/Src/cmsis_dsp_compat.c
)

set(APPLICATION_SOURCES
    application/CAN_receive.c
    application/Can_task.c
    application/Chassis_power_control.c
    application/INS_task.c
    application/calibrate_task.c
    application/chassis_behaviour.c
    application/chassis_task.c
    application/detect_task.c
    application/gimbal_task.c
    application/led_flow_task.c
    application/oled_task.c
    application/referee.c
    application/referee_usart_task.c
    application/remote_control.c
    application/set_power_task.c
    application/shoot.c
    application/test_task.c
    application/vofa.c
    application/voltage_task.c
)

set(BSP_SOURCES
    bsp/boards/bsp_adc.c
    bsp/boards/bsp_buzzer.c
    bsp/boards/bsp_can.c
    bsp/boards/bsp_delay.c
    bsp/boards/bsp_flash.c
    bsp/boards/bsp_i2c.c
    bsp/boards/bsp_imu_pwm.c
    bsp/boards/bsp_led.c
    bsp/boards/bsp_rc.c
    bsp/boards/bsp_spi.c
    bsp/boards/bsp_usart.c
)

set(COMPONENT_SOURCES
    components/algorithm/AHRS_middleware.c
    components/algorithm/CRC8_CRC16.c
    components/algorithm/fifo.c
    components/algorithm/user_lib.c
    components/controller/pid.c
    components/controller/smc.c
    components/devices/BMI088Middleware.c
    components/devices/BMI088driver.c
    components/devices/OLED.c
    components/devices/ist8310driver.c
    components/devices/ist8310driver_middleware.c
    components/UI/ui_default.c
    components/UI/ui_interface.c
    components/UI/ui_middle.c
    components/UI/ui_side.c
)

file(GLOB HAL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/*.c")

set(FREERTOS_SOURCES
    Middlewares/Third_Party/FreeRTOS/Source/croutine.c
    Middlewares/Third_Party/FreeRTOS/Source/event_groups.c
    Middlewares/Third_Party/FreeRTOS/Source/list.c
    Middlewares/Third_Party/FreeRTOS/Source/queue.c
    Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c
    Middlewares/Third_Party/FreeRTOS/Source/tasks.c
    Middlewares/Third_Party/FreeRTOS/Source/timers.c
    Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
    Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
    Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/cmsis_os.c
)

add_executable(${PROJECT_NAME}
    ${CORE_SOURCES}
    ${APPLICATION_SOURCES}
    ${BSP_SOURCES}
    ${COMPONENT_SOURCES}
    ${HAL_SOURCES}
    ${FREERTOS_SOURCES}
    Core/Startup/startup_stm32f407xx.s
)

#-------------------------------------------------------------------------------
# Include paths
#-------------------------------------------------------------------------------
target_include_directories(${PROJECT_NAME} PRIVATE
    Core/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    Drivers/CMSIS/Device/ST/STM32F4xx/Include
    Drivers/CMSIS/Include
    Middlewares/Third_Party/FreeRTOS/Source/include
    Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS
    Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
    application
    bsp/boards
    components/algorithm
    components/algorithm/Include
    components/controller
    components/devices
    components/UI
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
    -T${CMAKE_CURRENT_SOURCE_DIR}/STM32F407IGHX_FLASH.ld
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.map
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    -specs=nano.specs
    -specs=nosys.specs
)

# AHRS.lib is an ARM Compiler (Keil) archive. GNU ld can link its AAPCS
# objects directly; arm_cortexM4lf_math.lib is replaced by
# Core/Src/cmsis_dsp_compat.c.
target_link_libraries(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/components/algorithm/AHRS.lib"
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
