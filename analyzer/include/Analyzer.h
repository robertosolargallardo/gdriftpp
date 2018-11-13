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
using boost::property_tree::ptree;

class Analyzer : public Node{
private: 
	// id incremental
	uint32_t incremental_id;

	map<uint32_t, ptree> _data;
	map<uint32_t, map<string, map<uint32_t, map<uint32_t, map<string, double>>>> > _data_indices;
	map<uint32_t, uint32_t> finished;
	
	// En esta version, next_batch es por simulacion (aunque se procesa cada escenario por separado)
	map<uint32_t, uint32_t> next_batch;
	
	// Mapa equivalente al anterior, pero solo usado para actualizar resultados (frecuentemente)
//	static const uint32_t update_results = 100;
	static uint32_t update_results;
	map<uint32_t, uint32_t> next_results;
	
	DBCommunication db_comm;
	enum Types{SIMULATED=416813159, DATA=1588979285, CANCEL=3692849629, RESTORE=2757426757};
	
	// Mapa de indices: <sample, chrid, genid, map<statistic, value>>
	// Retorna el numero total de indices parseados
	unsigned int parseIndices(const ptree &json, map<string, map<uint32_t, map<uint32_t, map<string, double>>>> &indices, bool mostrar = false);
	
	// MEtodo obsoleto
	void trainModel(uint32_t id, uint32_t scenario_id, uint32_t feedback, ptree &settings, map<string, Distribution> &posterior_map, map<string, Distribution> &adjustment_map, map<string, map<string, double>> &statistics_map);
	
	void trainModelv2(uint32_t id, uint32_t feedback);
	
	string logs_path;
	string log_file;
	string id_file;

	ptree getProfile(const map<uint32_t,map<uint32_t,map<uint32_t,vector<Marker>>>> &_samples,const uint32_t &_ploidy);
	
	// Metodo obsoleto
	ptree updateTrainingResults(uint32_t id, uint32_t feedback);
	
	// Nueva version simplificada
	void updateResults(uint32_t id, uint32_t feedback);
	
	// Metodo para simplificar la toma de valor opcional de un json
	// Version int, retorna 0 si no encuentra el valor
	uint32_t getUInt(ptree &json, const string &field);
	
	// Metodo para simplificar la toma de valor opcional de un json
	// Version string, retorna "NULL" si no encuentra el valor
	string getString(ptree &json, const string &field);
	
	// Metodo para simplificar la extraccion de parametros de un escenario (para establecer el orden)
	map<uint32_t, vector<string>> getEventsParams(ptree &scenario);
	
	void getCleanData(uint32_t id, uint32_t scenario_id, uint32_t feedback, ptree &individual, ptree &scenario, map<string, uint32_t> &params_positions, vector<double> &distances, vector<vector<double>> &params);
	
public: 
	Analyzer(ptree&);
	~Analyzer(void);
	
	ptree run(const std::string&);
	ptree run(ptree&);
	
};
#endif
