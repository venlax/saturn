#include "log.h"
#include <memory>

int main() {
    using namespace saturn;

    Logger logger("test");

    auto appender = std::make_shared<StdoutLogAppender>(); 
    
    auto formatter = std::make_shared<LogFormatter>("[%p]: %logger: %d{UNIX} %F [%thread] [%f]%line %msg %%  %ems");
    
    appender->setFormatter(formatter);
    
    logger.addAppender(appender);

    auto appender2 = std::make_shared<FileLogAppender>("log.txt");

    appender2->setFormatter(formatter);

    logger.addAppender(appender2);
    
    logger.setLevel(saturn::LogLevel::INFO);
    
    LogEvent event("/usr", 5, 6 , 7 , 8 , 9, "okk");
    
    logger.log(LogLevel::INFO, event);
    return 0;
}