#ifndef LOG_H_
#define LOG_H_

#include <fstream>
#include <time.h>
#include <string>
#include <mutex>

const std::string currentDateTime();

enum logLevel
{
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
};

class Log
{
private:
    std::mutex _locker;
    std::fstream _file;
public:
    Log( const char * file );
    ~Log();

    void write(logLevel lvl, const char * msg, int errCode = 0);
    void write(logLevel lvl, const std::string & str, int errCode = 0);
};

#endif // LOG_H_
