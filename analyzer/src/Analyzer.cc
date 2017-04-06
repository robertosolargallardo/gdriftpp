#include "Analyzer.h"
Analyzer::Analyzer(boost::property_tree::ptree &_fhosts):Node(_fhosts){
	this->_mongo = make_shared<util::Mongo>(this->_fhosts.get<string>("database.uri"));
}
Analyzer::~Analyzer(void){
}

double distance(const boost::property_tree::ptree &_data,const boost::property_tree::ptree &_simulated){
	//TODO

	map<string, map<uint32_t, map<uint32_t, map<string, double>>>> indices_data;
	map<string, map<uint32_t, map<uint32_t, map<string, double>>>> indices_simulated;
	
	for(auto& p : _data.get_child("populations")){
		string pop_name = p.second.get<string>("name");
		for(auto c : p.second.get_child("chromosomes")){
			uint32_t cid = c.second.get<uint32_t>("id");
			for(auto g : c.second.get_child("genes")){
				uint32_t gid = g.second.get<uint32_t>("id");
				for(auto i : g.second.get_child("indices")){
//					indices_data[pop_name][cid][gid][i.first] = boost::lexical_cast<double>(i.second.data());
					indices_data[pop_name][cid][gid][i.first] = std::stod(i.second.data());
				}
			}
		}
	}
	for(auto& p : _simulated.get_child("populations")){
		string pop_name = p.second.get<string>("name");
		for(auto c : p.second.get_child("chromosomes")){
			uint32_t cid = c.second.get<uint32_t>("id");
			for(auto g : c.second.get_child("genes")){
				uint32_t gid = g.second.get<uint32_t>("id");
				for(auto i : g.second.get_child("indices")){
//					indices_simulated[pop_name][cid][gid][i.first] = boost::lexical_cast<double>(i.second.data());
					indices_simulated[pop_name][cid][gid][i.first] = std::stod(i.second.data());
				}
			}
		}
	}

	/*double d=0.0;
	for(auto i : indices_data){
		for(auto j : i.second){
			for(auto k : j.second){
				double sum=0.0;
				for(auto l : k.second){
					double diff=indices_data[i.first][j.first][k.first][l.first]-indices_simulated[i.first][j.first][k.first][l.first];
					sum+=diff*diff;
				}
				d+=sqrt(sum);
			}
		}
	}*/

	double a=0.0,b=0.0,s=0.0,n=0.0,diff=0.0;
	for(auto i : indices_data){
		for(auto j : i.second){
			for(auto k : j.second){
				for(auto l : k.second){
					a = indices_data[i.first][j.first][k.first][l.first];
					b = indices_simulated[i.first][j.first][k.first][l.first];
					diff = (a-b)/max(a,b);
					s += diff*diff;
					n += 1.0;
				}
			}
		}
	}
	return(sqrt(s/n));
}

boost::property_tree::ptree Analyzer::run(boost::property_tree::ptree &_frequest){

	uint32_t id = _frequest.get<uint32_t>("id");
	
	switch(util::hash(_frequest.get<string>("type"))){
		case SIMULATED:{

//			std::stringstream ss;
//			write_json(ss, _frequest);
//			cout<<ss.str()<<"\n";
			
//			cout<<"Analyzer::run - Leyendo scenario.id\n";
			uint32_t scenario_id = _frequest.get<uint32_t>("scenario.id");
//			pair<uint32_t, uint32_t> id_pair(id, scenario_id);
			this->feedback_size[id] = _frequest.get<uint32_t>("feedback_size");
			// Solo agrego feedback_size[id] como valor inicial la primera vez, de ahi en adelante se mantiene la suma de feedback * feedback_size[i];
			this->next_feedback.emplace(id, feedback_size[id]);
			
//			cout<<"Analyzer::run - Leyendo feedback\n";
			unsigned int feedback = 0;
			boost::property_tree::ptree::assoc_iterator it = _frequest.find("feedback");
			if( it != _frequest.not_found() ){
				feedback = _frequest.get<uint32_t>("feedback");
			}
			
			cout<<"Analyzer::run - SIMULATED (id: "<<id<<", scenario: "<<scenario_id<<", _batch_size[id]: "<<_batch_size[id]<<", feedback: "<<feedback<<")\n";
			
			if(this->_accepted.count(id)==0) return(_frequest);

			this->_batch_size[id]++;
			double dist = distance(this->_data[id].get_child("posterior"), _frequest.get_child("posterior"));
			cout << dist << endl;

			//if(dist <= MAX_DIST){
				this->_accepted[id]++;
				_frequest.put("distance", dist);
				this->_mongo->write(this->_fhosts.get<string>("database.name"),this->_fhosts.get<string>("database.collections.results"),_frequest);
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
//					cout<<"Analyzer::run - Preparando reload (proximo feedback en simulacion "<<next_feedback[id_pair]<<")\n";
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




