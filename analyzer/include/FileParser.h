#ifndef _FILEPARSER_H_
#define _FILEPARSER_H_
#include <Marker.h>
#include <iostream>
#include <vector>
#include <regex>
#include <boost/algorithm/string.hpp>

enum FileType {GENEPOP,AUTOMATIC};
class FileParser {
private:
    std::string _content;
    MarkerType  _markertype;
    FileType    _filetype;

public:
    FileParser(void);
    FileParser(const FileParser&);
    FileParser(const std::string&,const FileType&,const MarkerType&);
    ~FileParser(void);

    FileParser& operator=(const FileParser&);
    std::vector<Marker> parse(void);
};
#endif
