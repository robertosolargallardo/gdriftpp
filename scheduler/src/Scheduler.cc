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
				 cout << "Error::Unknown Scheduling Type::" << type << endl;
				 exit(EXIT_FAILURE);
				 break;
			 }
   }
	return(_frequest);
}
//TODO This function parse the distributions. Now only works with uniform.
template<class T>
T generate(const boost::property_tree::ptree &_fparams){
   std::uniform_real_distribution<double> uniform(_fparams.get<double>("min-value"),_fparams.get<double>("max-value"));
   return(T(uniform(rng)));
}
boost::property_tree::ptree parse_individual(boost::property_tree::ptree _findividual){
   for(auto &fchromosome : _findividual.get_child("chromosomes")){
      for(auto &fgene: fchromosome.second.get_child("genes")){
         if(fgene.second.get<string>("mutation-rate.type")=="random"){
            boost::property_tree::ptree fparams=fgene.second.get_child("mutation-rate");
            fgene.second.erase("mutation-rate");
            fgene.second.put<double>("mutation-rate",generate<double>(fparams));
         }
      }
   }
   return(_findividual);
}
boost::property_tree::ptree parse_scenario(boost::property_tree::ptree _fscenario){
	for(auto &fevent : _fscenario.get_child("events")){
		boost::property_tree::ptree fparams=fevent.second.get_child("timestamp");
      fevent.second.erase("timestamp");
		fevent.second.put<uint32_t>("timestamp",(fparams.get<string>("type")=="random")?generate<uint32_t>(fparams):fparams.get<uint32_t>("value"));

		switch(util::hash(fevent.second.get<string>("type"))){
			case CREATE:{ 	
								boost::property_tree::ptree fparams=fevent.second.get_child("params.population.size");
      						fevent.second.get_child("params.population").erase("size");
      						fevent.second.get_child("params.population").put<uint32_t>("size",(fparams.get<string>("type")=="random")?generate<uint32_t>(fparams):fparams.get<uint32_t>("value"));
								break;
							}
			case INCREMENT:
			case DECREMENT:
			case MIGRATION:{
									boost::property_tree::ptree fparams=fevent.second.get_child("params.source.population.percentage");
      							fevent.second.get_child("params.source.population").erase("percentage");
      							fevent.second.get_child("params.source.population").put<double>("percentage",(fparams.get<string>("type")=="random")?generate<double>(fparams):fparams.get<double>("value"));
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
