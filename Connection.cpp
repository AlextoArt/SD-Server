#include <sstream>
#include <algorithm>

#include "Connection.h"

Connection::Connection( SOCKET masterSocket, HANDLE ioPort )
{
    Internal = 0;
    InternalHigh = 0;
    Offset = 0;
    OffsetHigh = 0;
    hEvent = 0;

    _ioPort = ioPort;
    _wsabuf.buf = _data;
    _wsabuf.len = MAX_PACKAGE_SIZE;

    _state = WAIT_ACCEPT;
    _masterSocket = masterSocket;
    _slaveSocket = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED );
    CreateIoCompletionPort( reinterpret_cast<HANDLE>(_slaveSocket), _ioPort, COMPLETION_KEY_IO, 0 );
    open();
}

Connection::~Connection()
{
    shutdown( _slaveSocket, SD_BOTH );
    closesocket( _slaveSocket );
}

void Connection::handle()
{
    switch( _state )
    {
        case WAIT_ACCEPT:
            onAccept();
            break;
        case WAIT_AUTH:
            onAuth();
            break;
        case WAIT_UPDATE:
            onUpdate();
            break;
        case WAIT_SEND:
            onSend();
            break;
        case WAIT_RESET:
            onReset();
            break;
    }
}

void Connection::open()
{
    _state = WAIT_ACCEPT;
    DWORD receiveLen = 0;
    AcceptEx( _masterSocket, _slaveSocket, myAddrBlock, 0, (sizeof( struct sockaddr_in) + 16), (sizeof( struct sockaddr_in) + 16), &receiveLen, this );
}

void Connection::reset()
{
    Locator::log( LOG_INFO, std::string("Client disconnected ") + _user );
    _state = WAIT_RESET;
    TransmitFile( _slaveSocket, 0, 0, 0, this, 0, TF_DISCONNECT | TF_REUSE_SOCKET );
}

void Connection::onAccept()
{
    setsockopt( _slaveSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&_masterSocket, sizeof(SOCKET) );

    sockaddr_in addr = {0};
	int addrLen = sizeof( addr );
	getpeername( _slaveSocket,(SOCKADDR*)&addr, &addrLen );
	_address = inet_ntoa( addr.sin_addr );

    char hostName[NI_MAXHOST] = {0};
    char servInfo[NI_MAXSERV] = {0};
    getnameinfo( (struct sockaddr *) &addr, sizeof (struct sockaddr), hostName, NI_MAXHOST, servInfo, NI_MAXSERV, NI_NUMERICSERV );
    _host = hostName;

    _state = WAIT_AUTH;
    recvPackage();
}

void Connection::onAuth()
{
    finalizePackage( _data );
    if ( checkUsername( _data ) )
    {
        _user = _data;
        std::transform( _user.begin(), _user.end(), _user.begin(), ::tolower );
        Locator::log( LOG_INFO, std::string("Client connected ") + _user + " from host : " + _host + " " + _address );
        _state = WAIT_SEND;
        strcpy( _data, __cmd_connected.c_str() );
        sendPackage();
    }
    else
    {
        Locator::log( LOG_ERROR, std::string("Bad username received from ") + _host + " " + _address );
        strcpy( _data, __cmd_unauthorized.c_str() );
        sendPackage();
        reset();
    }
}

void Connection::onSend()
{
    std::string str( std::move( _buffer.popPackage() ) );
    if( str != __cmd_wait )
    {
        strcpy( _data, str.c_str() );
        sendPackage();
    }
    else
        _state = WAIT_UPDATE;
}

void Connection::onUpdate()
{
    std::vector<TaskTable> buffer = Locator::getData( _user );
    _buffer.clear();
    _buffer.pushCmdPackage( CMD_START );
    for( auto & it : buffer )
    {
        _buffer.pushPackage( it );
    }
    _buffer.pushCmdPackage( CMD_OVER );
    _state = WAIT_SEND;
    onSend();
}

void Connection::onReset()
{
    Internal = 0;
    InternalHigh = 0;
    Offset = 0;
    OffsetHigh = 0;
    hEvent = 0;

    _address.clear();
    _host.clear();
    _user.clear();
    _buffer.clear();
    open();
}

void Connection::notify()
{
    if( _state == WAIT_UPDATE )
    {
        _buffer.clear();
        PostQueuedCompletionStatus( _ioPort, 0, COMPLETION_KEY_UPDATE, (OVERLAPPED *)this );
    }
}

void Connection::sendPackage()
{
    _wsabuf.len = strlen( _data );

    DWORD ioSize = 0;
    ULONG flags = 0;
    UINT ret = WSASend( _slaveSocket, &_wsabuf, 1, &ioSize, flags, (OVERLAPPED*)this, NULL );
    if ( ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING )
        reset();
}

void Connection::recvPackage()
{
    ZeroMemory( _data, MAX_PACKAGE_SIZE );
    _wsabuf.len = MAX_PACKAGE_SIZE;

    DWORD ioSize = 0;
    ULONG flags = 0;
    UINT ret = WSARecv( _slaveSocket, &_wsabuf, 1, &ioSize, &flags, (OVERLAPPED*)this , NULL );
    if ( ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING )
        reset();
}

bool checkUsername( const std::string & str )
{
    int length = str.length();
    bool correct = true;

    if( length != 0 )
    {
        for( int i = 0; i < length; ++i )
        {
            switch( str[i] )
            {
                case '+': correct = false;
                case '=': correct = false;
                case ',': correct = false;
                case '.': correct = false;
                case '(': correct = false;
                case ')': correct = false;
            }
        }
    }
    else
        correct = false;
    return correct;
}

void finalizePackage( char * str )
{
    int i = 0;
    while( i < MAX_PACKAGE_SIZE && !( str[i] == '\0' || str[i] == '\n') )
        ++i;
    if( i < MAX_PACKAGE_SIZE )
        str[i] = '\0';
}
