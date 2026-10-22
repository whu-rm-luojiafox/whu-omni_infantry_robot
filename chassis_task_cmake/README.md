# chassis_task · 底盘固件

STM32F407IGHx，使用 CMake、Ninja 和 GNU Arm 工具链。两个工程分别配置、编译，各自携带源文件与依赖。

## 目录结构

```text
chassis_task_cmake/
├── CMakeLists.txt          # 工程入口
├── CMakePresets.json       # debug / release
├── cmake/
│   ├── Firmware.cmake      # 固件目标、头文件路径、编译/链接选项
│   ├── Sources.cmake       # 本工程的源文件清单
│   └── toolchains/gcc-arm-none-eabi.cmake
├── src/
│   ├── application/        # 业务任务
│   ├── bsp/boards/         # 板级驱动
│   └── components/         # 算法、控制器、设备、UI
├── platform/stm32f407/
│   ├── include/            # 外设配置头、FreeRTOSConfig.h
│   ├── src/                # main、任务入口、外设初始化
│   ├── cubemx/chassis_task.ioc
│   ├── startup_stm32f407xx.s
│   └── STM32F407IGHX_FLASH.ld
├── third_party/
│   ├── cmsis/              # CMSIS Core、Device、DSP
│   ├── stm32_hal/          # STM32F4 HAL
│   └── freertos/
└── build/                  # 编译时生成，Git 忽略
```

分层参照 `E:/02GC/balance_infantry/balance_infantry`，仅保留本工程需要的内容。

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

FreeRTOS 使用 `third_party/freertos/Source/portable/GCC/ARM_CM4F` 移植层。`platform/stm32f407/src/cmsis_dsp_compat.c` 继续提供基于 newlib 的正弦、余弦实现。

CubeMX 配置移至 `platform/stm32f407/cubemx/chassis_task.ioc`，文件内容保留。此分层由 CMake 维护；需要重新生成时，请先在临时目录生成，再将初始化代码和配置合入 `platform/stm32f407`，保留 GCC 兼容修改及本工程的源文件清单。

底盘保留 `src/components/algorithm/AHRS.lib`。这是 ARM Compiler 归档，GNU ld 直接链接其中的 AAPCS 对象，可能提示 `wchar_t` 宽度警告。

`gimbal_task` 的声明与定义使用 CMSIS-RTOS 要求的 `void (*)(const void *)` 任务接口，兼容 GCC 14 的回调类型检查。
