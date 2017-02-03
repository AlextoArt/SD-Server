#ifndef SERVICE_H_
#define SERVICE_H_

#include <memory>
#include <string>

#include <windows.h>

#include "Log.h"
#include "DatabaseReader.h"
#include "Server.h"

class Service
{

private:
    static Service * instance;

    #define NAME_MAX_LENGTH 128
    char _name[NAME_MAX_LENGTH];
    SERVICE_STATUS _status;
    SERVICE_STATUS_HANDLE _statusHandle;

    Service(const char *name);

    void start();
    void stop();

    static void WINAPI serviceMain(DWORD argc, LPTSTR *argv);
    static void WINAPI serviceHandler(DWORD control);
    void setServiceStatus(DWORD currentState, DWORD exitCode = NO_ERROR, DWORD waitHint = 0);

public:
    ~Service();
    static std::unique_ptr<Service> create(const char *name);

    bool run();
    bool install();
    bool uninstall();
};

#endif // SERVICE_H_
