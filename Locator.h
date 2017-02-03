#ifndef LOCATOR_H_
#define LOCATOR_H_

#include <string>

#include "Data.h"
#include "DataReader.h"
#include "Log.h"
#include "Config.h"

class DataReader;

class Locator
{
private:
    static Data * _providerData;
    static DataReader * _providerDB;
    static Log * _providerLog;
    static Config * _providerConfig;

    Locator() = delete;
    ~Locator() = delete;
public:
    static void regProvider( Data * data )             { _providerData   = data; }
    static void regProvider( DataReader * dataReader ) { _providerDB     = dataReader; }
    static void regProvider( Log * log )               { _providerLog    = log; }

    // DBReader provider methods
    static bool updateData();

    // Data provider methods
    static void addData( std::string && user, TaskTable && task );
    static void clearData();
    static std::vector<TaskTable> getData( const std::string & user );

    // Log provider methods
    static void log( logLevel lvl, const char * msg, int errCode = 0 );
    static void log( logLevel lvl, const std::string & str, int errCode = 0 );

    // Config provider methods
    static std::string getParam( const std::string & name );
};

#endif // LOCATOR_H_

