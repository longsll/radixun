#ifndef RADIXUN_LOG_H__
#define RADIXUN_LOG_H__

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdarg.h>
#include <map>
#include "singleton.h"
#include "thread.h"
#include "util.h"
#include "config.h"


//使用流式方式将日志级别level的日志写入到logger
#define RADIXUN_LOG_LEVEL(logger, level) \
    if(logger->getLevel() <= level) \
        radixun::LogEventWrap(radixun::LogEvent::ptr(new radixun::LogEvent(logger, level, \
                        __FILE__, __LINE__, 0, radixun::GetThreadId(),\
                radixun::GetFiberId(), time(0), radixun::Thread::GetName()))).getSS()
#define RADIXUN_LOG_DEBUG(logger) RADIXUN_LOG_LEVEL(logger, radixun::LogLevel::DEBUG)
#define RADIXUN_LOG_INFO(logger) RADIXUN_LOG_LEVEL(logger, radixun::LogLevel::INFO)
#define RADIXUN_LOG_WARN(logger) RADIXUN_LOG_LEVEL(logger, radixun::LogLevel::WARN)
#define RADIXUN_LOG_ERROR(logger) RADIXUN_LOG_LEVEL(logger, radixun::LogLevel::ERROR)
#define RADIXUN_LOG_FATAL(logger) RADIXUN_LOG_LEVEL(logger, radixun::LogLevel::FATAL)

//使用格式化方式将日志级别level的日志写入到logger
#define RADIXUN_LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if(logger->getLevel() <= level) \
        radixun::LogEventWrap(radixun::LogEvent::ptr(new radixun::LogEvent(logger, level, \
                        __FILE__, __LINE__, 0, radixun::GetThreadId(),\
                radixun::GetFiberId(), time(0), radixun::Thread::GetName()))).getEvent()->format(fmt, __VA_ARGS__)
#define RADIXUN_LOG_FMT_DEBUG(logger, fmt, ...) RADIXUN_LOG_FMT_LEVEL(logger, radixun::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define RADIXUN_LOG_FMT_INFO(logger, fmt, ...) RADIXUN_LOG_FMT_LEVEL(logger, radixun::LogLevel::INFO, fmt, __VA_ARGS__)
#define RADIXUN_LOG_FMT_WARN(logger, fmt, ...) RADIXUN_LOG_FMT_LEVEL(logger, radixun::LogLevel::WARN, fmt, __VA_ARGS__)
#define RADIXUN_LOG_FMT_ERROR(logger, fmt, ...) RADIXUN_LOG_FMT_LEVEL(logger, radixun::LogLevel::ERROR, fmt, __VA_ARGS__)
#define RADIXUN_LOG_FMT_FATAL(logger, fmt, ...) RADIXUN_LOG_FMT_LEVEL(logger, radixun::LogLevel::FATAL, fmt, __VA_ARGS__)

//获取主日志器
#define RADIXUN_LOG_ROOT() radixun::LoggerMgr::GetInstance()->getRoot()
//获取name的日志器
#define RADIXUN_LOG_NAME(name) radixun::LoggerMgr::GetInstance()->getLogger(name)

namespace radixun{

class Logger;
class LoggerManager;

//日志级别
class LogLevel{
public:
    enum Level {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };

    static const char* ToString(LogLevel::Level);

    static LogLevel::Level FormString(const std::string& str);

};

//日志事件
class LogEvent {

public:
    typedef std::shared_ptr<LogEvent> ptr;

    //构造函数
    //elapse 程序启动依赖的耗时(毫秒)
    LogEvent(std::shared_ptr<Logger> logger , LogLevel::Level level
    ,const char* file , int32_t line , uint32_t elapse
    ,uint32_t thread_id , uint32_t fiber_id , uint64_t time
    ,const std::string& thread_name
    );

    const char* getFile() const {return m_file;}
    uint32_t getLine() const {return m_line;}
    uint32_t getElapse() const {return m_elapse;}
    uint32_t getThreadId() const {return m_threadId;}
    uint32_t getFiberId() const {return m_fiberId;}
    uint64_t getTime() const {return m_time;}
    const std::string& getThreadName()const {return m_threadName;}
    std::string getContent() const {return m_ss.str();}
    std::shared_ptr<Logger> getLogger() const {return m_logger;}
    LogLevel::Level getLevel() const {return m_level;}
    std::stringstream& getSS() {return m_ss;}

    //格式化写入日志内容
    void format(const char* fmt , ...);
    void format(const char* fmt , va_list al);
private:
    // 文件名
    const char* m_file = nullptr;
    // 行号
    int32_t m_line = 0;
    // 程序启动开始到现在的毫秒数
    uint32_t m_elapse = 0;
    // 线程ID
    uint32_t m_threadId = 0;
    // 协程ID
    uint32_t m_fiberId = 0;
    // 时间戳
    uint32_t m_time = 0;
    // 线程名称
    std::string m_threadName;
    // 日志内容流
    std::stringstream m_ss;
    // 日志器
    std::shared_ptr<Logger> m_logger;
    // 日志等级
    LogLevel::Level m_level;
};

//日志事件包装器
class LogEventWrap {

public:
    LogEventWrap(LogEvent:: ptr e);
    //析构函数写入日志
    ~LogEventWrap();
    //获取日志事件
    LogEvent::ptr getEvent() const {return m_event;}
    //获取日志内容流
    std::stringstream& getSS();

private:

    LogEvent::ptr m_event;
};


//日志格式器
class LogFormatter {

public:
    typedef std::shared_ptr<LogFormatter> ptr;
    /**
     *  构造函数
     *  %m 消息
     *  %p 日志级别
     *  %r 累计毫秒数
     *  %c 日志名称
     *  %t 线程id
     *  %n 换行
     *  %d 时间
     *  %f 文件名
     *  %l 行号
     *  %T 制表符
     *  %F 协程id
     *  %N 线程名称
     *
     *  默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
     */
    LogFormatter(const std::string& pattern);
    //返回格式化日志文本
    std::string format(std::shared_ptr<Logger> logger ,LogLevel::Level level , LogEvent::ptr event);

public:

    class FormatItem{
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem() {}
        //格式化日志到流
        virtual void format(std::ostream& os , std::shared_ptr<Logger> logger , LogLevel::Level level , LogEvent::ptr event) = 0;
    };

    void init();
    bool isError() const {return m_error;}
    const std::string getPattern() const {return m_pattern;}

private:
    //日志格式模板
    std::string m_pattern;
    //日志格式解析后格式
    std::vector<FormatItem::ptr> m_items;
    //是否有错误
    bool m_error = false;
};


//日志输出目标
class LogAppender{
friend class Logger;
public:
    typedef std::shared_ptr<LogAppender> ptr;
    typedef Spinlock MutexType;
    
    virtual ~LogAppender() {}

    //写入日志
    virtual void log(std::shared_ptr<Logger> logger , LogLevel::Level level , LogEvent::ptr  event) = 0;

    //将日志输出目标的配置转成YAML String
    virtual std::string toYamlString() = 0;

    void setFormatter(LogFormatter::ptr val);
    LogFormatter::ptr getFormatter();
    LogLevel::Level getLevel() const {return m_level;}
    void setLevel(LogLevel::Level val) {m_level = val;}

protected:
    LogLevel::Level m_level = LogLevel::DEBUG;
    bool m_hasFormatter = false;
    MutexType m_mutex;
    LogFormatter::ptr m_formatter;
};

//日志器
class Logger :public std::enable_shared_from_this<Logger> {
friend class LoggerManager;
public:
    typedef std::shared_ptr<Logger> ptr;
    typedef Spinlock MutexType;

    Logger(const std::string& name = "root");

    void log(LogLevel::Level level , LogEvent::ptr event);
    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    void clearAppenders();
    LogLevel::Level getLevel()const {return m_level;}
    void setLevel(LogLevel::Level val){m_level = val;}
    const std::string& getName() const {return m_name;}
    //设置日志格式器
    void setFormatter(LogFormatter::ptr val);
    //设置日志格式模板
    void setFormatter(const std::string& val);
    LogFormatter::ptr getFormatter();
    std::string toYamlString();

private:

    //日志名称
    std::string m_name;
    //日志级别
    LogLevel::Level m_level;
    // Mutex
    MutexType m_mutex;
    //日志输出地集合
    std::list<LogAppender::ptr> m_appenders;
    //日志格式器
    LogFormatter::ptr m_formatter;
    //主日志器
    Logger::ptr m_root;

};

class StdoutLogAppender :public LogAppender {
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(Logger::ptr logger , LogLevel::Level level , LogEvent::ptr event) override;
    std::string toYamlString() override;
};


class FileLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    void log(Logger::ptr logger , LogLevel::Level level , LogEvent::ptr event) override;
    std::string toYamlString() override;
    bool reopen();

private:
    //文件路径
    std::string m_filename;
    //文件流
    std::ofstream m_filestream;    
    //上次重新打开时间
    uint64_t m_lastTime = 0;
};

class LoggerManager {

public:
    typedef Spinlock MutexType;
    LoggerManager();
    Logger::ptr getLogger(const std::string& name);

    void init();
    Logger::ptr getRoot() const {return m_root;}
    std::string toYamlString();
private:
    //mutex
    MutexType m_mutex;
    //日志器容器
    std::map<std::string , Logger::ptr> m_loggers;
    //主日志器
    Logger::ptr m_root;

};

/// 日志器管理类单例模式
typedef radixun::Singleton<LoggerManager> LoggerMgr;

}

#endif