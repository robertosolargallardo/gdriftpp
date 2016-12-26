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
#include "ResultsManager.h"

using namespace std;
using namespace restbed;

random_device seed;
mt19937 rng(seed());

void handler(const shared_ptr<Session> &session)
{
	const auto headers=multimap<string,string>{
      	{"Connection","close"},
      	{"Content-Type","application/json"},
      	{"Access-Control-Allow-Methods","GET,PUT,POST,DELETE,OPTIONS"},
      	{"Access-Control-Allow-Origin","*"},
      	{"Access-Control-Allow-Headers","Content-Type, Authorization, Content-Length, X-Requested-With, Origin, Accept"}
   };
	session->yield(OK,headers,[](const shared_ptr<Session> session){});
	
    const auto request=session->get_request();

	 std::stringstream ss;
	 shared_ptr<ResultsManager> manager=make_shared<ResultsManager>("mongodb://localhost:27017");
	 if(!request->has_query_parameter("id")){
		boost::property_tree::ptree fresults=manager->get_results();
		write_json(ss,fresults);
	 }
	 else{
		uint32_t id=boost::lexical_cast<uint32_t>(request->get_query_parameter("id"));
		boost::property_tree::ptree fresults=manager->generate(id);
		write_json(ss,fresults);
	 }

    session->close(ss.str());
}
int main(const int argc,const char** argv)
{
   auto resource=make_shared<Resource>();
   resource->set_path("/results");
   resource->set_method_handler("GET",handler);

   auto settings=make_shared<Settings>();
   settings->set_port(1988);
   settings->set_default_header("Connection","close");

   Service service;
   service.publish(resource);
   service.start(settings);

   return(EXIT_SUCCESS);
}
