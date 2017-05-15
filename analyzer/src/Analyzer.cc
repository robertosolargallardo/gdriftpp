#include "Analyzer.h"

string Analyzer::log_file = "logs/analyzer.log";

Analyzer::Analyzer(boost::property_tree::ptree &fhosts) : Node(fhosts){
	db_comm = DBCommunication(fhosts.get<string>("database.uri"), fhosts.get<string>("database.name"), fhosts.get<string>("database.collections.data"), fhosts.get<string>("database.collections.results"), fhosts.get<string>("database.collections.settings"), fhosts.get<string>("database.collections.training"));
	
	// Prueba de toma de resultados (para fase de entrenamiento)
//	list<boost::property_tree::ptree> results;
//	_mongo->readStatistics(results, "gdrift", "results", 0, 1, 3);
//	
//	unsigned int count = 0;
//	for(auto &json : results){
//		if(count >= 3) break;
//		cout<<"Res["<<count++<<"]:\n";
//		map<string, map<uint32_t, map<uint32_t, map<string, double>>>> statistics_map;
//		parseIndices(json.get_child("posterior"), statistics_map);
//		for(auto i : statistics_map){
//			for(auto j : i.second){
//				for(auto k : j.second){
//					for(auto l : k.second){
//						double value = statistics_map[i.first][j.first][k.first][l.first];
//						cout<<"statistics_map["<<i.first<<"]["<<j.first<<"]["<<k.first<<"]["<<l.first<<"]: "<<value<<"\n";
//					}
//				}
//			}
//		}
//	}
	
}
Analyzer::~Analyzer(void){
}

unsigned int Analyzer::parseIndices(const boost::property_tree::ptree &json, map<string, map<uint32_t, map<uint32_t, map<string, double>>>> &indices){
	unsigned int inserts = 0;
	for(auto& p : json.get_child("populations")){
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
	return inserts;
}

double Analyzer::distance(uint32_t id, const boost::property_tree::ptree &_simulated){
	
	map<string, map<uint32_t, map<uint32_t, map<string, double>>>> indices_simulated;
	parseIndices(_simulated, indices_simulated);
	
	double a=0.0, b=0.0, s=0.0, n=0.0, diff=0.0;
	map<string, vector<double>> deltas;
	for(auto i : _data_indices[id]){
		for(auto j : i.second){
			for(auto k : j.second){
				for(auto l : k.second){
					a = _data_indices[id][i.first][j.first][k.first][l.first];
					b = indices_simulated[i.first][j.first][k.first][l.first];
					if(a == 0){
						continue;
					}
					diff = (a>b)?(a-b):(b-a);
					// Por ahora voy a calcular la distancia normalizando cada indice por el target
					// Podria usarse la normalizacion del par, o por un maximo o varianza empirica, o idealmente por un maximo exacto (eso depende del estadistico)
					diff /= a;
					s += diff * diff;
					++n;
//					cout<<"Analyzer::distance - s: "<<s<<", diff: "<<diff<<" (a: "<<a<<", b: "<<b<<", "<<l.first<<")\n";
				}
			}
		}
	}
	
//	cout<<"Analyzer::distance - n: "<<n<<", s: "<<s<<", dist: "<<sqrt(s/n)<<"\n";
	// Notar que si se retorna inf aqui, entonces NO habia estadisticos validos
	// En ese caso, la distancia no deberia ser considerada en lo absoluto (por lo que ES correcto retornar inf u otra marca)
	return sqrt(s/n);
}

bool Analyzer::trainModel(uint32_t id, uint32_t scenario_id, uint32_t feedback, uint32_t max_feedback, boost::property_tree::ptree &fresponse, map<string, vector<double>> &estimations_map){
	cout<<"Analyzer::trainModel - Inicio ("<<id<<", "<<scenario_id<<", "<<feedback<<" / "<<max_feedback<<")\n";
	
//	vector<string> fields = db_comm.getFields(id, scenario_id, feedback);
//	cout<<"Analyzer::trainModel - Fields: \n";
//	for(unsigned int i = 0; i < fields.size(); ++i){
//		cout<<"\""<<fields[i]<<"\"\n";
//	}
	
	// Los parametros del create (size de la poblacion inicial) deberia omitirse durante las fases de crecimiento de poblacion
	map<uint32_t, vector<string>> events_params = db_comm.getEventsParams(id, scenario_id, (feedback < max_feedback));
	
	map<string, uint32_t> params_positions;
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
	for(auto &c : fresponse.get_child("individual.chromosomes")){
		uint32_t cid = c.second.get<uint32_t>("id");
		for(auto g : c.second.get_child("genes")){
			uint32_t gid = g.second.get<uint32_t>("id");
			string param_name = "chromosomes.";
			param_name += std::to_string(cid);
			param_name += ".genes.";
			param_name += std::to_string(gid);
			param_name += ".mutation.rate";
			params_positions.emplace(param_name, 0);
		}
	}
	// Posiciones
	unsigned int count = 0;
	for(map<string, uint32_t>::iterator it = params_positions.begin(); it != params_positions.end(); it++){
		it->second = count++;
		cout<<"Analyzer::trainModel - params_positions["<<it->first<<"]: "<<it->second<<"\n";
	}
	
	vector<vector<double>> params;
	vector<vector<double>> statistics;
	// POR AHORA ESTOY FIJANDO EL NUMERO DE ESTADISTICOS EN 15
	// Obviamente eso hay que decidirlo de mejor manera
	db_comm.getResults(id, scenario_id, feedback, params, events_params, params_positions.size(), statistics, 15);
	
	// Por ahora, asumimos que las distribuciones son normales
	// De ese modo, la distribucion de cada parametro se representa por la media y la varianza
	// El metodo entonces debe entregar exactamente params_positions.size() pares de valores
	// Luego, itero por el escenario en fresponse
	// En cada evento y chromosoma, genero el string absoluto de la ruta del parametro, y reemplazo los valores con los de la nueva distribucion
	
	bool finish = false;
	vector< pair<double, double> > res_dist;
	
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
	
	// Prueba de objeto estadistico
	cout<<"Analyzer::trainModel - Probando objeto estadistico\n";
	
	/**** Comienzo analisis *****/ 
	SimulationStatistics statsAnalisis;/*Declaracion de Objeto statsAnalisis*/
	cout<<"Analyzer::trainModel - storeTarget...\n";
	statsAnalisis.storeTarget(target);/*Almacena target*/ 
	cout<<"Analyzer::trainModel - loadData...\n";
	statsAnalisis.loadData(statistics, params); /*Almacena estadisticos y parametros*/ 
	int medidaDistancia = 4;
	int opcionNormalizar = 1;
	cout<<"Analyzer::trainModel - computeDistances...\n";
	statsAnalisis.computeDistances(medidaDistancia, opcionNormalizar);/*Calcula distancias*/
//	statsAnalisis.selectSample(1.0);
	cout<<"Analyzer::trainModel - selectSample...\n";
	double threshold = statsAnalisis.selectSample(0.1);/*Selecciona muestra segun porcentaje de datos ej: porcentajeSelection=0.1 (10%) esto se deja como opcion en la interfaz del frontend*/
	
	ofstream escritor(log_file, ofstream::app);
	escritor<<"simulation "<<id<<", scenario "<<scenario_id<<", feedback "<<feedback<<", threshold "<<threshold<<"\n";
	
	int tipoDistribucion = 0;
	cout<<"Analyzer::trainModel - distPosterior...\n";
	statsAnalisis.distPosterior(tipoDistribucion);/*Obtiene la distribucion posterior*/ 
	/**** Fin analisis *****/
	
	cout<<"Analyzer::trainModel - Extrayendo resultados de "<<params_positions.size()<<" parametros\n";
	for(map<string, uint32_t>::iterator it = params_positions.begin(); it != params_positions.end(); it++){
		unsigned int opcionGraficoOut = it->second;
		string nombre = it->first;
		vector<double> dataGrafica;
		FuncionDensidad dist = statsAnalisis.getDistribution(opcionGraficoOut);
		cout<<"Analyzer::trainModel - Mostrando resultados["<<opcionGraficoOut<<"] ("<<nombre<<", med: "<<dist.sampleMediana<<", std: "<<dist.sampleStd<<")\n";
		//Por ahora se usa el mismo numero de puntos que muestra, lunes os comento
		unsigned int dimG = dist.samplePostNormalized.size();
		//Parametro con numeracion:opcionGraficoOut
		dist.outVectorGrafico1(dimG, dataGrafica);
//		for(unsigned int i = 0; i < dataGrafica.size(); ++i){
//			cout<<"grafico["<<i<<"]: "<<dataGrafica[i]<<"\n";
//		}
		estimations_map[nombre] = dataGrafica;
		
		res_dist.push_back( pair<double, double>(dist.sampleMediana, dist.sampleStd) );
		
		// Estos datos tambien podrian retornarse al llamador (para agregarlos al resultado de training)
//		distribution_map[nombre] = pair<double, double>( dist.sampleMediana, dist.sampleStd );
		
	}
	
	if(finish){
		cout<<"Analyzer::trainModel - Señal de parada, preparando mensaje y saliendo\n";
		return true;
	}
	
	cout<<"Analyzer::trainModel - Actualizando Genes\n";
	for(auto &c : fresponse.get_child("individual.chromosomes")){
		uint32_t cid = c.second.get<uint32_t>("id");
		for(auto &g : c.second.get_child("genes")){
			uint32_t gid = g.second.get<uint32_t>("id");
			string param_name = "chromosomes.";
			param_name += std::to_string(cid);
			param_name += ".genes.";
			param_name += std::to_string(gid);
			param_name += ".mutation.rate";
			cout<<"Analyzer::trainModel - Modificando "<<param_name<<" (position "<<params_positions[param_name]<<")\n";
			g.second.get_child("mutation.rate.distribution").put<string>("type", "normal");
			// Elimino parametros previos
			g.second.get_child("mutation.rate.distribution").erase("params");
			g.second.get_child("mutation.rate.distribution").put_child("params", boost::property_tree::ptree());
			g.second.get_child("mutation.rate.distribution.params").put<double>("mean", res_dist[params_positions[param_name]].first);
			g.second.get_child("mutation.rate.distribution.params").put<double>("stddev", res_dist[params_positions[param_name]].second);
		}
	}
	
	cout<<"Analyzer::trainModel - Actualizando Eventos\n";
	for(auto &s : fresponse.get_child("scenarios")){
		uint32_t s_id = s.second.get<uint32_t>("id");
		if(s_id == scenario_id){
			for(auto &e : s.second.get_child("events")){
				// En principio cada evento tiene timestamp y parametros
				// Los parametros que tengan type random deben ser agregados
				uint32_t eid = e.second.get<uint32_t>("id");
				string etype = e.second.get<string>("type");
//				cout<<"Analyzer::trainModel - Evento "<<eid<<" ("<<etype<<")\n";
				
				string param_name = "events.";
				param_name += std::to_string(eid);
				param_name += ".timestamp";
				map<string, uint32_t>::iterator it = params_positions.find(param_name);
				if( it != params_positions.end() ){
					cout<<"Analyzer::trainModel - Modificando "<<param_name<<" (position "<<it->second<<")\n";
					e.second.get_child("timestamp.distribution").put<string>("type", "normal");
					// Elimino parametros previos
					e.second.get_child("timestamp.distribution").erase("params");
					e.second.get_child("timestamp.distribution").put_child("params", boost::property_tree::ptree());
					e.second.get_child("timestamp.distribution.params").put<double>("mean", res_dist[params_positions[param_name]].first);
					e.second.get_child("timestamp.distribution.params").put<double>("stddev", res_dist[params_positions[param_name]].second);
				}
				
				param_name = "events.";
				param_name += std::to_string(eid);
				param_name += ".params.population.size";
				it = params_positions.find(param_name);
				if( it != params_positions.end() ){
					cout<<"Analyzer::trainModel - Modificando "<<param_name<<" (position "<<it->second<<")\n";
					e.second.get_child("params.population.size.distribution").put<string>("type", "normal");
					// Elimino parametros previos
					e.second.get_child("params.population.size.distribution").erase("params");
					e.second.get_child("params.population.size.distribution").put_child("params", boost::property_tree::ptree());
					e.second.get_child("params.population.size.distribution.params").put<double>("mean", res_dist[params_positions[param_name]].first);
					e.second.get_child("params.population.size.distribution.params").put<double>("stddev", res_dist[params_positions[param_name]].second);
				}
				
				param_name = "events.";
				param_name += std::to_string(eid);
				param_name += ".params.source.population.percentage";
				it = params_positions.find(param_name);
				if( it != params_positions.end() ){
					cout<<"Analyzer::trainModel - Modificando "<<param_name<<" (position "<<it->second<<")\n";
					e.second.get_child("params.source.population.percentage.distribution").put<string>("type", "normal");
					// Elimino parametros previos
					e.second.get_child("params.source.population.percentage.distribution").erase("params");
					e.second.get_child("params.source.population.percentage.distribution").put_child("params", boost::property_tree::ptree());
					e.second.get_child("params.source.population.percentage.distribution.params").put<double>("mean", res_dist[params_positions[param_name]].first);
					e.second.get_child("params.source.population.percentage.distribution.params").put<double>("stddev", res_dist[params_positions[param_name]].second);
				}
				
//				cout<<"Analyzer::trainModel - Evento resultante:\n";
//				std::stringstream ss;
//				write_json(ss, e.second);
//				cout<<ss.str()<<"\n";
				
			}
		}
	}
	
	cout<<"Analyzer::trainModel - Fin\n";
	return false;
}

boost::property_tree::ptree Analyzer::run(boost::property_tree::ptree &_frequest){
	
	uint32_t id = _frequest.get<uint32_t>("id");
	
	switch(util::hash(_frequest.get<string>("type"))){
		case SIMULATED:{

//			std::stringstream ss;
//			write_json(ss, _frequest);
//			cout<<ss.str()<<"\n";
			
			// Nodo de prueba para buscar atributos
			boost::optional<boost::property_tree::ptree&> test_child;
			
			uint32_t scenario_id = 0;
			test_child = _frequest.get_child_optional("scenario.id");
			if( test_child ){
				scenario_id = _frequest.get<uint32_t>("scenario.id");
			}
			pair<uint32_t, uint32_t> id_pair(id, scenario_id);
			
			uint32_t batch_size = 0;
			test_child = _frequest.get_child_optional("batch-size");
			if( test_child ){
				batch_size = _frequest.get<uint32_t>("batch-size");
			}
			
//			// Solo agrego batch_size[id] como valor inicial la primera vez, de ahi en adelante se mantiene la suma de feedback * batch_size[i];
			this->next_batch.emplace(id, batch_size);
			
			unsigned int feedback = 0;
			test_child = _frequest.get_child_optional("feedback");
			if( test_child ){
				feedback = _frequest.get<uint32_t>("feedback");
			}
			
//			cout<<"Analyzer::run - SIMULATED (id: "<<id<<", scenario: "<<scenario_id<<", _batch_size[id]: "<<_batch_size[id]<<", feedback: "<<feedback<<")\n";
			cout<<"Analyzer::run - SIMULATED (id: "<<id<<", scenario: "<<scenario_id<<", feedback: "<<feedback<<")\n";
			
			if(finished.count(id)==0) return(_frequest);
			
			finished[id]++;
			
			// Solo calcular distancia y guardar resultado si NO hay error
			
			uint32_t error = 0;
			test_child = _frequest.get_child_optional("error");
			if( test_child ){
				error = _frequest.get<uint32_t>("error");
			}
			
			// Verificar si este feedback ya fue entrenado, quizas sea mejor descartar esos resultados
			// if( db_comm.alreadyTrained(id, scenario_id, feedback) ){
			// 	error = 10;
			// }
			
			if(error == 0){
				double dist = distance(id, _frequest.get_child("posterior"));
				cout<<"Analyzer::run - dist: "<<dist<<"\n";
				_frequest.put("distance", dist);
				db_comm.writeResults(_frequest);
			}
			else{
				cout<<"Analyzer::run - Error en simulacion detectado (error: "<<error<<")\n";
			}
			
			// La idea aqui es que, cuando se tengan suficientes resultados para la simulacion id
			// ...se ejecute el algoritmo de entrenamiento y ajuste de parametros
			// ...los parametros nuevos se le pasan al scheduler con un reload y un continue
			// Si aun faltan simulaciones, simplemente se continua
			// Este modulo o el modulo de estadisticas debe considerar el crecimiento de la poblacion
			// Eso podria almacenarse agregando otro valor al id de las simulaciones, como el ciclo de realimentacion
			// Un valor feedback podria partir en 0 e incrementarse en 1 en cada fase de reload
			// El tamaño de la poblacion podria escalarse por una funcion de feedback y el numero de iteraciones de entrenamiento esperado
			
			// Notar que feedback depende de simulacion Y ESCENARIO
			// una opcion es indexar la informacion por [id][scenario_id] (o pair de ambos)
			
//			cout<<"Analyzer::run - Batch "<<_batch_size[id]<<" / "<<BATCH_LENGTH*this->_fhosts.get_child("controller").size()<<", Finished: "<<finished[id]<<" / "<<next_batch[id]<<"\n";
			cout<<"Analyzer::run - Finished: "<<finished[id]<<" / "<<next_batch[id]<<"\n";
			
			
			boost::property_tree::ptree fresponse;
			fresponse.put("id", id);
			
			if( finished[id] >= _frequest.get<uint32_t>("max-number-of-simulations") ){
				cout<<"Analyzer::run - Preparando finalize\n";
				finished.erase(finished.find(id));
//				this->_batch_size.erase(this->_batch_size.find(id));
				fresponse.put("type", "finalize");
				comm::send(this->_fhosts.get<string>("scheduler.host"), this->_fhosts.get<string>("scheduler.port"), this->_fhosts.get<string>("scheduler.resource"), fresponse);
			}
			else if( finished[id] >= this->next_batch[id] ){
				cout<<"Analyzer::run - Feedback iniciado (batch_size: "<<batch_size<<")\n";
				// Codigo de feedback, preparacion de nuevos parametros
//				this->next_batch[id_pair] += batch_size;
				this->next_batch[id] += batch_size;
				cout<<"Analyzer::run - Preparando reload (proximo feedback en simulacion "<<this->next_batch[id]<<")\n";
				
				// fresponse debe contener un documento completo de settings
				// La idea es cargar los settings de id, feedback, y luego agregar los parametros nuevos
				// La otra forma, es pasarle el settings al modulo de entrenamiento para que actualice los parametros
				// Notar que con esto estoy REEMPLAZANDO fresponse (pero el id tambien se incluye)
				fresponse = db_comm.readSettings(id, feedback);
				
				// Creo que esto hay que hacerlo para CADA escenario del setting
				// Eso es debido a que, por ahora, feedback se aplica a la simulacion completa
				// Iterar por cada scenario_id de la simulacion
				// Notar que dejo el ciclo aqui (en lugar de en trainModel) pues en la version final, deberia entrenarse solo el escenario actual
				// trainModel PODRIA usar max_feedback para algo
				// Por ahora lo usara para descartar events.create.size durante las fases de crecimiento de poblacion
				uint32_t max_feedback = 0;
				test_child = fresponse.get_child_optional("population-increase-phases");
				if( test_child ){
					max_feedback = fresponse.get<uint32_t>("population-increase-phases");
				}
				
				// Por seguridad (del iterador) primero extraigo los scenario.id de fresponse
				vector<uint32_t> s_ids;
				for(auto s : fresponse.get_child("scenarios")){
					uint32_t s_id = s.second.get<uint32_t>("id");
					s_ids.push_back(s_id);
				}
				bool finish = false;
				
				boost::property_tree::ptree scenarios;
				for(unsigned int i = 0; i < s_ids.size() && !finish; ++i){
					// Notar que es valido pasarle el mismo ptree fresponse para cada escenario
					// Eso es por que cada llamada a trainModel SOLO REEMPLAZA LOS VALORES DEL ESCENARIO DADO
					// Al final del ciclo, todos los escenarios han sido actualizados en fresponse
					map<string, vector<double>> estimations_map;
					finish = trainModel(id, s_ids[i], feedback, max_feedback, fresponse, estimations_map);
					
					// Agregar estimations_map al json de resultados de entrenamiento
					boost::property_tree::ptree scenario;
					string name = "Scenario " + std::to_string(s_ids[i]);
					scenario.put("name", name);
					scenario.put("id", s_ids[i]);
					boost::property_tree::ptree estimations;
					
					for(map<string, vector<double>>::iterator it = estimations_map.begin(); it != estimations_map.end(); it++){
					
						boost::property_tree::ptree estimation;
						estimation.put("parameter", it->first);
						boost::property_tree::ptree fvalue;
						boost::property_tree::ptree fvalues;
						for( unsigned int j = 0; j < it->second.size(); ++j ){
							fvalue.put("y", it->second[j]);
							fvalues.push_back(make_pair("", fvalue));
						}
						estimation.add_child("values", fvalues);
						estimations.push_back(make_pair("", estimation));
					}
					scenario.add_child("estimations", estimations);
					scenarios.push_back(make_pair("", scenario));
					
					scenario.add_child("estimations", estimations);
					
					estimations_map.clear();
				}
				
				boost::property_tree::ptree estimations;
				estimations.add_child("scenarios", scenarios);
				
				boost::property_tree::ptree training_results;
				training_results.put("id", id);
				training_results.add_child("estimations", estimations);
				
				db_comm.storeTrainingResults(training_results);
				
				// Almacenar el json de resultados de entrenamiento
				
				if(finish){
					cout<<"Analyzer::run - Preparando finalize\n";
					finished.erase(finished.find(id));
					fresponse.put("type", "finalize");
				}// if... simulacion terminada
				else{
					fresponse.put("type", "reload");
					fresponse.put("feedback", 1 + feedback);
					
//					cout<<"Analyzer::run - Enviando settings a scehduler\n";
//					std::stringstream ss;
//					write_json(ss, fresponse);
//					cout << ss.str() << endl;
					
					comm::send(this->_fhosts.get<string>("scheduler.host"), this->_fhosts.get<string>("scheduler.port"), this->_fhosts.get<string>("scheduler.resource"), fresponse);
					// Enviar nuevos parametros al scheduler
					cout<<"Analyzer::run - Preparando continue\n";
//					this->_batch_size[id] = 0;
					fresponse.put("type", "continue");
				}// else... Continue
				comm::send(this->_fhosts.get<string>("scheduler.host"), this->_fhosts.get<string>("scheduler.port"), this->_fhosts.get<string>("scheduler.resource"), fresponse);
			}
			break;
		};
		case DATA:  {
			finished[id] = 0;
			
			boost::property_tree::ptree fposterior;
			fposterior.put("id", _frequest.get<string>("id"));
			fposterior.put("type", "data");

			boost::property_tree::ptree fpopulations;
			Sample all("summary");
			for(auto& population : _frequest.get_child("populations")){
				Sample p(Ploidy(_frequest.get<uint32_t>("ploidy")), population.second, _frequest);
				fpopulations.push_back(std::make_pair("", p.indices()));
				all.merge(&p);
			}
			fpopulations.push_back(std::make_pair("", all.indices()));

			fposterior.push_back(make_pair("populations", fpopulations));

			_frequest.push_back(make_pair("posterior",fposterior));
			this->_data[id] = _frequest;
			_data_indices.emplace(id, map<string, map<uint32_t, map<uint32_t, map<string, double>>>>{});
			parseIndices(this->_data[id].get_child("posterior"), _data_indices[id]);

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




