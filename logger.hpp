#pragma once

// A small C++17 logger for the Linux projects in 116_117-master.
// Usage: LOG(LogLevel::INFO) << "connected: " << client_fd;

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <atomic>
#include <iomanip>
#include <iostream>3
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

#include <sys/types.h>
#include <unistd.h>

namespace PersonalLog {

enum class LogLevel { DEBUG = 0, INFO, WARNING, ERROR, FATAL };

inline const char *ToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        case LogLevel::FATAL:   return "FATAL";
    }
    return "UNKNOWN";
}

enum class Target { NONE, CONSOLE, FILE, BOTH };

class Logger {
public:
    static Logger &Instance() {
        static Logger logger;
        return logger;
    }

    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    // Call once at program startup. Before Configure(), the default is
    // console + INFO; Configure() defaults to writing to both destinations.
    void Configure(std::string app_name,
                   std::filesystem::path directory = "./log",
                   Target target = Target::BOTH,
                   LogLevel min_level = LogLevel::INFO) {
        std::lock_guard<std::mutex> lock(mutex_);
        app_name_ = std::move(app_name);
        directory_ = std::move(directory);
        target_ = target;
        min_level_.store(static_cast<int>(min_level), std::memory_order_relaxed);
    }

    void SetTarget(Target target) {
        std::lock_guard<std::mutex> lock(mutex_);
        target_ = target;
    }

    // Names kept for source compatibility with the archive's Logger.hpp.
    void UseConsoleStrategy() { SetTarget(Target::CONSOLE); }
    void UseFileStrategy() { SetTarget(Target::FILE); }

    void SetMinLevel(LogLevel level) {
        min_level_.store(static_cast<int>(level), std::memory_order_relaxed);
    }

    bool Enabled(LogLevel level) const {
        return static_cast<int>(level) >=
               min_level_.load(std::memory_order_relaxed);
    }

    class LogMessage {
    public:
        LogMessage(Logger &owner, LogLevel level, const char *file, int line)
            : owner_(owner), level_(level), file_(file), line_(line),
              enabled_(owner.Enabled(level)) {
            if (enabled_) {
                stream_ << '[' << FormatTime() << "] [" << ToString(level_)
                        << "] [pid=" << static_cast<long>(::getpid())
                        << "] [tid=" << std::this_thread::get_id() << "] ["
                        << ShortFileName(file_) << ':' << line_ << "] ";
            }
        }

        LogMessage(const LogMessage &) = delete;
        LogMessage &operator=(const LogMessage &) = delete;
        LogMessage(LogMessage &&) = delete;
        LogMessage &operator=(LogMessage &&) = delete;

        template <typename T>
        LogMessage &operator<<(const T &value) {
            if (enabled_) stream_ << value;
            return *this;
        }

        ~LogMessage() {
            if (enabled_) owner_.Write(stream_.str());
        }

    private:
        static std::string ShortFileName(const char *file) {
            const std::string path(file ? file : "");
            const auto pos = path.find_last_of("/\\");
            return pos == std::string::npos ? path : path.substr(pos + 1);
        }

        static std::string FormatTime() {
            const auto now = std::chrono::system_clock::now();
            const auto seconds = std::chrono::system_clock::to_time_t(now);
            const auto micros = std::chrono::duration_cast<std::chrono::microseconds>(
                                    now.time_since_epoch()) % 1000000;
            std::tm local{};
            ::localtime_r(&seconds, &local);
            std::ostringstream out;
            out << std::put_time(&local, "%Y-%m-%d %H:%M:%S")
                << '.' << std::setfill('0') << std::setw(6) << micros.count();
            return out.str();
        }

        Logger &owner_;
        LogLevel level_;
        const char *file_;
        int line_;
        bool enabled_;
        std::ostringstream stream_;
    };

    LogMessage operator()(LogLevel level, const char *file, int line) {
        return LogMessage(*this, level, file, line);
    }

private:
    Logger() = default;

    static std::string DateString() {
        const auto now = std::time(nullptr);
        std::tm local{};
        ::localtime_r(&now, &local);
        std::ostringstream out;
        out << std::put_time(&local, "%Y-%m-%d");
        return out.str();
    }

    void Write(const std::string &message) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (target_ == Target::NONE) return;

        if (target_ == Target::CONSOLE || target_ == Target::BOTH) {
            std::cerr << message << '\n';
        }

        if (target_ == Target::FILE || target_ == Target::BOTH) {
            std::error_code error;
            std::filesystem::create_directories(directory_, error);
            if (error) {
                std::cerr << "[logger] cannot create " << directory_ << ": "
                          << error.message() << '\n';
                return;
            }

            const auto file = directory_ / (app_name_ + "-" + DateString() + ".log");
            std::ofstream output(file, std::ios::app);
            if (!output) {
                std::cerr << "[logger] cannot open " << file << '\n';
                return;
            }
            output << message << '\n';
        }
    }

    std::mutex mutex_;
    std::string app_name_ = "personal";
    std::filesystem::path directory_ = "./log";
    Target target_ = Target::CONSOLE;
    std::atomic<int> min_level_{static_cast<int>(LogLevel::INFO)};
};

}  // namespace PersonalLog

// The archive uses `using namespace NS_LOG_MODULE;`. Keep that namespace as
// a thin compatibility layer so this header can replace its Logger.hpp.
namespace NS_LOG_MODULE {
using ::PersonalLog::LogLevel;
using ::PersonalLog::Logger;
using ::PersonalLog::Target;
inline Logger &logger = Logger::Instance();
}  // namespace NS_LOG_MODULE

// Compatible with the archive's existing call style.
using PersonalLog::LogLevel;
using PersonalLog::Logger;
#define LOG(level) ::PersonalLog::Logger::Instance()(level, __FILE__, __LINE__)

#define ENABLE_CONSOLE_LOG_STRATEGY() \
    ::PersonalLog::Logger::Instance().SetTarget(::PersonalLog::Target::CONSOLE)
#define ENABLE_FILE_LOG_STRATEGY() \
    ::PersonalLog::Logger::Instance().SetTarget(::PersonalLog::Target::FILE)
#define ENABLE_BOTH_LOG_STRATEGY() \
    ::PersonalLog::Logger::Instance().SetTarget(::PersonalLog::Target::BOTH)