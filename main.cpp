#include "Server.h"
#include "Data.h"
#include "DataReader.h"

int main(int argc, char *argv[])
{
    std::string server = Locator::getParam( "#DB_SERVER" );
    std::string port = Locator::getParam( "#SERVER_PORT" );
    Server srv( "12345", 12 );

    Data data;
    Log log("log.txt");
    DataReader dbReader( "sd-sql", "sd-base" );

    Locator::regProvider( &data );
    Locator::regProvider( &dbReader );
    Locator::regProvider( &log );

    srv.start();
    dbReader.start();
    srv.loop();
    return 0;
}
