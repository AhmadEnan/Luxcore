#pragma once
#include <atomic>
#include <array>
#include <thread>
#include <cstdio>

#include <lux/core/types.h>

namespace Lux
{

    enum class logLevel : u8
    {
        TRACE = 0,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        OFF
    };

    class Logger
    {
    public:
        static Logger &Get();

        void Initialize();
        void Shutdown();

        void SetLevel(logLevel level) { m_level.store(level, std::memory_order_relaxed); }
        logLevel GetLevel() const { return m_level.load(std::memory_order_relaxed); }

        template <typename... Args>
        void Log(logLevel level, const char *format, Args... args)
        {
            // Fast path: check level before any work
            if (level < m_level.load(std::memory_order_relaxed))
                return;

            // Reserve slot in ring buffer
            u32 idx = m_producer.fetch_add(1, std::memory_order_acq_rel) % BUFFER_SIZE;
            LogEntry &entry = m_buffer[idx];

            // Populate entry
            entry.timestamp = std::chrono::steady_clock::now();
            entry.level = level;

            // Format message directly into pre-allocated buffer
            const int written = snprintf(entry.message, sizeof(entry.message), format, args...);
            if (written < 0 || static_cast<size_t>(written) >= sizeof(entry.message))
            {
                // Handle truncation
                constexpr size_t trunc_pos = sizeof(entry.message) - 4;
                memcpy(entry.message + trunc_pos, "...", 4);
            }
        }

    private:
        Logger();
        ~Logger();

        struct LogEntry
        {
            std::chrono::steady_clock::time_point timestamp;
            logLevel level;
            char message[256];
        };

        // Lock-free MPSC queue
        static constexpr u32 BUFFER_SIZE = 65536; // 64k entries
        std::array<LogEntry, BUFFER_SIZE> m_buffer;
        alignas(64) std::atomic<u32> m_producer{0};
        alignas(64) std::atomic<u32> m_consumer{0};

        alignas(64) std::atomic<logLevel> m_level{logLevel::INFO};
        std::atomic<bool> m_running{false};
        std::thread m_worker;
        FILE *m_logFile = nullptr;

        void WorkerThread();
        void WriteOutput(const LogEntry &entry);
    };

    // Compile-time format string validation
    template <typename... Args>
    constexpr void ValidateFormat(const char *format, Args... args)
    {
        if constexpr (sizeof...(Args) > 0)
        {
            __attribute__((unused)) auto check = [&]
            {
                char buf[1];
                return snprintf(buf, 1, format, args...);
            };
        }
    }

#define LUX_LOG(level, format, ...)                                                   \
    do                                                                                \
    {                                                                                 \
        if (level >= Lux::Logger::Get().GetLevel())                                   \
        {                                                                             \
            Lux::ValidateFormat(format, ##__VA_ARGS__);                               \
            Lux::Logger::Get().Log(level, format, ##__VA_ARGS__); \
        }                                                                             \
    } while (0)

#define LUX_TRACE(format, ...) LUX_LOG(Lux::logLevel::TRACE, format, ##__VA_ARGS__)
#define LUX_DEBUG(format, ...) LUX_LOG(Lux::logLevel::DEBUG, format, ##__VA_ARGS__)
#define LUX_INFO(format, ...) LUX_LOG(Lux::logLevel::INFO, format, ##__VA_ARGS__)
#define LUX_WARN(format, ...) LUX_LOG(Lux::logLevel::WARN, format, ##__VA_ARGS__)
#define LUX_ERROR(format, ...) LUX_LOG(Lux::logLevel::ERROR, format, ##__VA_ARGS__)
#define LUX_FATAL(format, ...) LUX_LOG(Lux::logLevel::FATAL, format, ##__VA_ARGS__)
}