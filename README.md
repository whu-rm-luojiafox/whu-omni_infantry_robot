# whu-omni_infantry

珞珈狐战队全向轮步兵固件，保留两个独立的 CMake 工程：

- [chassis_task_cmake](chassis_task_cmake/README.md)：底盘。
- [gimbal_task_cmake](gimbal_task_cmake/README.md)：云台，包含 USB CDC。

两个工程采用相同分层：`src/` 放业务代码，`platform/stm32f407/` 放芯片初始化、启动/链接文件和 CubeMX 配置，`third_party/` 放依赖，`cmake/` 放构建配置。各工程单独携带源文件与依赖，构建互不依赖。

在对应工程目录执行：

```powershell
cmake --preset release
cmake --build --preset release
```

调试构建将 `release` 换成 `debug`。产物位于各自的 `build/release/` 或 `build/debug/`。
