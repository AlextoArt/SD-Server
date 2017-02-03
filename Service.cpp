#include <iostream>

#include "Service.h"

Service * Service::instance = nullptr;

std::unique_ptr<Service> Service::create(const char *name)
{
    if(instance == nullptr)
    {
        instance = new Service(name);
        return std::unique_ptr<Service>(instance);
    }
    return std::unique_ptr<Service>(nullptr);
}

bool Service::run()
{
    SERVICE_TABLE_ENTRY serviceTable[] =
    {
        { instance->_name, serviceMain },
        { NULL, NULL }
    };
    return StartServiceCtrlDispatcher(serviceTable);
}

void WINAPI Service::serviceMain(DWORD argc, LPTSTR *argv)
{
    instance->_statusHandle = RegisterServiceCtrlHandler(instance->_name, serviceHandler);
    if(!instance->_statusHandle)
    {
        // On error
    }
    else
        instance->start();
}

void WINAPI Service::serviceHandler(DWORD control)
{
    switch (control)
    {
        case SERVICE_CONTROL_STOP:
            instance->stop();
            break;
        case SERVICE_CONTROL_SHUTDOWN:
            instance->stop();
            break;
    }
}

Service::Service(const char *name)
{
    strncpy(_name, name, NAME_MAX_LENGTH);
    _statusHandle = NULL;
    _status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    _status.dwCurrentState = SERVICE_START_PENDING;

    DWORD controlsAccepted = 0;
    controlsAccepted |= SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

    _status.dwControlsAccepted = controlsAccepted;
    _status.dwWin32ExitCode = NO_ERROR;
    _status.dwServiceSpecificExitCode = 0;
    _status.dwCheckPoint = 0;
    _status.dwWaitHint = 0;
}

Service::~Service()
{

}

void Service::start()
{
    setServiceStatus(SERVICE_START_PENDING);
    Server::instance().init("12345");
    Server::instance().start();
    setServiceStatus(SERVICE_RUNNING);
    //setServiceStatus(SERVICE_STOPPED);
}

void Service::stop()
{
    DWORD currentState = _status.dwCurrentState;
    try
    {
        setServiceStatus(SERVICE_STOP_PENDING);
        /**
            Stop service. Stop DBReader, Server etc
        */
        setServiceStatus(SERVICE_STOPPED);
    }
    catch (DWORD error)
    {
        Log::instance().write(LOG_ERROR, "Service failed to stop", error);
        setServiceStatus(currentState);
    }
    catch (...)
    {
        Log::instance().write(LOG_ERROR, "Service failed to stop");
        setServiceStatus(currentState);
    }
}

void Service::setServiceStatus(DWORD currentState, DWORD exitCode, DWORD waitHint)
{
    static DWORD checkPoint = 1;
    _status.dwCurrentState = currentState;
    _status.dwWin32ExitCode = exitCode;
    _status.dwWaitHint = waitHint;

    if( currentState == SERVICE_RUNNING || currentState == SERVICE_STOPPED )
    {
        _status.dwCheckPoint = 0;
    }
    else
    {
        checkPoint++;
    }
    ::SetServiceStatus(_statusHandle, &_status);
}


bool Service::install()
{
    std::cout << "Installing service" << std::endl;
    Log::instance().write(LOG_INFO, "Installing service");
    char path[MAX_PATH];
    SC_HANDLE serviceManager = NULL;
    SC_HANDLE serviceHandle = NULL;

    if (GetModuleFileName(NULL, path, MAX_PATH) == 0)
    {
        int error = GetLastError();
        switch(error)
        {
        case ERROR_INSUFFICIENT_BUFFER:
            std::cout << "GetModuleFileName failed. Path is too long" << std::endl;
            Log::instance().write(LOG_ERROR, "GetModuleFileName failed. Path is too long");
            break;
        default:
            std::cout << "GetModuleFileName failed. Undefined Error " << error << std::endl;
            Log::instance().write(LOG_ERROR, "GetModuleFileName failed. Undefined Error", error);
        }
    }
    else
    {
        serviceManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
        if (serviceManager == NULL)
        {
            int error = GetLastError();
            switch(error)
            {
            case ERROR_ACCESS_DENIED:
                std::cout << "Opening service manager failed. Access denied" << std::endl;
                Log::instance().write(LOG_ERROR, "Opening service manager failed. Access denied");
                break;
            default:
                std::cout << "Opening service manager failed. Undefined error " << error << std::endl;
                Log::instance().write(LOG_ERROR, "Opening service manager failed. Undefined error ", error);
                break;
            }
        }
        else
        {
            serviceHandle = CreateService (
                             serviceManager,    // SCManager database
                             _name,             // Name of service
                             _name,             // Name to display
                             SERVICE_QUERY_STATUS,
                             SERVICE_WIN32_OWN_PROCESS,
                             SERVICE_AUTO_START,
                             SERVICE_ERROR_NORMAL,
                             path,
                             NULL, NULL, NULL, NULL, NULL
                         );
            if (serviceHandle == NULL)
            {
                int error = GetLastError();
                switch(error)
                {
                    case ERROR_ACCESS_DENIED:
                        std::cout << "Service creation fail. Access denied" << std::endl;
                        Log::instance().write(LOG_ERROR, "Service creation fail. Access denied");
                        break;
                    case ERROR_CIRCULAR_DEPENDENCY:
                        std::cout << "Service creation fail. Circular dependency" << std::endl;
                        Log::instance().write(LOG_ERROR, "Service creation fail. Circular dependency");
                        break;
                    case ERROR_DUPLICATE_SERVICE_NAME:
                        std::cout << "Service creation fail. Service name already exist" << std::endl;
                        Log::instance().write(LOG_ERROR, "Service creation fail. Service name already exist");
                        break;
                    case ERROR_INVALID_HANDLE:
                        std::cout << "Service creation fail. Invalid handle" << std::endl;
                        Log::instance().write(LOG_ERROR, "Service creation fail. Invalid handle");
                        break;
                    case ERROR_INVALID_NAME:
                        std::cout << "Service creation fail. Invalid name" << std::endl;
                        Log::instance().write(LOG_ERROR, "Service creation fail. Invalid name");
                        break;
                    case ERROR_INVALID_PARAMETER:
                        std::cout << "Service creation fail. Invalid parameter" << std::endl;
                        Log::instance().write(LOG_ERROR, "Service creation fail. Invalid parameter");
                        break;
                    case ERROR_INVALID_SERVICE_ACCOUNT:
                        std::cout << "Service creation fail. Bad Account" << std::endl;
                        Log::instance().write(LOG_ERROR, "Service creation fail. Bad Account");
                        break;
                    case ERROR_SERVICE_EXISTS:
                        std::cout << "Service creation fail. Service already exist" << std::endl;
                        Log::instance().write(LOG_ERROR, "Service creation fail. Service already exist");
                        break;
                    default:
                        std::cout << "Service creation fail. Undefined error " << error << std::endl;
                        Log::instance().write(LOG_ERROR, "Service creation fail. Undefined error ", error);
                        break;
                }
            }
            else
            {
                Log::instance().write( LOG_INFO, "Service successfully installed");
                std::cout << _name << " was installed" << std::endl;
            }
        }
    }

    if (serviceManager)
    {
        CloseServiceHandle(serviceManager);
        serviceManager = NULL;
    }
    if (serviceHandle)
    {
        CloseServiceHandle(serviceHandle);
        serviceHandle = NULL;
    }
}

bool Service::uninstall()
{
    std::cout << "Uninstalling service" << std::endl;
    Log::instance().write( LOG_INFO, "Uninstalling service");
    SC_HANDLE serviceManager = NULL;
    SC_HANDLE serviceHandle = NULL;

    serviceManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (serviceManager == NULL)
    {
        int error = GetLastError ();
        switch(error)
        {
        case ERROR_ACCESS_DENIED:
            std::cout << "Opening service manager fail. Access denied" << std::endl;
            Log::instance().write(LOG_ERROR, "Opening service manager fail. Access denied");
            break;
        default:
            std::cout << "Opening service manager fail. Undefined error " << error << std::endl;
            Log::instance().write(LOG_ERROR, "Opening service manager fail. Undefined error ", error);
        }
    }
    else
    {
        serviceHandle = OpenService(serviceManager, _name, SERVICE_STOP | SERVICE_QUERY_STATUS | DELETE);
        if (serviceHandle == NULL)
        {
            int error = GetLastError ();
            switch(error)
            {
            case ERROR_ACCESS_DENIED:
                std::cout << "Open service fail. Access denied" << std::endl;
                Log::instance().write(LOG_ERROR, "Open service fail. Access denied");
                break;
            case ERROR_INVALID_HANDLE:
                std::cout << "Open service fail. Invalid handle" << std::endl;
                Log::instance().write(LOG_ERROR, "Open service fail. Invalid handle");
                break;
            case ERROR_INVALID_NAME:
                std::cout << "Open service fail. Invalid name" << std::endl;
                Log::instance().write(LOG_ERROR, "Open service fail. Invalid name");
                break;
            case ERROR_SERVICE_DOES_NOT_EXIST:
                std::cout << "Open service fail. Service doesn't exist" << std::endl;
                Log::instance().write(LOG_ERROR, "Open service fail. Service doesn't exist");
                break;
            default:
                std::cout << "Open service fail. Undefined error " << error << std::endl;
                Log::instance().write(LOG_ERROR, "Open service fail. Service doesn't exist ", error);
            }
        }
        else
        {
            if (ControlService(serviceHandle, SERVICE_CONTROL_STOP, &_status))
            {
                Log::instance().write(LOG_INFO, "Stopping service");
                std::cout << "Stopping " << _name;
                Sleep(1000);

                while (QueryServiceStatus(serviceHandle, &_status))
                {
                    if (_status.dwCurrentState == SERVICE_STOP_PENDING)
                    {
                        std::cout << ".";
                        Sleep(1000);
                    }
                    else break;
                }

                if (_status.dwCurrentState == SERVICE_STOPPED)
                {
                    Log::instance().write(LOG_INFO, "Service stopped");
                    std::cout << std::endl << _name << " is stopped" << std::endl;
                }
                else
                {
                    Log::instance().write(LOG_ERROR, "Service failed to stop");
                    std::cout << std::endl << _name << " failed to stop" << std::endl;
                }
            }

            if (!DeleteService(serviceHandle))
            {
                int error = GetLastError ();
                switch(error)
                {
                case ERROR_ACCESS_DENIED:
                    Log::instance().write(LOG_ERROR, "Delete service failed. Access denied");
                    std::cout << "Delete service failed. Access denied" << std::endl;
                    break;
                case ERROR_INVALID_HANDLE:
                    Log::instance().write(LOG_ERROR, "Delete service failed. Invalid handle");
                    std::cout << "Delete service failed. Invalid handle" << std::endl;
                    break;
                case ERROR_SERVICE_MARKED_FOR_DELETE:
                    Log::instance().write(LOG_ERROR, "Delete service failed. Service already marked for delete");
                    std::cout << "Delete service failed. Service already marked for delete" << std::endl;
                    break;
                default:
                    Log::instance().write(LOG_ERROR, "Delete service failed. Undefined error ", error);
                    std::cout << "Delete service failed. Undefined error " << error << std::endl;
                }
            }
            else
            {
                Log::instance().write(LOG_INFO, "Service was removed");
                std::cout << _name << " was removed" << std::endl;
            }
        }
    }

    if (serviceManager)
    {
        CloseServiceHandle(serviceManager);
        serviceManager = NULL;
    }
    if (serviceHandle)
    {
        CloseServiceHandle(serviceHandle);
        serviceHandle = NULL;
    }
}
