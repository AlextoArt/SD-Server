#ifndef DATA_H_
#define DATA_H_

#include <map>
#include <vector>
#include <string>
#include <mutex>
#include <cstring>

#include <iostream>

#include "TaskTable.h"

class Data
{
    private:
        std::mutex _locker;
        std::map<std::string, std::vector<TaskTable>/*, caseInsensitiveComparator*/> _table;
    public:
        Data() = default;
        ~Data() = default;

        std::vector<TaskTable> get( const std::string & user );
        void add( std::string && key, TaskTable && task );
        void clear();
};

#endif // DATA_H_
