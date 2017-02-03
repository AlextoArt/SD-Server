#include "Locator.h"

#include <iostream>

Data * Locator::_providerData = nullptr;
DataReader * Locator::_providerDB = nullptr;
Log * Locator::_providerLog = nullptr;
Config * Locator::_providerConfig = nullptr;

// DBReader provider methods
bool Locator::updateData()
{
    if( _providerDB != nullptr )
        return _providerDB -> updateData();
    return false;
}

// Data provider methods
void Locator::addData( std::string && user, TaskTable && task )
{
    if( _providerData != nullptr )
        _providerData -> add( std::move( user ), std::move( task ) );
}

void Locator::clearData()
{
    if( _providerData != nullptr )
        _providerData -> clear();
}

std::vector<TaskTable> Locator::getData( const std::string & user )
{
    if( _providerData != nullptr )
        return _providerData -> get( user );
    return std::vector<TaskTable>();
}

// Log provider methods
void Locator::log( logLevel lvl, const char * msg, int errCode )
{
    if( _providerLog != nullptr )
        _providerLog -> write( lvl, msg, errCode );
}

void Locator::log( logLevel lvl, const std::string & str, int errCode )
{
    if( _providerLog != nullptr )
        _providerLog -> write( lvl, str, errCode );
}

// Config provider methods
std::string Locator::getParam( const std::string & name )
{
    if( _providerConfig == nullptr )
            _providerConfig = Config::instance();
    return _providerConfig -> getParam( name );
}
