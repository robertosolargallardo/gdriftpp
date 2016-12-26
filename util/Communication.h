#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_
namespace comm{
	static void send(const std::string &_host,const std::string &_port,const std::string &_resource,const boost::property_tree::ptree &_fresponse){
   	std::stringstream ss;
   	write_json(ss,_fresponse);

   	CURL *curl=nullptr;
   	curl_global_init(CURL_GLOBAL_ALL);
   	curl=curl_easy_init();

   	curl_easy_setopt(curl,CURLOPT_URL,std::string(_host+std::string(":")+_port+std::string("/")+_resource).c_str());
                                                 
   	size_t length=ss.str().length();
   	char *buffer=(char*)malloc(length);
   	memcpy(buffer,ss.str().c_str(),length);
   	curl_easy_setopt(curl,CURLOPT_POSTFIELDS,buffer);
   	curl_easy_setopt(curl,CURLOPT_POSTFIELDSIZE,length);

   	if(curl_easy_perform(curl)!=CURLE_OK)
      	std::cerr << "curl_easy_perform() failed" << std::endl;

   	curl_easy_cleanup(curl);
   	curl_global_cleanup();
   
   	free(buffer);
	}
}
#endif
