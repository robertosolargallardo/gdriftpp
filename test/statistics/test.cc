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

	if(argc != 3){
		cout<<"\nModo de Uso: test_statistics hosts out_file\n";
		cout<<"\n";
		return 0;
	}
	
	string hosts = argv[1];
	string out_file = argv[2];
	
	boost::property_tree::ptree fhosts;
	read_json(hosts, fhosts);
	
//	DBCommunication db_comm("mongodb://localhost:27017", "gdrift", "data", "results", "settings", "training");
	DBCommunication db_comm(fhosts.get<string>("database.uri"), fhosts.get<string>("database.name"), fhosts.get<string>("database.collections.data"), fhosts.get<string>("database.collections.results"), fhosts.get<string>("database.collections.settings"), fhosts.get<string>("database.collections.training"));
	
//	db_comm.storeResults(0, 0, out_file);
	db_comm.storeDistributions(0, 0, out_file);
	
	
	
	
	return 0;
}





















