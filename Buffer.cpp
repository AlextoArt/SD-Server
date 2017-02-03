#include "Buffer.h"

void Buffer::pushPackage( TaskTable task )
{
    std::stringstream sstr;
    sstr.clear();
    sstr << task.stat << "\n"
         << task.name << "\n"
         << task.path << "\n"
         << task.desc << "\n" << '\0';
    _locker.lock();
    _data.emplace( std::move( sstr.str() ) );
    _locker.unlock();
}

void Buffer::pushCmdPackage( Command command )
{
    _locker.lock();
    switch( command )
    {
        case CMD_START:
            _data.push( __cmd_start );
            break;
        case CMD_CONNECTED:
            _data.push( __cmd_connected );
            break;
        case CMD_WAIT:
            _data.push( __cmd_wait );
            break;
        case CMD_OVER:
            _data.push( __cmd_over );
            break;
    }
    _locker.unlock();
}

std::string Buffer::popPackage()
{
    std::string result;
    _locker.lock();
    if( !_data.empty() )
    {
        result = _data.front();
        _data.pop();
    }
    else
        result = __cmd_wait;
    _locker.unlock();
    return result;
}

void Buffer::clear()
{
    while( !_data.empty() )
        _data.pop();
}
