#ifndef _ANALYZER_H_
#define _ANALYZER_H_

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/optional/optional.hpp>
#include <Simulator.h>

#include <map>
#include <set>
#include <vector>

#include "Comm.h"
#include "Node.h"
#include "Const.h"
#include "DBCommunication.h"

#include "simEvaluation.h"

using namespace std;

class Analyzer : public Node{
private: 
	map<uint32_t, boost::property_tree::ptree> _data;
	map<uint32_t, map<string, map<uint32_t, map<uint32_t, map<string, double>>>> > _data_indices;
	map<uint32_t, uint32_t> finished;
	
	// next feedback es para cada escenario (por eso el indice, <sim, scen>)
//	map<pair<uint32_t, uint32_t>, uint32_t> next_batch;
	// Por ahora lo dejo dependiendo de la simulacion, pues finished cuanta por simulacion
	map<uint32_t, uint32_t> next_batch;
	
	DBCommunication db_comm;
	enum Types{SIMULATED=416813159, DATA=1588979285};
	
	// Mapa de indices: <sample, chrid, genid, map<statistic, value>>
	// Retorna el numero total de indices parseados
	unsigned int parseIndices(const boost::property_tree::ptree &json, map<string, map<uint32_t, map<uint32_t, map<string, double>>>> &indices);
	
	// Retorna true si la simulacion debe terminar (en efecto, el mismo retorno de computeDistributions)
	bool trainModel(uint32_t id, uint32_t scenario_id, uint32_t feedback, uint32_t max_feedback, boost::property_tree::ptree &fresponse, map<string, vector<double>> &estimations_map);
	
	// Retorna true si el ultimo batch de simulacion es lo suficientemente bueno (es decir, si hay que parar)
	// Recibe los P parametros de las N simulaciones
	// ...las E estadisticas de esas mismas N simulaciones
	// ...las E estadsiticas target
	// Entrega las P distribuciones como un par <media, varianza> para cada parametro
//	bool computeDistributions(vector<vector<double>> &params, vector<vector<double>> &statistics, vector<double> &target, vector< pair<double, double> > &res_dist);
	
//	pair<double, double> evaluateDistribution(vector<double> values);
	
public: 
	Analyzer(boost::property_tree::ptree&);
	~Analyzer(void);
	
	boost::property_tree::ptree run(boost::property_tree::ptree&);
	double distance(uint32_t id, const boost::property_tree::ptree&);
	
};
#endif
