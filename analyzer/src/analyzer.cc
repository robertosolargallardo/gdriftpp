#include <Simulator.h>
#include <memory>
#include <cstdlib>
#include <restbed>
#include <armadillo>

#include "Analyzer.h"
#include "custom_logger.hpp"

using namespace std;
using namespace restbed;

random_device seed;
mt19937 rng(seed());

shared_ptr<Analyzer> analyzer=make_shared<Analyzer>();

void handler_options(const shared_ptr<Session> &session)
{
   const auto headers=multimap<string,string>{
      {"Connection","close"},
      {"Content-Type","application/json"},
      {"Access-Control-Allow-Methods","GET,PUT,POST,DELETE,OPTIONS"},
      {"Access-Control-Allow-Origin","*"},
      {"Access-Control-Allow-Headers","Content-Type, Authorization, Content-Length, X-Requested-With, Origin, Accept"}
   };
   session->close(OK,headers);
}

void run(boost::property_tree::ptree &_fmessage){
	analyzer->analyze(_fmessage);
}

void handler_post(const shared_ptr<Session> &session)
{
    const auto request=session->get_request();
    size_t content_length=0;
    content_length=request->get_header("Content-Length",content_length);

    session->fetch(content_length,[](const shared_ptr<Session> &session,const Bytes &body){
   	const auto headers=multimap<string,string>{
      	{"Connection","close"},
      	{"Content-Type","application/json"},
      	{"Access-Control-Allow-Methods","GET,PUT,POST,DELETE,OPTIONS"},
      	{"Access-Control-Allow-Origin","*"},
      	{"Access-Control-Allow-Headers","Content-Type, Authorization, Content-Length, X-Requested-With, Origin, Accept"}
   	};

		boost::property_tree::ptree fmessage;
      istringstream is(string(body.begin(),body.end()));
      session->close(OK,headers);

      read_json(is,fmessage);
		boost::thread t(&run,fmessage);
      t.detach();
	 });
}

int main( const int, const char** )
{
    auto resource=make_shared<Resource>();
    resource->set_path("/analyzer");
    resource->set_method_handler("OPTIONS",handler_options);
    resource->set_method_handler("POST",handler_post);

    auto settings=make_shared<Settings>();
    settings->set_port(1986);
    settings->set_default_header("Connection","close");

    Service service;
    service.publish(resource);
	 service.set_logger( make_shared< CustomLogger >( ) );
    service.start(settings);

    return(EXIT_SUCCESS);
}
