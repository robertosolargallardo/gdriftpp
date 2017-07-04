#include "Analyzer.h"

//string Analyzer::log_file = "logs/analyzer.log";
uint32_t Analyzer::update_results = 100;

Analyzer::Analyzer(boost::property_tree::ptree &fhosts) : Node(fhosts){

	cout<<"Analyzer - Inicio\n";
	
	db_comm = DBCommunication(fhosts.get<string>("database.uri"), fhosts.get<string>("database.name"), fhosts.get<string>("database.collections.data"), fhosts.get<string>("database.collections.results"), fhosts.get<string>("database.collections.settings"), fhosts.get<string>("database.collections.training"));
	
	// Default del path
	logs_path = "~";
	boost::optional<boost::property_tree::ptree&> test_child;
	test_child = fhosts.get_child_optional("globals.logs");
	if( test_child ){
		logs_path = fhosts.get<string>("globals.logs");
	}
	// Archivos de log
	log_file = logs_path + "/analyzer.log";
	id_file = logs_path + "/analyzer_id.log";
	cout<<"Analyzer - logs_path: "<<logs_path<<", log_file: "<<log_file<<", id_file: "<<id_file<<"\n";
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
//	fstream lector(id_file, fstream::in | fstream::binary);
//	if( lector.is_open() && lector.good() ){
//		lector.read((char*)&incremental_id, sizeof(int));
//		lector.close();
//	}
	cout<<"Analyzer - Fin (incremental_id: "<<incremental_id<<")\n";
}
Analyzer::~Analyzer(void){
}

unsigned int Analyzer::parseIndices(const boost::property_tree::ptree &posterior, map<string, map<uint32_t, map<uint32_t, map<string, double>>>> &indices, bool mostrar){
	unsigned int inserts = 0;
	boost::optional<const boost::property_tree::ptree&> test_child;
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
		cout<<"Analyzer::parseIndices - Statistics:\t";
		for(auto p : indices){
			for(auto c : p.second){
				for(auto g : c.second){
					for(auto i : g.second){
						cout<<i.second<<"\t";
					}
				}
			}
		}
		cout<<"\n";
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

// Este metodo debe ser resistente a concurrencia
//	- Asumo que las llamadas a db_comm son thread safe (dependen de Mongo)
bool Analyzer::trainModel(uint32_t id, uint32_t scenario_id, uint32_t feedback, uint32_t max_feedback, boost::property_tree::ptree &settings, map<string, Distribution> &posterior_map, map<string, Distribution> &adjustment_map, map<string, map<string, double>> &statistics_map){
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
	for(auto &c : settings.get_child("individual.chromosomes")){
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
	vector<vector<double>> stats;
	db_comm.getResults(id, scenario_id, feedback, params, events_params, stats);
	
	// Los minimos y maximos (para normalizar y otras cosas) los puedo leer de la base de datos, o calcular
	vector<double> min_stats;
	vector<double> max_stats;
	vector<double> min_params;
	vector<double> max_params;
	Statistics::getMinMax(params, min_params, max_params);
	Statistics::getMinMax(stats, min_stats, max_stats);
	
	// Por ahora, asumimos que las distribuciones son normales
	// De ese modo, la distribucion de cada parametro se representa por la media y la varianza
	// El metodo entonces debe entregar exactamente params_positions.size() pares de valores
	// Luego, itero por el escenario en settings
	// En cada evento y chromosoma, genero el string absoluto de la ruta del parametro, y reemplazo los valores con los de la nueva distribucion
	
	bool finish = false;
	
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
	SimulationStatistics statsAnalisis;
	cout<<"Analyzer::trainModel - loadData...\n";
	statsAnalisis.loadData(stats, params, target);
	int medidaDistancia = 4;
	int opcionNormalizar = 1;
	cout<<"Analyzer::trainModel - computeDistances...\n";
	// Antes de esto abria que leer o calcular y guardar los min/max por stat
	statsAnalisis.computeDistances(medidaDistancia, opcionNormalizar, min_stats, max_stats);
	cout<<"Analyzer::trainModel - selectSample...\n";
	double min_dist = 0;
	double max_dist = 0;
	double mean_dist = 0;
	double worst_dist = 0;
	/*Selecciona muestra segun porcentaje de datos ej: porcentajeSelection=0.1 (10%) esto se deja como opcion en la interfaz del frontend*/
	unsigned int used_data = statsAnalisis.selectSample(0.1, min_dist, max_dist, mean_dist);
	
	worst_dist = statsAnalisis.getWorstDistance();
	
	ofstream escritor(log_file, ofstream::app);
	if( escritor.is_open() && escritor.good() ){
		escritor<<"simulation "<<id<<", scenario "<<scenario_id<<", feedback "<<feedback<<", min_dist "<<min_dist<<", max_dist "<<max_dist<<", mean_dist "<<mean_dist<<", worst_dist: "<<worst_dist<<"\n";
		escritor.close();
	}
	
	int tipoDistribucion = 0;
	cout<<"Analyzer::trainModel - distPosterior...\n";
	/*Obtiene la distribucion posterior*/ 
	statsAnalisis.distPosterior(tipoDistribucion);
	
	cout<<"Analyzer::trainModel - ajustarWLS...\n";
	statsAnalisis.ajustarWLS();
	
	/**** Fin analisis *****/
	
	cout<<"Analyzer::trainModel - Extrayendo resultados de "<<params_positions.size()<<" parametros\n";
	for(map<string, uint32_t>::iterator it = params_positions.begin(); it != params_positions.end(); it++){
		unsigned int pos_param = it->second;
		string nombre = it->first;
		Distribution dist = statsAnalisis.getDistribution(pos_param);
		cout<<"Analyzer::trainModel - Mostrando resultados["<<pos_param<<"] ("<<nombre<<", med: "<<dist.getValue(0)<<", std: "<<dist.getValue(1)<<")\n";
		
		posterior_map[nombre] = dist;
		
		statistics_map[nombre]["median"] = dist.getSampleMedian();
		statistics_map[nombre]["mean"] = dist.getSampleMean();
		statistics_map[nombre]["stddev"] = dist.getSampleStddev();
		statistics_map[nombre]["var"] = dist.getSampleVar();
		statistics_map[nombre]["min"] = dist.getSampleMin();
		statistics_map[nombre]["max"] = dist.getSampleMax();
		statistics_map[nombre]["used_data"] = used_data;
		statistics_map[nombre]["threshold"] = max_dist;
		
		// Distribucion de Ajuste
		double adjust_median = Statistics::getMean(statsAnalisis.getAdjustmentData(pos_param));
		double adjust_var = Statistics::getVariance(statsAnalisis.getAdjustmentData(pos_param));
		double adjust_stddev = pow(adjust_var, 0.5);
		cout<<"Analyzer::trainModel - Ajuste ("<<used_data<<") med: "<<adjust_median<<", std: "<<adjust_stddev<<"\n";
		adjustment_map[nombre] = Distribution("normal", adjust_median, adjust_stddev);
		
	}
	
	if(finish){
		cout<<"Analyzer::trainModel - Señal de parada, preparando mensaje y saliendo\n";
		return true;
	}
	
	cout<<"Analyzer::trainModel - Actualizando Genes\n";
	for(auto &c : settings.get_child("individual.chromosomes")){
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
			g.second.get_child("mutation.rate.distribution.params").put<double>("mean", posterior_map[param_name].getValue(0));
			g.second.get_child("mutation.rate.distribution.params").put<double>("stddev", posterior_map[param_name].getValue(1));
		}
	}
	
	cout<<"Analyzer::trainModel - Actualizando Eventos\n";
	for(auto &s : settings.get_child("scenarios")){
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
					e.second.get_child("timestamp.distribution.params").put<double>("mean", posterior_map[param_name].getValue(0));
					e.second.get_child("timestamp.distribution.params").put<double>("stddev", posterior_map[param_name].getValue(1));
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
					e.second.get_child("params.population.size.distribution.params").put<double>("mean", posterior_map[param_name].getValue(0));
					e.second.get_child("params.population.size.distribution.params").put<double>("stddev", posterior_map[param_name].getValue(1));
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
					e.second.get_child("params.source.population.percentage.distribution.params").put<double>("mean", posterior_map[param_name].getValue(0));
					e.second.get_child("params.source.population.percentage.distribution.params").put<double>("stddev", posterior_map[param_name].getValue(1));
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

// Esto es solo para debug
//unsigned int training_id = 0;

// Este codigo deberia ser resistente a concurrencia
//	- Asumo que todas las operaciones de db_comm son thread safe (dependen de Mongo)
//	- trainModel debe ser thread safe
boost::property_tree::ptree Analyzer::updateTrainingResults(uint32_t id, uint32_t feedback, bool &finish){

	// fresponse debe contener un documento completo de settings
	// La idea es cargar los settings de id, feedback, y luego agregar los parametros nuevos
	// La otra forma, es pasarle el settings al modulo de entrenamiento para que actualice los parametros
	// Notar que con esto estoy REEMPLAZANDO fresponse (pero el id tambien se incluye)
	// Por ultimo, notar que hay que guardar las distribuciones a priori antes de modificarlo
	boost::property_tree::ptree settings = db_comm.readSettings(id, feedback);
	boost::optional<boost::property_tree::ptree&> test_child;
	
	// Creo que esto hay que hacerlo para CADA escenario del setting
	// Eso es debido a que, por ahora, feedback se aplica a la simulacion completa
	// Iterar por cada scenario_id de la simulacion
	// Notar que dejo el ciclo aqui (en lugar de en trainModel) pues en la version final, deberia entrenarse solo el escenario actual
	// trainModel PODRIA usar max_feedback para algo
	// Por ahora lo usara para descartar events.create.size durante las fases de crecimiento de poblacion
	uint32_t max_feedback = 0;
	test_child = settings.get_child_optional("population-increase-phases");
	if( test_child ){
		max_feedback = settings.get<uint32_t>("population-increase-phases");
	}
	
	// Por seguridad (del iterador) primero extraigo los scenario.id de settings
	vector<uint32_t> s_ids;
	for(auto s : settings.get_child("scenarios")){
		uint32_t s_id = s.second.get<uint32_t>("id");
		s_ids.push_back(s_id);
	}
	finish = false;
	
	// La idea es cargar distribuciones de cada feedback aqui
	// La dist de feedback 0 es prior, la de cada otro valor es propuesta, y la estiamcion actual es posterior
	// scen_id -> name -> dist
	// Ahroa es feedback -> scen_id -> name -> dist
//	map<uint32_t, map<string, Distribution>> dists_prior = db_comm.getDistributions(id, feedback);
	map<uint32_t, map<uint32_t, map<string, Distribution>>> dists_prior;
	for(unsigned int f = 0; f <= feedback; ++f){
		dists_prior[f] = db_comm.getDistributions(id, f);
		
//		cout<<"Analyzer::updateTrainingResults - Revisando distribuciones feedback "<<f<<"\n";
//		for( auto scenario_dist : dists_prior[f] ){
//			for( auto dist : scenario_dist.second ){
//				cout<<"Analyzer::updateTrainingResults - dists_prior["<<f<<"]["<<scenario_dist.first<<"]["<<dist.first<<"]: "<<dist.second.to_string()<<"\n";
//			}
//		}
		
	}
	
	boost::property_tree::ptree scenarios;
	for(unsigned int i = 0; i < s_ids.size() && !finish; ++i){
		// Notar que es valido pasarle el mismo ptree settings para cada escenario
		// Eso es por que cada llamada a trainModel SOLO REEMPLAZA LOS VALORES DEL ESCENARIO DADO
		// Al final del ciclo, todos los escenarios han sido actualizados en settings
		map<string, Distribution> posterior_map;
		map<string, Distribution> adjustment_map;
		map<string, map<string, double>> statistics_map;
		finish = trainModel(id, s_ids[i], feedback, max_feedback, settings, posterior_map, adjustment_map, statistics_map);
		
		// Agregar posterior_map al json de resultados de entrenamiento
		boost::property_tree::ptree scenario;
		string name = "Scenario " + std::to_string(s_ids[i]);
		scenario.put("name", name);
		scenario.put("id", s_ids[i]);
		
		boost::property_tree::ptree estimations;
		
//		unsigned int param_id = 0;
		for(map<string, Distribution>::iterator it = posterior_map.begin(); it != posterior_map.end(); it++){
			
			string dist_name = it->first;
			Distribution dist_posterior = it->second;
			
			boost::property_tree::ptree estimation;
			estimation.put("parameter", dist_name);
//			cout<<"Parameter "<<dist_name<<"\n";
			
			//estimation.put("min", min);
			//estimation.put("max", max);
			//estimation.put("median", mediana);
			map<string, double> stats = statistics_map[dist_name];
			for(map<string, double>::iterator stat = stats.begin(); stat != stats.end(); stat++){
				estimation.put(stat->first, stat->second);
			}
			
			// Para graficar las curvas necesito los factores de escalamiento globales
			// De hecho, basta con min/max de cualquiera de las curvas, quizas convenga usar las de rango menor
			// Notar tambien que min y max no son necesariamente directos para la normal
			//  - Puedo usar los observados (y graficar solo en ese rango)
			//  - O puedo usar min y max que cubran razonablemente la curva (como +- 3 stddev)
			
			double min_post = stats["min"];
			double max_post = stats["max"];
			vector<pair<double, double>> vals;
			
			boost::property_tree::ptree curves;
			
			boost::property_tree::ptree curve;
			curve.put("name", "Posterior");
			
			boost::property_tree::ptree fvalue;
			boost::property_tree::ptree fvalues;
			cout<<"Posterior\n";
			
//			char buff[1024];
//			ofstream escritor;
			
			vals = Statistics::generateDistributionGraph(dist_posterior, min_post, max_post, min_post, max_post);
//			sprintf(buff, "logs/posterior_%d_%d_%d_%d.log", id, s_ids[i], param_id, training_id);
//			escritor.open(buff, fstream::trunc | fstream::out);
			for(unsigned int j = 0; j < vals.size(); ++j){
//				escritor<<""<<vals[j].first<<"\t"<<vals[j].second<<"\n";
				fvalue.put("x", vals[j].first);
				fvalue.put("y", vals[j].second);
				fvalues.push_back(make_pair("", fvalue));
			}
//			escritor.close();
			vals.clear();
			
			// Agrego values a la curva
			curve.add_child("values", fvalues);
			
			// Agrego la curva posterior a curves
			curves.push_back(make_pair("", curve));
			
			
			// Busqueda de distribucion prior del parametro
			// Aqui hay que iterar por cada feedback
			// Con 0 es la verdadera prior, en los demas casos es la proposal N
			for( unsigned f = 0; f <= feedback; ++f ){
				map<uint32_t, map<string, Distribution>> dists_feedback = dists_prior[f];
				map<string, Distribution> prior = dists_feedback[s_ids[i]];
				if( prior.find(dist_name) == prior.end() ){
					cerr<<"Analyzer::updateTrainingResults - Distribucion a priori de "<<dist_name<<" no encontrada\n";
				}
				else{
					Distribution dist_prior = prior[dist_name];
	//				cout<<"Analyzer::updateTrainingResults - Distribucion a priori de "<<dist_name<<": "<<dist_prior.to_string()<<"\n";
				
					string curve_name;
					if(f == 0){
						curve_name = "Prior";
					}
					else{
						curve_name = "Proposal ";
						curve_name += std::to_string(f);
					}
					boost::property_tree::ptree curve_prior;
					curve_prior.put("name", curve_name);
					
					boost::property_tree::ptree fvalue_prior;
					boost::property_tree::ptree fvalues_prior;
					cout<<""<<curve_name<<"\n";
				
					// Para la distribucion prior, no tengo necesariamente min y max
					// Por ello, en ese caso pido minimos y maximos analiticos
					// La distribucion conoce valores razonables
					double min_prior = dist_prior.getMinValue();
					if(min_prior < 0.0){
						min_prior = 0.0;
					}
					vals = Statistics::generateDistributionGraph(dist_prior, min_prior, dist_prior.getMaxValue(), min_post, max_post);
//					sprintf(buff, "logs/prior_%d_%d_%d_%d.log", id, s_ids[i], param_id, training_id);
//					escritor.open(buff, fstream::trunc | fstream::out);
					for( unsigned int j = 0; j < vals.size(); ++j ){
//						escritor<<""<<vals[j].first<<"\t"<<vals[j].second<<"\n";
						fvalue_prior.put("x", vals[j].first);
						fvalue_prior.put("y", vals[j].second);
						fvalues_prior.push_back(make_pair("", fvalue_prior));
					}
//					escritor.close();
					// Agrego values a la curva
					curve_prior.add_child("values", fvalues_prior);
			
					// Agrego la curva posterior a curves
					curves.push_back(make_pair("", curve_prior));
				
				}
			}// for... cada feedback
			
			
			// Aqui se puede agregar la curva de otra distribucion, por ejemplo la ajustada
			if( adjustment_map.find(dist_name) != adjustment_map.end() ){
				Distribution dist_adjustment = adjustment_map[dist_name];
				
				boost::property_tree::ptree curve_adjustment;
				curve_adjustment.put("name", "Adjustment");
					
				boost::property_tree::ptree fvalue_adjustment;
				boost::property_tree::ptree fvalues_adjustment;
				cout<<"Adjustment\n";
				
				double min_adjustment = dist_adjustment.getMinValue();
				if(min_adjustment < 0.0){
					min_adjustment = 0.0;
				}
				vals = Statistics::generateDistributionGraph(dist_adjustment, min_adjustment, dist_adjustment.getMaxValue(), min_post, max_post);
//				sprintf(buff, "logs/prior_%d_%d_%d_%d.log", id, s_ids[i], param_id, training_id);
//				escritor.open(buff, fstream::trunc | fstream::out);
				for( unsigned int j = 0; j < vals.size(); ++j ){
//					escritor<<""<<vals[j].first<<"\t"<<vals[j].second<<"\n";
					fvalue_adjustment.put("x", vals[j].first);
					fvalue_adjustment.put("y", vals[j].second);
					fvalues_adjustment.push_back(make_pair("", fvalue_adjustment));
				}
//				escritor.close();
				// Agrego values a la curva
				curve_adjustment.add_child("values", fvalues_adjustment);
		
				// Agrego la curva posterior a curves
				curves.push_back(make_pair("", curve_adjustment));
			
			}
			
			// Finalmente, agrego curves a estimation de este parametro
			estimation.add_child("curves", curves);
			
			estimations.push_back(make_pair("", estimation));
			
//			++param_id;
			
		}
		scenario.add_child("estimations", estimations);
		scenarios.push_back(make_pair("", scenario));
		
		scenario.add_child("estimations", estimations);
		
		posterior_map.clear();
	}
	
//	++training_id;
	
	boost::property_tree::ptree estimations;
	estimations.add_child("scenarios", scenarios);
	
	boost::property_tree::ptree training_results;
	training_results.put("id", std::to_string(id));
	training_results.put("feedback", feedback);
		
	uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	cout<<"Analyzer::updateTrainingResults - Agregando timestamp "<<timestamp<<"\n";
	training_results.put("timestamp", std::to_string(timestamp));
	
	training_results.add_child("estimations", estimations);
	
	// Almacenar el json de resultados de entrenamiento
	db_comm.storeTrainingResults(training_results);
	
	return settings;	
}

boost::property_tree::ptree Analyzer::run(boost::property_tree::ptree &_frequest){

	boost::optional<boost::property_tree::ptree&> test_child;
	uint32_t id = 0;
	string type = "unknown";
	
	test_child = _frequest.get_child_optional("id");
	if( test_child ){
		id = _frequest.get<uint32_t>("id");
	}
	
	test_child = _frequest.get_child_optional("type");
	if( test_child ){
		type = _frequest.get<string>("type");
	}
	
	cout<<"Analyzer::run - Inicio 1 (id: "<<id<<", type: "<<type<<")\n";
	
	switch(util::hash(_frequest.get<string>("type"))){
		case RESTORE:  {
			cout<<"Analyzer::run - RESTORE ("<<id<<")\n";
			
			finished[id] = 0;
			test_child = _frequest.get_child_optional("finished");
			if( test_child ){
				finished[id] = _frequest.get<uint32_t>("finished");
			}
			cout<<"Analyzer::run - Finished: "<<finished[id]<<"\n";
			
			// Tambien hay que actualizar next_batch y next_results con finished
			// Uso (finished / batch_size) + 1 por batch_size para reestablecer el valor dependiendo del punto actual
//			this->next_batch.emplace(id, ((uint32_t)(finished[id]/batch_size) + 1) * batch_size );
//			this->next_results.emplace(id, ((uint32_t)(finished[id]/update_results) + 1) * update_results );
//			cout<<"Analyzer::run - next_batch: "<<next_batch[id]<<", next_results: "<<next_results[id]<<"\n";
			
			// En esta version, data DEBE estar guardado en la bd (pues el restore parte de scheduler que necesita haber pasado por el inicio de analyzer)
			boost::property_tree::ptree fdata = db_comm.readData(id);
			this->_data[id] = fdata;
			
			_data_indices.emplace(id, map<string, map<uint32_t, map<uint32_t, map<string, double>>>>{});
			parseIndices(this->_data[id].get_child("posterior"), _data_indices[id], true);
			
			break;
		};
		case CANCEL: {
			cout<<"Analyzer::run - CANCEL ("<<id<<")\n";
			break;
		}
		case SIMULATED:{

//			std::stringstream ss;
//			write_json(ss, _frequest);
//			cout<<ss.str()<<"\n";
			
			if( finished.find(id) == finished.end() ){
				// Resultado de simulacion NO iniciada, o cancelada
				// Omitir esto antes de hacer cualquier cosa adicional
				
				cout<<"Analyzer::run - SIMULATED (Omitiendo resultados de simulacion "<<id<<")\n";
				return _frequest;
				
			}
			
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
			unsigned int next_batch_emplace = ((unsigned int)(finished[id] / batch_size) + 1) * batch_size;
			unsigned int update_results_emplace = ((unsigned int)(finished[id] / update_results) + 1) * update_results;
			this->next_batch.emplace(id, next_batch_emplace);
			this->next_results.emplace(id, update_results_emplace);
			
			unsigned int feedback = 0;
			test_child = _frequest.get_child_optional("feedback");
			if( test_child ){
				feedback = _frequest.get<uint32_t>("feedback");
			}
			
			cout<<"Analyzer::run - SIMULATED (id: "<<id<<", scenario: "<<scenario_id<<", feedback: "<<feedback<<")\n";
			
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
			bool fresponse_creado = false;
			bool finish = false;
			
			// Notar que aqui puedo agregar un caso para actualizar resultados aunque no este reentrenando
			// Basta con un if independiente en rango fijo (cada 100 por ejemplo) que invoque los metodos estadisticos y genere un json de training
			// El problema es como identificarlo, pues habra varios de un mismo feedback
			// Ese json tambien puede tener fecha absoluta para medir el tiempo, quizas se pueda usar eso para seleccionar el "ultimo" (tiempo mayor)
			
			// if independiente para actualizar resultados
			// CREO que este if podria ir como tercer caso de los if/else que siguen
			// En ese caso, seria una jerarquia por finished[id] (1.- terminar, 2.- reentrenar (que actualiza resultados), 3.- actualizar resultados (sin reentrenar))
			// Notar que el codigo de actualizar resultados deberia ser resistente a concurrencia
			// Por ahora lo dejo independiente y reuso este resultado si TAMBIEN hay que reentrenar
			if( finished[id] >= this->next_results[id] ){
				this->next_results[id] += update_results;
				fresponse = updateTrainingResults(id, feedback, finish);
				fresponse_creado = true;
			}
			
			// Para el calculo de tiempos es importante definir tiempo y progreso de que
			// Lo obvio es tiempo hasta resultados (aunque se puedan seguir mejorando con el tiempo), pero no es claro como llamar a eso
			// Progreso de un feedback dado simple es: max run de results con feedback / batch-size de settings con ese feedback
			// Tiempo transcurrido max timestamp de training - timestamp de settings
			// Tambien esta el tiempo total transcurrido, max timestamp de training - timestamp de settings con feedback 0
			// Tiempo restante es tpo transcurrido * batch-size / max run de results 
			// Entonces se requiere max run de results, batch-size de settings, max timestamp de training, timestamp de settings
			// run, bath-size, id y feedback son uint32, timestamp es uint64
			// Eso para un cierto id y feedback (feedback podria considerarse como max feedback de settings)
			// Propuesta de resultado?
			
			if( finished[id] >= _frequest.get<uint32_t>("max-number-of-simulations") ){
				cout<<"Analyzer::run - Preparando finalize\n";
				finished.erase(finished.find(id));
				fresponse.put("id", std::to_string(id));
				fresponse.put("type", "finalize");
				comm::send(this->_fhosts.get<string>("scheduler.host"), this->_fhosts.get<string>("scheduler.port"), this->_fhosts.get<string>("scheduler.resource"), fresponse);
			}
			else if( finished[id] >= this->next_batch[id] ){
				cout<<"Analyzer::run - Feedback iniciado (batch_size: "<<batch_size<<")\n";
				// Codigo de feedback, preparacion de nuevos parametros
				this->next_batch[id] += batch_size;
				cout<<"Analyzer::run - Preparando reload (proximo feedback en simulacion "<<this->next_batch[id]<<")\n";
				
				// Si ya prepare fresponse por actualizacon de resultados, reuso eso mismo
				if(! fresponse_creado){
					fresponse = updateTrainingResults(id, feedback, finish);
				}
				
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

			_frequest.push_back(make_pair("posterior", fposterior));
			this->_data[id] = _frequest;
			_data_indices.emplace(id, map<string, map<uint32_t, map<uint32_t, map<string, double>>>>{});
			parseIndices(this->_data[id].get_child("posterior"), _data_indices[id], true);

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

boost::property_tree::ptree Analyzer::run(const std::string &_body){
	cout<<"Analyzer::run - Inicio 2\n";
	boost::property_tree::ptree fsettings;
	
	uint32_t id = this->incremental_id++;
	
	// global_mutex.lock();
	// Por ahora lo guardo como texto (para facilitar el debug) pero la forma correcta es sizeof(int) en binario
	ofstream escritor(id_file, fstream::trunc | fstream::out);
	escritor<<incremental_id<<"\n";
//	fstream escritor(id_file, fstream::trunc | fstream::out | fstream::binary);
//	escritor.write((char*)&incremental_id, sizeof(int));
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
	
	cout<<"Analyzer::run - MultipartFormParser...\n";
	MultipartFormParser m(_body);
	cout<<"Analyzer::run - MultipartFormParser terminado\n";

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

	cout<<"Analyzer::run - Seteando id y timestamp\n";
	 
	 
	 uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	fsettings.put("id", std::to_string(id));
	fsettings.put("timestamp", std::to_string(timestamp));
	
	cout<<"Analyzer::run - id: "<<id<<", timestamp: "<<timestamp<<"\n";

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

		cout<<"Analyzer::run - Creando FileParser\n";
		FileParser fp(m.get("File"), FileType(filetype), MarkerType(markertype));
		m.remove("File");

		cout<<"Analyzer::run - fp.parse...\n";
		vector<Marker> markers = fp.parse();
		cout<<"Analyzer::run - fp.parse terminado\n";
		samples[sample][chromosome][gene] = markers;
	}

	cout<<"Analyzer::run - Creando samples\n";
	boost::property_tree::ptree fsamples;
	for(auto& sample : samples){
		boost::property_tree::ptree fsample;
		fsample.put("name","sample" + std::to_string(sample.first));
		fsamples.push_back(std::make_pair("", fsample));
	}

	cout<<"Analyzer::run - Creando Profile\n";
	boost::property_tree::ptree findividual = get_profile(samples, ploidy);
	
	fsettings.add_child("samples", fsamples);
	fsettings.add_child("individual", findividual);

	 /*computing statistics*/
	cout<<"Analyzer::run - Preparando estadisticos\n";
	this->finished[id] = 0;

	boost::property_tree::ptree fdata, fposterior;
	fdata.put("id", fsettings.get<std::string>("id"));
	fdata.put("type", "data");

	boost::property_tree::ptree fpopulations;
	Sample all("summary");
	for(auto& sample : samples){
		Sample p("sample" + std::to_string(sample.first), sample.second, fsettings);
		fpopulations.push_back(std::make_pair("", p.indices()));
		all.merge(&p);
	}
	fpopulations.push_back(std::make_pair("", all.indices()));
	fposterior.push_back(make_pair("populations", fpopulations));
	fdata.push_back(make_pair("posterior", fposterior));
	
	this->_data[id] = fdata;
	_data_indices.emplace(id, map<string, map<uint32_t, map<uint32_t, map<string, double>>>>{});
	parseIndices(this->_data[id].get_child("posterior"), _data_indices[id]);
	
	cout<<"Analyzer::run - Guardando data\n";
	db_comm.writeData(fdata);
	
	std::stringstream ss;
	write_json(ss,fdata);
	cout << ss.str() << endl;
	 /*computing statistics*/

	cout<<"Analyzer::run - Fin\n";
	return(fsettings);
}

boost::property_tree::ptree Analyzer::get_profile(const map<uint32_t,map<uint32_t,map<uint32_t,vector<Marker>>>> &_samples,const uint32_t &_ploidy) {
	boost::property_tree::ptree findividual;

	findividual.put("ploidy", std::to_string(_ploidy));

	for(auto& sample : _samples){
		boost::property_tree::ptree fchromosomes;
		for(auto& chromosome : sample.second){
			boost::property_tree::ptree fchromosome;
			fchromosome.put("id",chromosome.first);

			boost::property_tree::ptree fgenes;
			for(auto& gene : chromosome.second){
				boost::property_tree::ptree fgene;
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


