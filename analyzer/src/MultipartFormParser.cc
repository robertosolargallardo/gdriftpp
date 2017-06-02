#include "../include/MultipartFormParser.h"
MultipartFormParser::MultipartFormParser(void) {

}
MultipartFormParser::MultipartFormParser(const MultipartFormParser &_mfp) {
	this->_fields=_mfp._fields;
}
MultipartFormParser::MultipartFormParser(const std::string &_body) {
    std::regex expr("------WebKitFormBoundary([a-zA-Z]|[0-9])+");
    this->_fields = std::vector<std::string>(
    	std::sregex_token_iterator(_body.begin(), _body.end(), expr, -1), 
    	std::sregex_token_iterator()
    );
    this->_fields.erase(std::remove_if(this->_fields.begin(),this->_fields.end(),[](const std::string &str)->bool {return(std::regex_search(str,std::regex("^\\s*$")));}));
    std::for_each(this->_fields.begin(),this->_fields.end(),[](std::string &str)->void {boost::trim(str);});
    std::for_each(this->_fields.begin(),this->_fields.end(),[](std::string &str)->void {str=std::regex_replace(str,std::regex("Content-Disposition:\\s+form-data;\\s+"),"");});
    this->_fields.pop_back();
}
MultipartFormParser::~MultipartFormParser(void) {
    this->_fields.clear();
}
std::string MultipartFormParser::get(const std::string &_key) {
    auto iter=std::find_if(this->_fields.begin(),this->_fields.end(),[&_key](const std::string &str)->bool {return(str.find("name=\""+_key+"\"")!=std::string::npos);});
	
	 if(iter==this->_fields.end()){
		std::cerr << "Error::Field \"" << _key << "\" not found" << std::endl; 
		exit(EXIT_FAILURE);
	 }
	 /*std::cout << "\tBEGIN::FIELDS" <<std::endl;
	 for(auto& f : this->_fields){
      if(f.find("File")==std::string::npos)
		   std::cout <<"\t"<< f << std::endl;
	 }
	 std::cout << "\tEND::FIELDS" <<std::endl;*/

    std::string value=std::regex_replace(*iter,std::regex("name=\""+_key+"\""),"");
    boost::trim(value);
    return(value);
}
void MultipartFormParser::remove(const std::string &_key) {
	std::vector<std::string>::iterator field;
	for(field=this->_fields.begin();field!=this->_fields.end();++field)
		if(field->find("name=\""+_key+"\"")!=std::string::npos)
			break;
	if(field!=this->_fields.end())
		this->_fields.erase(field);
	else{
		std::cerr << "Error::Field \"" << _key << "\" not found" << std::endl; 
		exit(EXIT_FAILURE);
	}
    //this->_fields.erase(std::remove_if(this->_fields.begin(),this->_fields.end(),[&_key](const std::string &str)->bool {return(std::regex_search(str,std::regex("name=\""+_key+"\"")));}));
}
bool MultipartFormParser::empty(void) {
    return(this->_fields.empty());
}
MultipartFormParser& MultipartFormParser::operator=(const MultipartFormParser &_mfp) {
	this->_fields=_mfp._fields;
	return(*this);
}
