#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <curl/curl.h>
#include <memory>
#include <cstdlib>
#include <restbed>
#include <armadillo>
#include "custom_logger.hpp"

using namespace std;
using namespace restbed;

random_device seed;
mt19937 rng(seed());

void handler_options(const shared_ptr<Session> &session){
	const auto headers=multimap<string,string>{
		{"Connection","close"},
		{"Content-Type","*"},
		{"Access-Control-Allow-Method","*"},
		{"Access-Control-Allow-Origin","*"}
	};
	session->yield(OK,headers,[](const shared_ptr<Session> session){});
}
void handler(const shared_ptr<Session> &session)
{
	const auto headers=multimap<string,string>{
		{"Connection","close"},
		{"Content-Type","*"},
		{"Access-Control-Allow-Method","*"},
		{"Access-Control-Allow-Origin","*"}
	};
	session->yield(OK,headers,[](const shared_ptr<Session> session){});
	
    /*const auto request=session->get_request();

	 std::stringstream ss;
	 string id;
	 if(!request->has_query_parameter("id")){
		id="void";
	 }
	 else{
		id=request->get_query_parameter("id");
	 }*/

    session->close("fuck authority");
}
int main(const int argc,const char** argv)
{
   auto resource=make_shared<Resource>();
   resource->set_path("/test");
   resource->set_method_handler("OPTIONS",handler_options);
   resource->set_method_handler("POST",handler);

   auto settings=make_shared<Settings>();
   settings->set_port(1987);
   settings->set_default_header("Connection","close");

   Service service;
   service.publish(resource);
	service.set_logger( make_shared< CustomLogger >( ) );
   service.start(settings);

   return(EXIT_SUCCESS);
}
