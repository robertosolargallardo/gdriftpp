#include <ctime>
#include <cstdlib>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <Const.h>
#include <Logger.h>
#include <Method.h>
#include <mutex>
#include <thread>

#include <mongocxx/instance.hpp>

#include <Scheduler.h>

random_device seed;
mt19937 rng(seed());
mongocxx::instance inst{};

shared_ptr<Scheduler> scheduler;

// Notar que el mutex SOLO es para las pruebas, el sistema real no puede ser bloqueante a este nivel
//std::mutex internal_mutex;

void run(boost::property_tree::ptree _frequest){
//	cout<<"scheduler::main::run - Inicio\n";
//	lock_guard<mutex> lock(internal_mutex);

 /*std::stringstream ss;
 write_json(ss,_frequest);
 cout << ss.str() << endl;*/

	scheduler->run(_frequest);
}

int main(int argc,char** argv){

	/*boost::property_tree::ptree fhosts;
	read_json(argv[1],fhosts);

	boost::property_tree::ptree frequest;
	read_json(argv[2],frequest);

	scheduler=make_shared<Scheduler>(fhosts);

	scheduler->run(frequest);

	return(0);*/
	if(argc < 2){
		cerr << "Error::Hosts File not Specified" << endl;
		exit(EXIT_FAILURE);
	}

	boost::property_tree::ptree fhosts;
	read_json(argv[1], fhosts);

	scheduler=make_shared<Scheduler>(fhosts);

	auto resource=make_shared<Resource>();
	resource->set_path(fhosts.get<string>("scheduler.resource"));
	resource->set_method_handler("OPTIONS", method::options);
	resource->set_method_handler("POST", method::post<run>);

	auto settings = make_shared<Settings>();
	settings->set_port(fhosts.get<int>("scheduler.port"));
	settings->set_default_header("Connection", "close");

	Service service;
	service.publish(resource);
	service.set_logger(make_shared<CustomLogger>());
	service.start(settings);

	return(EXIT_SUCCESS);
}
