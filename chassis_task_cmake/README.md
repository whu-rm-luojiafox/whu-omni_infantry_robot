# chassis_task（STM32CubeMX + CMake + GCC）

底盘工程的 STM32CubeMX / CMake 版本，目标芯片为 **STM32F407IGHx**（UFBGA176，1 MB Flash，128 KB RAM + 64 KB CCM）。

原来的 `../chassis_project`（Keil MDK-ARM）保持不动，本目录是独立的可编译工程。

## 目录结构

```
chassis_task_cmake/
├── CMakeLists.txt                  # 顶层构建脚本
├── CMakePresets.json               # debug / release 预设
├── cmake/gcc-arm-none-eabi.cmake   # arm-none-eabi-gcc 工具链
├── Core/
│   ├── Inc/                        # CubeMX 生成的配置头
│   ├── Src/                        # CubeMX 生成的初始化代码 + main/freertos
│   └── Startup/startup_stm32f407xx.s  # GCC 版启动文件
├── Drivers/                        # CMSIS + STM32F4 HAL
├── Middlewares/Third_Party/FreeRTOS
│   └── Source/portable/GCC/ARM_CM4F/  # FreeRTOS GCC 移植层
├── application/                    # 业务任务
├── bsp/boards/                     # 板级驱动
├── components/                     # 算法、控制器、设备驱动、UI
├── STM32F407IGHX_FLASH.ld          # GCC 链接脚本
└── chassis_task.ioc                # CubeMX 外设配置
```

## 环境要求

- `arm-none-eabi-gcc` 10.x 或更新（已在 10.3.1 验证）
- CMake >= 3.22
- Ninja（推荐）
- 调试/下载：OpenOCD、STM32CubeProgrammer 或 J-Link 均可

## 编译指令

以下命令都在工程目录内执行（例如 `cd chassis_task_cmake`）。

### 1. Release（实际跑车 / 比赛用）

```bash
cmake --preset release
cmake --build --preset release
```

等价于：

```bash
cmake -S . -B build/release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake
cmake --build build/release
```

### 2. Debug（在线调试用）

```bash
cmake --preset debug
cmake --build --preset debug
```

等价于：

```bash
cmake -S . -B build/debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake
cmake --build build/debug
```

Debug 使用 `-Og -g3`，保留完整调试符号；Release 使用 `-O2 -DNDEBUG`。

### 3. 清理后重新编译

```bash
cmake --build --preset release --clean-first

# 或者直接删除构建目录
rm -rf build/release
cmake --preset release
cmake --build --preset release
```

### 4. 从仓库根目录一次编译两个工程

```bash
cmake --preset release -S chassis_task_cmake -B chassis_task_cmake/build/release
cmake --build chassis_task_cmake/build/release

cmake --preset release -S gimbal_task_cmake -B gimbal_task_cmake/build/release
cmake --build gimbal_task_cmake/build/release
```

### 5. 输出文件

Release 输出在 `build/release/`，Debug 输出在 `build/debug/`：

| 文件 | 用途 |
|---|---|
| `chassis_task.elf` | 带符号的 ELF，用于 OpenOCD / J-Link / GDB 调试和下载 |
| `chassis_task.hex` | Intel HEX，STM32CubeProgrammer / J-Flash 直接烧录 |
| `chassis_task.bin` | 原始二进制，烧录时需指定地址 `0x08000000` |
| `chassis_task.map` | 链接映射表，查看 ROM/RAM 占用和符号地址 |

实际占用：

- Release：ROM 73,952 B / 1 MB，RAM 52,600 B / 128 KB
- Debug：ROM 84,004 B / 1 MB，RAM 52,640 B / 128 KB

### 6. 烧录与调试

STM32CubeProgrammer 烧录 Release 固件：

```bash
# 使用 HEX
STM32_Programmer_CLI -c port=SWD -w build/release/chassis_task.hex -v -rst

# 或使用 BIN（必须指定起始地址）
STM32_Programmer_CLI -c port=SWD -w build/release/chassis_task.bin 0x08000000 -v -rst
```

OpenOCD 烧录 Debug ELF：

```bash
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
  -c "program build/debug/chassis_task.elf verify reset exit"
```

OpenOCD + GDB 在线调试：

```bash
# 终端 1：启动 OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg

# 终端 2：连接并下载
arm-none-eabi-gdb build/debug/chassis_task.elf \
  -ex "target remote :3333" \
  -ex "monitor reset halt" \
  -ex "load"
```

如果使用 J-Link，把 OpenOCD 配置里的 `interface/stlink.cfg` 换成 `interface/jlink.cfg`。

## 与 Keil 工程的差异

1. **启动/链接**：使用 ST 官方 GCC 启动文件和 `STM32F407IGHX_FLASH.ld`，栈 8 KB、堆 8 KB（与原 Keil scatter 文件一致）。
2. **FreeRTOS 移植层**：`portable/RVDS` 替换为 `portable/GCC/ARM_CM4F`（FreeRTOS V10.3.1 官方源码）。
3. **CMSIS-DSP**：不再链接 ARMCC 的 `arm_cortexM4lf_math.lib`。代码只用到 `arm_sin_f32` / `arm_cos_f32`，由 `Core/Src/cmsis_dsp_compat.c` 基于 newlib 实现。
4. **AHRS**：`components/algorithm/AHRS.lib` 仍是 ARM Compiler 生成的归档，GNU ld 可以直接链接其中的 AAPCS 目标文件；构建时会出现一条 `wchar_t` 宽度警告，可忽略。AHRS 不跨模块传递 `wchar_t`。
5. **GCC 兼容性修正**：
   - `__packed struct` 改为 GCC 可识别的 `struct __attribute__((packed))`；
   - `struct_typedef.h` / `AHRS_middleware.h` 改用标准 `<stdint.h>`；
   - 修正 `bsp_flash.c`、`chassis_task.c` 中 `static` 声明与定义不一致的问题；
   - 修正 `AHRS.h` / `INS_task.c` 中大小写不一致的 include（便于在 Linux 下构建）。
6. **`.ioc` 保留**：仍可用 CubeMX 查看/修改外设配置。若在 CubeMX 中重新生成，请把 Toolchain 选为 CMake，并注意它会重写 `Core/`、`Drivers/`、`Middlewares/` 和 `CMakeLists.txt`；重新生成后需要把本工程的自定义源文件列表、FreeRTOS GCC 移植层和 GCC 兼容修改重新合并。
