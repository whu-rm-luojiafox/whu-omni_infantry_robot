# User-owned business sources. CubeMX does not manage this file.
set(APPLICATION_SOURCES
    src/application/CAN_receive.c
    src/application/Can_task.c
    src/application/Chassis_power_control.c
    src/application/INS_task.c
    src/application/calibrate_task.c
    src/application/chassis_behaviour.c
    src/application/chassis_task.c
    src/application/detect_task.c
    src/application/gimbal_task.c
    src/application/led_flow_task.c
    src/application/oled_task.c
    src/application/referee.c
    src/application/referee_usart_task.c
    src/application/remote_control.c
    src/application/set_power_task.c
    src/application/shoot.c
    src/application/test_task.c
    src/application/vofa.c
    src/application/voltage_task.c
)

set(BSP_SOURCES
    src/bsp/boards/bsp_adc.c
    src/bsp/boards/bsp_buzzer.c
    src/bsp/boards/bsp_can.c
    src/bsp/boards/bsp_delay.c
    src/bsp/boards/bsp_flash.c
    src/bsp/boards/bsp_i2c.c
    src/bsp/boards/bsp_imu_pwm.c
    src/bsp/boards/bsp_led.c
    src/bsp/boards/bsp_rc.c
    src/bsp/boards/bsp_spi.c
    src/bsp/boards/bsp_usart.c
)

set(COMPONENT_SOURCES
    src/components/algorithm/AHRS_middleware.c
    src/components/algorithm/CRC8_CRC16.c
    src/components/algorithm/fifo.c
    src/components/algorithm/user_lib.c
    src/components/controller/pid.c
    src/components/controller/smc.c
    src/components/devices/BMI088Middleware.c
    src/components/devices/BMI088driver.c
    src/components/devices/OLED.c
    src/components/devices/ist8310driver.c
    src/components/devices/ist8310driver_middleware.c
    src/components/UI/ui_default.c
    src/components/UI/ui_interface.c
    src/components/UI/ui_middle.c
    src/components/UI/ui_side.c
)

set(COMPAT_SOURCES src/bsp/compat/cmsis_dsp_compat.c)
