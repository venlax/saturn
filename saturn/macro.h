#ifndef __SATURN_MACRO_H__
#define __SATURN_MACRO_H__

namespace saturn {
    #define LOCK(guard, mutex) std::guard<decltype(mutex)> lock(mutex)

    
    #define SATURN_LOG_LEVEL(logger, level) \
    if(logger->getLevel() <= level) \
        saturn::LogEventWrapper(level, logger, std::make_shared<LogEvent>(__FILE__, \
        __LINE__, 0, getThreadId(), getFiberId(), getCurrentTime())).getSS()

    #define SATURN_LOG_DEBUG(logger) SATURN_LOG_LEVEL(logger, LogLevel::DEBUG)
    #define SATURN_LOG_INFO(logger) SATURN_LOG_LEVEL(logger, LogLevel::INFO)
    #define SATURN_LOG_WARN(logger) SATURN_LOG_LEVEL(logger, LogLevel::WARN)
    #define SATURN_LOG_ERROR(logger) SATURN_LOG_LEVEL(logger, LogLevel::ERROR)
    #define SATURN_LOG_FATAL(logger) SATURN_LOG_LEVEL(logger, LogLevel::FATAL)

    #define LOGGER(name) LoggerManager::getInstance()->getLogger(name)
    

}

#endif // !__SATURN_MACRO_H__
