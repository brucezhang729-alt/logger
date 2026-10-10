#include "personal_logger.hpp"

#include <stdexcept>

int main() {
    using namespace PersonalLog;

    Logger::Instance().Configure(
        "my-diary",             // 文件名前缀
        "./log",                // 日志目录
        Target::BOTH,            // 控制台和文件都输出
        LogLevel::DEBUG);        // 最低记录级别

    LOG(LogLevel::INFO) << "个人日志程序启动";
    LOG(LogLevel::DEBUG) << "今天学习了 C++ 网络编程";

    const int finished_tasks = 3;
    LOG(LogLevel::INFO) << "完成任务数: " << finished_tasks;
    LOG(LogLevel::WARNING) << "还有一个任务没有完成";

    try {
        throw std::runtime_error("示例异常");
    } catch (const std::exception &error) {
        LOG(LogLevel::ERROR) << "捕获异常: " << error.what();
    }

    return 0;
}