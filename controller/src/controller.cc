#include <restbed>

#include "../../util/Semaphore.h"
#include "../../util/Method.h"
#include "../../util/Logger.h"
#include "Controller.h"

using namespace std;
using namespace restbed;
using namespace boost::property_tree;

random_device seed;
mt19937 rng(seed());

shared_ptr<util::Semaphore>  semaphore;
shared_ptr<Controller> controller;

void run(boost::property_tree::ptree _frequest){
   semaphore->lock();
	controller->run(_frequest);
   semaphore->unlock();
}
int main(int argc,char** argv){
	if(argc<3){
   	cerr << "Error::Hosts File and/or ControllerID not Specified" << endl;
		exit(EXIT_FAILURE);
	}
	
	ptree fhosts;
	read_json(argv[1],fhosts);

	uint32_t id=atoi(argv[2]);
   unsigned MAX_THREADS=std::thread::hardware_concurrency();
   semaphore=make_shared<util::Semaphore>(MAX_THREADS);
	controller=make_shared<Controller>(fhosts,id);
	
	auto myself=std::find_if(fhosts.get_child("controller").begin(),fhosts.get_child("controller").end(),[&id](const pair<const basic_string<char>,basic_ptree<basic_string<char>,basic_string<char>>> &p)->bool{return p.second.get<uint32_t>("id")==id;});

   auto resource=make_shared<Resource>();
   resource->set_path(myself->second.get<string>("resource"));
   resource->set_method_handler("OPTIONS",method::options);
   resource->set_method_handler("POST",method::post<run>);

   auto settings=make_shared<Settings>();
   settings->set_port(myself->second.get<int>("port"));
   settings->set_default_header("Connection","close");

   Service service;
   service.publish(resource);
	service.set_logger(make_shared<CustomLogger>());
   service.start(settings);

   return(EXIT_SUCCESS);
}
