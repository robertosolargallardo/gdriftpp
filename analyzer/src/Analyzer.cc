#include "Analyzer.h"
Analyzer::Analyzer(boost::property_tree::ptree &_fhosts):Node(_fhosts){
	this->_mongo = make_shared<util::Mongo>(this->_fhosts.get<string>("database.uri"));
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

	/*
	double a=0.0, b=0.0, s=0.0, n=0.0, diff=0.0;
	map<string, vector<double>> deltas;
	for(auto i : _data_indices[id]){
		for(auto j : i.second){
			for(auto k : j.second){
				for(auto l : k.second){
					a = _data_indices[id][i.first][j.first][k.first][l.first];
					b = indices_simulated[i.first][j.first][k.first][l.first];
					if(a == 0 && b == 0){
						continue;
					}
					diff = (a - b) / max(a, b);
					s += diff * diff;
					n += 1.0;
					cout<<"Analyzer::distance - s: "<<s<<", diff: "<<diff<<" (a: "<<a<<", b: "<<b<<", "<<l.first<<")\n";
				}
			}
		}
	}
	*/

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

boost::property_tree::ptree Analyzer::run(boost::property_tree::ptree &_frequest){

	uint32_t id = _frequest.get<uint32_t>("id");
	
	switch(util::hash(_frequest.get<string>("type"))){
		case SIMULATED:{

//			std::stringstream ss;
//			write_json(ss, _frequest);
//			cout<<ss.str()<<"\n";
			
			uint32_t scenario_id = _frequest.get<uint32_t>("scenario.id");
//			pair<uint32_t, uint32_t> id_pair(id, scenario_id);
			this->feedback_size[id] = _frequest.get<uint32_t>("feedback_size");
			// Solo agrego feedback_size[id] como valor inicial la primera vez, de ahi en adelante se mantiene la suma de feedback * feedback_size[i];
			this->next_feedback.emplace(id, feedback_size[id]);
			
			unsigned int feedback = 0;
			boost::property_tree::ptree::assoc_iterator it = _frequest.find("feedback");
			if( it != _frequest.not_found() ){
				feedback = _frequest.get<uint32_t>("feedback");
			}
			
			cout<<"Analyzer::run - SIMULATED (id: "<<id<<", scenario: "<<scenario_id<<", _batch_size[id]: "<<_batch_size[id]<<", feedback: "<<feedback<<")\n";
			
			if(this->_accepted.count(id)==0) return(_frequest);
			
			this->_batch_size[id]++;
//			double dist = distance(this->_data[id].get_child("posterior"), _frequest.get_child("posterior"));
			double dist = distance(id, _frequest.get_child("posterior"));
			cout << dist << endl;
			
			//if(dist <= MAX_DIST){
				this->_accepted[id]++;
				_frequest.put("distance", dist);
				this->_mongo->write(this->_fhosts.get<string>("database.name"), this->_fhosts.get<string>("database.collections.results"), _frequest);
			//}
			
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
			
			cout<<"Analyzer::run - _batch_size[id]: "<<_batch_size[id]<<" vs "<<BATCH_LENGTH*this->_fhosts.get_child("controller").size()<<"\n";
			if(this->_batch_size[id] == (BATCH_LENGTH*this->_fhosts.get_child("controller").size())){
				boost::property_tree::ptree fresponse;
				fresponse.put("id", id);
				
				if(this->_accepted[id] >= uint32_t(_frequest.get<double>("max-number-of-simulations")*PERCENT)){
					cout<<"Analyzer::run - Preparando finalize\n";
					this->_accepted.erase(this->_accepted.find(id));
					this->_batch_size.erase(this->_batch_size.find(id));
					fresponse.put("type", "finalize");
				}
//				else if( this->_accepted[id] >= this->next_feedback[id_pair] ){
				else if( this->_accepted[id] >= this->next_feedback[id] ){
					cout<<"Analyzer::run - Feedback iniciado\n";
					// Codigo de feedback, preparacion de nuevos parametros
//					Statistics st(id);
//					st.run();
//					res = st.getResults()
//					this->next_feedback[id_pair] += feedback_size[id];
					this->next_feedback[id] += feedback_size[id];
					cout<<"Analyzer::run - Preparando reload (proximo feedback en simulacion "<<this->next_feedback[id]<<")\n";
					fresponse.put("type", "reload");
					fresponse.put("feedback", 1 + feedback);
					comm::send(this->_fhosts.get<string>("scheduler.host"), this->_fhosts.get<string>("scheduler.port"), this->_fhosts.get<string>("scheduler.resource"), fresponse);
					// Enviar nuevos parametros al scheduler
					cout<<"Analyzer::run - Preparando continue\n";
					this->_batch_size[id] = 0;
					fresponse.put("type", "continue");
				}
				else{
					cout<<"Analyzer::run - Preparando continue\n";
					this->_batch_size[id] = 0;
					fresponse.put("type", "continue");
				}

				comm::send(this->_fhosts.get<string>("scheduler.host"), this->_fhosts.get<string>("scheduler.port"), this->_fhosts.get<string>("scheduler.resource"), fresponse);
			}
			break;
		};
		case DATA:  {
			this->_accepted[id] = 0;	
			this->_batch_size[id] = 0;
			
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

			this->_mongo->write(this->_fhosts.get<string>("database.name"),this->_fhosts.get<string>("database.collections.data"),_frequest);

			break;
		};
		default: {
			cerr << "Unknown Result Type" << endl;
			exit(EXIT_FAILURE);
		};
	}
	return(_frequest);
}




