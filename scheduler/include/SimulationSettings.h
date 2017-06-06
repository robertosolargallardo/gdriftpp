#ifndef _MY_SETTINGS_H_
#define _MY_SETTINGS_H_

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

#include "Comm.h"
#include "Node.h"
#include "Mongo.h"
#include "Const.h"
#include "Semaphore.h"

extern mt19937 rng;

enum Distributions {UNIFORM=3830327609, NORMAL=1330140418, GAMMA=0};

using namespace std;
using boost::property_tree::ptree;

class SimulationSettings{
public:
	ptree _fsettings;
	uint32_t _run;
	uint32_t _batch;
	uint32_t _feedback;
	uint32_t _training_size;
	
	bool cancel;

	SimulationSettings();
	SimulationSettings(ptree &_fsettings);
	~SimulationSettings();
	
	template<class T> T generate(const ptree&, bool force_limits = false, double forced_min = 0.0, double forced_max = 100000000);
	ptree parse_individual(ptree);
	ptree parse_scenario(ptree _fscenario, unsigned int min_pop, unsigned int feedback, unsigned int max_feedback);
	void send(const uint32_t&,const ptree&);

};
	
	
	
	
	
	
	
	
	
	
	

#endif
