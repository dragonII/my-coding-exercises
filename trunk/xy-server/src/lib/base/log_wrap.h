#ifndef __LOG_WRAPPER_H__
#define __LOG_WRAPPER_H__

#include <log4cpp/Export.hh>
#include <log4cpp/Portability.hh>
#include <log4cpp/Category.hh>

extern log4cpp::Category* gCategory;
extern log4cpp::Category* gMemCategory;     // log for mem check only

#define FORMATLOG(msg, fmt, ...) \
    try {   \
        FormatLog(mes, fmt, ##__VA_ARGS__); \
    } catch (boost::io::format_error& _error) { \
        message = _error.what();    \
    }

/*
 * while(0) will essure caller treat the macro as a common line,
 * to reduce exception
 */

#define LOGCOMMSIMP(category, priority, fmt, ...) \
    do  \
    {   \
        std::string message;    \
        FORMATLOG(message, fmt, ##__VA_ARGS__); \
        category->log(priority, message);   \
    } while(0)

// File name, line num for log
#define LOGCOMMFILE(category, priority, fmt, ...)   \
    do  \
    {   \
        std::string message;    \
        FORMATLOG(message, fmt, ##__VA_ARGS__); \
        message.append(boost::str(boost::format(" @%d:%d") %__FILE__%__LINE__));    \
    } while(0)

#ifdef  LOG_SIMPLE
#define LOGCOMM LOGCOMMSIMP
#else
#define LOGCOMM LOGCOMMFILE
#endif // LOG_SIMPLE

#define LOG_DEBUG(...)  \
    do  \
    {   \
        if(gCategory != 0 && gCategory->isDebugEnabled())   \
            LOGCOMM(gCategory, log4cpp::Priority::DEBUG, ##__VA_ARGS__);    \
    } while(0)

#define LOG_INFO(...)   \
    do  \
    {   \
        if(gCategory != 0 && gCategory->isInfoEnabled())    \
            LOGCOMM(gCategory, log4cpp::Priority::INFO, ##__VA_ARGS__); \
    } while(0)

#define LOG_WARN(...)   \
    do  \
    {   \
        if(gCategory != 0 && gCategory->isWarnEnabled())    \
            LOGCOMM(gCategory, log4cpp::Priority::WARN, ##__VA_ARGS__); \
    } while(0)

#define LOG_FATAL(...)  \
    do  \
    {   \
        if(gCategory != 0 && gCategory->isFatalEnabled())   \
            LOGCOMM(gCategory, log4cpp::Priority::FATAL, ##__VA_ARGS__);    \
    } while(0)

#define LOG_MEM_OPR(...)    \
    do  \
    {   \
        if(gMemCategory != 0 && gMemCategory->isDebugEnabled()) \
            LOGCOMMSIMP(gMemCategory, log4cpp::Priority::DEBUG, ##__VA_ARGS__); \
    } while(0)


#endif
