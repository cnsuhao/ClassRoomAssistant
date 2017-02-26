#ifndef CONFIGFILE_H
#define CONFIGFILE_H

/********************************************************
 *
 *   Read Configure file in STL, if no section you can set it
 * empty as "" it can read INI file or linux conf file.
 *
 * Author: Yoohoo Niu     mail: wand_niu@live.com
 *
 *
 ********************************************************/

#include <string>
#include <iostream>
#include <map>

class ConfigFile
{
public:
    ConfigFile(const std::string& fileName);
    std::string getValue(const std::string &key,const std::string& section="");
    void addValue(const std::string &key,const std::string value,const std::string& section="");
    void removeValue(const std::string &key,const std::string& section="");
    void save();
    void addCommont(const std::string &key,const std::string &commont,const std::string& section="");
private:
    std::map<std::string,std::string> m_content;
    std::map<std::string,std::string> m_comment;
    std::string FilePathName;
    std::string trim(const std::string &source,const char* delims=" \t\r\n");
    std::string makeLower(const std::string& str);

};
#endif // CONFIGFILE_H


