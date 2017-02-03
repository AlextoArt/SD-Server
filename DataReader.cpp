#include <algorithm>

#include "DataReader.h"

DataReader::DataReader( const char * srv, const char * db )
    : _henv( SQL_NULL_HENV ),
      _hdbc( SQL_NULL_HDBC ),
      _hstmt( SQL_NULL_HSTMT ),
      _srv( srv ),
	  _db( db ),
      _connected( false )
{
    _conStr = "DRIVER={SQL Server};SERVER=" + _srv + ";DATABASE=" + _db + ";Trusted_Connection=yes;";
}

DataReader::~DataReader()
{
    stop();
}

void DataReader::start()
{
	Locator::log( LOG_INFO, "Start database connector" );
	if( ( _srv != "" ) || ( _db != "" ) )
    {
        if( !_connected )
        {
            SQLAllocEnv( &_henv );
            SQLSetEnvAttr( _henv, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0 );
            SQLAllocConnect( _henv, &_hdbc );

            Locator::log( LOG_INFO, std::string("Connecting to sql-server : ") + _srv + ", database : " + _db );
            _rc = SQLDriverConnectA( _hdbc, NULL, (SQLCHAR *)_conStr.c_str(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE );
            if ( ( _rc == SQL_SUCCESS ) || ( _rc == SQL_SUCCESS_WITH_INFO ) )
            {
                Locator::log( LOG_INFO, "Connection successful" );
                _connected = true;
            }
            else
            {
                Locator::log( LOG_ERROR, "Connection fail" );
                SQLFreeConnect( _hdbc );
                _hdbc = SQL_NULL_HDBC;

                SQLFreeEnv( _henv );
                _henv = SQL_NULL_HENV;
                _connected = false;
            }
        }
    }
    else
    {
        Locator::log( LOG_ERROR, "Server or database name doesn't assigned, check config" );
    }
}

void DataReader::stop()
{
	Locator::log( LOG_INFO, "Stopping database connector" );
	if( _connected )
	{
		if ( _hstmt != SQL_NULL_HSTMT )
			SQLFreeHandle(SQL_HANDLE_STMT, _hstmt);

		if ( _hdbc != SQL_NULL_HDBC ) {
			SQLDisconnect( _hdbc );
			SQLFreeHandle( SQL_HANDLE_DBC, _hdbc );
		}

		if ( _henv != SQL_NULL_HENV)
			SQLFreeHandle(SQL_HANDLE_ENV, _henv );

        _hstmt = SQL_NULL_HSTMT;
        _hdbc = SQL_NULL_HDBC;
        _henv = SQL_NULL_HENV;
		_connected = false;
	}
}

bool DataReader::updateData()
{
	bool success = true;
	if( _connected )
	{
		_rc = SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &_hstmt);
		if ( (_rc != SQL_SUCCESS) && _rc != (SQL_SUCCESS_WITH_INFO) )
        {
            Locator::log(LOG_ERROR, "Allocation Error", _rc);
        }
        std::stringstream strs;
        strs << "SELECT U.Login, T.StatusId, T.Name, T.Path, T.Description FROM [Task] AS T "
             << "INNER JOIN [ExecutorGroupExecutor] AS EG ON T.ExecutorGroupId = EG.ExecutorGroupId "
             << "INNER JOIN [User] AS U ON EG.UserId = U.Id OR T.Executors LIKE CONCAT('%', U.Login, '%') "
             << "WHERE T.StatusId NOT IN (28, 29, 30) ORDER BY U.Login, T.Path;"  << '\0';
        _query = std::move(strs.str());
        _rc = SQLExecDirectA( _hstmt, (SQLCHAR *)_query.c_str(), SQL_NTS);
        if( checkRC( _rc ) )
        {
            char login[64];
            SQLLEN loginLen;
            TaskTable temp = {0};
            SQLLEN statLen;
            SQLLEN nameLen;
            SQLLEN pathLen;
            SQLLEN descLen;
            _rc = SQLBindCol(_hstmt, 1, SQL_C_CHAR,  &(login), RES_LOGIN_LENGTH, &(loginLen));
            if( checkRC( _rc ) )
            {
                _rc = SQLBindCol(_hstmt, 2, SQL_C_LONG,  &(temp.stat), RES_STID_LENGTH, &(statLen));
                if( checkRC( _rc ) )
                {
                    _rc = SQLBindCol(_hstmt, 3, SQL_C_CHAR, &(temp.name), RES_NAME_LENGTH, &(nameLen));
                    if( checkRC( _rc ) )
                    {
                        _rc = SQLBindCol(_hstmt, 4, SQL_C_CHAR, &(temp.path), RES_PATH_LENGTH, &(pathLen));
                        if( checkRC( _rc ) )
                        {
                            _rc = SQLBindCol(_hstmt, 5, SQL_C_CHAR, &(temp.desc), RES_DESC_LENGTH, &(descLen));
                            if( checkRC( _rc ) )
                            {
                                Locator::clearData();
                                while(SQL_SUCCEEDED(SQLFetch(_hstmt)))
                                {
                                    std::string str( login );
                                    std::transform( str.begin(), str.end(), str.begin(), ::tolower );
                                    Locator::addData( std::move( str ), std::move( temp ) );
                                }
                            }
                        }
                    }
                }
            }

            if( !checkRC( _rc ) )
            {
                success = false;
                Locator::log(LOG_ERROR, "Binding column error");
            }
        }
        else
        {
            success = false;
            Locator::log(LOG_ERROR, "Query execution error");
        }
	}
	else
        success = false;

	SQLFreeHandle(SQL_HANDLE_STMT, _hstmt);
	_hstmt = SQL_NULL_HSTMT;
	return success;
}

bool checkRC( SQLRETURN rc )
{
    if( ( rc == SQL_SUCCESS ) || ( rc == SQL_SUCCESS_WITH_INFO ) )
        return true;
    return false;
}
