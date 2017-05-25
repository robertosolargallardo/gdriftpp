#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <iostream>
#include <chrono>
#include <random>
#include <ctime>
#include <cstdlib>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <map>
#include <vector>
#include <Simulator.h>

#include <Comm.h>
#include <Node.h>
#include <Mongo.h>
#include <Const.h>
#include <Semaphore.h>

#include "SimulationSettings.h"

using namespace std;

class Scheduler : public Node{
private:
	enum Types {INIT=305198855, CONTINUE=810372829, RELOAD=1571056488, FINALIZE=3761776383};
	
	shared_ptr<util::Mongo> _mongo;
	shared_ptr<util::Semaphore> _semaphore;
	map<uint32_t, shared_ptr<SimulationSettings>> _settings;

public:
	Scheduler(const boost::property_tree::ptree&);
	~Scheduler();

	boost::property_tree::ptree run(boost::property_tree::ptree&);

};


#endif
