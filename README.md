    Logger::Instance().Configure(
        "my-app",                    // 日志文件名前缀
        "./logs",                    // 日志目录
        PersonalLog::Target::BOTH,    // 输出到控制台和文件
        LogLevel::DEBUG);             // 最低日志级别

    LOG(LogLevel::INFO) << "程序启动";
    LOG(LogLevel::DEBUG) << "当前用户: " << username;
    LOG(LogLevel::WARNING) << "配置文件不存在，使用默认配置";
    LOG(LogLevel::ERROR) << "打开文件失败: " << filename;

    return 0;
}
```

## 配置接口

### `Configure`

```cpp
Logger::Instance().Configure(
    "my-app",
    "./logs",
    PersonalLog::Target::FILE,
    LogLevel::INFO);
```

参数依次表示：

| 参数 | 说明 |
|---|---|
| `app_name` | 日志文件名前缀 |
| `directory` | 日志目录 |
| `target` | 日志输出位置 |
| `min_level` | 最低记录级别 |

### 输出位置

```cpp
PersonalLog::Target::NONE       // 关闭输出
PersonalLog::Target::CONSOLE    // 只输出到控制台
PersonalLog::Target::FILE       // 只写入文件
PersonalLog::Target::BOTH       // 控制台和文件都输出
```

也可以使用兼容原项目的宏：

```cpp
ENABLE_CONSOLE_LOG_STRATEGY();
ENABLE_FILE_LOG_STRATEGY();
ENABLE_BOTH_LOG_STRATEGY();
```

### 最低日志级别

```cpp
Logger::Instance().SetMinLevel(LogLevel::WARNING);
```

设置为 `WARNING` 后，`DEBUG` 和 `INFO` 日志会被忽略，只记录：

```text
WARNING
ERROR
FATAL
```

## 日志格式

日志格式如下：

```text
[时间] [等级] [pid=进程ID] [tid=线程ID] [文件名:行号] 日志内容
```

示例：

```text
[2026-10-12 20:30:15.123456] [ERROR] [pid=1234] [tid=...] [main.cpp:28] 打开文件失败
```

## 接入 `116_117-master`

将 `personal_logger.hpp` 复制到原项目目录，并把原来的：

```cpp
#include "Logger.hpp"
```

替换为：

```cpp
#include "personal_logger.hpp"
```

原项目中的以下写法可以继续使用：

```cpp
using namespace NS_LOG_MODULE;

ENABLE_FILE_LOG_STRATEGY();
LOG(LogLevel::INFO) << "服务器启动";
```

## 设计说明

- `Logger::Instance()` 使用函数内静态对象实现单例
- `LogMessage` 使用 RAII，在临时对象析构时提交日志
- `std::ostringstream` 支持 `<<` 链式拼接任意可输出类型
- `std::mutex` 保护实际输出过程
- `std::atomic` 保存最低日志等级，便于多线程读取
- `std::filesystem` 负责创建目录和拼接文件路径

## 注意事项

1. 代码使用 `getpid()` 和 `localtime_r()`，当前实现面向 Linux。
2. 编译时需要添加 `-std=c++17` 和 `-pthread`。
3. 日志文件采用追加模式，程序重启不会覆盖已有日志。
4. 每天会根据日期生成新的日志文件。
5. `FATAL` 只表示严重级别，不会自动终止程序；如果需要终止程序，需要在记录日志后显式调用 `exit` 或抛出异常。

## 清理编译文件

```bash
make clean
```
