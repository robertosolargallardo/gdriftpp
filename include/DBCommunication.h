#ifndef _DB_COMMUNICATION_H_
#define _DB_COMMUNICATION_H_

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <map>

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

	public:
		DBCommunication(){
			db_name = "gdrift";
			collection_data = "data";
			collection_results = "results";
			collection_settings = "settings";
		}
		
		DBCommunication(string _uri, string _db_name, string _collection_data, string _collection_results, string _collection_settings){
			uri = _uri;
			mongo = util::Mongo(uri);
			db_name = _db_name;
			collection_data = _collection_data;
			collection_results = _collection_results;
			collection_settings = _collection_settings;
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
		
		vector<string> getEventsIds(uint32_t id, uint32_t scenario_id, uint32_t feedback){
			vector<string> id_events;
			boost::property_tree::ptree settings = mongo.readSettings(db_name, collection_settings, id, feedback);
			
			for(auto s : settings.get_child("scenarios")){
				uint32_t s_id = s.second.get<uint32_t>("id");
				if(s_id == scenario_id){
					for(auto e : s.second.get_child("events")){
						// En principio cada evento tiene timestamp y parametros
						// Los parametros que tengan type random deben ser agregados
						
						string e_id = e.second.get<string>("id");
						string e_type = e.second.get<string>("type");
						
						if( e_type.compare("endsim") != 0 && e_type.compare("split") ){
							cout<<"DBCommunication::getEventsIds - id_events.push_back("<<e_id<<")\n";
							id_events.push_back(e_id);
						}
						
					}
				}
			}
			
			return id_events;
		}
			
		uint32_t getResults(uint32_t id, uint32_t scenario_id, uint32_t feedback, vector<double> &params, vector<string> &id_events, vector<double> &statistics){
			
			cout<<"DBCommunication::getResults - Inicio\n";
			
			list<boost::property_tree::ptree> results;
			mongo.readStatistics(results, db_name, collection_results, id, scenario_id, feedback);
			
			unsigned int inserts = 0;
			unsigned int count = 0;
			double value = 0;
			for(auto &json : results){
				if(count >= 3) break;
				cout<<"Res["<<count++<<"]:\n";
				
				std::stringstream ss;
				write_json(ss, json);
				cout<<ss.str()<<"\n";

				// scenario.events.0.params.population.size
				// scenario.events.2.params.source.population.percentage
							
				for(unsigned int i = 0; i < id_events.size(); ++i){
					string field = "scenario.events.";
					field += id_events[i];
					
					boost::property_tree::ptree::assoc_iterator it = json.find(field + ".params.population.size");
					if( it != json.not_found() ){
						value = json.get<double>(field + ".params.population.size");
						cout<<field<<".params.population.size: "<<value<<"\n";
					}
					
					it = json.find(field + ".source.population.percentage");
					if( it != json.not_found() ){
						value = json.get<double>(field + ".source.population.percentage");
						cout<<field<<".source.population.percentage: "<<value<<"\n";
					}
					
				}
				
				/*
				map<string, map<uint32_t, map<uint32_t, map<string, double>>>> statistics_map;
				parseIndices(json.get_child("posterior"), statistics_map);
				for(auto i : statistics_map){
					for(auto j : i.second){
						for(auto k : j.second){
							for(auto l : k.second){
								double value = statistics_map[i.first][j.first][k.first][l.first];
								cout<<"statistics_map["<<i.first<<"]["<<j.first<<"]["<<k.first<<"]["<<l.first<<"]: "<<value<<"\n";
							}
						}
					}
				}
				++inserts;
				*/
			}
			
			
			
			cout<<"DBCommunication::getResults - Fin (inserts: "<<inserts<<")\n";
			return count;
		}
		
		
		
		
		
};





#endif
