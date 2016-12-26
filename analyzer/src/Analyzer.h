#ifndef _ANALYZER_H_
#define _ANALYZER_H_
#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <Simulator.h>
#include <curl/curl.h>

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>

using namespace std;

#include "../../conf.h"
enum resultsType{SIMULATED=416813159,DATA=1588979285}; 
class Analyzer{
   private: map<uint32_t,boost::property_tree::ptree> _data;
            /*map<uint32_t,map<uint32_t,boost::property_tree::ptree>> _simulated;
            map<uint32_t,map<uint32_t,double>> _distances;*/
				map<uint32_t,uint32_t> _accepted;
				map<uint32_t,uint32_t> _batch_size;

				shared_ptr<mongocxx::client>	_client;
				shared_ptr<mongocxx::collection> _collection_data;
				shared_ptr<mongocxx::collection> _collection_results;

   public:  Analyzer(void);
            void analyze(boost::property_tree::ptree&);
            ~Analyzer(void);

            friend double distance(const boost::property_tree::ptree&,const boost::property_tree::ptree&);
				friend void send(const boost::property_tree::ptree&);
};
#endif
