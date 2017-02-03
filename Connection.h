#ifndef CONNECTION_H_
#define CONNECTION_H_

#define _WIN32_WINNT 0x501

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>

#include <string>
#include <iostream>
#include <vector>

#include "Locator.h"
#include "Buffer.h"

enum STATE
{
    WAIT_ACCEPT,
    WAIT_AUTH,
    WAIT_SEND,
    WAIT_UPDATE,
    WAIT_RESET
};

enum CompletionKey
{
    COMPLETION_KEY_NONE,
	COMPLETION_KEY_IO,
	COMPLETION_KEY_UPDATE,
	COMPLETION_KEY_SHUTDOWN
};

bool checkUsername( const std::string & str );
void finalizePackage( char * str );

class Connection : public OVERLAPPED
{
private:
    std::string _user;
    std::string _host;
    std::string _address;

    SOCKET _masterSocket;
    SOCKET _slaveSocket;

    BYTE myAddrBlock[ ((sizeof( struct sockaddr_in) + 16)) * 2 ];

    HANDLE _ioPort;
    WSABUF _wsabuf;
    char _data[ MAX_PACKAGE_SIZE ];

    STATE _state;
    Buffer _buffer;

    void sendPackage();
    void recvPackage();

    void onAccept();
    void onAuth();
    void onSend();
    void onUpdate();
    void onReset();

public:
    Connection( SOCKET masterSocket, HANDLE ioPort );
    ~Connection();

    void open();
    void reset();
    void handle();
    void notify();
};

#endif // CONNECTION_H_
