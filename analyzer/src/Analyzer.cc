#include "Analyzer.h"

//string Analyzer::log_file = "logs/analyzer.log";
uint32_t Analyzer::update_results = 100;

Analyzer::Analyzer(ptree &fhosts) : Node(fhosts){

	cout << "Analyzer - Start\n";
	
	db_comm = DBCommunication(fhosts.get<string>("database.uri"), fhosts.get<string>("database.name"), fhosts.get<string>("database.collections.data"), fhosts.get<string>("database.collections.results"), fhosts.get<string>("database.collections.settings"), fhosts.get<string>("database.collections.training"));
	
	// Default del path
	logs_path = "~";
	boost::optional<ptree&> test_child;
	test_child = fhosts.get_child_optional("globals.logs");
	if( test_child ){
		logs_path = fhosts.get<string>("globals.logs");
	}
	// Archivos de log
	log_file = logs_path + "/analyzer.log";
	id_file = logs_path + "/analyzer_id.log";
	cout << "Analyzer - logs_path: " << logs_path << ", log_file: " << log_file << ", id_file: " << id_file << "\n";
	// Carga de datos iniciales (o de config)
	// Por ahora cargo el id como texto (para facilitar el debug)
	incremental_id = 0;
	ifstream lector(id_file, ifstream::in);
	if( lector.is_open() && lector.good() ){
		unsigned int buff_size = 128;
		char buff[buff_size];
		buff[0] = 0;
		lector.getline(buff, buff_size);
		if( strlen(buff) > 0 ){
			incremental_id = atoi(buff);
		}
		lector.close();
	}
	
	cout << "Analyzer - End (incremental_id: " << incremental_id << ")\n";
}
Analyzer::~Analyzer(void){
}

unsigned int Analyzer::parseIndices(const ptree &posterior, map<string, map<uint32_t, map<uint32_t, map<string, double>>>> &indices, bool mostrar){
	unsigned int inserts = 0;
	boost::optional<const ptree&> test_child;
	test_child = posterior.get_child_optional("populations");
	if( ! test_child ){
		return 0;
	}
	for(auto& p : posterior.get_child("populations")){
		string pop_name = p.second.get<string>("name");
		for(auto c : p.second.get_child("chromosomes")){
			uint32_t cid = c.second.get<uint32_t>("id");
			for(auto g : c.second.get_child("genes")){
				uint32_t gid = g.second.get<uint32_t>("id");
				for(auto i : g.second.get_child("indices")){
					indices[pop_name][cid][gid][i.first] = std::stod(i.second.data());
					++inserts;
				}
			}
		}
	}
	
	if(mostrar){
		// Prueba de resultados
		cout << "Analyzer::parseIndices - Statistics:\t";
		for(auto p : indices){
			for(auto c : p.second){
				for(auto g : c.second){
					for(auto i : g.second){
						cout<<i.second<<"\t";
					}
				}
			}
		}
		cout << "\n";
	}
	
	return inserts;
}

map<uint32_t, vector<string>> Analyzer::getEventsParams(ptree &scenario){
	map<uint32_t, vector<string>> events_params;
	for(auto e : scenario.get_child("events")){
		// En principio cada evento tiene timestamp y parametros
		// Los parametros que tengan type random deben ser agregados
		uint32_t eid = e.second.get<uint32_t>("id");
		string etype = e.second.get<string>("type");
		
		string dist_type = e.second.get<string>("timestamp.type");
		if( dist_type.compare("random") == 0 ){
			events_params[eid].push_back("timestamp");
		}
		
		// Mientras se incrementa la poblacion, omito population.size de los parametros
		if( etype.compare("create") == 0 ){
			dist_type = e.second.get<string>("params.population.size.type");
			if( dist_type.compare("random") == 0 ){
				events_params[eid].push_back("params.population.size");
			}
		}
		else if( etype.compare("endsim") != 0 && etype.compare("split") != 0 && etype.compare("extinction") != 0 ){
			dist_type = e.second.get<string>("params.source.population.percentage.type");
			if( dist_type.compare("random") == 0 ){
				events_params[eid].push_back("params.source.population.percentage");
			}
		}
	}// for... cada evento
	return events_params;
}


void Analyzer::getCleanData(uint32_t id, uint32_t scenario_id, uint32_t feedback, ptree &individual, ptree &scenario, map<string, uint32_t> &params_positions, vector<double> &distances, vector<vector<double>> &params){
	
	map<uint32_t, vector<string>> events_params = getEventsParams(scenario);
	
	// El bloque siguiente es para convertir posiciones en parametros (incluyendo eventos y individual)
	// Muestra, para cada codigo de texto, la posicion en params
	// parametros de eventos
	for(map<uint32_t, vector<string>>::iterator it = events_params.begin(); it != events_params.end(); it++){
		for(unsigned int i = 0; i < it->second.size(); ++i){
			string param_name = "events.";
			param_name += std::to_string(it->first);
			param_name += ".";
			param_name += it->second[i];
			params_positions.emplace(param_name, 0);
		}
	}
	// chromosomas
	for(auto &c : individual.get_child("chromosomes")){
		uint32_t cid = c.second.get<uint32_t>("id");
		for(auto g : c.second.get_child("genes")){
			string dist_type = g.second.get<string>("mutation.rate.type");
			if( dist_type.compare("random") == 0 ){
				uint32_t gid = g.second.get<uint32_t>("id");
				string param_name = "chromosomes.";
				param_name += std::to_string(cid);
				param_name += ".genes.";
				param_name += std::to_string(gid);
				param_name += ".mutation.rate";
				params_positions.emplace(param_name, 0);
			}
		}
	}
	// Posiciones
	unsigned int count = 0;
	for(map<string, uint32_t>::iterator it = params_positions.begin(); it != params_positions.end(); it++){
		it->second = count++;
		cout << "Analyzer::getCleanData - params_positions[" << it->first << "]: " << it->second << "\n";
	}
	
	if( params_positions.empty() ){
		cerr << "Analyzer::getCleanData - Data not found\n";
		return;
	}
	
	// Recupero los stats y params de la base de datos (en el orden de params_positions)
	vector<vector<double>> stats;
	db_comm.getResults(id, scenario_id, feedback, params, events_params, stats);
	
	if( params.size() == 0 || stats.size() == 0 ){
		cerr << "Analyzer::getCleanData - Data not found\n";
		return;
	}
	
	cout << "Analyzer::getCleanData - params data: " << params.size() << " (" << params[0].size() << " params)\n";
	cout << "Analyzer::getCleanData - stats data: " << stats.size() << " (" << stats[0].size() << " stats)\n";
	
	// Todo lo que sigue depende de las distancias, y los minimos y maximos por parametro
	// Despues, stats se puede obviar (usamos distancias en su lugar)
	
	// Target (calculado localmente de _data_indices)
	vector<double> target;
	for(auto p : _data_indices[id]){
		for(auto c : p.second){
			for(auto g : c.second){
				for(auto i : g.second){
					target.push_back(i.second);
				}
			}
		}
	}
	// una distancia por simulacion
	Statistics::getDistances(stats, target, distances);
	for(unsigned int i = 0; i < ((distances.size()<10)?distances.size():10); ++i){
		cout << "Analyzer::getCleanData - Dist[" << i << "]: " << distances[i] << "\n";
	}
	
}

vector<vector<double>> generateNormalParams(unsigned int batch_size, vector<pair<double, double>> &values_dists, vector<double> &min_params, vector<double> &max_params){
	vector<vector<double>> new_params;
	random_device seed;
	mt19937 generator(seed());
	vector<std::normal_distribution<>> normal_dists;
	for(unsigned int j = 0; j < values_dists.size(); ++j){
		std::normal_distribution<> normal(values_dists[j].first, values_dists[j].second);
		normal_dists.push_back(normal);
	}
	for(unsigned int i = 0; i < batch_size; ++i){
		vector<double> params_sim;
		for(unsigned int j = 0; j < values_dists.size(); ++j){
			double value = normal_dists[j](generator);
			if( value < min_params[j] ){
				value = min_params[j];
			}
			if( value > max_params[j] ){
				value = max_params[j];
			}
			params_sim.push_back(value);
		}
		new_params.push_back(params_sim);
	}
	return new_params;
}

void Analyzer::trainModel(uint32_t id, uint32_t feedback){
	
	cout << "Analyzer::trainModel - Start (sim: " << id << ", feedback: " << feedback << ")\n";
	
	ptree settings = db_comm.readSettings(id, feedback);
	
	//  Primero tomo el json del escenario, que usare despues
	ptree scenario;
	for(auto &s : settings.get_child("scenarios")){
		uint32_t scenario_id = s.second.get<uint32_t>("id");
		scenario = s.second;
		
		cout << "Analyzer::trainModel - Processing Scenario " << scenario_id << ")\n";
		
		map<string, uint32_t> params_positions;
		
		vector<double> distances;
		vector<vector<double>> params;
		
		getCleanData(id, scenario_id, feedback, settings.get_child("individual"), scenario, params_positions, distances, params);
		
		// un min/max por parametro
		vector<double> min_params = Statistics::getMinVector(params);
		vector<double> max_params = Statistics::getMaxVector(params);
		
		cout << "Analyzer::trainModel - min/max params\n";
		for(unsigned int i = 0; i < min_params.size(); ++i){
			cout << "Param[" << i << "]: (" << min_params[i] << ", " << max_params[i] << ")\n";
		}
		
		// Supongamos que ahora tenemos un nuevo conjunto de parametros para relanzar, param_update
		// En esta prueba, reuso los datos de la iteracion anterior, pero esto es el producto de algun proceso
//		vector<vector<double>> params_update = params;
		
		// Version original: distribuciones posterior con topk, y gegenrar valores con esas distribuciones
		cout << "Analyzer::trainModel - Preparing Posterior Normal Distribution\n";
		double min_dist = 0.0;
		double cut_dist = 0.0;
		vector<pair<double, double>> values_dists = Statistics::getNormalValues(distances, params, 0.05, min_dist, cut_dist);
		// TODO: Generar vector de limites (del json de settings quizas) y pasarlo al generador de valores
		vector<vector<double>> params_update = generateNormalParams(getUInt(settings, "batch-size"), values_dists, min_params, max_params);
		
		vector<ptree> controllers;
		for(auto &fcontroller: _fhosts.get_child("controller")){
			controllers.push_back(fcontroller.second);
		}
	
		for(unsigned int pos = 0; pos < params_update.size(); ++pos){
			vector<double> new_params = params_update[pos];
			// Preparar un nuevo json para el controller, basado en settings pero reemplazando los datos
			ptree job = settings;
			job.erase("scenarios");
			job.add_child("scenario", scenario);
		
			job.erase("_id");
			job.put("feedback", feedback+1);
			job.put("batch", feedback+1);
			job.put("run", pos);
		
		
			// Actualizar chromosomas
			for(auto &c : job.get_child("individual.chromosomes")){
				uint32_t cid = c.second.get<uint32_t>("id");
				for(auto &g : c.second.get_child("genes")){
					string dist_type = g.second.get<string>("mutation.rate.type");
					if( dist_type.compare("random") == 0 ){
						uint32_t gid = g.second.get<uint32_t>("id");
						string param_name = "chromosomes.";
						param_name += std::to_string(cid);
						param_name += ".genes.";
						param_name += std::to_string(gid);
						param_name += ".mutation.rate";
						
						double value = new_params[ params_positions[param_name] ];
						std::ostringstream stream1;
						stream1 << std::setprecision(std::numeric_limits<double>::digits10) << value;
						string str_value = stream1.str();
					
						cout << "Analyzer::trainModel - Replacing " << param_name << " with value " << value << "\n";
					
						g.second.get_child("mutation").erase("rate");
						g.second.put("mutation.rate", str_value);
					}
				}
			}
		
			// Actualizar eventos
			for(auto &e : job.get_child("scenario.events")){
				// En principio cada evento tiene timestamp y parametros
				// Los parametros que tengan type random deben ser agregados
				uint32_t eid = e.second.get<uint32_t>("id");
				string etype = e.second.get<string>("type");
			
				string dist_type = e.second.get<string>("timestamp.type");
				if( dist_type.compare("random") == 0 ){
					string param_name = "events.";
					param_name += std::to_string(eid);
					param_name += ".";
					param_name += "timestamp";
				
					double value = new_params[ params_positions[param_name] ];
					std::ostringstream stream1;
					stream1 << std::setprecision(std::numeric_limits<double>::digits10) << value;
					string str_value = stream1.str();
				
					cout << "Analyzer::trainModel - Replacing " << param_name << " with value " << value << "\n";
				
					e.second.erase("timestamp");
					e.second.put("timestamp", str_value);
				
				}
			
				// Mientras se incrementa la poblacion, omito population.size de los parametros
				if( etype.compare("create") == 0 ){
					dist_type = e.second.get<string>("params.population.size.type");
					if( dist_type.compare("random") == 0 ){
						string param_name = "events.";
						param_name += std::to_string(eid);
						param_name += ".";
						param_name += "params.population.size";
					
						double value = new_params[ params_positions[param_name] ];
						std::ostringstream stream1;
						stream1 << std::setprecision(std::numeric_limits<double>::digits10) << value;
						string str_value = stream1.str();
					
						cout << "Analyzer::trainModel - Replacing " << param_name << " with value " << value << "\n";
					
						e.second.get_child("params.population").erase("size");
						e.second.put("params.population.size", str_value);
					}
					else if(dist_type.compare("fixed") == 0){
						string str_value = e.second.get<string>("params.population.size.value");
						e.second.get_child("params.population").erase("size");
						e.second.put("params.population.size.value", str_value);
					}
				}
				else if( etype.compare("endsim") != 0 && etype.compare("split") != 0 && etype.compare("extinction") != 0 ){
					dist_type = e.second.get<string>("params.source.population.percentage.type");
					if( dist_type.compare("random") == 0 ){
						string param_name = "events.";
						param_name += std::to_string(eid);
						param_name += ".";
						param_name += "params.source.population.percentage";
					
						double value = new_params[ params_positions[param_name] ];
						std::ostringstream stream1;
						stream1 << std::setprecision(std::numeric_limits<double>::digits10) << value;
						string str_value = stream1.str();
					
						cout << "Analyzer::trainModel - Replacing " << param_name << " with value " << value << "\n";
					
						e.second.get_child("params.source.population").erase("percentage");
						e.second.put("params.source.population.percentage", str_value);
					}
					else if(dist_type.compare("fixed") == 0){
						string str_value = e.second.get<string>("params.source.population.percentage.value");
						e.second.get_child("params.source.population").erase("percentage");
						e.second.put("params.source.population.percentage", str_value);
					}
				}
			
			
			}// for... cada evento
		
			uint32_t pos_controller = pos%controllers.size();
			cout << "Analyzer::trainModel - Sending to controller " << pos_controller << "\n";
			comm::send(controllers[pos_controller].get<string>("host"), controllers[pos_controller].get<string>("port"), controllers[pos_controller].get<string>("resource"), job);
		
		}
	
	} // for... cada escenario
	
	cout << "Analyzer::trainModel - End\n";
	
}

// Este codigo deberia ser resistente a concurrencia
//	- Asumo que todas las operaciones de db_comm son thread safe (dependen de Mongo)
//	- trainModel debe ser thread safe
void Analyzer::updateResults(uint32_t id, uint32_t feedback){
	
	cout << "Analyzer::updateResults - Start\n";
	
	ptree settings = db_comm.readSettings(id, feedback);
	
	// scenario_id -> param_name -> distribution
	map<uint32_t, map<string, Distribution>> dists_prior = db_comm.getDistributions(id, 0);
	
	ptree scenarios;
	for(auto &s : settings.get_child("scenarios")){
		// Datos del Escenario
		uint32_t scenario_id = s.second.get<uint32_t>("id");
		map<string, Distribution> posterior_map;
		map<string, map<string, double>> statistics_map;
		
		map<string, uint32_t> params_positions;
		vector<double> distances;
		vector<vector<double>> params;
		double min_dist = 0.0;
		double cut_dist = 0.0;
		
		cout << "Analyzer::updateResults - Preparing data for scenario " << scenario_id << "\n";
		getCleanData(id, scenario_id, feedback, settings.get_child("individual"), s.second, params_positions, distances, params);
		
		cout << "Analyzer::updateResults - Preparing Posterior Normal Distribution\n";
		vector<pair<double, double>> values_dists = Statistics::getNormalValues(distances, params, 0.05, min_dist, cut_dist);
		
		vector<double> min_params = Statistics::getMinVector(params);
		vector<double> max_params = Statistics::getMaxVector(params);
		
		// Agregar posterior_map al json de resultados de entrenamiento
		ptree scenario;
		string name = "Scenario " + std::to_string(scenario_id);
		scenario.put("name", name);
		scenario.put("id", scenario_id);
		scenario.put("Min Distance", min_dist);
		scenario.put("Cut Distance", cut_dist);
		scenario.put("Similarity", (1.0 - min_dist));
		
		ptree estimations;
		
		for( auto it_params : params_positions ){
			cout << "Analyzer::updateResults - Preparing estimations for parameter " << it_params.first << "\n";
			
			ptree estimation;
			estimation.put("parameter", it_params.first);
			
			double mean = values_dists[ it_params.second ].first;
			double stddev = values_dists[ it_params.second ].second;
			
			estimation.put("mean", mean);
			estimation.put("stddev", stddev);
			
			double min_post = min_params[it_params.second];
			double max_post = max_params[it_params.second];
			vector<pair<double, double>> vals;
			
			ptree curves;
			
			// Curva Posterior
			
			ptree curve_posterior;
			curve_posterior.put("name", "Posterior");
			
			ptree fvalue_posterior;
			ptree fvalues_posterior;
			cout << "Analyzer::updateResults - Curve Posterior\n";
			
			Statistics::graphNormalDistribution(vals, mean, stddev, min_post, max_post, min_post, max_post);
			for(unsigned int j = 0; j < vals.size(); ++j){
				fvalue_posterior.put("x", vals[j].first);
				fvalue_posterior.put("y", vals[j].second);
				fvalues_posterior.push_back(make_pair("", fvalue_posterior));
			}
			vals.clear();
			
			// Agrego values a la curva
			curve_posterior.add_child("values", fvalues_posterior);
			
			// Agrego la curva posterior a curves
			curves.push_back(make_pair("", curve_posterior));
			
			// Curva Prior
			
			ptree curve_prior;
			curve_prior.put("name", "Prior");
			
			ptree fvalue_prior;
			ptree fvalues_prior;
			cout << "Analyzer::updateResults - Curve Prior\n";
			
			Distribution dist_prior = dists_prior[scenario_id][it_params.first];
			
			cout << "Analyzer::updateResults - Preparing values\n";
		
			// Para la distribucion prior, no tengo necesariamente min y max
			// Por ello, en ese caso pido minimos y maximos analiticos
			// La distribucion conoce valores razonables
			vals = Statistics::generateDistributionGraph(dist_prior, dist_prior.getMinValue(), dist_prior.getMaxValue(), min_post, max_post);
			for( unsigned int j = 0; j < vals.size(); ++j ){
				fvalue_prior.put("x", vals[j].first);
				fvalue_prior.put("y", vals[j].second);
				fvalues_prior.push_back(make_pair("", fvalue_prior));
			}
			vals.clear();
			// Agrego values a la curva
			curve_prior.add_child("values", fvalues_prior);
	
			// Agrego la curva posterior a curves
			curves.push_back(make_pair("", curve_prior));
			
			// Finalmente, agrego curves a estimation de este parametro
			estimation.add_child("curves", curves);
			
			estimations.push_back(make_pair("", estimation));
			
		}
		
		scenario.add_child("estimations", estimations);
		scenarios.push_back(make_pair("", scenario));
		
		scenario.add_child("estimations", estimations);
		
		posterior_map.clear();
	}
	
	ptree estimations;
	estimations.add_child("scenarios", scenarios);
	
	ptree training_results;
	training_results.put("id", std::to_string(id));
	training_results.put("feedback", feedback);
		
	uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	cout << "Analyzer::updateResults - Adding timestamp " << timestamp << "\n";
	training_results.put("timestamp", std::to_string(timestamp));
	
	training_results.add_child("estimations", estimations);
	
	// Almacenar el json de resultados de entrenamiento
	db_comm.storeTrainingResults(training_results);
	
	cout << "Analyzer::updateResults - End\n";
	
}

// Metodo para simplificar la toma de valor opcional de un json
// Version int, retorna 0 si no encuentra el valor
uint32_t Analyzer::getUInt(ptree &json, const string &field){
	boost::optional<ptree&> test_child;
	test_child = json.get_child_optional(field);
	if( test_child ){
		return json.get<uint32_t>(field);
	}
	return 0;
}

// Metodo para simplificar la toma de valor opcional de un json
// Version string, retorna "NULL" si no encuentra el valor
string Analyzer::getString(ptree &json, const string &field){
	boost::optional<ptree&> test_child;
	test_child = json.get_child_optional(field);
	if( test_child ){
		return json.get<string>(field);
	}
	return "NULL";
}

ptree Analyzer::run(ptree &_frequest){
	cout << "Analyzer::run - Start Simulated\n";

	uint32_t id = getUInt(_frequest, "id");
	string type = getString(_frequest, "type");
	
	cout << "Analyzer::run - id: " << id << ", type: " << type << "\n";
	
	switch(util::hash(_frequest.get<string>("type"))){
		case SIMULATED:{
			
			if( finished.find(id) == finished.end() ){
				// Resultado de simulacion NO iniciada, o cancelada
				// Omitir esto antes de hacer cualquier cosa adicional
				cout << "Analyzer::run - SIMULATED (Omiting unknown simulation id " << id << ")\n";
				return _frequest;
			}
			
			uint32_t scenario_id = getUInt(_frequest, "scenario.id");
			uint32_t feedback = getUInt(_frequest, "feedback");
			uint32_t batch_size = getUInt(_frequest, "batch-size");
			uint32_t max_sims = getUInt(_frequest, "max-number-of-simulations");
			
			// Solo agrego batch_size[id] como valor inicial la primera vez, de ahi se suma batch_size
			next_batch.emplace(id, batch_size);
			next_results.emplace(id, update_results);
			
			cout << "Analyzer::run - SIMULATED (id: " << id << ", scenario: " << scenario_id << ", feedback: " << feedback << ", batch_size: " << batch_size << ", max_sims: " << max_sims << ")\n";
			
			finished[id]++;
			
			// Guardo el resultado solo si NO hay error
			uint32_t error = getUInt(_frequest, "error");
			if(error == 0){
				uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				_frequest.put("timestamp", std::to_string(timestamp));
				db_comm.writeResults(_frequest);
			}
			else{
				cout << "Analyzer::run - Omiting simulation with errors (error: " << error << ")\n";
			}
			
			// Revisar numero de resultados para hacer una de varias cosas:
			//  - Terminar simulacion si se llega al maximo
			//  - Entrenar si se tiene un batch completo
			//  - Quizas actualizar resultados parciales si se aplica eso
			
			cout << "Analyzer::run - Finished: " << finished[id] << " / " << next_batch[id] << " / " << max_sims << "\n";
			
			if( finished[id] >= next_results[id] ){
				// Update de resultados
				next_results[id] += update_results;
				updateResults(id, feedback);
			}
			
			if( finished[id] >= next_batch[id] ){
				cout << "Analyzer::run - Feedback (batch_size: " << batch_size << ")\n";
				// Codigo de feedback, preparacion de nuevos parametros
				next_batch[id] += batch_size;
				trainModel(id, feedback);
			}
			
			if( finished[id] >= max_sims ){
				cout << "Analyzer::run - Preparing FINALIZE\n";
				finished.erase(id);
				next_batch.erase(id);
				ptree fresponse;
				fresponse.put("id", std::to_string(id));
				fresponse.put("type", "finalize");
				comm::send(_fhosts.get<string>("scheduler.host"), _fhosts.get<string>("scheduler.port"), _fhosts.get<string>("scheduler.resource"), fresponse);
			}
			break;
		};
		case DATA:  {
			finished[id] = 0;
			
			ptree fposterior;
			fposterior.put("id", _frequest.get<string>("id"));
			fposterior.put("type", "data");
			
			ptree fpopulations;
			Sample all("summary");
			for(auto& population : _frequest.get_child("populations")){
				Sample p(Ploidy(_frequest.get<uint32_t>("ploidy")), population.second, _frequest);
				fpopulations.push_back(std::make_pair("", p.indices()));
				all.merge(&p);
			}
			fpopulations.push_back(std::make_pair("", all.indices()));

			fposterior.push_back(make_pair("populations", fpopulations));

			_frequest.push_back(make_pair("posterior", fposterior));
			_data[id] = _frequest;
			_data_indices.emplace(id, map<string, map<uint32_t, map<uint32_t, map<string, double>>>>{});
			parseIndices(_data[id].get_child("posterior"), _data_indices[id], true);
			
			db_comm.writeData(_frequest);

			break;
		};
		default: {
			cerr << "Unknown Result Type" << endl;
			exit(EXIT_FAILURE);
		};
	}
	return(_frequest);
}

ptree Analyzer::run(const std::string &_body){
	cout << "Analyzer::run - Start Data\n";
	ptree fsettings;
	
	uint32_t id = incremental_id++;
	
	// global_mutex.lock();
	ofstream escritor(id_file, fstream::trunc | fstream::out);
	escritor << incremental_id << "\n";
	escritor.close();
	// global_mutex.unlock();
	
	{
		string out_file = logs_path + "/run_body_";
		out_file += std::to_string(id);
		out_file += ".log";
		ofstream escritor(out_file, ofstream::app);
		if( escritor.is_open() && escritor.good() ){
			escritor<<_body<<"\n";
			escritor.close();
		}
	}
	
	cout << "Analyzer::run - MultipartFormParser...\n";
	MultipartFormParser m(_body);
	cout << "Analyzer::run - MultipartFormParser finished\n";
	
	fsettings.put("max-number-of-simulations", m.get("max-number-of-simulations"));
	m.remove("max-number-of-simulations");
	
	fsettings.put("name", m.get("simulation-name"));
	m.remove("simulation-name");
	
	fsettings.put("user", m.get("user"));
	m.remove("user");
	
	fsettings.put("type", "init");
	fsettings.put("batch-size", "10000");
	fsettings.put("population-increase-phases", "0");
	fsettings.put("feedback", "0");

	uint32_t ploidy = boost::lexical_cast<uint32_t>(m.get("ploidy"));
	m.remove("ploidy");

	fsettings.put("similarity-threshold", m.get("similarity-threshold"));
	m.remove("similarity-threshold");

	cout << "Analyzer::run - Setting id and timestamp\n";
	
	uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	fsettings.put("id", std::to_string(id));
	fsettings.put("timestamp", std::to_string(timestamp));
	
	cout << "Analyzer::run - id: " << id << ", timestamp: " << timestamp << "\n";

	map<uint32_t, map<uint32_t, map<uint32_t, vector<Marker>>>> samples;

	while(!m.empty()) {
		uint32_t sample = boost::lexical_cast<uint32_t>(m.get("Sample"));
		m.remove("Sample");

		uint32_t chromosome = boost::lexical_cast<uint32_t>(m.get("Chromosome"));
		m.remove("Chromosome");

		uint32_t gene = boost::lexical_cast<uint32_t>(m.get("Gene"));
		m.remove("Gene");

		uint32_t markertype = boost::lexical_cast<uint32_t>(m.get("MarkerType"));
		m.remove("MarkerType");

		uint32_t filetype = boost::lexical_cast<uint32_t>(m.get("FileType"));
		m.remove("FileType");

		cout << "Analyzer::run - Preparing FileParser\n";
		FileParser fp(m.get("File"), FileType(filetype), MarkerType(markertype));
		m.remove("File");

		cout << "Analyzer::run - fp.parse...\n";
		vector<Marker> markers = fp.parse();
		cout << "Analyzer::run - fp.parse finished\n";
		samples[sample][chromosome][gene] = markers;
	}

	cout << "Analyzer::run - Preparing samples\n";
	ptree fsamples;
	for(auto& sample : samples){
		ptree fsample;
		fsample.put("name","sample" + std::to_string(sample.first));
		fsamples.push_back(std::make_pair("", fsample));
	}

	cout << "Analyzer::run - Preparing Profile\n";
	ptree findividual = getProfile(samples, ploidy);
	
	fsettings.add_child("samples", fsamples);
	fsettings.add_child("individual", findividual);

	cout << "Analyzer::run - Preparing statistics\n";
	finished[id] = 0;

	ptree fdata, fposterior;
	fdata.put("id", fsettings.get<std::string>("id"));
	fdata.put("type", "data");

	ptree fpopulations;
	Sample all("summary");
	for(auto& sample : samples){
		Sample p("sample" + std::to_string(sample.first), sample.second, fsettings);
		fpopulations.push_back(std::make_pair("", p.indices()));
		all.merge(&p);
	}
	fpopulations.push_back(std::make_pair("", all.indices()));
	fposterior.push_back(make_pair("populations", fpopulations));
	fdata.push_back(make_pair("posterior", fposterior));
	
	_data[id] = fdata;
	_data_indices.emplace(id, map<string, map<uint32_t, map<uint32_t, map<string, double>>>>{});
	parseIndices(_data[id].get_child("posterior"), _data_indices[id]);
	
	cout << "Analyzer::run - Storing data\n";
	db_comm.writeData(fdata);
	
	std::stringstream ss;
	write_json(ss,fdata);
	cout << ss.str() << endl;

	cout << "Analyzer::run - End\n";
	return(fsettings);
}

ptree Analyzer::getProfile(const map<uint32_t,map<uint32_t,map<uint32_t,vector<Marker>>>> &_samples,const uint32_t &_ploidy) {
	ptree findividual;

	findividual.put("ploidy", std::to_string(_ploidy));

	for(auto& sample : _samples){
		ptree fchromosomes;
		for(auto& chromosome : sample.second){
			ptree fchromosome;
			fchromosome.put("id",chromosome.first);

			ptree fgenes;
			for(auto& gene : chromosome.second){
				ptree fgene;
				fgene.put("id",gene.first);

				Marker m=gene.second.back();
				fgene.put("type",std::to_string(uint32_t(m.type())));
				switch(m.type()){
					case SEQUENCE:{
						fgene.put("nucleotides", std::to_string(m.sequence()->data().length()));
						break;
					}
					case MICROSATELLITE:{
						fgene.put("number-of-repeats", std::to_string(m.microsatellite()->repeats()));
						fgene.put("tandem-length", std::to_string(m.microsatellite()->tandem().length()));
						break;
					}
					default:{
						std::cerr << "Error::Unknown marker type: " << m.type() << std::endl;
						exit(EXIT_FAILURE);
					}
				}
				fgenes.push_back(std::make_pair("", fgene));
			}
			fchromosome.add_child("genes",fgenes);
			fchromosomes.push_back(std::make_pair("", fchromosome));
		}
		findividual.add_child("chromosomes",fchromosomes);
		break;
	}
	return(findividual);
}


