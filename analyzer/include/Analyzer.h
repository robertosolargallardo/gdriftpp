#ifndef _ANALYZER_H_
#define _ANALYZER_H_

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/optional/optional.hpp>
#include <iostream>
#include <fstream>

#include <map>
#include <set>
#include <vector>
#include <chrono>

#include <Simulator.h>
#include "FileParser.h"
#include "MultipartFormParser.h"

#include "Comm.h"
#include "Node.h"
#include "Const.h"
#include "DBCommunication.h"

#include "SimulationStatistics.h"
#include "Statistics.h"

using namespace std;
using namespace chrono;

class Analyzer : public Node{
private: 
	/*id incremental*/
	uint32_t _incremental_id;

	map<uint32_t, boost::property_tree::ptree> _data;
	map<uint32_t, map<string, map<uint32_t, map<uint32_t, map<string, double>>>> > _data_indices;
	map<uint32_t, uint32_t> finished;
	
	// next feedback es para cada escenario (por eso el indice, <sim, scen>)
//	map<pair<uint32_t, uint32_t>, uint32_t> next_batch;
	// Por ahora lo dejo dependiendo de la simulacion, pues finished cuanta por simulacion
	map<uint32_t, uint32_t> next_batch;
	
	// Mapa equivalente al anterior, pero solo usado para actualizar resultados (frecuentemente)
//	static const uint32_t update_results = 100;
	static uint32_t update_results;
	map<uint32_t, uint32_t> next_results;
	
	DBCommunication db_comm;
	enum Types{SIMULATED=416813159, DATA=1588979285, CANCEL=3692849629};
//	enum Types{SIMULATED=416813159, DATA=1588979285, QUERY=2172811635, PROGRESS=1089828659, TIME=258444835};
	
	// Mapa de indices: <sample, chrid, genid, map<statistic, value>>
	// Retorna el numero total de indices parseados
	unsigned int parseIndices(const boost::property_tree::ptree &json, map<string, map<uint32_t, map<uint32_t, map<string, double>>>> &indices);
	
	// Retorna true si la simulacion debe terminar (en efecto, el mismo retorno de computeDistributions)
	bool trainModel(uint32_t id, uint32_t scenario_id, uint32_t feedback, uint32_t max_feedback, boost::property_tree::ptree &settings, map<string, Distribution> &posterior_map, map<string, Distribution> &adjustment_map, map<string, map<string, double>> &statistics_map);
	
	static string log_file;

	boost::property_tree::ptree get_profile(const map<uint32_t,map<uint32_t,map<uint32_t,vector<Marker>>>> &_samples,const uint32_t &_ploidy);
	
	boost::property_tree::ptree updateTrainingResults(uint32_t id, uint32_t feedback, bool &finish);
	
public: 
	Analyzer(boost::property_tree::ptree&);
	~Analyzer(void);
	
	boost::property_tree::ptree run(const std::string&);
	boost::property_tree::ptree run(boost::property_tree::ptree&);
	double distance(uint32_t id, const boost::property_tree::ptree&);
	
};
#endif
