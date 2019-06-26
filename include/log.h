#ifndef LOG_H_
#define LOG_H_

#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <mutex>
#include <ctime>
#include <iomanip>

namespace Logger
{

enum Level {Console, File, All};

extern std::ostream &operator<< (std::ostream &stream, const tm *time);

class BasicLogger
{
public:
    void addLine(std::string message);
    virtual void output(const tm *time, const char *message) = 0;

private:
    const tm* getLocalTime();

    std::mutex m_mutex;
    tm _localTime;
};

//文件日志类
class FileLogger : public BasicLogger
{
public:
    FileLogger(std::string &logfile);
    ~FileLogger();

    void output(const tm *time, const char *message);

private:
    std::ofstream _file;
};

//控制台日志类
class ConsoleLogger : public BasicLogger
{
public:
    ConsoleLogger();

    void output(const tm *time, const char *message);
};

//文件日志代理类
class FLoggerProxy
{
public:
    static FileLogger *instance();

private:
    FLoggerProxy();
    ~FLoggerProxy();
    static FileLogger *m_flogger;
};

/*控制台打印代理类 */
class CLoggerProxy
{
public:
    static ConsoleLogger *instance();

private:
    CLoggerProxy();
    ~CLoggerProxy();

	static ConsoleLogger *m_clogger;
};

extern void log(Level type, const char *message);

} // namespace Logger

#endif // LOG_H_
