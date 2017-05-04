#include <restbed>
#include <boost/lexical_cast.hpp>

#include <Semaphore.h>
#include <Method.h>
#include <Logger.h>
#include <Controller.h>

#include <mutex>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <list>

using namespace std;
using namespace restbed;
using namespace boost::property_tree;

random_device seed;
mt19937 rng(seed());

shared_ptr<util::Semaphore>  semaphore;
shared_ptr<Controller> controller;
static unsigned int id_controller = 0;

// Variables para nuevo modelo de threads
list<shared_ptr<boost::property_tree::ptree>> work_list;
std::mutex list_mutex;
unsigned MAX_THREADS = std::thread::hardware_concurrency();
unsigned n_threads = (MAX_THREADS>1)?(MAX_THREADS-1):1;
//unsigned n_threads = 4;

// Inicializacion de los threads (Lo dejo EN el main por ahora para agregar hosts)
//bool Controller::thread_started = Controller::startThreads(n_threads, &work_list, &list_mutex);

void run(boost::property_tree::ptree _frequest){
	
	lock_guard<mutex> lock(list_mutex);
	unsigned int id = id_controller++;
	cout<<"controller - Agregando trabajo "<<id<<"\n";
	work_list.push_back(make_shared<boost::property_tree::ptree>(_frequest));
	cout<<"controller - Fin "<<id<<"\n";
	
//	semaphore->lock();
//	unsigned int id = id_controller++;
//	cout<<"controller - Inicio ("<<id<<")\n";
//	controller->run(_frequest);
//	cout<<"controller - Fin ("<<id<<")\n";
//	semaphore->unlock();
}

int main(int argc, char **argv){
	if(argc < 3){
		cerr<<"Error::Hosts File and/or ControllerID not Specified"<<endl;
		exit(EXIT_FAILURE);
	}
	
	ptree fhosts;
	read_json(argv[1], fhosts);
	uint32_t id = atoi(argv[2]);
	
	// semaphore = make_shared<util::Semaphore>(MAX_THREADS);
	semaphore = make_shared<util::Semaphore>(1);
	
	controller = make_shared<Controller>(fhosts, id);
	Controller::startThreads(n_threads, &work_list, &list_mutex, fhosts);
	
	auto myself = std::find_if(
						fhosts.get_child("controller").begin(), 
						fhosts.get_child("controller").end(), 
						[&id](const pair<const basic_string<char>, 
										basic_ptree<basic_string<char>, basic_string<char>>> &p)->bool{return p.second.get<uint32_t>("id")==id;}
						);
	
	auto resource = make_shared<Resource>();
	resource->set_path(myself->second.get<string>("resource"));
	resource->set_method_handler("OPTIONS", method::options);
	resource->set_method_handler("POST", method::post<run>);
	
	auto settings = make_shared<Settings>();
	settings->set_port(myself->second.get<int>("port"));
	settings->set_default_header("Connection", "close");
	
	Service service;
	service.publish(resource);
	service.set_logger(make_shared<CustomLogger>());
	service.start(settings);
	
	return(EXIT_SUCCESS);
}




