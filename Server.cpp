#include "Server.h"

Server::Server( const std::string & port, int maxConnections )
{
    _started = false;
    _port = port;
    _totalWorkers = 0;
    _ioPort = 0;
    _stopEvent = 0;
    _masterSocket = INVALID_SOCKET;
    _dataReaderThread = 0;
    _dataReaderId = 0;
    _totalConnections = maxConnections;
    _totalWorkers = std::thread::hardware_concurrency();
    _workers = nullptr;
    _workerIds = nullptr;
    _connections = nullptr;
}

Server::~Server()
{
    stop();
}

bool Server::start()
{
    Locator::log( LOG_INFO, "Starting server on port : " + _port );
    bool success = true;
    if( !_started )
    {
        _stopEvent = WSACreateEvent();
        _ioPort = CreateIoCompletionPort( INVALID_HANDLE_VALUE, 0, 0, _totalWorkers );
        if( _ioPort == NULL )
        {
            success = false;
            Locator::log( LOG_ERROR, "IO completion port creating error", WSAGetLastError() );
        }
        else
        {
            WSADATA ws = {0};
            if( FAILED( WSAStartup( MAKEWORD(2, 2), &ws ) ) )
            {
                success = false;
                Locator::log( LOG_ERROR, "WSA Startup error", WSAGetLastError() );
            }
            else
            {
                _masterSocket = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED );
                if( _masterSocket == INVALID_SOCKET )
                {
                    success = false;
                    Locator::log( LOG_ERROR, "Cannot create listener socket", WSAGetLastError() );
                }
                else
                {
                    std::stringstream sstr;
                    sstr << _port;
                    int temp;
                    sstr >> temp;

                    sockaddr_in addr = {0};
                    addr.sin_family = AF_INET;
                    addr.sin_addr.s_addr = INADDR_ANY;
                    addr.sin_port = htons( temp );

                    if ( SOCKET_ERROR == bind( _masterSocket, (sockaddr *) &addr, sizeof(addr) ) )
                    {
                        success = false;
                        closesocket( _masterSocket );
                        Locator::log( LOG_ERROR, "Cannot bind listener socket", WSAGetLastError() );
                    }
                    else
                    {
                        if ( SOCKET_ERROR == listen( _masterSocket, SOMAXCONN ))
                        {
                            success = false;
                            closesocket( _masterSocket );
                            Locator::log( LOG_ERROR, "Listen failed with error", WSAGetLastError() );
                        }
                        else
                        {
                            Locator::log( LOG_INFO, "Server accepting clients", WSAGetLastError() );

                            _workers = new HANDLE[_totalWorkers];
                            memset(_workers, 0, _totalWorkers);
                            _workerIds = new unsigned int [_totalWorkers];
                            memset(_workerIds, 0, _totalWorkers);

                            for( int i = 0; i < _totalWorkers; ++i )
                            {
                                _workers[i] = (HANDLE)_beginthreadex( 0, 0, workerThread, (void *)this, 0, _workerIds + i );
                            }

                            _dataReaderThread = (HANDLE)_beginthreadex( 0, 0, dataReaderThread, (void *)this, 0, &_dataReaderId );

                            CreateIoCompletionPort( (HANDLE)_masterSocket, _ioPort, COMPLETION_KEY_IO, 0 );

                            _connections = new Connection*[_totalConnections];
                            for( int i = 0; i < _totalConnections; ++i )
                            {
                                _connections[i] =  new Connection( _masterSocket, _ioPort );
                            }
                        }
                    }
                }
            }
        }
        if(success)
            _started = true;
    }
    return success;
}

void Server::stop()
{
    if( _started )
    {
        WSASetEvent( _stopEvent );
        for( int i = 0; i < _totalWorkers; ++i )
            PostQueuedCompletionStatus( _ioPort, 0, COMPLETION_KEY_SHUTDOWN, 0 );

        WaitForSingleObject( _dataReaderThread, INFINITE );
        CloseHandle( _dataReaderThread );

        WaitForMultipleObjects( _totalWorkers, _workers, TRUE, INFINITE );

        for( size_t i = 0; i < _totalWorkers; i++ )
            CloseHandle( _workers[i] );

        CloseHandle( _stopEvent );
        shutdown( _masterSocket, SD_BOTH );
        closesocket( _masterSocket );
        CloseHandle( _ioPort );

        delete [] _workers;
        _workers = nullptr;
        delete [] _workerIds;
        _workerIds = nullptr;

        for( int i = 0; i < _totalConnections; ++i )
        {
             delete _connections[i];
        }
        delete [] _connections;
        _connections = nullptr;

        WSACleanup();
    }
    _started = false;
}

void Server::loop()
{
    WaitForSingleObject( _stopEvent, INFINITE );
}

unsigned int __stdcall Server::dataReaderThread( void * pServer )
{
    Server & server = *(reinterpret_cast<Server *>( pServer ));
    bool working = true;
    while( working )
    {
        if ( WAIT_TIMEOUT == WaitForSingleObject( server._stopEvent, 1000 ) )
        {
            Locator::updateData();
            for( int i = 0; i < server._totalConnections; ++i )
            {
                if( server._connections != nullptr )
                {
                    server._connections[i] -> notify();
                }
            }
        }
        else
            working = false;
        std::this_thread::sleep_for( std::chrono::seconds(5) );
    }
}

unsigned int __stdcall Server::workerThread( void * pServer )
{
    Server & server = *(reinterpret_cast<Server *>( pServer ));
    bool working = true;
    while( working )
    {
        BOOL status = 0;
        DWORD bytes = 0;
        DWORD key = COMPLETION_KEY_NONE;
        LPOVERLAPPED overlapped = 0;
        status = GetQueuedCompletionStatus( server._ioPort, &bytes, &key, &overlapped, INFINITE );
        Connection & connection = *(reinterpret_cast<Connection *>( overlapped ));

        if ( FALSE == status && bytes == 0 )
        {
            connection.reset();
            continue;
        }

        switch( key )
        {
            case COMPLETION_KEY_IO:
                connection.handle();
                break;
            case COMPLETION_KEY_UPDATE:
                connection.handle();
                break;
            case COMPLETION_KEY_SHUTDOWN:
                working = false;
                break;
        }
    }
}
