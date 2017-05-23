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

	if(argc != 7){
		cout<<"\nModo de Uso: ./extract_data hosts results_file distributions_file feedback n_stats n_params\n";
		cout<<"\n";
		return 0;
	}
	
	string hosts = argv[1];
	string results_file = argv[2];
	string distributions_file = argv[3];
	unsigned int feedback = atoi(argv[4]);
	unsigned int n_stats = atoi(argv[5]);
	unsigned int n_params = atoi(argv[6]);
	
	boost::property_tree::ptree fhosts;
	read_json(hosts, fhosts);
	
//	DBCommunication db_comm("mongodb://localhost:27017", "gdrift", "data", "results", "settings", "training");
	DBCommunication db_comm(fhosts.get<string>("database.uri"), fhosts.get<string>("database.name"), fhosts.get<string>("database.collections.data"), fhosts.get<string>("database.collections.results"), fhosts.get<string>("database.collections.settings"), fhosts.get<string>("database.collections.training"));
	
	db_comm.storeResults(0, 0, results_file, feedback, n_stats, n_params);
	db_comm.storeDistributions(0, 0, distributions_file);
	
	
	
	
	return 0;
}





















