#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <boost/property_tree/ptree.hpp>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <list>

#include <Simulator.h>
#include <Sample.h>
#include <Node.h>
#include <Comm.h>

class Controller : public Node{
private:
	uint32_t _id;
	// Total de errores encontrados
	static unsigned int total_errores;

public:
	Controller(const boost::property_tree::ptree&, const uint32_t&);
	boost::property_tree::ptree run(boost::property_tree::ptree&);
	friend boost::property_tree::ptree indices(map<string, Sample*>);
	~Controller(void);
	
	// Marcador para iniciar los threads procesadores
	// El programa debe iniciarlos con "Controller::thread_started = Controller::startThreads(n_threads, queue, mutex);" antes del main
	static bool thread_started;
	// Proceso de inicio de los threads
	static bool startThreads(unsigned int n_threads, list<shared_ptr<boost::property_tree::ptree>> *work_list, std::mutex *list_mutex);
	
class ThreadData{
	public:
		unsigned int id;
		list<shared_ptr<boost::property_tree::ptree>> *work_list;
		std::mutex *list_mutex;
		
		ThreadData(){
			id = 0;
			work_list = NULL;
			list_mutex = NULL;
		}
		~ThreadData(){}
};
	
};
#endif
