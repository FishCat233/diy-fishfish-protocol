# fishfish

一个用于学习系统编程的实验项目：从零实现一个基于 TCP 的自定义聊天协议——**fishfish 协议**。

## 是什么

- 基于 TCP 的客户端-服务端架构
- 使用 [Protocol Buffers](https://protobuf.dev/) 序列化消息
- C++20 + CMake + vcpkg 构建
- 目前仅支持 Windows，跨平台支持待补齐

## 项目结构

```
fishfish/
├── proto/              # protobuf 消息定义
│   └── messages.proto
├── src/
│   ├── server/         # 服务端（监听、连接管理、协议处理）
│   ├── client/         # 客户端
│   └── socket/         # 跨平台 socket 封装
├── tests/              # 测试
├── CMakelists.txt      # 顶层 CMake
├── vcpkg.json          # 依赖清单
└── .gitignore
```

> 当前 `src/server/` 下的文件还在根 `src/` 目录中，待迁移到 `src/server/`。

## 构建

```bash
# 安装 vcpkg（如果还没有）
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg && bootstrap-vcpkg.bat

# 构建项目
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

## 当前进度

- [x] Windows 下 TCP echo server（`main.cpp`，临时原型）
- [x] Socket 封装层：`ServiceGuard`、`InetAddress`、`Socket`
- [ ] `Listener` / `Receiver` 抽象
- [ ] 模块化 `Server` 类（替换 `main.cpp` 原型）
- [ ] fishfish 协议帧格式定义
- [ ] protobuf 消息编解码
- [ ] 客户端实现
- [ ] 跨平台（Linux）支持

## License

MIT
