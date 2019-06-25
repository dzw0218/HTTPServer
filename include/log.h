#ifndef LOG_H_
#define LOG_H_

#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <sstream>
#include <regex>
#include <mutex>
#include <ctime>
#include <iomanip>

namespace Logger
{

enum Level {Console, File, All};

std::ostream &operator<< (std::ostream &stream, const tm *time)
{
	return stream << 1900 + time->tm_year << "."
			<< std::setfill('0') << std::setw(2) << time->tm_mon + 1 << "."
			<< std::setfill('0') << std::setw(2) << time->tm_mday << " "
			<< std::setfill('0') << std::setw(2) << time->tm_hour << ":"
			<< std::setfill('0') << std::setw(2) << time->tm_min << ":"
			<< std::setfill('0') << std::setw(2) << time->tm_sec;
}

class BasicLogger
{
public:
    void addLine(std::string message)
    {
        std::unique_lock<std::mutex> lck(m_mutex);
        output(getLocalTime(), message.c_str());
    }
	
    virtual void output(const tm *time, const char *message) = 0;

private:
    const tm* getLocalTime()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        localtime_r(&in_time_t, &_localTime);
        return &_localTime;
    }

    std::mutex m_mutex;
    tm _localTime;
};

//文件日志类
class FileLogger : public BasicLogger
{
public:
    FileLogger(std::string &logfile)
    {
        _file.open(logfile, std::fstream::out | std::fstream::app | std::fstream::ate);
        if(_file.fail())
        {
            std::cout << "Can't create log file." << std::endl;
            return;
        }
    }
    ~FileLogger()
    {
        _file.flush();
        _file.close();
    }

    void output(const tm *time, const char *message)
    {
        _file << '[' << time << ']' << '\t' << message << std::endl;
        _file.flush();
    }

private:
    std::ofstream _file;
};

//控制台日志类
class ConsoleLogger : public BasicLogger
{
public:
    ConsoleLogger()
    {}

    void output(const tm *time, const char *message)
    {
        std::cout << '[' << time << ']' << '\t' << message << std::endl;
        std::cout.flush();
    }
};

//文件日志代理类
class FLoggerProxy
{
public:
    static FileLogger *instance()
    {
        std::unique_lock<std::mutex> lck(std::mutex);
        if(nullptr == m_flogger)
		{
			std::string log_file = "./log.txt";
            m_flogger = new FileLogger(log_file);
		}
        return m_flogger;
    }

private:
    FLoggerProxy(){}
    ~FLoggerProxy(){}
    static FileLogger *m_flogger;
};

FileLogger* FLoggerProxy::m_flogger = nullptr;

/*控制台打印代理类 */
class CLoggerProxy
{
public:
    static ConsoleLogger *instance()
    {
        if(nullptr == m_clogger)
		{
			std::unique_lock<std::mutex> lck(std::mutex);
        	if(nullptr == m_clogger)
            	m_clogger = new ConsoleLogger();
		}
        return m_clogger;
    }

private:
    CLoggerProxy(){}
    ~CLoggerProxy(){}

	static ConsoleLogger *m_clogger;
};

ConsoleLogger* CLoggerProxy::m_clogger = nullptr;

void log(Level type, const char *message)
{
	if(type == Console)
		CLoggerProxy::instance()->addLine(message);
	else if(type == File)
		FLoggerProxy::instance()->addLine(message);
    else
    {
        CLoggerProxy::instance()->addLine(message);
        FLoggerProxy::instance()->addLine(message);
    }
}

} // namespace Logger

#endif // LOG_H_
