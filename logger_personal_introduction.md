# 个人日志代码

这份代码参考压缩包中 `code/lesson49/3.Logger/Logger.hpp` 到
`code/lesson63/Logger.hpp` 的设计，保留了原项目的链式调用方式：

```cpp
LOG(LogLevel::INFO) << "client fd: " << fd;
```

## 编译运行

在 Linux 环境进入本目录执行：

```bash
make
./example
```

运行后会在控制台输出日志，并在 `log/my-diary-YYYY-MM-DD.log` 中追加同样的内容。

## 接入自己的程序

把 `personal_logger.hpp` 复制到项目中，然后在 `main` 开始处配置：

```cpp
#include "personal_logger.hpp"

int main() {
    Logger::Instance().Configure(
        "my-app",             // 日志文件名前缀
        "./logs",             // 日志目录
        PersonalLog::Target::FILE,
        LogLevel::INFO);       // DEBUG/INFO/WARNING/ERROR/FATAL

    LOG(LogLevel::INFO) << "程序启动";
    LOG(LogLevel::ERROR) << "打开文件失败: " << path;
}
```

日志格式示例：

```text
[2026-10-09 23:59:01.123456] [INFO] [pid=1234] [tid=...] [main.cpp:12] 程序启动
```

`Target::CONSOLE` 只打印控制台，`Target::FILE` 只写文件，`Target::BOTH` 两者同时输出；`Target::NONE` 可以临时关闭输出。按日期切换文件，不同日期会生成不同的日志文件。