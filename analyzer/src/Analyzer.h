#ifndef _ANALYZER_H_
#define _ANALYZER_H_
#include <Simulator.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <map>

#include "../../util/Mongo.h"
#include "../../util/Comm.h"
#include "../../util/Node.h"
#include "../../util/Const.h"

using namespace std;

class Analyzer:public Node{
   private: map<uint32_t,boost::property_tree::ptree> _data;
				map<uint32_t,uint32_t> _accepted;
				map<uint32_t,uint32_t> _batch_size;

				shared_ptr<util::Mongo> _mongo;

   public:  Analyzer(boost::property_tree::ptree&);
				void run(boost::property_tree::ptree&);
            ~Analyzer(void);

            friend double distance(const boost::property_tree::ptree&,const boost::property_tree::ptree&);
};
#endif
