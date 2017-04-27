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

#include "Mongo.h"

using namespace std;
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
		string collection_statistics;

	public:
		DBCommunication(){
			db_name = "gdrift";
			collection_data = "data";
			collection_results = "results";
			collection_settings = "settings";
			collection_statistics = "statistics";
		}
		
		// TODO: agregar collection_statistics a los parametros (para eso hay que agregarlo tambien a hosts.json)
		DBCommunication(string _uri, string _db_name, string _collection_data, string _collection_results, string _collection_settings){
			uri = _uri;
			mongo = util::Mongo(uri);
			db_name = _db_name;
			collection_data = _collection_data;
			collection_results = _collection_results;
			collection_settings = _collection_settings;
			collection_statistics = "statistics";
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
						else if( e_type.compare("endsim") != 0 && e_type.compare("split") ){
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
						else if( etype.compare("endsim") != 0 && etype.compare("split") != 0 ){
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
			
		uint32_t getResults(uint32_t id, uint32_t scenario_id, uint32_t feedback, vector<vector<double>> &params, map<uint32_t, vector<string>> &events_params, uint32_t total_params, vector<vector<double>> &statistics){
			
			cout<<"DBCommunication::getResults - Inicio ("<<id<<", "<<scenario_id<<", "<<feedback<<", total_params: "<<total_params<<")\n";
			
			list<boost::property_tree::ptree> results;
			mongo.readStatistics(results, db_name, collection_results, id, scenario_id, feedback);
			
			unsigned int inserts = 0;
			unsigned int count = 0;
			double value = 0;
			boost::optional< boost::property_tree::ptree& > child;
			map<string, double> params_res;
			map<string, map<uint32_t, map<uint32_t, map<string, double>>>> stats_res;
			
			for(auto &json : results){
//				if(count >= 3) break;
				
//				cout<<"Res["<<count++<<"]:\n";
				
//				std::stringstream ss;
//				write_json(ss, json);
//				cout<<ss.str()<<"\n";

				// scenario.events.0.params.population.size
				// scenario.events.2.params.source.population.percentage
				
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
//				vector<double> values(params_res.size(), 0.0);
				vector<double> values;
				for(map<string, double>::iterator it = params_res.begin(); it != params_res.end(); it++){
					values.push_back(it->second);
				}
				params_res.clear();
				
				if( values.size() != total_params){
					continue;
				}
				
				//test
//				cout<<"DBCommunication::getResults - Agregando res ("<<values[0];
//				for(unsigned int i = 1; i < values.size(); ++i){
//					cout<<", "<<values[i];
//				}
//				cout<<")\n";

				params.push_back(values);
				
				for(auto &p : json.get_child("posterior.populations")){
					string name = p.second.get<string>("name");
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
				statistics.push_back(stats);
				stats_res.clear();
				
				++inserts;
				
			}
			
			cout<<"DBCommunication::getResults - Fin (inserts: "<<inserts<<")\n";
			return count;
		}
		
		void storeTrainingResults(boost::property_tree::ptree &result){
			
			cout<<"DBCommunication::storeTrainingResults - Inicio\n";
			
			cout<<"Analyzer::run - Resultados de entrenamiento: \n";
			std::stringstream ss;
			write_json(ss, result);
			cout << ss.str() << endl;
			
			// Quizas sea necesario crear una coleccion adicional
//			mongo.write(db_name, collection_statistics, json);
			
			
			cout<<"DBCommunication::storeTrainingResults - Fin\n";
			
		}
		
		
		
		
};





#endif
