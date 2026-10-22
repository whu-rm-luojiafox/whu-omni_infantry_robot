# chassis_task · 底盘固件

STM32F407IGHx，使用 CMake、Ninja 和 GNU Arm 工具链。两个工程分别配置、编译，各自携带源文件与依赖。

## 目录结构

```text
chassis_task_cmake/
├── CMakeLists.txt          # 工程入口
├── CMakePresets.json       # debug / release
├── chassis_task.ioc        # CubeMX 配置入口
├── Core/                  # CubeMX 初始化、中断与 FreeRTOS 配置
│   ├── Inc/
│   └── Src/
├── Drivers/               # STM32F4 HAL、CMSIS
├── Middlewares/           # FreeRTOS
├── startup_stm32f407xx.s
├── STM32F407XX_FLASH.ld
├── cmake/
│   ├── stm32cubemx/        # CubeMX 管理的源文件清单
│   ├── Firmware.cmake      # 自定义头文件路径、编译/链接选项
│   ├── UserSources.cmake   # 自写业务源文件清单
│   └── toolchains/gcc-arm-none-eabi.cmake
├── src/
│   ├── application/        # 业务任务
│   ├── bsp/boards/         # 板级驱动
│   └── components/         # 算法、控制器、设备、UI
└── build/                  # 编译时生成，Git 忽略
```

`Core/`、`Drivers/`、`Middlewares/` 保留 CubeMX 生成结构；`src/`、自定义 CMake 和测试由项目维护。

## 编译

安装 CMake ≥ 3.22、Ninja、GNU Arm 工具链，将 `arm-none-eabi-gcc` 等命令加入 PATH。在本目录执行：

```powershell
cmake --preset release
cmake --build --preset release

cmake --preset debug
cmake --build --preset debug
```

Release 使用 `-O2`，Debug 使用 `-Og -g3`。清理重编译可执行 `cmake --build --preset release --clean-first`。

输出位于 `build/release/` 或 `build/debug/`，包含 `chassis_task.elf`、`.hex`、`.bin`、`.map`。ELF 用于调试/下载，HEX 用于直接烧录，BIN 烧录地址为 `0x08000000`。

## 平台与依赖

FreeRTOS 使用 `Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F` 移植层。自写的 `src/bsp/compat/cmsis_dsp_compat.c` 提供基于 newlib 的正弦、余弦实现，不放入生成目录。

直接用 CubeMX 打开根目录 `chassis_task.ioc`。Project Manager 中保持 Toolchain 为 CMake、在工程根目录生成、Keep User Code 开启。修改外设后可直接 Generate Code，再执行上述 CMake 配置与编译命令。

外设文件由 CubeMX 更新，业务任务创建、CAN 队列初始化等扩展保留在 `Core/Src/freertos.c` 的 `USER CODE` 区。USART3、USART6 和 DMA2 Stream2 使用业务自定义中断入口，`.ioc` 已关闭对应的自动处理函数生成；修改这些中断前应先核对 `src/` 的实现。

新增业务 `.c` 文件只需加入 `cmake/UserSources.cmake`；自定义 include、宏和链接设置修改 `cmake/Firmware.cmake`。不要向 `cmake/stm32cubemx/CMakeLists.txt` 添加手写业务配置，该文件会被 CubeMX 重新生成。根目录 CMakeLists 使用 CubeMX 的稳定扩展方式接入自定义配置。

底盘保留 `src/components/algorithm/AHRS.lib`。这是 ARM Compiler 归档，GNU ld 直接链接其中的 AAPCS 对象，可能提示 `wchar_t` 宽度警告。

`gimbal_task` 的声明与定义使用 CMSIS-RTOS 要求的 `void (*)(const void *)` 任务接口，兼容 GCC 14 的回调类型检查。
