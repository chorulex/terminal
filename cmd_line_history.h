#ifndef CMD_LINE_HISTORY_H_
#define CMD_LINE_HISTORY_H_

#include <fstream>
#include <string>
#include <vector>

class HistoryFile
{
private:
    HistoryFile()
    {
        std::ifstream reader(_file_path);

        std::string cmd;
        while( std::getline(reader, cmd) ){
            _list.push_back(cmd);
        }
        reader.close();

        _file_stream.open(_file_path, std::ofstream::app);
    }

public:
    ~HistoryFile()
    {
        if( _file_stream ){
            _file_stream.clear();
            _file_stream.close();
        }

    }

public:
    static HistoryFile& Instance() 
    {
        static HistoryFile _instance;
        return _instance;
    }

    std::vector<std::string> Load()
    {
        return _list;
    }

    void Append(const char* cmd)
    {
        _list.push_back(cmd);
        _file_stream << cmd << std::endl;
    }

private: 
    std::vector<std::string> _list;

    std::ofstream _file_stream;
    std::string _file_path = ".history";
};

class CommandLineHistory
{
public:
    CommandLineHistory()
    {
        _list = HistoryFile::Instance().Load();
        _index = _list.size();
    }

    void Append(const char* cmd)
    {
        _list.push_back(cmd);
        _index = _list.size();

        HistoryFile::Instance().Append(cmd);
    }

    std::string Next()
    {
        if( _index >= _list.size() - 1 || _list.empty() )
            return "";

        return _list[++_index];
    }

    std::string Pre()
    {
        if( _index == 0 )
            return "";

        return _list[--_index];
    }

private:
    std::vector<std::string> _list;
    size_t _index = 0;

};

#endif
