#ifndef __SATURN_LOG_H__
#define __SATURN_LOG_H__

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace saturn {

    class LogEvent {
    public:
        using ptr = std::shared_ptr<LogEvent>;
        LogEvent() {};
        LogEvent( std::filesystem::path file,
            uint32_t line,
            uint32_t elapse,
            pid_t threadId,
            uint32_t fiberId,
            uint64_t time) :
            m_file(file), m_line(line), m_elapse(elapse), m_threadId(threadId),
            m_fiberId(fiberId), m_time(time) {}

        const std::filesystem::path& getFile() const {return this->m_file;};

        uint32_t getLine() const {return this->m_line;};

        uint32_t getElapse() const {return this->m_elapse;};

        pid_t getThreadId() const {return this->m_threadId;};

        uint32_t getFiberId() const {return this->m_fiberId;};

        uint64_t getTime() const {return this->m_time;};

        std::string getContent() const {return this->ss.str();};

        std::stringstream& getSS() {return this->ss;};
    private:
        std::filesystem::path m_file;
        uint32_t m_line = 0;
        uint32_t m_elapse = 0;
        pid_t m_threadId = 0;
        uint32_t m_fiberId = 0;
        uint64_t m_time = 0;
        std::stringstream ss;
    };


    enum class LogLevel {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL
    };


    constexpr std::array<std::string_view, 5> level_str = {
        "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
    };

    class LogFormatter {
    public:
        using ptr = std::shared_ptr<LogFormatter>;
        LogFormatter(std::string_view format) : m_format(format) {
            parseFormat();

        }
        std::string format(std::string_view logger_name, LogLevel level, LogEvent::ptr event);

    private:
        //std::unordered_map<char,  std::function<std::string(LogEvent::ptr)>> m_format_rules;
        class FormatPattern {
            protected:
                std::string m_pattern;
            public:
                using ptr = std::shared_ptr<FormatPattern>;
                FormatPattern(std::string_view pattern) : m_pattern(pattern) {}

                virtual std::string str(std::string_view logger_name, LogLevel level, LogEvent::ptr event) {
                    return m_pattern; 
                }
        };

        class LoggerFormatPattern : public FormatPattern {
            private:
            public:
                using ptr = std::shared_ptr<LoggerFormatPattern>;
                LoggerFormatPattern() : FormatPattern("") {}
                std::string str(std::string_view logger_name, LogLevel level, LogEvent::ptr event) override {return std::string{logger_name};};
        
        };
        class TimeFormatPattern : public FormatPattern {
            private:
                enum class TimeType {
                    ISO8601,
                    UNIX,
                    UNIX_MILLIS,
                    SELF_DEFINED
                };
                
                TimeType m_type;

            public:
                using ptr = std::shared_ptr<TimeFormatPattern>; 
                TimeFormatPattern(std::string_view pattern) : FormatPattern(pattern) {
                    if (pattern == "UNIX") {
                        m_type = TimeType::UNIX;
                    } else if (pattern == "UNIX_MILLIS") {
                        m_type = TimeType::UNIX_MILLIS;
                    } else if (pattern == "ISO8601") {
                        m_type = TimeType::ISO8601;
                    } else if (pattern == "") {
                        m_type = TimeType::UNIX;
                    }  else {
                        m_type = TimeType::SELF_DEFINED;
                    }

                }
                std::string str(std::string_view logger_name, LogLevel level, LogEvent::ptr event) override;
        };

        class LevelFormatPattern : public FormatPattern {
            private:
                // enum class LevelColor {

                // }    
            
            public:
                using ptr = std::shared_ptr<LevelFormatPattern>;
                LevelFormatPattern() : FormatPattern("") {}
                std::string str(std::string_view logger_name, LogLevel level, LogEvent::ptr event) override {return std::string{level_str[static_cast<size_t>(level)]};}

        };

        class MessageFormatPattern : public FormatPattern {
            private:
            public:
                using ptr = std::shared_ptr<MessageFormatPattern>;
                MessageFormatPattern() : FormatPattern("") {}
                std::string str(std::string_view logger_name, LogLevel level, LogEvent::ptr event) override {return std::string{event->getContent()};};
        };
        class FileFormatPattern : public FormatPattern {
            private:
            public:
                using ptr = std::shared_ptr<FileFormatPattern>;
                FileFormatPattern() : FormatPattern("") {}
                std::string str(std::string_view logger_name, LogLevel level, LogEvent::ptr event) override {return std::string{event->getFile()};};
            
        };

        class LineFormatPattern : public FormatPattern {
            private:
            public:
                using ptr = std::shared_ptr<LineFormatPattern>;
                LineFormatPattern() : FormatPattern("") {}
                std::string str(std::string_view logger_name, LogLevel level, LogEvent::ptr event) override {return std::to_string(event->getLine());};
        };
        class ThreadFormatPattern : public FormatPattern {
            private:
            public:
                using ptr = std::shared_ptr<ThreadFormatPattern>;
                ThreadFormatPattern() : FormatPattern("") {}
                std::string str(std::string_view logger_name, LogLevel level, LogEvent::ptr event) override {return std::to_string(event->getThreadId());};
        };
        class FiberFormatPattern : public FormatPattern {
            private:
            public:
                using ptr = std::shared_ptr<FiberFormatPattern>;
                FiberFormatPattern() : FormatPattern("") {}
                std::string str(std::string_view logger_name, LogLevel level, LogEvent::ptr event) override {return std::to_string(event->getFiberId());};
        };
        class ElapseFormatPattern : public FormatPattern {
            private:
            public:
                using ptr = std::shared_ptr<FiberFormatPattern>;
                ElapseFormatPattern() : FormatPattern("") {}
                std::string str(std::string_view logger_name, LogLevel level, LogEvent::ptr event) override {return std::to_string(event->getElapse());};
        };

        std::string m_format; // "%m xxx %t xxx"
        std::vector<FormatPattern::ptr> m_patterns;
        void parseFormat();
        
    };

    class LogAppender {
    public:
        using ptr = std::shared_ptr<LogAppender>;
        virtual ~LogAppender(){};

        virtual void log(std::string_view logger_name, LogLevel level, LogEvent::ptr event) = 0;

        void setFormatter(LogFormatter::ptr formatter) {this->m_formatter = formatter;};
        LogFormatter::ptr getFormatter() const {return this->m_formatter;}; 

    protected:
        LogLevel m_level;
        LogFormatter::ptr m_formatter;
    };

    class StdoutLogAppender : public LogAppender {
    public:
        using ptr = std::shared_ptr<StdoutLogAppender>;
        void log(std::string_view logger_name, LogLevel level, LogEvent::ptr event) override;
    private:
    };

    class FileLogAppender : public LogAppender {
    public:
        using ptr = std::shared_ptr<FileLogAppender>;
        FileLogAppender(const std::filesystem::path& filepath);
        void log(std::string_view logger_name, LogLevel level, LogEvent::ptr event) override;
        bool reopen();
    private:
        std::filesystem::path m_filepath;
        std::ofstream m_filestream;
    };


    class Logger {
    public:
        using ptr = std::shared_ptr<Logger>;

        Logger(std::string_view name = "root")  : m_name(name) {}

        void log(LogLevel level, LogEvent::ptr event);

        void debug(LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);

        void addAppender(LogAppender::ptr appender);
        void delAppender(LogAppender::ptr appender);

        LogLevel getLevel() const { return m_level; };
        void setLevel(LogLevel level) { this->m_level = level; };

    private:
        std::string m_name;
        LogLevel m_level;
        std::list<LogAppender::ptr> m_appenders;
    };

    class LoggerManager {
    public:
        static LoggerManager* getInstance() {
            static LoggerManager* instance = new LoggerManager();
            return instance; 
        }
        Logger::ptr getLogger(std::string name = "root") {
            return m_loggers.contains(name) ? m_loggers[name] : nullptr;
        }
    private:
        std::map<std::string, Logger::ptr> m_loggers;
        LoggerManager() {
            Logger::ptr root = std::make_shared<Logger>();
            root->addAppender(std::make_shared<StdoutLogAppender>());
            m_loggers["root"] = root;
        }
    };

    class LogEventWrapper {
    public:
        LogEventWrapper(LogLevel level, Logger::ptr logger, LogEvent::ptr event) : m_level(level), m_logger(logger), m_event(event) {}
        ~LogEventWrapper() {
            m_logger->log(m_level , m_event);
        }
        std::stringstream& getSS() {return m_event->getSS();};
    private:
        LogEvent::ptr m_event;
        Logger::ptr m_logger;
        LogLevel m_level;
    };

    #define SATURN_LOG_LEVEL(logger, level) \
    if(logger->getLevel() <= level) \
        saturn::LogEventWrapper(level, logger, std::make_shared<LogEvent>(__FILE__, \
        __LINE__, 0, get_thread_id(), get_fiber_id(), get_current_time())).getSS()

    #define SATURN_LOG_DEBUG(logger) SATURN_LOG_LEVEL(logger, LogLevel::DEBUG)
    #define SATURN_LOG_INFO(logger) SATURN_LOG_LEVEL(logger, LogLevel::INFO)
    #define SATURN_LOG_WARN(logger) SATURN_LOG_LEVEL(logger, LogLevel::WARN)
    #define SATURN_LOG_ERROR(logger) SATURN_LOG_LEVEL(logger, LogLevel::ERROR)
    #define SATURN_LOG_FATAL(logger) SATURN_LOG_LEVEL(logger, LogLevel::FATAL)

    #define LOGGER(name) LoggerManager::getInstance()->getLogger(name)
    
}

#endif // !__SATURN_LOG_H__