#ifndef _DB_COMMUNICATION_H_
#define _DB_COMMUNICATION_H_

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>

#include "Const.h"
#include "Mongo.h"

using namespace std;
using boost::property_tree::ptree;
//using bsoncxx::builder::stream::document;
//using bsoncxx::builder::stream::finalize;
//using bsoncxx::builder::stream::open_document;
//using bsoncxx::builder::stream::close_document;

class DBCommunication{
	private:
		util::Mongo mongo;
		string uri;
		string db_name;
		string collection_data;
		string collection_results;
		string collection_settings;
		string collection_training;

	public:
		DBCommunication(){
			db_name = "gdrift";
			collection_data = "data";
			collection_results = "results";
			collection_settings = "settings";
			collection_training = "training";
		}
		
		DBCommunication(string _uri, string _db_name, string _collection_data, string _collection_results, string _collection_settings, string _collection_training){
			uri = _uri;
			mongo = util::Mongo(uri);
			db_name = _db_name;
			collection_data = _collection_data;
			collection_results = _collection_results;
			collection_settings = _collection_settings;
			collection_training = _collection_training;
			cout<<"DBCommunication - db_name: \""<<db_name<<"\", collections: \""<<collection_data<<"\", \""<<collection_results<<"\", \""<<collection_settings<<"\"\n";
		}
		
		DBCommunication(string _uri) : DBCommunication() {
			uri = _uri;
			mongo = util::Mongo(uri);
		}
		
		~DBCommunication(){}
		
		void writeResults(boost::property_tree::ptree &json){
			mongo.write(db_name, collection_results, json);
		}
		
		boost::property_tree::ptree readSettings(uint32_t id, uint32_t feedback){
			return mongo.readSettings(db_name, collection_settings, id, feedback);
		}
		
		void writeData(boost::property_tree::ptree &json){
			mongo.write(db_name, collection_data, json);
		}
		
		// Este metodo esta guardando directamente las rutas a los valores que genere Scheduler
		// Eso debería leerse de la base de datos sin necesidad de saberlo a priori
		vector<string> getFields(uint32_t id, uint32_t scenario_id, uint32_t feedback = 0){
			cout<<"DBCommunication::getFields - Inicio ("<<id<<", "<<scenario_id<<", "<<feedback<<")";
			vector<string> res;
			res.push_back("id");
			res.push_back("scenario.id");
			
			boost::property_tree::ptree settings = mongo.readSettings(db_name, collection_settings, id, feedback);
			
//			std::stringstream ss;
//			write_json(ss, settings);
//			cout<<ss.str()<<"\n";

			// Genes
			// individual.chromosomes.0.genes.0.mutation.model
			// individual.chromosomes.0.genes.0.mutation.rate
			unsigned int genid = 0;
			boost::property_tree::ptree individual = settings.get_child("individual");
			for(auto c : individual.get_child("chromosomes")){
				for(auto g : c.second.get_child("genes")){
					string field = "individual.chromosomes.";
					field += c.second.get<string>("id");
					field += ".genes.";
					field += g.second.get<string>("id");
					field += ".mutation.model";
					res.push_back(field);
					// if rate is random...
					string type = g.second.get<string>("mutation.rate.type");
					if(type.compare("random") == 0){
						field = "individual.chromosomes.";
						field += c.second.get<string>("id");
						field += ".genes.";
						field += g.second.get<string>("id");
						field += ".mutation.rate";
						res.push_back(field);
					}
					++genid;
				}
			}
			
			// Events
			// scenario.events.0.params.population.size
			// scenario.events.2.params.source.population.percentage
			for(auto s : settings.get_child("scenarios")){
				uint32_t s_id = s.second.get<uint32_t>("id");
				if(s_id == scenario_id){
					for(auto e : s.second.get_child("events")){
						// En principio cada evento tiene timestamp y parametros
						// Los parametros que tengan type random deben ser agregados
						
						string e_id = e.second.get<string>("id");
						string e_type = e.second.get<string>("type");
						string field = "scenario.events.";
						field += e_id;
						res.push_back(field + ".timestamp");
						cout<<"DBCommunication::getFields - field "<<field<<", ("<<e_type<<")\n";
						
						if( e_type.compare("create") == 0 ){
							field += ".params.population.size";
							res.push_back(field);
						}
						else if( e_type.compare("endsim") != 0 && e_type.compare("split") != 0 && e_type.compare("extinction") != 0 ){
							field += ".params.source.population.percentage";
							res.push_back(field);
						}
						
					}
				}
			}
			
			return res;
		}
		
		map<uint32_t, vector<string>> getEventsParams(uint32_t id, uint32_t scenario_id, bool population_increase = false){
			map<uint32_t, vector<string>> events_params;
			boost::property_tree::ptree settings = mongo.readSettings(db_name, collection_settings, id, 0);
			
			for(auto s : settings.get_child("scenarios")){
				uint32_t s_id = s.second.get<uint32_t>("id");
				if(s_id == scenario_id){
					for(auto e : s.second.get_child("events")){
						// En principio cada evento tiene timestamp y parametros
						// Los parametros que tengan type random deben ser agregados
						
						uint32_t eid = e.second.get<uint32_t>("id");
						string etype = e.second.get<string>("type");
						
						events_params[eid].push_back("timestamp");
						// Mientras se incrementa la poblacion, omito population.size de los parametros
						if( etype.compare("create") == 0 ){
							string dist_type = e.second.get<string>("params.population.size.type");
							if( dist_type.compare("random") == 0 && !population_increase ){
								events_params[eid].push_back("params.population.size");
							}
						}
						else if( etype.compare("endsim") != 0 && etype.compare("split") != 0 && etype.compare("extinction") != 0 ){
							string dist_type = e.second.get<string>("params.source.population.percentage.type");
							if( dist_type.compare("random") == 0 ){
								events_params[eid].push_back("params.source.population.percentage");
							}
						}
						
					}
				}
			}
			
			return events_params;
		}
			
		uint32_t getResults(uint32_t id, uint32_t scenario_id, uint32_t feedback, vector<vector<double>> &params, map<uint32_t, vector<string>> &events_params, vector<vector<double>> &statistics){
			
			cout<<"DBCommunication::getResults - Inicio ("<<id<<", "<<scenario_id<<", "<<feedback<<")\n";
			
			list<boost::property_tree::ptree> results;
			mongo.readStatistics(results, db_name, collection_results, id, scenario_id, feedback);
			
			unsigned int count = 0;
			double value = 0;
			boost::optional< boost::property_tree::ptree& > child;
			map<string, double> params_res;
			map<string, map<uint32_t, map<uint32_t, map<string, double>>>> stats_res;
			
			// Mapas de n_params y n_stat por cantidad de veces usado
			// Al final, conservo el mayor en cada caso y descarto los resultados incorrectos
			map<unsigned int, unsigned int> params_used;
			map<unsigned int, unsigned int> statistics_used;
			
			// Arreglos locales para la insercion previa antes del filtrado
			vector<vector<double>> params_local;
			vector<vector<double>> statistics_local;
			
			for(auto &json : results){
				
//				std::stringstream ss;
//				write_json(ss, json);
//				cout<<ss.str()<<"\n";
				
				boost::property_tree::ptree scenario = json.get_child("scenario");
				if( scenario.get<uint32_t>("id") != scenario_id ){
					cerr<<"DBCommunication::getResults - Scenario with wrong id.\n";
					continue;
				}
				
				// LLeno params_res con los parametros de este resultado
				for(auto &event : scenario.get_child("events")){
					uint32_t eid = event.second.get<uint32_t>("id");
					for(unsigned int i = 0; i < events_params[eid].size(); ++i){
//						cout<<"DBCommunication::getResults - get \""<<events_params[eid][i]<<"\" de evento "<<eid<<"\n";
						value = event.second.get<double>(events_params[eid][i]);
						string param_name = "events.";
						param_name += std::to_string(eid);
						param_name += ".";
						param_name += events_params[eid][i];
						params_res[param_name] = value;
//						cout<<"DBCommunication::getResults - Event: "<<eid<<", "<<param_name<<": "<<value<<"\n";
					}
				}
				
				// Tambien incluyo las mutaciones de cada gen
				boost::property_tree::ptree individual = json.get_child("individual");
				for(auto &c : individual.get_child("chromosomes")){
					uint32_t cid = c.second.get<uint32_t>("id");
					for(auto g : c.second.get_child("genes")){
						uint32_t gid = g.second.get<uint32_t>("id");
						value = g.second.get<double>("mutation.rate");
						string param_name = "chromosomes.";
						param_name += std::to_string(cid);
						param_name += ".genes.";
						param_name += std::to_string(gid);
						param_name += ".mutation.rate";
						params_res[param_name] = value;
//						cout<<"DBCommunication::getResults - "<<param_name<<": "<<value<<"\n";
					}
				}
				
				// volcar params_res en vector
				// Notar que NO estoy verificando el largo esperado en esta version
				// Dejo los parametros en values, los verifico y agrego al final
				vector<double> values;
				for(map<string, double>::iterator it = params_res.begin(); it != params_res.end(); it++){
					values.push_back(it->second);
				}
				params_res.clear();
				
				// Aqui se pueden verificar los estadisticos en busca de valores invalidos
				// Una opcion es eliminar los resultados si no hay ningun valor valido
				// El proceso deberia ser por poblacion, pero no es evidente cuando los valores SON validos
				// Mientras realizamos pruebas, asumo solo 1 gen, de modo que el set de indices mapea directamente a poblacion
				for(auto &p : json.get_child("posterior.populations")){
					string name = p.second.get<string>("name");
//					bool valid = false;
					for(auto &c : p.second.get_child("chromosomes")){
						uint32_t cid = c.second.get<uint32_t>("id");
						for(auto &g : c.second.get_child("genes")){
							uint32_t gid = g.second.get<uint32_t>("id");
							for(auto i : g.second.get_child("indices")){
								stats_res[name][cid][gid][i.first] = std::stod(i.second.data());
//								cout<<"DBCommunication::getResults - stat["<<name<<"]["<<cid<<"]["<<gid<<"]["<<i.first<<"]: "<<i.second.data()<<"\n";
							}
						}
					}
				}
				
				// volcar statistics_map en vector
				// Notar que NO estoy verificando el largo esperado en esta version
				// Al final verifico y agrego
				vector<double> stats;
				for(auto p : stats_res){
					for(auto c : p.second){
						for(auto g : c.second){
							for(auto i : g.second){
								stats.push_back(i.second);
							}
						}
					}
				}
				stats_res.clear();
				
				// Verifico y agrego los valores
				
				bool omitir = false;
				for(unsigned int i = 0; i < values.size(); ++i){
					if( values[i] < MIN_VALID_VALUE || values[i] > MAX_VALID_VALUE ){
						omitir = true;
						break;
					}
				}
				for(unsigned int i = 0; i < stats.size(); ++i){
					if( stats[i] < MIN_VALID_VALUE || stats[i] > MAX_VALID_VALUE ){
						omitir = true;
						break;
					}
				}
				if(omitir){
					continue;
				}
				
				params_local.push_back(values);
				statistics_local.push_back(stats);
				
				// n_params y n_stats usadaos, para luego conservar los mas usados y eliminar el resto
				params_used[values.size()]++;
				statistics_used[stats.size()]++;
				
			}
			
			cout<<"DBCommunication::getResults - "<<params_local.size()<<" resultados agregados\n";
			
			// Eliminar resultados incorrectos a posteriori si no se conocen los parametros desde el inicio
			// Una forma es quedarse (a posteriori) con total_params y total_statistics mas frecuentes
			unsigned int total_params = 0;
			unsigned int total_statistics = 0;
			unsigned max_uso = 0;
			for(map<unsigned int, unsigned int>::iterator it = params_used.begin(); it != params_used.end(); it++){
				if(it->second > max_uso){
					total_params = it->first;
					max_uso = it->second;
				}
			}
			max_uso = 0;
			for(map<unsigned int, unsigned int>::iterator it = statistics_used.begin(); it != statistics_used.end(); it++){
				if(it->second > max_uso){
					total_statistics = it->first;
					max_uso = it->second;
				}
			}
			
			cout<<"DBCommunication::getResults - total_params: "<<total_params<<", total_statistics: "<<total_statistics<<", filtrando resultados\n";
			// ELiminar resultados que no cumplan
			for(unsigned int i = 0; i < params_local.size(); ++i){
				if(params_local[i].size() == total_params && statistics_local[i].size() == total_statistics){
					params.push_back(params_local[i]);
					statistics.push_back(statistics_local[i]);
					++count;
				}
			}
			
			cout<<"DBCommunication::getResults - Fin (Resultados filtrados: "<<count<<")\n";
			return count;
		}
		
		void storeTrainingResults(boost::property_tree::ptree &result){
			
			cout<<"DBCommunication::storeTrainingResults - Inicio\n";
			
//			cout<<"DBCommunication::storeTrainingResults - Resultados de entrenamiento: \n";
//			std::stringstream ss;
//			write_json(ss, result);
//			cout << ss.str() << endl;
			
			// Quizas sea necesario crear una coleccion adicional
			mongo.write(db_name, collection_training, result);
			
			cout<<"DBCommunication::storeTrainingResults - Fin\n";
			
		}
		
		bool alreadyTrained(uint32_t id, uint32_t scenario_id, uint32_t feedback){
			
			
			
			return false;
		}
		
		void storeResults(uint32_t id, uint32_t scenario_id, string out_file, uint32_t feedback, uint32_t n_stats, uint32_t n_params){
			
			cout<<"DBCommunication::storeResults - Inicio (id: "<<id<<", scenario_id: "<<scenario_id<<", \""<<out_file<<"\")\n";
			
			// Tomar TODOS los resultados de id/scenario_id, agruparlas por feedback en map< feedback, vector<result> >
			vector< vector<double> > results;
			list<ptree> results_list;
			mongo.readResults(results_list, db_name, collection_results, id, scenario_id, feedback);
			
			unsigned int contador = 0;
			boost::optional<ptree&> test_child;
//			unsigned int n_stats = 0;
//			unsigned int n_params = 0;
			cout<<"DBCommunication::storeTimes - Procesando "<<results_list.size()<<" resultados\n";
			for(list<ptree>::iterator it = results_list.begin(); it != results_list.end(); it++){
				
//				std::stringstream ss;
//				write_json(ss, *it);
//				cout<<ss.str()<<"\n";
				
//				cout<<"DBCommunication::storeResults - Res["<<contador<<"]\n";
				
				// Extraer estadisticos
				// Primero los extraigo a la estructura de mapa para que queden con el mismo orden
				map<string, map<uint32_t, map<uint32_t, map<string, double>>>> indices;
				for(auto& p : it->get_child("posterior.populations")){
					string pop_name = p.second.get<string>("name");
					for(auto c : p.second.get_child("chromosomes")){
						uint32_t cid = c.second.get<uint32_t>("id");
						for(auto g : c.second.get_child("genes")){
							uint32_t gid = g.second.get<uint32_t>("id");
							for(auto i : g.second.get_child("indices")){
								indices[pop_name][cid][gid][i.first] = std::stod(i.second.data());
							}
						}
					}
				}
				vector<double> values;
				for(auto i : indices){
					for(auto j : i.second){
						for(auto k : j.second){
							for(auto l : k.second){
								values.push_back(l.second);
							}
						}
					}
				}
				
//				cout<<"DBCommunication::storeResults - Res["<<contador<<"] n_stats: "<<n_stats<<", ";
//				for(unsigned int i = 0; i < values.size(); ++i){
//					cout<<values[i]<<" ";
//				}
//				cout<<"\n";
				
				// Extraer parametros
				// Primero los extraigo en un mapa para que queden con el mismo orden
				map<string, double> params;
				
				// Genes
				for(auto &c : it->get_child("individual.chromosomes")){
					uint32_t cid = c.second.get<uint32_t>("id");
					for(auto g : c.second.get_child("genes")){
						uint32_t gid = g.second.get<uint32_t>("id");
						double value = g.second.get<double>("mutation.rate");
						string param_name = "chromosomes.";
						param_name += std::to_string(cid);
						param_name += ".genes.";
						param_name += std::to_string(gid);
						param_name += ".mutation.rate";
						params[param_name] = value;
//						cout<<"DBCommunication::storeResults - params["<<param_name<<"] = "<<params[param_name]<<"\n";
					}
				}
				
				// Eventos
				for(auto e : it->get_child("scenario.events")){
					// En principio cada evento tiene timestamp y parametros
					// Los parametros que tengan type random deben ser agregados
					string e_id = e.second.get<string>("id");
					string e_type = e.second.get<string>("type");
					string param_base = "scenario.events.";
					param_base += e_id;
					uint32_t timestamp = e.second.get<uint32_t>("timestamp");
					string param_name = param_base + ".timestamp";
					params[param_name] = timestamp;
//					cout<<"DBCommunication::storeResults - params["<<param_name<<"] = "<<params[param_name]<<"\n";
					
					if( e_type.compare("create") == 0 ){
						uint32_t size = e.second.get<uint32_t>("params.population.size");
						string param_name = param_base + ".params.population.size";
						params[param_name] = size;
//						cout<<"DBCommunication::storeResults - params["<<param_name<<"] = "<<params[param_name]<<"\n";
					}
					else if( e_type.compare("endsim") != 0 && e_type.compare("split") != 0 && e_type.compare("extinction") != 0 ){
						double percentage = e.second.get<double>("params.source.population.percentage");
						string param_name = param_base + ".params.source.population.percentage";
						params[param_name] = percentage;
//						cout<<"DBCommunication::storeResults - params["<<param_name<<"] = "<<params[param_name]<<"\n";
					}
				
				}
				
				// Verificar el tamaño de params?
				// Para eso habria que recibir externamente (leer al inicio) el tamaño del escenario
				// Por ahora los agrego directamente a values
				for(map<string, double>::iterator i = params.begin(); i != params.end(); i++){
					values.push_back(i->second);
				}
				if( values.size() == (n_stats + n_params) ){
					results.push_back(values);
				}
				
				++contador;
//if(contador>10) break;
			}
			
			cout<<"DBCommunication::storeResults - Total Revisados: "<<contador<<" (Agregados: "<<results.size()<<")\n";
			
			ofstream escritor(out_file, fstream::trunc | fstream::out);
			for(unsigned int i = 0; i < results.size(); ++i){
				vector<double> values = results[i];
				escritor<<i<<"\t";
				for(unsigned int j = 0; j < values.size(); ++j){
					escritor<<values[j]<<"\t";
				}
				escritor<<"\n";
				
			}
			escritor.close();
			
//			cout<<"DBCommunication::storeResults - Fin (pos, n_stats: "<<n_stats<<", n_params: "<<n_params<<")\n";
			cout<<"DBCommunication::storeResults - Fin\n";
		}
		
		void storeTimes(uint32_t id, uint32_t scenario_id, string out_file, uint32_t feedback){
			
			cout<<"DBCommunication::storeTimes - Inicio (id: "<<id<<", scenario_id: "<<scenario_id<<", \""<<out_file<<"\")\n";
			
			// Tomar TODOS los resultados de id/scenario_id, agruparlas por feedback en map< feedback, vector<result> >
			vector< vector<double> > results;
			list<ptree> results_list;
			mongo.readResults(results_list, db_name, collection_results, id, scenario_id, feedback);
			
			unsigned int contador = 0;
			boost::optional<ptree&> test_child;
			
			cout<<"DBCommunication::storeTimes - Procesando "<<results_list.size()<<" resultados\n";
			long double total = 0.0;
			for(list<ptree>::iterator it = results_list.begin(); it != results_list.end(); it++){
				
//				std::stringstream ss;
//				write_json(ss, *it);
//				cout<<ss.str()<<"\n";
				
//				cout<<"DBCommunication::storeTimes - Res["<<contador<<"]\n";
				
				// mutacion, poblacion inicial, generaciones
				// Las generaciones son el tiempo del ultimo evento, menos el tiempo del primero
				vector<double> values;
				
				// Extraer parametros
				// Primero los extraigo en un mapa para que queden con el mismo orden
				map<string, double> params;
				
				// Genes
				for(auto &c : it->get_child("individual.chromosomes")){
					uint32_t cid = c.second.get<uint32_t>("id");
					for(auto g : c.second.get_child("genes")){
						uint32_t gid = g.second.get<uint32_t>("id");
						double value = g.second.get<double>("mutation.rate");
						string param_name = "chromosomes.";
						param_name += std::to_string(cid);
						param_name += ".genes.";
						param_name += std::to_string(gid);
						param_name += ".mutation.rate";
						params[param_name] = value;
//						cout<<"DBCommunication::storeTimes - params["<<param_name<<"] = "<<params[param_name]<<"\n";
					}
				}
				for(map<string, double>::iterator i = params.begin(); i != params.end(); i++){
					values.push_back(i->second);
				}
				
				// Eventos
				// De aqui me interesa: timestamp primer evento, size del create, tiempo del ultimo evento
				uint32_t create_time = 0;
				uint32_t create_size = 0;
				uint32_t endsim_time = 0;
				for(auto e : it->get_child("scenario.events")){
					// En principio cada evento tiene timestamp y parametros
					// Los parametros que tengan type random deben ser agregados
					string e_id = e.second.get<string>("id");
					string e_type = e.second.get<string>("type");
					string param_base = "scenario.events.";
					param_base += e_id;
					if( e_type.compare("create") == 0 ){
						create_time = e.second.get<uint32_t>("timestamp");
						create_size = e.second.get<uint32_t>("params.population.size");
					}
					if( e_type.compare("endsim") == 0 ){
						endsim_time = e.second.get<uint32_t>("timestamp");
					}
				}
				values.push_back(create_size);
				values.push_back(endsim_time - create_time);
				
				double milisec = -1;
				boost::optional<ptree&> test_child;
				test_child = it->get_child_optional("ms_run");
				if( test_child ){
					milisec = it->get<double>("ms_run");
				}
				
				if( (milisec > 0) && (endsim_time > create_time) ){
					total += milisec;
					values.push_back(milisec);
					results.push_back(values);
				}
				
				++contador;
			}
			
			cout<<"DBCommunication::storeTimes - Total Revisados: "<<contador<<" (Agregados: "<<results.size()<<", tpo medio: "<<total/results.size()<<")\n";
			
			ofstream escritor(out_file, fstream::trunc | fstream::out);
			for(unsigned int i = 0; i < results.size(); ++i){
				vector<double> values = results[i];
				escritor<<i<<"\t";
				for(unsigned int j = 0; j < values.size(); ++j){
					escritor<<values[j]<<"\t";
				}
				escritor<<"\n";
				
			}
			escritor.close();
			
			cout<<"DBCommunication::storeTimes - Fin\n";
		}
		
		pair< string, pair<double, double> > parseDistribution(ptree dist){
			double param1 = 0;
			double param2 = 0;
			string type = dist.get<string>("type");
			if( type.compare("normal") == 0 ){
				param1 = dist.get<double>("params.mean");
				param2 = dist.get<double>("params.stddev");
			}
			else if( type.compare("uniform") == 0 ){
				param1 = dist.get<double>("params.a");
				param2 = dist.get<double>("params.b");
			}
			else{
				cout<<"DBCommunication::parseDistribution - Error, distribucion no soportada ("<<type<<")\n";
			}
			return pair<string, pair<double, double> >(type, pair<double, double>(param1, param2));
		}
		
		// Este metodo de prueba extrae todos los settings de una simulacion
		// Guarda los parametros de las distribuciones por cada feedback
		// Notar que feedback 0 partira con distribuciones uniformes
		void storeDistributions(uint32_t id, uint32_t scenario_id, string out_file, uint32_t feedback){
			
			cout<<"DBCommunication::storeDistributions - Inicio (id: "<<id<<", scenario_id: "<<scenario_id<<", \""<<out_file<<"\")\n";
			
			ptree settings = mongo.readSettings(db_name, collection_settings, id, feedback);
			// Guardar: genes, params (ordenados por mapa de nombre)
			boost::optional<ptree&> test_child;
			// Mapa <feedback, vector <dist_type, <param1, param2> > >
			vector< pair<string, pair<double, double> > > distributions;
				
			cout<<"DBCommunication::storeDistributions - Agregando parametros\n";
			
			std::stringstream ss;
			write_json(ss, settings);
			cout<<ss.str()<<"\n-----     -----\n";
			
			// Mapa para ordenar los parametros
			map<string, pair<string, pair<double, double> > > params;
			
			// genes
			for(auto &c : settings.get_child("individual.chromosomes")){
				uint32_t cid = c.second.get<uint32_t>("id");
				for(auto g : c.second.get_child("genes")){
					uint32_t gid = g.second.get<uint32_t>("id");
//						double value = g.second.get<double>("mutation.rate");
					string type = g.second.get<string>("mutation.rate.type");
					pair< string, pair<double, double> > dist = parseDistribution(g.second.get_child("mutation.rate.distribution"));
					string param_name = "chromosomes.";
					param_name += std::to_string(cid);
					param_name += ".genes.";
					param_name += std::to_string(gid);
					param_name += ".mutation.rate";
					params[param_name] = dist;
					cout<<"DBCommunication::storeDistributions - params["<<param_name<<"] = ["<<dist.first<<", "<<dist.second.first<<", "<<dist.second.second<<"]\n";
				}
			}
			
			// eventos (del escenario correcto)
			for(auto s : settings.get_child("scenarios")){
				uint32_t s_id = s.second.get<uint32_t>("id");
				if(s_id == scenario_id){
					for(auto e : s.second.get_child("events")){
						// En principio cada evento tiene timestamp y parametros
						// Los parametros que tengan type random deben ser agregados
					
						string e_id = e.second.get<string>("id");
						string e_type = e.second.get<string>("type");
						string param_base = "scenario.events.";
						param_base += e_id;
						pair< string, pair<double, double> > dist;
						
						string value_type = e.second.get<string>("timestamp.type");
						if( value_type.compare("random") == 0 ){
							dist = parseDistribution(e.second.get_child("timestamp.distribution"));
							string param_name = param_base + ".timestamp";
							params[param_name] = dist;
							cout<<"DBCommunication::storeDistributions - params["<<param_name<<"] = ["<<dist.first<<", "<<dist.second.first<<", "<<dist.second.second<<"]\n";
						}
						if( e_type.compare("create") == 0 ){
							string value_type = e.second.get<string>("params.population.size.type");
							if( value_type.compare("random") == 0 ){
								dist = parseDistribution(e.second.get_child("params.population.size.distribution"));
								string param_name = param_base + ".params.population.size";
								params[param_name] = dist;
								cout<<"DBCommunication::storeDistributions - params["<<param_name<<"] = ["<<dist.first<<", "<<dist.second.first<<", "<<dist.second.second<<"]\n";
							}
						}
						else if( e_type.compare("endsim") != 0 && e_type.compare("split") != 0 && e_type.compare("extinction") != 0 ){
							string value_type = e.second.get<string>("params.source.population.percentage");
							if( value_type.compare("random") == 0 ){
								dist = parseDistribution(e.second.get_child("params.source.population.percentage.distribution"));
								string param_name = param_base + ".params.source.population.percentage";
								params[param_name] = dist;
								cout<<"DBCommunication::storeDistributions - params["<<param_name<<"] = ["<<dist.first<<", "<<dist.second.first<<", "<<dist.second.second<<"]\n";
							}
						}
						
						
					}
				}
			}
			
			// Volcar resultados en distributions
			for( map<string, pair<string, pair<double, double> > >::iterator it_params = params.begin(); it_params != params.end(); it_params++ ){
				distributions.push_back(it_params->second);
			}
			
			// Escribir resultados
			ofstream escritor(out_file, fstream::trunc | fstream::out);
			escritor<<feedback<<"\t";
			for(unsigned int i = 0; i < distributions.size(); ++i){
				string type = distributions[i].first;
				pair<double, double> params = distributions[i].second;
				escritor<<type<<"\t"<<params.first<<"\t"<<params.second<<"\t";
			}
			escritor<<"\n";
			escritor.close();
			
			cout<<"DBCommunication::storeDistributions - Fin\n";
			
		}
		
		
		
		
		
		
		
		
		
		
};





#endif
