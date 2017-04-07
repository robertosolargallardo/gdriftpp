#ifndef _ANALYZER_H_
#define _ANALYZER_H_
#include <Simulator.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/optional/optional.hpp>
#include <map>

#include "Mongo.h"
#include "Comm.h"
#include "Node.h"
#include "Const.h"

using namespace std;

class Analyzer : public Node{
private: 
	map<uint32_t, boost::property_tree::ptree> _data;
	map<uint32_t, map<string, map<uint32_t, map<uint32_t, map<string, double>>>> > _data_indices;
	map<uint32_t, uint32_t> _accepted;
	map<uint32_t, uint32_t> _batch_size;
	
	// next feedback es para cada escenario (por eso el indice, <sim, scen>)
//	map<pair<uint32_t, uint32_t>, uint32_t> next_feedback;
	// Por ahora lo dejo dependiendo de la simulacion, pues accepted cuanta por simulacion
	map<uint32_t, uint32_t> next_feedback;
	// cada simulacion puede definir su propio feedback_size (y su propio max_feedback)
	map<uint32_t, uint32_t> feedback_size;

	shared_ptr<util::Mongo> _mongo;
	enum Types{SIMULATED=416813159, DATA=1588979285};
	
	// Mapa de indices: <sample, chrid, genid, map<statistic, value>>
	// Retorna el numero total de indices parseados
	unsigned int parseIndices(const boost::property_tree::ptree &json, map<string, map<uint32_t, map<uint32_t, map<string, double>>>> &indices);
	
public: 
	Analyzer(boost::property_tree::ptree&);
	~Analyzer(void);
	
	boost::property_tree::ptree run(boost::property_tree::ptree&);
	double distance(uint32_t id, const boost::property_tree::ptree&);
	
};
#endif
