//
// Created by test2 on 19-5-15.
//

#ifndef MUDUO_CONFIGFILEREADER_H
#define MUDUO_CONFIGFILEREADER_H


#include "Singleton.h"

#include <string>
#include <map>

class ConfigFileReader:public Singleton<ConfigFileReader>
{
public:
    //ConfigFileReader(const char* filename);
    ConfigFileReader();
    ~ConfigFileReader();
    void LoadFromFile(const char* filename);

    char* GetConfigName(const char* name);
    int SetConfigValue(const char* name, const char*  value);
private:
    void _LoadFile(const char* filename);
    int _WriteFIle(const char*filename = NULL);
    void _ParseLine(char* line);
    char* _TrimSpace(char* name);

    bool m_load_ok;
    std::map<std::string, std::string> m_config_map;
    std::string m_config_file;
};



#endif //MUDUO_CONFIGFILEREADER_H
