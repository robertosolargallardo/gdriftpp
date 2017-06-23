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

enum Distributions {UNIFORM_HASH=3830327609, NORMAL_HASH=1330140418, GAMMA_HASH=0};

using namespace std;
using boost::property_tree::ptree;

class SimulationSettings{
public:
	ptree fsettings;
	uint32_t run;
	// Creo que la variable _batch NO esta en uso (feedback la reemplaza)
	uint32_t batch;
	uint32_t feedback;
	uint32_t training_size;
	
	bool cancel;
	bool pause;
	uint32_t cur_job;
	vector<ptree> fjobs;

	SimulationSettings();
	SimulationSettings(ptree &_fsettings);
	~SimulationSettings();
	
	template<class T> T generate(const ptree&, bool force_limits = false, double forced_min = 0.0, double forced_max = 100000000);
	ptree parse_individual(ptree);
	ptree parse_scenario(ptree _fscenario, unsigned int min_pop, unsigned int feedback, unsigned int max_feedback);
	
	// Notar que estos metodos PODRIAN evitar recibir batch_size y usar training_size interno
	// Lo dejo asi por si se nos ocurre usar un batch size diferente en algun caso
	void send(const uint32_t _batch_length, const ptree &_fhosts);
	void resume_send(const uint32_t _batch_length, const ptree &_fhosts);
	void generateJobs(const uint32_t _batch_length);

};
	
	
	
	
	
	
	
	
	
	
	

#endif
