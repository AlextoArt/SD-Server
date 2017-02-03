#include "Log.h"
#include <iostream>
Log::Log( const char * file )
{
    _file.open( file , std::fstream::out );
}

Log::~Log()
{
    _file.close();
}

void Log::write(logLevel lvl, const char * msg, int errCode)
{
    _locker.lock();
    _file << currentDateTime() << " - ";
    switch(lvl)
    {
      case LOG_INFO:
        _file << "[INFO]";
        break;
      case LOG_WARN:
        _file << "[WARN]";
        break;
      case LOG_ERROR:
        _file << "[ERROR]";
        break;
    }
    _file << " - " << msg;

    if( errCode != 0 )
        _file << " - " << errCode;
    _file << std::endl;
    _locker.unlock();
}

void Log::write(logLevel lvl, const std::string & str, int errCode)
{
    write(lvl, str.c_str(), errCode);
}

const std::string currentDateTime()
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    return buf;
}
