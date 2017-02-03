#include <fstream>
#include <cstring>
#include <iostream>

#include "Locator.h"
#include "Config.h"

Config * Config::_instance = nullptr;

Config * Config::instance()
{
    if( !_instance )
        _instance = new Config( configPath );
    return _instance;
}

Config::Config( const char * path )
{
    std::fstream file;
    file.open( path, std::ios_base::in );
    if( file.is_open() )
    {
        std::string str;
        std::getline( file, str );
        while( !file.eof() && str != "#CONFIG_SECTION" )
        {
            std::getline( file, str );
        }
        std::getline( file, str );

        while( !file.eof() )
        {
            std::getline( file, str );
            auto param = getParamPair( str );
            if( param.first != "" && param.second != "" )
                _parameters[param.first] = param.second;
        }
    }
    else
    {
        Locator::log( LOG_ERROR, "Config file does not exist, check the existence of 'config.ini' in the program folder" );
    }
}

std::string Config::getParam( const std::string & name )
{
    auto it = _parameters.find( name );
    if( it != _parameters.end() )
        return _parameters[name];
    return std::string("");
}

std::pair<std::string, std::string> getParamPair( const std::string & str )
{
    std::pair<std::string, std::string> ret( "", "" );
    std::size_t pos = str.find(' = ');
    if( pos != -1 )
    {
        ret.first = str.substr ( 0, pos );
        if( pos + 3 < str.length() )
        {
            ret.second = str.substr( pos + 3 );
        }
    }
    return ret;
}
