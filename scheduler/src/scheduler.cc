#include <ctime>
#include <cstdlib>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <restbed>
#include "../../conf.h"
#include <Util.h>
#include "../../util/MongoDB.h"
#include "custom_logger.hpp"

#include<string>

#include "ConfigGenerator.h"

using namespace std;
using namespace restbed;

random_device seed;
mt19937 rng(seed());

map<uint32_t, shared_ptr<ConfigGenerator>> configurators;
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
void run(boost::property_tree::ptree _fconfiguration){
   uint32_t id = _fconfiguration.get<uint32_t>("id");
   uint32_t type = util::hash(_fconfiguration.get<string>("type"));
   switch( type ){
          case SCHED_INIT:{
				shared_ptr<Util::MongoDB> mongo=make_shared<Util::MongoDB>("mongodb://localhost:27017");
			   mongo->write("gdrift","settings",_fconfiguration);

            configurators[id]=make_shared<ConfigGenerator>(_fconfiguration);
            cout << "SCHED_INIT id=" << id << endl;
            configurators[id]->processConfiguration(BATCH_LENGTH);
            configurators[id]->sendBatch( );
            break;
			 }
          case SCHED_CONTINUE:{
            cout << "SCHED_CONTINUE id=" << id << endl;
            configurators[id]->processConfiguration(BATCH_LENGTH);
            configurators[id]->sendBatch( );
            break;
          }
          case SCHED_FINALIZE:{
            cout << "SCHED_FINALIZE id=" << id << endl;
            configurators.erase(configurators.find(id));
            break;
          }
			 default:{
				 cout << "default::"<< _fconfiguration.get<string>("type") << endl;
				 break;
			 }
   }
}
void handler_post(const shared_ptr<Session> &session)
{
    const auto request=session->get_request();
    size_t content_length=0;
    content_length=request->get_header("Content-Length",content_length);

    session->fetch(content_length,[](const shared_ptr<Session> &session,const Bytes &body)
    {
		const auto headers=multimap<string,string>{
      	{"Connection","close"},
      	{"Content-Type","application/json"},
      	{"Access-Control-Allow-Methods","GET,PUT,POST,DELETE,OPTIONS"},
      	{"Access-Control-Allow-Origin","*"},
      	{"Access-Control-Allow-Headers","Content-Type, Authorization, Content-Length, X-Requested-With, Origin, Accept"}
   	};

      boost::property_tree::ptree fconfiguration;
      istringstream is(string(body.begin(),body.end()));
      session->close(OK,headers);
		
		read_json(is,fconfiguration);
		boost::thread t(&run,fconfiguration);
		t.detach();
    });
}

int main(int argc,char** argv){
   
   auto resource=make_shared<Resource>();
   resource->set_path("/scheduler");
   resource->set_method_handler("OPTIONS",handler_options);
   resource->set_method_handler("POST",handler_post);

   auto settings=make_shared<Settings>();
   settings->set_port(1987);

   settings->set_default_header("Connection","close");

   Service service;
   service.publish(resource);
	service.set_logger( make_shared< CustomLogger >( ) );

   service.start(settings);

   return(EXIT_SUCCESS);
}
