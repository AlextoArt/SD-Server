#ifndef DATA_READER_H_
#define DATA_READER_H_

#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#include <iostream>

#include <string>
#include <sstream>

#include "Locator.h"

bool checkRC( SQLRETURN rc );

class DataReader
{
private:
	SQLHENV _henv;
	SQLHDBC _hdbc;
	SQLHSTMT _hstmt;
	SQLRETURN _rc;
	std::string _conStr;
	std::string _query;
	std::string _srv;
	std::string _db;
	bool _connected;

public:
    DataReader( const char * srv, const char * db );
    ~DataReader();
    void start();
    void stop();

    bool isConnected() { return _connected; }
	bool updateData();
};

#endif // DATABASE_READER_H_
