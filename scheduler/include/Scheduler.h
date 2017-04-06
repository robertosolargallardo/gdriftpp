#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <ctime>
#include <cstdlib>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <boost/property_tree/ini_parser.hpp>

#include <map>
#include <vector>
#include <Simulator.h>

#include <Comm.h>
#include <Node.h>
#include <Mongo.h>
#include <Const.h>
#include <Semaphore.h>

extern mt19937 rng;

enum Distributions {UNIFORM=3830327609,NORMAL=1330140418,GAMMA=0};

using namespace std;
class Scheduler:public Node{
   private: shared_ptr<util::Mongo> _mongo;
				shared_ptr<util::Semaphore> _semaphore;
  				enum Types {INIT=305198855, CONTINUE=810372829, RELOAD=1571056488, FINALIZE=3761776383};

				class Settings{
					public:	boost::property_tree::ptree _fsettings;
								uint32_t _run;
								uint32_t _batch;
								uint32_t _feedback;
						
								Settings(boost::property_tree::ptree &_fsettings);

								template<class T>
								friend T generate(const boost::property_tree::ptree&);
								friend boost::property_tree::ptree parse_individual(boost::property_tree::ptree);
								friend boost::property_tree::ptree parse_scenario(boost::property_tree::ptree);
								void send(const uint32_t&,const boost::property_tree::ptree&);
								~Settings(void);
								
				};

				map<uint32_t, shared_ptr<Settings>> _settings;

   public:
      Scheduler(const boost::property_tree::ptree&);
		boost::property_tree::ptree run(boost::property_tree::ptree&);
      ~Scheduler(void);

};
#endif
