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
	map<uint32_t,boost::property_tree::ptree> _data;
	map<uint32_t,uint32_t> _accepted;
	map<uint32_t,uint32_t> _batch_size;
	
	map<uint32_t,uint32_t> next_feedback;
	static const unsigned int feedback_size = 8;

	shared_ptr<util::Mongo> _mongo;
	enum Types{SIMULATED=416813159, DATA=1588979285};

public: 
	Analyzer(boost::property_tree::ptree&);
	~Analyzer(void);
	
	boost::property_tree::ptree run(boost::property_tree::ptree&);
	friend double distance(const boost::property_tree::ptree&,const boost::property_tree::ptree&);
};
#endif
