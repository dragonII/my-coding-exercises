#ifndef __LIB_LOG_WRAP_HEADER__
#define __LIB_LOG_WRAP_HEADER__

#include <log4cpp/Export.hh>
#include <log4cpp/Portability.hh>
#include <log4cpp/Category.hh>

extern log4cpp::Category* gCategory;
extern log4cpp::Category* gMemCategory;

#define FORMATLOG(msg, fmt, ...) \
    try {\
    FormatLog(msg, fmt, ##__VA_ARGS__); \
    } catch (boost::io::format_error& _error) { \
        msg = _error.what(); \
    }

#define LOGCOMMSIMP(category, priority, fmt, ...)\
    do \
    {\
        std::string message;\
        FORMATLOG(message, fmt, ##__VA_ARGS__);\
        category->log(priority, message); \
    } while (0)

#define LOGCOMMFILE(category, priority, fmt, ...) \
    do \
    {\
        std::string message;\
        FORMATLOG(message, fmt, ##__VA_ARGS__);\
        message.append(boost::str(boost::format(" @%s:%d") %__FILE__%__LINE__));\
        category->log(priority, message); \
    } while (0)
    
#ifdef LOG_SIMPLE
#define LOGCOMM LOGCOMMSIMP
#else
#define LOGCOMM LOGCOMMFILE
#endif /* LOG_SIMPLE */

#define LOG_DEBUG(...) \
    do \
    {\
        if (0 != gCategory && gCategory->isDebugEnabled())\
            LOGCOMM(gCategory, log4cpp::Priority::DEBUG, ##__VA_ARGS__); \
    } while (0)

#define LOG_INFO(...) \
    do \
    {\
        if (0 != gCategory && gCategory->isInfoEnabled())\
            LOGCOMM(gCategory, log4cpp::Priority::INFO, ##__VA_ARGS__); \
    } while (0)


#define LOG_WARN(...) \
    do \
    {\
        if(gCategory != 0 && gCategory->isWarnEnabled()) \
            LOGCOMM(gCategory, log4cpp::Priority::WARN, ##__VA_ARGS__); \
    } while(0)

#define LOG_FATAL(...) \
    do \
    { \
        if(gCategory != 0 && gCategory->isFatalEnabled()) \
            LOGCOMM(gCategory, log4cpp::Priority::FATAL, ##__VA_ARGS__); \
    } while(0)

#endif
