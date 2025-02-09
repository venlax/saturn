#ifndef __SATURN_MACRO_H__
#define __SATURN_MACRO_H__

#include <cassert>

namespace saturn {
    #define LOCK(guard, mutex) std::guard<decltype(mutex)> lock(mutex)

    
    #define SATURN_LOG_LEVEL(logger, level) \
    if(logger->getLevel() <= level) \
        saturn::LogEventWrapper(level, logger, std::make_shared<saturn::LogEvent>(__FILE__, \
        __LINE__, 0, saturn::getThreadId(), saturn::getFiberId(), saturn::getCurrentTime())).getSS()

    #define SATURN_LOG_DEBUG(logger) SATURN_LOG_LEVEL(logger, saturn::LogLevel::DEBUG)
    #define SATURN_LOG_INFO(logger) SATURN_LOG_LEVEL(logger, saturn::LogLevel::INFO)
    #define SATURN_LOG_WARN(logger) SATURN_LOG_LEVEL(logger, saturn::LogLevel::WARN)
    #define SATURN_LOG_ERROR(logger) SATURN_LOG_LEVEL(logger, saturn::LogLevel::ERROR)
    #define SATURN_LOG_FATAL(logger) SATURN_LOG_LEVEL(logger, saturn::LogLevel::FATAL)

    #define LOGGER(name) saturn::LoggerManager::getInstance()->getLogger(name)
    
    #define SATURN_ASSERT(expression) \
            if (!(expression)) { \
                SATURN_LOG_ERROR(LOGGER()) << "Assertion fail: "#expression << \
                " backtrace : " << saturn::backtraceStr(); \
                assert(0); \
            }
}

#endif // !__SATURN_MACRO_H__
