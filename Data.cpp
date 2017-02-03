#include "Data.h"

std::vector<TaskTable> Data::get( const std::string & user )
{
    _locker.lock();
    std::vector<TaskTable> temp;
    auto it = _table.find( user );
    if( it != _table.end() )
    {
        temp = _table[user];
    }
    _locker.unlock();
    return std::move( temp );
}

void Data::add( std::string && user, TaskTable && task)
{
    _locker.lock();
    auto it = _table.find( user );
    if( it == _table.end() )
        _table[user] = std::vector<TaskTable>();
    _table[user].emplace_back( std::move( task ) );
    _locker.unlock();
}

void Data::clear()
{
    _locker.lock();
    for( auto & it : _table )
        it.second.clear();
    _locker.unlock();
}
