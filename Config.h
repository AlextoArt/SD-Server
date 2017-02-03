#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <map>

const char configPath[] = "config.ini";

class Config
{
private:
    std::map<std::string, std::string> _parameters;
    static Config * _instance;

    Config( const char * path );
public:
    static Config * instance();
    std::string getParam( const std::string & name );
};

std::pair<std::string, std::string> getParamPair( const std::string & str );

#endif // CONFIG_H_
