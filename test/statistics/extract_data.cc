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

	if(argc != 9){
		cout<<"\nModo de Uso: ./extract_data hosts simulation_id scenario_id results_file distributions_file feedback n_stats n_params\n";
		cout<<"\n";
		return 0;
	}
	
	string hosts = argv[1];
	unsigned int simulation_id = atoi(argv[2]);
	unsigned int scenario_id = atoi(argv[3]);
	string results_file = argv[4];
	string distributions_file = argv[5];
	unsigned int feedback = atoi(argv[6]);
	unsigned int n_stats = atoi(argv[7]);
	unsigned int n_params = atoi(argv[8]);
	
	boost::property_tree::ptree fhosts;
	read_json(hosts, fhosts);
	
//	DBCommunication db_comm("mongodb://localhost:27017", "gdrift", "data", "results", "settings", "training");
	DBCommunication db_comm(fhosts.get<string>("database.uri"), fhosts.get<string>("database.name"), fhosts.get<string>("database.collections.data"), fhosts.get<string>("database.collections.results"), fhosts.get<string>("database.collections.settings"), fhosts.get<string>("database.collections.training"));
	
	db_comm.storeResults(simulation_id, scenario_id, results_file, feedback, n_stats, n_params);
	db_comm.storeDistributions(simulation_id, scenario_id, distributions_file, feedback);
	
	
	
	
	return 0;
}





















