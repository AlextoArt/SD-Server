#ifndef Package_H_
#define Package_H_

#include <algorithm>
#include <sstream>
#include <queue>
#include <mutex>

#include <iostream> ///

#include <windows.h>

#include "Locator.h"
#include "TaskTable.h"

#define MAX_PACKAGE_SIZE 512

enum Command { CMD_START, CMD_CONNECTED, CMD_UNAUTHORIZED, CMD_WAIT, CMD_OVER };
const std::string __cmd_start("START");
const std::string __cmd_connected("CONNECTED");
const std::string __cmd_unauthorized("UNAUTHORIZED");
const std::string __cmd_wait("WAIT");
const std::string __cmd_over("OVER");

class Buffer
{
private:
    std::mutex _locker;
    std::queue<std::string> _data;
public:
    Buffer() = default;
    ~Buffer() = default;

    void pushPackage( TaskTable task );
    void pushCmdPackage( Command command );
    std::string popPackage();
    void clear();
};

#endif // Package_H_
