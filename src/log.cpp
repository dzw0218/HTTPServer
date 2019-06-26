#include "log.h"

namespace Logger
{

std::ostream &operator<< (std::ostream &stream, const tm *time)
{
	return stream << 1900 + time->tm_year << "."
			<< std::setfill('0') << std::setw(2) << time->tm_mon + 1 << "."
			<< std::setfill('0') << std::setw(2) << time->tm_mday << " "
			<< std::setfill('0') << std::setw(2) << time->tm_hour << ":"
			<< std::setfill('0') << std::setw(2) << time->tm_min << ":"
			<< std::setfill('0') << std::setw(2) << time->tm_sec;
}

void BasicLogger::addLine(std::string message)
{
    std::unique_lock<std::mutex> lck(m_mutex);
    output(getLocalTime(), message.c_str());
}

const tm* BasicLogger::getLocalTime()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    localtime_r(&in_time_t, &_localTime);
    return &_localTime;
}

FileLogger::FileLogger(std::string &logfile)
{
    _file.open(logfile, std::fstream::out | std::fstream::app | std::fstream::ate);
    if(_file.fail())
    {
        std::cout << "Can't create log file." << std::endl;
        return;
    }
}

FileLogger::~FileLogger()
{
    _file.flush();
    _file.close();
}

void FileLogger::output(const tm *time, const char *message)
{
    _file << '[' << time << ']' << '\t' << message << std::endl;
    _file.flush();
}

ConsoleLogger::ConsoleLogger()
{

}

void ConsoleLogger::output(const tm *time, const char *message)
{
    std::cout << '[' << time << ']' << '\t' << message << std::endl;
    std::cout.flush();
}

FLoggerProxy::FLoggerProxy()
{

}

FLoggerProxy::~FLoggerProxy()
{

}

FileLogger *FLoggerProxy::instance()
{
    std::unique_lock<std::mutex> lck(std::mutex);
    if(nullptr == m_flogger)
	{
		std::string log_file = "./log.txt";
        m_flogger = new FileLogger(log_file);
	}
    return m_flogger;
}

FileLogger* FLoggerProxy::m_flogger = nullptr;

CLoggerProxy::CLoggerProxy()
{

}

CLoggerProxy::~CLoggerProxy()
{

}

ConsoleLogger *CLoggerProxy::instance()
{
    if(nullptr == m_clogger)
	{
		std::unique_lock<std::mutex> lck(std::mutex);
        if(nullptr == m_clogger)
            m_clogger = new ConsoleLogger();
	}
    return m_clogger;
}

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

} // Logger