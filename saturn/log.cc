#include "log.h"


#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>

#include "config.h"
#include "util.h"

namespace saturn {

    /*********************************************************/
    /**
      * @class LogEvent impl
     */
    /*********************************************************/


    /*********************************************************/
    /**
      * @class LogFormatter impl
     */
    /*********************************************************/


    void LogFormatter::parseFormat() {
        if (m_format.empty()) {
            error = true;
            return;
        } 
        // "%p %m %F %L %t %f %e %c %d{ISO8601}"
        std::string_view views {m_format};
        for (size_t i = 0; i < views.length(); ++i) {
            if (views[i] == '%') {
                if (i < views.length() - 1) {
                    std::string_view sv = views.substr(i + 1);
                    if (sv.starts_with("p") || 
                        sv.starts_with("level")) {
                        m_patterns.emplace_back(std::make_shared<LevelFormatPattern>());
                        i += sv.starts_with("p") ? 1 : 5;
                    } 
                    else if (sv.starts_with("m") || 
                        sv.starts_with("msg") ||
                        sv.starts_with("message")) {
                        m_patterns.emplace_back(std::make_shared<MessageFormatPattern>());        
                        i += sv.starts_with("message") ? 7 : (sv.starts_with("msg") ? 3 : 1 );
                    }
                    else if (sv.starts_with("F") ||
                        sv.starts_with("file")) {
                        m_patterns.emplace_back(std::make_shared<FileFormatPattern>());
                        i += sv.starts_with("F") ? 1 : 4;
                    }
                    else if (sv.starts_with("L") ||
                        sv.starts_with("line")) {
                        m_patterns.emplace_back(std::make_shared<LineFormatPattern>());
                        i += sv.starts_with("L") ? 1 : 4;
                    }
                    else if (sv.starts_with("t") ||
                        sv.starts_with("thread")) {
                        m_patterns.emplace_back(std::make_shared<ThreadFormatPattern>());
                        i += sv.starts_with("thread") ? 6 : 1;
                    }
                    else if (sv.starts_with("f") ||
                        sv.starts_with("fiber")) {
                        m_patterns.emplace_back(std::make_shared<FiberFormatPattern>());
                        i += sv.starts_with("fiber") ? 5 : 1;
                    }
                    else if (sv.starts_with("e") ||
                        sv.starts_with("elapse")) {
                        m_patterns.emplace_back(std::make_shared<ElapseFormatPattern>());
                        i += sv.starts_with("elapse") ? 6 : 1;
                    }
                    else if (sv.starts_with("c") ||
                        sv.starts_with("logger")) {
                        m_patterns.emplace_back(std::make_shared<LoggerFormatPattern>());
                        i += sv.starts_with("c") ? 1 : 6;
                    }
                    else if (sv.starts_with("d")) {
                        if (i + 2 < m_format.length()) {
                            if (views[i + 2] == '{') {
                                size_t tmp {i + 3};
                                while (tmp < m_format.length()) {
                                    if (views[tmp] == '}') {
                                        break;
                                    }
                                    tmp++;
                                }
                                if (tmp < m_format.length()) {
                                    m_patterns.emplace_back(std::make_shared<TimeFormatPattern>(views.substr(i + 3, tmp - i - 3)));
                                    i = tmp;
                                } else {
                                    m_patterns.emplace_back(std::make_shared<TimeFormatPattern>(""));
                                    i += 1;
                                }
                            } else {
                                m_patterns.emplace_back(std::make_shared<TimeFormatPattern>(""));
                                i += 1;
                            }
                        } else {
                            m_patterns.emplace_back(std::make_shared<TimeFormatPattern>(""));
                            i += 1;
                        }
                    }  else {
                        goto plain_str;
                    }
                } else {
                    m_patterns.emplace_back(std::make_shared<FormatPattern>(views.substr(i)));
                }
            } else {
plain_str:                
                size_t tmp = i + 1;
                while (views[tmp] != '%') {
                    tmp++;
                }
                m_patterns.emplace_back(std::make_shared<FormatPattern>(views.substr(i , tmp - i)));
                i = tmp - 1;
            }
        }
    }

    std::string LogFormatter::format(std::string_view logger_name, LogLevel level, LogEvent::ptr event) {
        std::stringstream ss;
        // ss << "TODO: LogFormatter::format not completed!" << std::endl;
        // std::cerr << "\033[35mWarning\033[0m: LogFormatter::format just implement the basic log format, not support self-define format string yet." << std::endl;
        
        // ss << timestampToString(event->getTime()) 
        // << " File: " << event->getFile().filename() 
        // << " Line" << event->getLine()
        // << " Thread" << event->getThreadId()
        // << " Fiber" << event->getFiberId()
        // << " Elapse: " << event->getElapse() << "ms"
        // << " Content:" << event->getContent()
        // << std::endl;
        for (auto& ptr : m_patterns) {
            ss << ptr->str(logger_name, level, event);
        }
        ss << std::endl;

        return ss.str();
    }

    std::string LogFormatter::TimeFormatPattern::str(std::string_view logger_name, LogLevel level, LogEvent::ptr event) {
        // TODO
        if (m_type == TimeType::UNIX) {
            return std::to_string(event->getTime());
        } else if (m_type == TimeType::UNIX_MILLIS) {
            std::chrono::seconds sec(event->getTime()); 
            std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(sec);
            return std::to_string(ms.count());
        } else if (m_type == TimeType::ISO8601) {
            return timestampToString(event->getTime());
        } else {
            return timestampToString(event->getTime(), this->m_pattern);
        }
        return "";
    }

    /*********************************************************/
    /**
      * @class LogAppender impl
     */
    /*********************************************************/
    // log 优先使用appender的formatter

    void StdoutLogAppender::log(std::string_view logger_name, LogLevel level, LogEvent::ptr event) {
        LOCK(lock_guard, m_mutex);
        if (level >= m_level) {
            if (m_formatter) std::cout << m_formatter->format(logger_name, level, event);
            else {
                Logger::ptr logger = LOGGER(logger_name.data());
                if (logger->getFormatter()) {
                    std::cout << logger->getFormatter()->format(logger_name, level, event);
                } else {
                    std::cout << "both logger's and appender's formatter invalid." << std::endl;
                    throw std::logic_error("formatter invalid");
                }
            }
        }
    }

    FileLogAppender::FileLogAppender(LogLevel level, LogFormatter::ptr formatter, const std::filesystem::path &filepath) :LogAppender(level, formatter),  m_filepath(filepath) {
        if (m_filestream) {
            m_filestream.open(m_filepath, std::ios::app);
        }
    }

    void FileLogAppender::log(std::string_view logger_name, LogLevel level, LogEvent::ptr event) {
        LOCK(lock_guard, m_mutex);
        if (level >= m_level) {
            if (m_formatter) m_filestream << m_formatter->format(logger_name, level, event);
            else {
                Logger::ptr logger = LOGGER(logger_name.data());
                if (logger->getFormatter()) {
                    m_filestream << logger->getFormatter()->format(logger_name, level, event);
                }  else {
                    std::cout << "both logger's and appender's formatter invalid." << std::endl;
                    throw std::logic_error("formatter invalid");
                }
            }
        }
    }

    bool FileLogAppender::reopen() {
        LOCK(lock_guard, m_mutex);
        if (m_filestream) {
            m_filestream.close();
        }
        m_filestream.open(m_filepath, std::ios::app);
        return !!m_filestream;
    }

    /*********************************************************/
    /**
      * @class Logger impl
     */
    /*********************************************************/
    

    void Logger::log(LogLevel level, LogEvent::ptr event) {
        LOCK(lock_guard, m_mutex);
        if (level >= m_level) {
            if (!m_appenders.empty()) {
                for (auto &appender : this->m_appenders) {
                    appender->log(this->m_name, level, event);
                }
            } else {
                LoggerManager::getInstance()->getLogger()->log(level, event);
            }
        }
    }

    void Logger::debug(LogEvent::ptr event) {
        log(LogLevel::DEBUG, event);
    }
    void Logger::info(LogEvent::ptr event) {
        log(LogLevel::INFO, event);
    }
    void Logger::warn(LogEvent::ptr event) {
        log(LogLevel::WARN, event);
    }
    void Logger::error(LogEvent::ptr event) {
        log(LogLevel::ERROR, event);
    }
    void Logger::fatal(LogEvent::ptr event) {
        log(LogLevel::FATAL, event);
    }

    void Logger::addAppender(LogAppender::ptr appender) {
        LOCK(lock_guard, m_mutex);
        m_appenders.push_back(appender);
    }
    void Logger::delAppender(LogAppender::ptr appender) {
        LOCK(lock_guard, m_mutex);
        for (auto iter = m_appenders.begin(); 
                iter != m_appenders.end(); 
                ++iter) {
            if (*iter == appender) {
                m_appenders.erase(iter);
                break;
            }
        }
    }

    ConfigVar<std::set<LogConfig>>::ptr glb_log_config = Config::add("log", std::set<LogConfig>(), "global log config");

    class LogIniter {
        public: 
            LogIniter() {
                glb_log_config->addListener([](const std::set<LogConfig>& old_value, const std::set<LogConfig>& new_value) {
                    for (auto& val : new_value) {
                        Logger::ptr logger;
                        logger = LOGGER(val.name);
                        logger->setLevel(val.level);
                        LogFormatter::ptr formatter = std::make_shared<LogFormatter>(val.formatter);
                        if (!formatter->isValid()) formatter = nullptr;   
                        logger->setFormatter(formatter);
                        logger->clearAppenders();
                        for (auto& ap : val.appenders) {
                            LogFormatter::ptr formatter = std::make_shared<LogFormatter>(ap.formatter);
                            if (!formatter->isValid()) formatter = nullptr;        
                            if (ap.type == 1) {
                                StdoutLogAppender::ptr stdout_ap = std::make_shared<StdoutLogAppender>(ap.level, formatter);
                                logger->addAppender(stdout_ap);
                            } else if (ap.type == 2) {
                                FileLogAppender::ptr file_ap = std::make_shared<FileLogAppender>(ap.level, formatter, ap.file);
                                logger->addAppender(file_ap);
                            }
                            
                        }
                    }
                    for (auto& val : old_value) {
                        if (!new_value.contains(val)) {
                            Logger::ptr logger = LOGGER(val.name);
                            logger->clearAppenders();
                            LoggerManager::getInstance()->delLogger(val.name); 
                        }
                    }
                    return;
                });
            }
    };

    static LogIniter log_initer;
}