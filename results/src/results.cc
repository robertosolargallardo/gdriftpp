#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <memory>
#include <cstdlib>
#include <restbed>
#include <armadillo>
#include "Results.h"
#include "../../util/Logger.h"
#include "../../util/Method.h"
#include "../../util/Semaphore.h"

using namespace std;
using namespace restbed;

random_device seed;
mt19937 rng(seed());
shared_ptr<Results> results;

boost::property_tree::ptree run(boost::property_tree::ptree _frequest){
	return(results->run(_frequest));
}
int main(const int argc,const char** argv)
{
	if(argc<2){
   	cerr << "Error::Hosts File not Specified" << endl;
		exit(EXIT_FAILURE);
	}
	
	boost::property_tree::ptree fhosts;
	read_json(argv[1],fhosts);

	results=make_shared<Results>(fhosts);

   auto resource=make_shared<Resource>();
   resource->set_path(fhosts.get<string>("results.resource"));
   resource->set_method_handler("OPTIONS",method::options);
   resource->set_method_handler("GET",method::get<run>);

   auto settings=make_shared<Settings>();
   settings->set_port(fhosts.get<int>("results.port"));
   settings->set_default_header("Connection","close");

   Service service;
   service.publish(resource);
	service.set_logger(make_shared<CustomLogger>());
   service.start(settings);

   return(EXIT_SUCCESS);
}
