#include "log.h"
#include "util.h"
#include <memory>

int main() {
    // using namespace saturn;

    // Logger::ptr logger = std::make_shared<Logger>("test");

    // auto appender = std::make_shared<StdoutLogAppender>(); 
    
    // auto formatter = std::make_shared<LogFormatter>("[%p]: %logger: %d{%Y%m%d%H%M%S} %F [%thread] [%f]%line %msg %%  %ems");
    
    // appender->setFormatter(formatter);
    
    // logger->addAppender(appender);

    // auto appender2 = std::make_shared<FileLogAppender>("log.txt");

    // appender2->setFormatter(formatter);

    // logger->addAppender(appender2);
    
    // logger->setLevel(saturn::LogLevel::INFO);
    
    // SATURN_LOG_INFO(logger) << "okk";

    return 0;
}