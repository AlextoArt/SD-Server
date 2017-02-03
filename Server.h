#ifndef SERVER_H_
#define SERVER_H_

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <windows.h>
#include <sstream>

#include <map>
#include <queue>
#include <string>
#include <thread>
#include <mutex>

#include <iostream>

#include "Connection.h"

class Server
{
private:
    SOCKET _masterSocket;
    std::string _port;
    HANDLE _ioPort;
    HANDLE _stopEvent;

    int _totalWorkers;
    HANDLE * _workers;
    unsigned int * _workerIds;

    HANDLE _dataReaderThread;
    unsigned int _dataReaderId;

    int _totalConnections;
    Connection ** _connections;

    bool _started;

    Server( const Server& ) = delete;
    Server& operator=( const Server& ) = delete;
public:
    Server( const std::string & port, int maxConnections );
    ~Server();

    bool start();
    void stop();
    void loop();

    static unsigned int __stdcall dataReaderThread( void * pServer );
    static unsigned int __stdcall workerThread( void * pServer );
};

#endif // SERVER_H_
