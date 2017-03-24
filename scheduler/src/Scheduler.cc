#include "Scheduler.h"

Scheduler::Settings::Settings(boost::property_tree::ptree &_fsettings){
	this->_fsettings=_fsettings;
	this->_run=0U;
	this->_batch=0U;
}
Scheduler::Settings::~Settings(void){
	;
}
Scheduler::Scheduler(const boost::property_tree::ptree &_fhosts):Node(_fhosts){
	this->_mongo=make_shared<util::Mongo>(this->_fhosts.get<string>("database.uri"));
	this->_semaphore=make_shared<util::Semaphore>(1);
}
Scheduler::~Scheduler(void){
	this->_settings.clear();
}
boost::property_tree::ptree Scheduler::run(boost::property_tree::ptree &_frequest){
	uint32_t id=_frequest.get<uint32_t>("id");
   uint32_t type=util::hash(_frequest.get<string>("type"));

   switch(type){
          case INIT:{
			   this->_mongo->write(this->_fhosts.get<string>("database.name"),this->_fhosts.get<string>("database.collections.settings"),_frequest);
				this->_semaphore->lock();
				this->_settings[id]=make_shared<Settings>(_frequest);
				this->_semaphore->unlock();
			 }
          case CONTINUE:{
				this->_settings[id]->send(BATCH_LENGTH,this->_fhosts);
            break;
          }
          case FINALIZE:{
				this->_semaphore->lock();
				this->_settings.erase(this->_settings.find(id));
				this->_semaphore->unlock();
            break;
          }
			 default:{
				 cerr<< "Error::Unknown Scheduling Type::" << type << endl;
				 exit(EXIT_FAILURE);
			 }
   }
	return(_frequest);
}
//TODO This function parse the distributions. Now only works with uniform.
template<class T>
T generate(const boost::property_tree::ptree &_fdistribution){

	/*std::stringstream ss;
   write_json(ss,_fdistribution);

   cout << ss.str() << endl;*/

   uint32_t type=util::hash(_fdistribution.get<string>("type"));

   switch(type){
		case UNIFORM:{
   		std::uniform_real_distribution<double> uniform(_fdistribution.get<double>("params.a"),_fdistribution.get<double>("params.b"));
   		return(T(uniform(rng)));
		}
		default:{
			cout << "Error::Unknown Distribution Type::" << type << endl;
			exit(EXIT_FAILURE);
			break;
		}
	}
}
boost::property_tree::ptree parse_individual(boost::property_tree::ptree _findividual){
   for(auto &fchromosome : _findividual.get_child("chromosomes")){
      for(auto &fgene: fchromosome.second.get_child("genes")){
			boost::property_tree::ptree frate=fgene.second.get_child("mutation.rate");
         fgene.second.get_child("mutation").erase("rate");
         fgene.second.get_child("mutation").put<double>("rate",util::hash(frate.get<string>("type"))==RANDOM?generate<double>(frate.get_child("distribution")):frate.get<double>("value"));
      }
   }
   return(_findividual);
}
boost::property_tree::ptree parse_scenario(boost::property_tree::ptree _fscenario){
	for(auto &fevent : _fscenario.get_child("events")){
		boost::property_tree::ptree ftimestamp=fevent.second.get_child("timestamp");
      fevent.second.erase("timestamp");
		fevent.second.put<uint32_t>("timestamp",util::hash(ftimestamp.get<string>("type"))==RANDOM?generate<double>(ftimestamp.get_child("distribution")):ftimestamp.get<double>("value"));

		switch(util::hash(fevent.second.get<string>("type"))){
			case CREATE:{ 	
								boost::property_tree::ptree fsize=fevent.second.get_child("params.population.size");
      						fevent.second.get_child("params.population").erase("size");
      						fevent.second.get_child("params.population").put<uint32_t>("size",util::hash(fsize.get<string>("type"))==RANDOM?generate<double>(fsize.get_child("distribution")):fsize.get<double>("value"));
								break;
							}
			case INCREMENT:
			case DECREMENT:
			case MIGRATION:{
									boost::property_tree::ptree fpercentage=fevent.second.get_child("params.source.population.percentage");
      							fevent.second.get_child("params.source.population").erase("percentage");
      							fevent.second.get_child("params.source.population").put<double>("percentage",util::hash(fpercentage.get<string>("type"))==RANDOM?generate<double>(fpercentage.get_child("distribution")):fpercentage.get<double>("value"));
									break;
								}
			default:	break;
		}
	}
	return(_fscenario);
}
void Scheduler::Settings::send(const uint32_t &_batch_length,const boost::property_tree::ptree &_fhosts){
	vector<boost::property_tree::ptree> controllers;
   for(auto &fcontroller: _fhosts.get_child("controller"))
      controllers.push_back(fcontroller.second);

	vector<boost::property_tree::ptree> scenarios;
   for(auto &fscenario : this->_fsettings.get_child("scenarios"))
      scenarios.push_back(fscenario.second);

	vector<boost::property_tree::ptree> fjobs;
	for(uint32_t i=0;i<(_batch_length*controllers.size());i++){
      boost::property_tree::ptree fjob;
		fjob.put("id",this->_fsettings.get<uint32_t>("id"));
      fjob.put("run",this->_run++);
      fjob.put("batch",this->_batch);
      fjob.put("max-number-of-simulations",this->_fsettings.get<uint32_t>("max-number-of-simulations"));
      fjob.add_child("individual",parse_individual(this->_fsettings.get_child("individual")));
      fjob.add_child("scenario",parse_scenario(scenarios[i%scenarios.size()]));
		
/*std::stringstream ss;
write_json(ss,fjob);
cout << ss.str() << endl;*/

		fjobs.push_back(fjob);
	}
	random_shuffle(fjobs.begin(),fjobs.end());

	for(uint32_t i=0;i<fjobs.size();i++)
		comm::send(controllers[i%controllers.size()].get<string>("host"),controllers[i%controllers.size()].get<string>("port"),controllers[i%controllers.size()].get<string>("resource"),fjobs[i]);

	fjobs.clear();
	scenarios.clear();
	controllers.clear();

	this->_run=0;
	this->_batch++;
}
