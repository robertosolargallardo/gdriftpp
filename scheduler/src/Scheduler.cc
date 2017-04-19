#include "Scheduler.h"

Scheduler::Settings::Settings(boost::property_tree::ptree &_fsettings){
	this->_fsettings = _fsettings;
	this->_run = 0;
	this->_batch = 0;
	this->_feedback = 0;
}

Scheduler::Settings::~Settings(void){
}

Scheduler::Scheduler(const boost::property_tree::ptree &_fhosts):Node(_fhosts){
	this->_mongo=make_shared<util::Mongo>(this->_fhosts.get<string>("database.uri"));
	this->_semaphore=make_shared<util::Semaphore>(1);
}

Scheduler::~Scheduler(void){
	this->_settings.clear();
}

boost::property_tree::ptree Scheduler::run(boost::property_tree::ptree &_frequest){
	uint32_t id = _frequest.get<uint32_t>("id");
	uint32_t type = util::hash(_frequest.get<string>("type"));
	cout<<"Scheduler::run - Inicio (id: "<<id<<", type: "<<type<<")\n";

	switch(type){
		case INIT:{
			cout<<"Scheduler::run - INIT\n";
			_frequest.put("feedback", 0);
			this->_mongo->write(this->_fhosts.get<string>("database.name"),this->_fhosts.get<string>("database.collections.settings"), _frequest);
			this->_semaphore->lock();
			this->_settings[id] = make_shared<Settings>(_frequest);
			this->_semaphore->unlock();
			this->_settings[id]->send(BATCH_LENGTH, this->_fhosts);
			break;
		}
		case CONTINUE:{
			cout<<"Scheduler::run - CONTINUE\n";
			this->_settings[id]->send(BATCH_LENGTH, this->_fhosts);
			break;
		}
		case RELOAD:{
			cout<<"Scheduler::run - RELOAD\n";
			// Aqui hay que guardar _frequest nuevamente en settings, pues trae nuevos parametros para el propio scheduler
			// Tambien hay que recrear settings (con los nuevos parametros)
			// Hay que asegurar entonces, que en reload el analizer envie el ptree de settings !!!
			
			std::stringstream ss;
			write_json(ss, _frequest);
			cout << ss.str() << endl;
			
			this->_mongo->write(this->_fhosts.get<string>("database.name"), this->_fhosts.get<string>("database.collections.settings"), _frequest);
			this->_semaphore->lock();
			this->_settings[id].reset(new Settings(_frequest));
			this->_semaphore->unlock();
			// Asigno independientemente el feedback, quzas podria definirse en el constructo si se encuentra (para ser resistente al init)
			this->_settings[id]->_feedback = _frequest.get<uint32_t>("feedback");
			break;
		}
		case FINALIZE:{
			cout<<"Scheduler::run - FINALIZE\n";
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
	cout<<"Scheduler::run - Fin\n";
	return(_frequest);
}

//TODO This function parse the distributions. Now only works with uniform.
template<class T>
T generate(const boost::property_tree::ptree &_fdistribution, bool force_positive = false){

	//std::stringstream ss;
	//write_json(ss,_fdistribution);
	//cout << ss.str() << endl;
	//exit(0);

	uint32_t type = util::hash(_fdistribution.get<string>("type"));
	
	switch(type){
		case UNIFORM:{
			double a = _fdistribution.get<double>("params.a");
			double b = _fdistribution.get<double>("params.b");
			std::uniform_real_distribution<> uniform(a, b);
			double value = uniform(rng);
			if(force_positive && value < 0.0){
				value = 0.0;
			}
			return(static_cast<T>(value));
		}
		case NORMAL:{
			double mean = _fdistribution.get<double>("params.mean");
			double stddev = _fdistribution.get<double>("params.stddev");
			std::normal_distribution<> normal(mean, stddev);
			double value = normal(rng);
			cout<<"Scheduler::generate - Normal (params: "<<mean<<", "<<stddev<<", value: "<<value<<")\n";
			if(force_positive && value < 0.0){
				value = 0.0;
			}
			return(static_cast<T>(value));
		}
		case GAMMA:{
			double alpha = _fdistribution.get<double>("params.alpha");
			double beta = _fdistribution.get<double>("params.beta");
			std::gamma_distribution<double> gamma(alpha, beta);
			double value = gamma(rng);
			if(force_positive && value < 0.0){
				value = 0.0;
			}
			return(static_cast<T>(value));
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

// Este metodo puede recibir el escalamiento de poblacion (feedback / max_feedback)
boost::property_tree::ptree parse_scenario(boost::property_tree::ptree _fscenario, unsigned int min_pop, unsigned int feedback, unsigned int max_feedback){
//boost::property_tree::ptree parse_scenario(boost::property_tree::ptree _fscenario, float population_factor){
//boost::property_tree::ptree parse_scenario(boost::property_tree::ptree _fscenario){
	uint32_t last_timestamp = 0;
	// TODO: Este limite de seguridad al tamaño de la poblacion es arbitrario
	// Necesitamos una mejor manera de validar que el numero sea correcto
	// Lo dejo activado pues el generador retorno 2^32 por alguna razon
	uint32_t max_population = 100000000;
	for(auto &fevent : _fscenario.get_child("events")){
		boost::property_tree::ptree ftimestamp = fevent.second.get_child("timestamp");
		fevent.second.erase("timestamp");
		uint32_t timestamp = util::hash(ftimestamp.get<string>("type"))==RANDOM?generate<uint32_t>(ftimestamp.get_child("distribution"), true):ftimestamp.get<uint32_t>("value");
		if(timestamp <= last_timestamp){
			timestamp = last_timestamp + 1;
		}
		last_timestamp = timestamp;
//		cout<<"Scheduler::parse_scenario - Event timestamp: "<<timestamp<<"\n";
		fevent.second.put<uint32_t>("timestamp", timestamp);

		switch(util::hash(fevent.second.get<string>("type"))){
			case CREATE:{
				boost::property_tree::ptree fsize = fevent.second.get_child("params.population.size");
				fevent.second.get_child("params.population").erase("size");
				unsigned int population_size = (util::hash(fsize.get<string>("type"))==RANDOM)?generate<uint32_t>(fsize.get_child("distribution"), true):fsize.get<uint32_t>("value");
				
//				std::ostringstream oss;
//				boost::property_tree::ini_parser::write_ini(oss, fsize.get_child("distribution"));
//				cout<<"Scheduler::parse_scenario - population_size: "<<population_size<<" ("<<oss.str()<<")\n";
				
				if( feedback < max_feedback ){
					cout<<"Scheduler::parse_scenario - Reduciendo population_size ("<<population_size<<", min_pop: "<<min_pop<<", feedback: "<<feedback<<", max_feedback: "<<max_feedback<<")\n";
					population_size = (unsigned int)((double)(population_size - min_pop) * feedback / max_feedback + min_pop);
				}
				cout<<"Scheduler::parse_scenario - population_size: "<<population_size<<"\n";
				if( population_size > max_population){
					population_size = max_population;
				}
				fevent.second.get_child("params.population").put<uint32_t>("size", population_size);
				
				break;
			}
			case INCREMENT:
			case DECREMENT:
			case MIGRATION:{
				boost::property_tree::ptree fpercentage=fevent.second.get_child("params.source.population.percentage");
				fevent.second.get_child("params.source.population").erase("percentage");
				fevent.second.get_child("params.source.population").put<double>("percentage",util::hash(fpercentage.get<string>("type"))==RANDOM?generate<double>(fpercentage.get_child("distribution"), true):fpercentage.get<double>("value"));
				break;
			}
			default:
				break;
		}
	}
	return(_fscenario);
}
void Scheduler::Settings::send(const uint32_t &_batch_length, const boost::property_tree::ptree &_fhosts){
	vector<boost::property_tree::ptree> controllers;
	for(auto &fcontroller: _fhosts.get_child("controller"))
	controllers.push_back(fcontroller.second);
	
	vector<boost::property_tree::ptree> scenarios;
	for(auto &fscenario : this->_fsettings.get_child("scenarios"))
	scenarios.push_back(fscenario.second);
	
	vector<boost::property_tree::ptree> fjobs;
	for(uint32_t i = 0; i < (_batch_length*controllers.size()); i++){
		boost::property_tree::ptree fjob;
		fjob.put("id", this->_fsettings.get<uint32_t>("id"));
		fjob.put("run", this->_run++);
		fjob.put("batch", this->_batch);
		fjob.put("feedback", this->_feedback);
		
		fjob.put("simulations-per-feedback", this->_fsettings.get<uint32_t>("simulations-per-feedback"));
		
		unsigned int max_feedback = 0;
		boost::optional<boost::property_tree::ptree&> child = this->_fsettings.get_child_optional("population-increase-phases");
		if( child ){
			max_feedback = this->_fsettings.get<uint32_t>("population-increase-phases");
		}
//		boost::property_tree::ptree::assoc_iterator it = this->_fsettings.find("population-increase-phases");
//		if( it != _fsettings.not_found() ){
//			max_feedback = this->_fsettings.get<uint32_t>("population-increase-phases");
//		}
		
//		double population_factor = 1.0
//		if(max_feedback > 0){
//			population_factor = (double)(this->_feedback) / max_feedback;
//		}
		// La otra opcion es pasarle _feedback y max_feedback a parse_scenario para que haga el calculo
		// population_size = min_size + (rand_population_size - min_size) * feedback / max_feedback;
		
		fjob.put("max-number-of-simulations", this->_fsettings.get<uint32_t>("max-number-of-simulations"));
		fjob.add_child("individual", parse_individual(this->_fsettings.get_child("individual")));
//		fjob.add_child("scenario", parse_scenario(scenarios[i%scenarios.size()]));
		fjob.add_child("scenario", parse_scenario(scenarios[i%scenarios.size()], 100, _feedback, max_feedback));
		
		/*std::stringstream ss;
		write_json(ss,fjob);
		cout << ss.str() << endl;
		exit(0);*/
		
		fjobs.push_back(fjob);
	}
	random_shuffle(fjobs.begin(), fjobs.end());

	for(uint32_t i = 0; i < fjobs.size(); i++){
		comm::send(controllers[i%controllers.size()].get<string>("host"),controllers[i%controllers.size()].get<string>("port"),controllers[i%controllers.size()].get<string>("resource"), fjobs[i]);
	}

	fjobs.clear();
	scenarios.clear();
	controllers.clear();

	this->_run=0;
	this->_batch++;
}



