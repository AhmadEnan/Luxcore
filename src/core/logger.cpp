#include <lux/core/logger.h>

#include <chrono>
#include <cstring>
#include <ctime>
#include <iomanip>

using namespace Lux;

namespace {
    const char* LevelToString(logLevel level) {
        switch(level) {
            case logLevel::TRACE: return "TRACE";
            case logLevel::DEBUG: return "DEBUG";
            case logLevel::INFO:  return "INFO";
            case logLevel::WARN:  return "WARN";
            case logLevel::ERROR: return "ERROR";
            case logLevel::FATAL: return "FATAL";
            default: return "UNKNOWN";
        }
    }
}

Logger::Logger() {
    // Initialize();
}

Logger::~Logger() {
    Shutdown();
}

Logger& Logger::Get() {
    static Logger instance;
    return instance;
}

void Logger::Initialize() {
    // Create timestamped log file
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    char filename[64];
    std::strftime(filename, sizeof(filename), "lux_%Y%m%d_%H%M%S.log", std::localtime(&time));
    
    m_logFile = fopen(filename, "w");
    if (!m_logFile) {
        // Fallback to stdout if file open fails
        m_logFile = stdout;
    }
    
    m_running.store(true, std::memory_order_relaxed);
    m_worker = std::thread(&Logger::WorkerThread, this);
}

void Logger::Shutdown() {
    m_running.store(false, std::memory_order_relaxed);
    
    // Notify worker thread
    if (m_worker.joinable()) {
        m_worker.join();
    }
    
    if (m_logFile && m_logFile != stdout) {
        fclose(m_logFile);
    }
}

void Logger::WorkerThread() {
    constexpr u32 BATCH_SIZE = 128;
    std::array<LogEntry, BATCH_SIZE> batch;
    u32 batch_count = 0;
    
    while (m_running.load(std::memory_order_relaxed) || 
           m_consumer.load(std::memory_order_relaxed) != m_producer.load(std::memory_order_relaxed)) {
        
        // Process in batches to minimize I/O calls
        batch_count = 0;
        while (batch_count < BATCH_SIZE) {
            const u32 consumer_idx = m_consumer.load(std::memory_order_relaxed);
            if (consumer_idx == m_producer.load(std::memory_order_acquire)) 
                break;
            
            batch[batch_count++] = m_buffer[consumer_idx % BUFFER_SIZE];
            m_consumer.store(consumer_idx + 1, std::memory_order_release);
        }
        
        // Process batch
        for (u32 i = 0; i < batch_count; ++i) {
            WriteOutput(batch[i]);
        }
        
        // Flush periodically
        if (batch_count > 0) {
            fflush(m_logFile);
        }
        
        // Sleep when idle
        if (batch_count == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

void Logger::WriteOutput(const LogEntry& entry) {
    // Convert to wall clock time
    auto wall_time = std::chrono::system_clock::now() + 
                    (entry.timestamp - std::chrono::steady_clock::now());
    
    std::time_t time = std::chrono::system_clock::to_time_t(
        std::chrono::time_point_cast<std::chrono::system_clock::duration>(wall_time));
    char time_buf[20];
    std::strftime(time_buf, sizeof(time_buf), "%H:%M:%S", std::localtime(&time));
    
    // Format log line
    char formatted[512];
    const int len = snprintf(formatted, sizeof(formatted), "%s.%03lld [%s] - %s\n",
        time_buf,
        std::chrono::duration_cast<std::chrono::milliseconds>(
            wall_time.time_since_epoch()).count() % 1000,
        LevelToString(entry.level),
        entry.message);
    
    // Dual output
    if (len > 0) {
        fwrite(formatted, 1, len, stdout);
        fwrite(formatted, 1, len, m_logFile);
    }
}
