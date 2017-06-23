#include "SimulationSettings.h"

SimulationSettings::SimulationSettings(){
	run = 0;
	batch = 0;
	feedback = 0;
	training_size = 0;
	cancel = false;
	pause = false;
	cur_job = 0;
}

SimulationSettings::SimulationSettings(ptree &_fsettings){
	// Archivo settings
	fsettings = _fsettings;
	// Valores por defecto
	run = 0;
	batch = 0;
	feedback = 0;
	training_size = BATCH_LENGTH;
	cancel = false;
	pause = false;
	cur_job = 0;
	// Cargo las variables que encuentre en settings
	boost::optional<boost::property_tree::ptree&> test_child;
	
	test_child = fsettings.get_child_optional("batch-size");
	if( test_child ){
		training_size = fsettings.get<uint32_t>("batch-size");
	}
	
	test_child = fsettings.get_child_optional("feedback");
	if( test_child ){
		feedback = fsettings.get<uint32_t>("feedback");
	}
	
}

SimulationSettings::~SimulationSettings(void){
}

template<class T>
T SimulationSettings::generate(const ptree &fdistribution, bool force_limits, double forced_min, double forced_max){
	
//	cout<<"SimulationSettings::generate - Inicio\n";
	
//	std::stringstream ss;
//	write_json(ss, fdistribution);
//	cout << ss.str() << endl;
	
	uint32_t type = util::hash(fdistribution.get<string>("type"));
	double value = 0.0;
	
	switch(type){
		case UNIFORM_HASH:{
//			cout<<"SimulationSettings::generate - UNIFORM\n";
			double a = fdistribution.get<double>("params.a");
			double b = fdistribution.get<double>("params.b");
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
		case NORMAL_HASH:{
//			cout<<"SimulationSettings::generate - NORMAL\n";
			double mean = fdistribution.get<double>("params.mean");
			double stddev = fdistribution.get<double>("params.stddev");
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
		case GAMMA_HASH:{
//			cout<<"SimulationSettings::generate - GAMMA\n";
			double alpha = fdistribution.get<double>("params.alpha");
			double beta = fdistribution.get<double>("params.beta");
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

ptree SimulationSettings::parse_individual(ptree findividual){
	for(auto &fchromosome : findividual.get_child("chromosomes")){
		for(auto &fgene: fchromosome.second.get_child("genes")){
			ptree frate=fgene.second.get_child("mutation.rate");
			fgene.second.get_child("mutation").erase("rate");
			fgene.second.get_child("mutation").put<double>("rate",util::hash(frate.get<string>("type"))==RANDOM?generate<double>(frate.get_child("distribution"), true, 0.0, 1.0):frate.get<double>("value"));
		}
	}
	return(findividual);
}

// Este metodo puede recibir el escalamiento de poblacion (feedback / max_feedback)
ptree SimulationSettings::parse_scenario(ptree fscenario, unsigned int min_pop, unsigned int feedback, unsigned int max_feedback){

//	std::stringstream ss;
//	write_json(ss, fscenario);
//	cout << ss.str() << endl;

	uint32_t last_timestamp = 0;
	// TODO: Este limite de seguridad al tamaño de la poblacion es arbitrario
	// Necesitamos una mejor manera de validar que el numero sea correcto
	// Lo dejo activado pues el generador retorno 2^32 por alguna razon
	uint32_t max_population = 100000000;
	for(auto &fevent : fscenario.get_child("events")){
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
//	write_json(ss, fscenario);
//	cout << ss.str() << endl;
//	cout<<"-----      ------\n";
	
	return(fscenario);
}

void SimulationSettings::resume_send(const uint32_t batch_length, const ptree &fhosts){

	cout<<"SimulationSettings::resume_send - Inicio (batch_length: "<<batch_length<<" desde cur_job: "<<cur_job<<")\n";
	
	vector<ptree> controllers;
	for(auto &fcontroller: fhosts.get_child("controller")){
		controllers.push_back(fcontroller.second);
	}

	for(; cur_job < fjobs.size(); cur_job++){
		if(cancel){
			cout<<"SimulationSettings::resume_send - Cancelando\n";
			break;
		}
		
		// Si la orden es pausa, lo ideal seria parar aqui y continuar desde esta misma iteracion con el resume
		// Para eso habria que guardar el vector de trabajos
		// Para eso, se necesitan dos ordenes diferentes (o un if en este metodo):
		//  - send para CREAR la lista de trabajos
		//  - continue para continuar con una lista ya creada
		// Notar que el pause NO cancela y no envia la señal al analyzer, por lo que no se perderian trabajos
		if(pause){
			cout<<"SimulationSettings::send - Pausando\n";
			break;
		}
		
		uint32_t pos = cur_job % controllers.size();
		cout<<"SimulationSettings::resume_send - Enviando trabajo "<<cur_job<<" a controler "<<pos<<"\n";
		comm::send(controllers[pos].get<string>("host"), controllers[pos].get<string>("port"), controllers[pos].get<string>("resource"), fjobs[cur_job]);
	}
	
	controllers.clear();
	
	if( !pause ){
		fjobs.clear();
		run = 0;
		batch++;
	}
	cout<<"SimulationSettings::resume_send - Fin\n";
}

void SimulationSettings::generateJobs(const uint32_t batch_length){
	fjobs.clear();
	
	vector<ptree> scenarios;
	for(auto &fscenario : fsettings.get_child("scenarios")){
		scenarios.push_back(fscenario.second);
	}
	
	cout<<"SimulationSettings::generateJobs - Iterando "<<batch_length<<" veces\n";
	for(uint32_t i = 0; i < batch_length; i++){
		ptree fjob;
		fjob.put("id", fsettings.get<std::string>("id"));
		fjob.put("run", run++);
		fjob.put("batch", batch);
		fjob.put("feedback", feedback);
		
		fjob.put("batch-size", fsettings.get<uint32_t>("batch-size"));
		
		unsigned int max_feedback = 0;
		boost::optional<ptree&> child = fsettings.get_child_optional("population-increase-phases");
		if( child ){
			max_feedback = fsettings.get<uint32_t>("population-increase-phases");
		}
		
		fjob.put("max-number-of-simulations", fsettings.get<uint32_t>("max-number-of-simulations"));
		fjob.add_child("individual", parse_individual(fsettings.get_child("individual")));
		fjob.add_child("scenario", parse_scenario(scenarios[i%scenarios.size()], 100, feedback, max_feedback));
		
		fjobs.push_back(fjob);
	}
	random_shuffle(fjobs.begin(), fjobs.end());
	scenarios.clear();
	
}

void SimulationSettings::send(const uint32_t batch_length, const ptree &fhosts){

	cout<<"SimulationSettings::send - Inicio (batch_length: "<<batch_length<<")\n";
	
	vector<ptree> controllers;
	for(auto &fcontroller: fhosts.get_child("controller")){
		controllers.push_back(fcontroller.second);
	}
	
	generateJobs(batch_length);

	for(cur_job = 0; cur_job < fjobs.size(); cur_job++){
		if(cancel){
			cout<<"SimulationSettings::send - Cancelando\n";
			break;
		}
		
		// Si la orden es pausa, lo ideal seria parar aqui y continuar desde esta misma iteracion con el resume
		// Para eso habria que guardar el vector de trabajos
		// Para eso, se necesitan dos ordenes diferentes (o un if en este metodo):
		//  - send para CREAR la lista de trabajos
		//  - continue para continuar con una lista ya creada
		// Notar que el pause NO cancela y no envia la señal al analyzer, por lo que no se perderian trabajos
		if(pause){
			cout<<"SimulationSettings::send - Pausando\n";
			break;
		}
		
		uint32_t pos = cur_job%controllers.size();
		cout<<"SimulationSettings::send - Enviando trabajo "<<cur_job<<" a controler "<<pos<<"\n";
		comm::send(controllers[pos].get<string>("host"), controllers[pos].get<string>("port"), controllers[pos].get<string>("resource"), fjobs[cur_job]);
	}
	
	controllers.clear();
	
	if( !pause ){
		fjobs.clear();
		run = 0;
		batch++;
	}
	cout<<"SimulationSettings::send - Fin\n";
}



