#include "SimulationSettings.h"

SimulationSettings::SimulationSettings(ptree &_fsettings){
	this->_fsettings = _fsettings;
	this->_run = 0;
	this->_batch = 0;
	this->_feedback = 0;
	this->_training_size = 0;
}

SimulationSettings::~SimulationSettings(void){
}

template<class T>
T SimulationSettings::generate(const ptree &_fdistribution, bool force_limits, double forced_min, double forced_max){
	
//	cout<<"SimulationSettings::generate - Inicio\n";
	
//	std::stringstream ss;
//	write_json(ss,_fdistribution);
//	cout << ss.str() << endl;
	
	uint32_t type = util::hash(_fdistribution.get<string>("type"));
	double value = 0.0;
	
	switch(type){
		case UNIFORM:{
//			cout<<"SimulationSettings::generate - UNIFORM\n";
			double a = _fdistribution.get<double>("params.a");
			double b = _fdistribution.get<double>("params.b");
			std::uniform_real_distribution<> uniform(a, b);
			value = uniform(rng);
			if(force_limits){
				if(value < forced_min){
					value = forced_min;
				}
				else if(value > forced_max){
					value = forced_max;
				}
			}
			break;
		}
		case NORMAL:{
//			cout<<"SimulationSettings::generate - NORMAL\n";
			double mean = _fdistribution.get<double>("params.mean");
			double stddev = _fdistribution.get<double>("params.stddev");
			std::normal_distribution<> normal(mean, stddev);
			value = normal(rng);
			if(force_limits){
				if(value < forced_min){
					value = forced_min;
				}
				else if(value > forced_max){
					value = forced_max;
				}
			}
//			cout<<"Scheduler::generate - Normal (params: "<<mean<<", "<<stddev<<", value: "<<value<<")\n";
			break;
		}
		case GAMMA:{
//			cout<<"SimulationSettings::generate - GAMMA\n";
			double alpha = _fdistribution.get<double>("params.alpha");
			double beta = _fdistribution.get<double>("params.beta");
			std::gamma_distribution<double> gamma(alpha, beta);
			value = gamma(rng);
			if(force_limits){
				if(value < forced_min){
					value = forced_min;
				}
				else if(value > forced_max){
					value = forced_max;
				}
			}
			break;
		}
		default:{
			cout << "Error::Unknown Distribution Type::" << type << endl;
			break;
		}
	}
	return(static_cast<T>(value));
}

ptree SimulationSettings::parse_individual(ptree _findividual){
	for(auto &fchromosome : _findividual.get_child("chromosomes")){
		for(auto &fgene: fchromosome.second.get_child("genes")){
			ptree frate=fgene.second.get_child("mutation.rate");
			fgene.second.get_child("mutation").erase("rate");
			fgene.second.get_child("mutation").put<double>("rate",util::hash(frate.get<string>("type"))==RANDOM?generate<double>(frate.get_child("distribution"), true, 0.0, 1.0):frate.get<double>("value"));
		}
	}
	return(_findividual);
}

// Este metodo puede recibir el escalamiento de poblacion (feedback / max_feedback)
ptree SimulationSettings::parse_scenario(ptree _fscenario, unsigned int min_pop, unsigned int feedback, unsigned int max_feedback){

	uint32_t last_timestamp = 0;
	// TODO: Este limite de seguridad al tamaño de la poblacion es arbitrario
	// Necesitamos una mejor manera de validar que el numero sea correcto
	// Lo dejo activado pues el generador retorno 2^32 por alguna razon
	uint32_t max_population = 100000000;
	for(auto &fevent : _fscenario.get_child("events")){
		ptree ftimestamp = fevent.second.get_child("timestamp");
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
				ptree fsize = fevent.second.get_child("params.population.size");
				fevent.second.get_child("params.population").erase("size");
				unsigned int population_size = (util::hash(fsize.get<string>("type"))==RANDOM)?generate<uint32_t>(fsize.get_child("distribution"), true):fsize.get<uint32_t>("value");
				
				if( feedback < max_feedback ){
//					cout<<"Scheduler::parse_scenario - Reduciendo population_size ("<<population_size<<", min_pop: "<<min_pop<<", feedback: "<<feedback<<", max_feedback: "<<max_feedback<<")\n";
					population_size = (unsigned int)((double)(population_size - min_pop) * feedback / max_feedback + min_pop);
				}
//				cout<<"Scheduler::parse_scenario - population_size: "<<population_size<<"\n";
				if( population_size > max_population){
					population_size = max_population;
				}
				fevent.second.get_child("params.population").put<uint32_t>("size", population_size);
				
				break;
			}
			case INCREMENT:{
				ptree fpercentage = fevent.second.get_child("params.source.population.percentage");
				fevent.second.get_child("params.source.population").erase("percentage");
				fevent.second.get_child("params.source.population").put<double>("percentage",util::hash(fpercentage.get<string>("type"))==RANDOM?generate<double>(fpercentage.get_child("distribution"), true, 0.0):fpercentage.get<double>("value"));
				break;
			}
			case DECREMENT:{
				ptree fpercentage = fevent.second.get_child("params.source.population.percentage");
				fevent.second.get_child("params.source.population").erase("percentage");
				fevent.second.get_child("params.source.population").put<double>("percentage",util::hash(fpercentage.get<string>("type"))==RANDOM?generate<double>(fpercentage.get_child("distribution"), true, 0.0, 1.0):fpercentage.get<double>("value"));
				break;
			}
			case MIGRATION:{
				ptree fpercentage = fevent.second.get_child("params.source.population.percentage");
				fevent.second.get_child("params.source.population").erase("percentage");
				fevent.second.get_child("params.source.population").put<double>("percentage",util::hash(fpercentage.get<string>("type"))==RANDOM?generate<double>(fpercentage.get_child("distribution"), true, 0.0, 1.0):fpercentage.get<double>("value"));
				break;
			}
			default:
				break;
		}
	}
	
//	cout<<"SimulationSettings::parse_scenario\n";
//	std::stringstream ss;
//	write_json(ss,_fscenario);
//	cout << ss.str() << endl;
//	cout<<"-----      ------\n";
	
	return(_fscenario);
}

void SimulationSettings::send(const uint32_t &_batch_length, const ptree &_fhosts){

	cout<<"SimulationSettings::send - Inicio (_batch_length: "<<_batch_length<<")\n";
	
	vector<ptree> controllers;
	for(auto &fcontroller: _fhosts.get_child("controller"))
	controllers.push_back(fcontroller.second);
	
	vector<ptree> scenarios;
	for(auto &fscenario : this->_fsettings.get_child("scenarios"))
	scenarios.push_back(fscenario.second);
	
	vector<ptree> fjobs;
	
	// Por que usa batch * n_controller?
	// Es mas claro DISTRIBUIR el batch entre los controllers
//	cout<<"SimulationSettings::send - Iterando "<<(_batch_length*controllers.size())<<" veces\n";
//	for(uint32_t i = 0; i < (_batch_length*controllers.size()); i++){

	cout<<"SimulationSettings::send - Iterando "<<_batch_length<<" veces\n";
	for(uint32_t i = 0; i < _batch_length; i++){
		ptree fjob;
		fjob.put("id", this->_fsettings.get<uint32_t>("id"));
		fjob.put("run", this->_run++);
		fjob.put("batch", this->_batch);
		fjob.put("feedback", this->_feedback);
		
		fjob.put("batch-size", this->_fsettings.get<uint32_t>("batch-size"));
		
		unsigned int max_feedback = 0;
		boost::optional<ptree&> child = this->_fsettings.get_child_optional("population-increase-phases");
		if( child ){
			max_feedback = this->_fsettings.get<uint32_t>("population-increase-phases");
		}
		
		fjob.put("max-number-of-simulations", this->_fsettings.get<uint32_t>("max-number-of-simulations"));
		fjob.add_child("individual", parse_individual(this->_fsettings.get_child("individual")));
		fjob.add_child("scenario", parse_scenario(scenarios[i%scenarios.size()], 100, _feedback, max_feedback));
		
		fjobs.push_back(fjob);
	}
	random_shuffle(fjobs.begin(), fjobs.end());

	for(uint32_t i = 0; i < fjobs.size(); i++){
		uint32_t pos = i%controllers.size();
		cout<<"SimulationSettings::send - Enviando trabajo "<<i<<" a controler "<<pos<<"\n";
		comm::send(controllers[pos].get<string>("host"),controllers[pos].get<string>("port"),controllers[pos].get<string>("resource"), fjobs[i]);
	}

	fjobs.clear();
	scenarios.clear();
	controllers.clear();

	this->_run = 0;
	this->_batch++;
}



