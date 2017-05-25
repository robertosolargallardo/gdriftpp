#include "../include/FileParser.h"
FileParser::FileParser(void) {
    ;
}
FileParser::FileParser(const FileParser &_fp) {
    this->_content=_fp._content;
    this->_markertype=_fp._markertype;
    this->_filetype=_fp._filetype;
}
FileParser::FileParser(const std::string &_content,const FileType &_filetype,const MarkerType& _markertype) {
    this->_content=std::regex_replace(_content,std::regex("^(.+)\\s+"),"",std::regex_constants::format_first_only);
    this->_content=std::regex_replace(this->_content,std::regex("^(.+)\\s+"),"",std::regex_constants::format_first_only);
    boost::trim(this->_content);
    this->_markertype=_markertype;
    this->_filetype=_filetype;
}
FileParser::~FileParser(void) {
    this->_content.clear();
}
FileParser& FileParser::operator=(const FileParser &_fp) {
    this->_content=_fp._content;
    this->_markertype=_fp._markertype;
    this->_filetype=_fp._filetype;
    return(*this);
}
std::vector<Marker> FileParser::parse(void) {
    std::vector<Marker> markers;

    switch(this->_filetype) {
    case GENEPOP: {
        /*single locus and single population*/
        std::string line;
        std::istringstream iss(this->_content);

        /*preamble data*/
        std::getline(iss,line);
        std::getline(iss,line);
        std::getline(iss,line);

        while(std::getline(iss,line)) {
            std::vector<std::string> result;
            boost::split(result,line,boost::is_any_of(","));

            switch(this->_markertype) {
            case SEQUENCE: {
                boost::trim(result[1]);
					 markers.push_back(Marker(this->_markertype,result[1]));
                break;
            }
            case MICROSATELLITE: {
                /*not yet implemented*/
                break;
            }
            }
            result.clear();
        }
        break;
    };
    case AUTOMATIC: {
        break;
    };
    default: {
        std::cerr << "Error::Unknown file type: " << this->_filetype << std::endl;
        exit(EXIT_FAILURE);
    }
    }
    return(markers);
}
