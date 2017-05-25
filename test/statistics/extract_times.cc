#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include <iostream>

#include <map>
#include <vector>
#include <list>

#include <mongocxx/instance.hpp>

#include "DBCommunication.h"

using namespace std;

random_device seed;
mt19937 rng(seed());
mongocxx::instance inst{};

int main(int argc, char** argv){

	if(argc != 6){
		cout<<"\nModo de Uso: ./extract_times hosts simulation_id scenario_id results_file feedback\n";
		cout<<"\n";
		return 0;
	}
	
	string hosts = argv[1];
	unsigned int simulation_id = atoi(argv[2]);
	unsigned int scenario_id = atoi(argv[3]);
	string results_file = argv[4];
	unsigned int feedback = atoi(argv[5]);
	
	boost::property_tree::ptree fhosts;
	read_json(hosts, fhosts);
	
//	DBCommunication db_comm("mongodb://localhost:27017", "gdrift", "data", "results", "settings", "training");
	DBCommunication db_comm(fhosts.get<string>("database.uri"), fhosts.get<string>("database.name"), fhosts.get<string>("database.collections.data"), fhosts.get<string>("database.collections.results"), fhosts.get<string>("database.collections.settings"), fhosts.get<string>("database.collections.training"));
	
	db_comm.storeTimes(simulation_id, scenario_id, results_file, feedback);
	
	return 0;
}





















