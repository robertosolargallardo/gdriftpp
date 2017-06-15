#ifndef _MONGO_H_
#define _MONGO_H_
#include <string>
#include <list>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>

using namespace std;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;

namespace util{
class Mongo{
	private:
		mongocxx::client client;
		string uri_name;

	public:
		Mongo(){}

		Mongo(const string &_uri_name){
			uri_name = _uri_name;
			cout<<"Mongo - uri_name: \""<<uri_name<<"\"\n";
			mongocxx::uri uri(uri_name);
			client = mongocxx::client(uri);
		}

		Mongo(const Mongo &original){
			uri_name = original.uri_name;
			mongocxx::uri uri(uri_name);
			client = mongocxx::client(uri);
		}
		
		~Mongo(){}
		
		Mongo& operator=(const Mongo &original){
			if (this != &original){
				uri_name = original.uri_name;
				mongocxx::uri uri(uri_name);
				client = mongocxx::client(uri);
			}
			return *this;
		}
		
		void write(const string &db_name, const string &collection_name, boost::property_tree::ptree &json){
//			cout<<"Mongo::write\n";
			
			json.erase("_id");
			std::stringstream ss;
			
//			write_json(ss, json);
//			cout<<ss.str()<<"\n";
			
			write_json(ss, json);
			bsoncxx::document::value doc = bsoncxx::from_json(ss.str().c_str());
			auto r = client[db_name][collection_name].insert_one(doc.view());
		}
		
		list<boost::property_tree::ptree> read(const string &db_name, const string &collection_name, uint32_t id){
			list<boost::property_tree::ptree> results;
			read(results, db_name, collection_name, id);
			return results;
		}
		
		unsigned int read(list<boost::property_tree::ptree> &results, const string &db_name, const string &collection_name, uint32_t id){
			mongocxx::cursor cursor = client[db_name][collection_name].find(document{} << "id" << std::to_string(id) << finalize);
			boost::property_tree::ptree json;
			unsigned int inserted = 0;
			for(auto doc : cursor){
				istringstream is(bsoncxx::to_json(doc));
				read_json(is, json);
				results.push_back(json);
				++inserted;
			}
			return inserted;
		}
		
		list<boost::property_tree::ptree> read_all(const string &db_name, const string &collection_name){
			list<boost::property_tree::ptree> results;
			read_all(results, db_name, collection_name);
			return results;
		}
		
		unsigned int read_all(list<boost::property_tree::ptree> &results, const string &db_name, const string &collection_name){
			mongocxx::cursor cursor = client[db_name][collection_name].find(document{} << finalize); 
			boost::property_tree::ptree json;
			unsigned int inserted = 0;
			for(auto doc : cursor){
				istringstream is(bsoncxx::to_json(doc));
				read_json(is, json);
				results.push_back(json);
			}
			return inserted;
		}
		
		list<boost::property_tree::ptree> readStatistics(const string &db_name, const string &collection_name, uint32_t id){
			list<boost::property_tree::ptree> results;
			read(results, db_name, collection_name, id);
			return results;
		}
		
		unsigned int readStatistics(list<boost::property_tree::ptree> &results, const string &db_name, const string &collection_name, uint32_t id, uint32_t scenid, uint32_t feedback){
			mongocxx::cursor cursor = client[db_name][collection_name].find(document{} << "id" << std::to_string(id) << "scenario.id" << std::to_string(scenid) << "feedback" << std::to_string(feedback) << "distance" << open_document << "$ne" << "inf"<< close_document << finalize);
			boost::property_tree::ptree json;
			unsigned int inserted = 0;
			for(auto doc : cursor){
				istringstream is(bsoncxx::to_json(doc));
				read_json(is, json);
//				json = json.get_child("posterior");
				
//				std::stringstream ss;
//				write_json(ss, json);
//				cout<<ss.str()<<"\n";
				
				results.push_back(json);
				++inserted;
			}
			return inserted;
		}
		
		list<boost::property_tree::ptree> readStatistics(const string &db_name, const string &collection_name, uint32_t id, uint32_t scenid, uint32_t feedback){
			list<boost::property_tree::ptree> results;
			readStatistics(results, db_name, collection_name, id, scenid, feedback);
			return results;
		}
		
		boost::property_tree::ptree readSettings(const string &db_name, const string &collection_name, uint32_t id, uint32_t feedback){
			mongocxx::cursor cursor = client[db_name][collection_name].find(document{} << "id" << std::to_string(id) << "feedback" << std::to_string(feedback) << finalize);
			boost::property_tree::ptree json;
			for(auto doc : cursor){
				istringstream is(bsoncxx::to_json(doc));
				read_json(is, json);
				
//				std::stringstream ss;
//				write_json(ss, json);
//				cout<<ss.str()<<"\n";
				
				break;
			}
			return json;
			/*
			// Aqui habria que usar find_one
			// Desúes revisar como usarlo correctamente
			mongocxx::stdx::optional<bsoncxx::document::value> maybe_result = client[db_name][collection_name].find_one(document{} << "id" << std::to_string(id) <<  finalize);
			if(maybe_result) {
				// Do something with *maybe_result;
			}
			*/
		}
		
		unsigned int readSettings(list<boost::property_tree::ptree> &results, const string &db_name, const string &collection_name, uint32_t id){
			cout<<"Mongo::readSettings - Inicio ("<<db_name<<", "<<collection_name<<", "<<id<<")\n";
			mongocxx::cursor cursor = client[db_name][collection_name].find(document{} << "id" << std::to_string(id) << finalize);
			boost::property_tree::ptree json;
			unsigned int inserted = 0;
			for(auto doc : cursor){
				istringstream is(bsoncxx::to_json(doc));
				read_json(is, json);
				
//				std::stringstream ss;
//				write_json(ss, json);
//				cout<<ss.str()<<"\n";

				results.push_back(json);
				++inserted;
			}
			cout<<"Mongo::readSettings - Fin ("<<inserted<<")\n";
			return inserted;
			
		}
		
		unsigned int readResults(list<boost::property_tree::ptree> &results, const string &db_name, const string &collection_name, uint32_t id, uint32_t scenid, uint32_t feedback){
			mongocxx::cursor cursor = client[db_name][collection_name].find(document{} << "id" << std::to_string(id) << "scenario.id" << std::to_string(scenid) << "feedback" << std::to_string(feedback) << finalize);
			boost::property_tree::ptree json;
			unsigned int inserted = 0;
			for(auto doc : cursor){
				istringstream is(bsoncxx::to_json(doc));
				read_json(is, json);
				
				results.push_back(json);
				++inserted;
			}
			return inserted;
		}
		
		void setStatus(const string &db_name, const string &collection_name, uint32_t id, string status){
			client[db_name][collection_name].update_many(
				document{} << "id" << std::to_string(id) << finalize,
				document{} << "$set" << open_document << "status" << status << close_document << finalize
			);
		}
		
};












}
#endif
