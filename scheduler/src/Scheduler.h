#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <ctime>
#include <cstdlib>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <map>
#include <vector>
#include <Simulator.h>

#include "../../util/Comm.h"
#include "../../util/Node.h"
#include "../../util/Mongo.h"
#include "../../util/Const.h"
#include "../../util/Semaphore.h"

using namespace std;

extern mt19937 rng;

class Scheduler:public Node{
   private: shared_ptr<util::Mongo> _mongo;
				shared_ptr<util::Semaphore> _semaphore;

				class Settings{
					public:	boost::property_tree::ptree _fsettings;
								uint32_t _run;
								uint32_t _batch;
						
								Settings(boost::property_tree::ptree &_fsettings);

								template<class T>
								friend T generate(const boost::property_tree::ptree&);
								friend boost::property_tree::ptree parse_individual(boost::property_tree::ptree);
								friend boost::property_tree::ptree parse_scenario(boost::property_tree::ptree);
								void send(const uint32_t&,const boost::property_tree::ptree&);
								~Settings(void);
								
				};

				map<uint32_t,shared_ptr<Settings>> _settings;

   public:
      Scheduler(const boost::property_tree::ptree&);
		void run(boost::property_tree::ptree&);
      ~Scheduler(void);

};
#endif
