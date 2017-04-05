#include <memory>
#include <cstdlib>
#include <Method.h>
#include <Logger.h>
#include <Semaphore.h>

#include <Analyzer.h>

#include <mongocxx/instance.hpp>

using namespace std;

random_device seed;
mt19937 rng(seed());
mongocxx::instance inst{};

shared_ptr<Analyzer>  analyzer;
shared_ptr<util::Semaphore> semaphore;
std::mutex internal_mutex;

void run(boost::property_tree::ptree _frequest){
//	semaphore->lock();
	lock_guard<mutex> lock(internal_mutex);
	analyzer->run(_frequest);
//	semaphore->unlock();
}

int main(int argc,char** argv){
	if(argc < 2){
		cerr << "Error::Hosts File not Specified" << endl;
		exit(EXIT_FAILURE);
	}

	boost::property_tree::ptree fhosts;
	read_json(argv[1], fhosts);

//	semaphore = make_shared<util::Semaphore>(1);
	analyzer = make_shared<Analyzer>(fhosts);

	auto resource = make_shared<Resource>();
	resource->set_path(fhosts.get<string>("analyzer.resource"));
	resource->set_method_handler("OPTIONS", method::options);
	resource->set_method_handler("POST", method::post<run>);

	auto settings = make_shared<Settings>();
	settings->set_port(fhosts.get<int>("analyzer.port"));
	settings->set_default_header("Connection", "close");

	Service service;
	service.publish(resource);
	service.set_logger(make_shared<CustomLogger>());
	service.start(settings);

	return(EXIT_SUCCESS);
}
