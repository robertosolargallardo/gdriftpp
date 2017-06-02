#ifndef _MULTIPARTFORMPARSER_H_
#define _MULTIPARTFORMPARSER_H_
#include <vector>
#include <string>
#include <iostream>
#include <regex>
#include <boost/algorithm/string.hpp>

class MultipartFormParser {
private:
    std::vector<std::string> _fields;

public:
    MultipartFormParser(void);
    MultipartFormParser(const std::string&);
	 MultipartFormParser(const MultipartFormParser&);
    ~MultipartFormParser(void);

	 MultipartFormParser& operator=(const MultipartFormParser&);

    std::string get(const std::string&);
    void remove(const std::string&);
    bool empty(void);
};
#endif
