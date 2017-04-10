#ifndef _COMM_H_
#define _COMM_H_
#include <curl/curl.h>

#include <iostream>       // std::cout, std::endl
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

namespace comm{
	inline void send(const std::string &_host,const std::string &_port,const std::string &_resource,const boost::property_tree::ptree &_fresponse){
   	std::stringstream ss;
   	write_json(ss,_fresponse);
	std::string json_str = ss.str();
	std::string url = std::string(_host + std::string(":") + _port + _resource);

   	CURL *curl = nullptr;
	CURLcode res;
   	curl_global_init(CURL_GLOBAL_ALL);
   	curl = curl_easy_init();

   	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                                                 
   	size_t length = json_str.length();
   	char *buffer = (char*)malloc(length);
   	memcpy(buffer, json_str.c_str(), length);
   	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buffer);
   	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, length);

	while( (res = curl_easy_perform(curl)) != CURLE_OK){
      	std::cerr << "curl_easy_perform() failed" << std::endl;
		std::cout<<"com::send - Error \""<<curl_easy_strerror(res)<<"\" while performing call to \""<<url<<"\", waiting to try again.\n";
		std::this_thread::sleep_for (std::chrono::seconds(1));
	}
	
   	curl_easy_cleanup(curl);
   	curl_global_cleanup();
   
   	free(buffer);
	}
}
#endif
