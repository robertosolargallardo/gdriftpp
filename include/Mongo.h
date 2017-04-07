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

namespace util{
class Mongo{
	private:
		mongocxx::client _client;

	public:
		Mongo(){}

		Mongo(const string &_uri_name){
			mongocxx::uri uri(_uri_name);
			_client = mongocxx::client(uri);
		}
		
		~Mongo(void){}
		
		void write(const string &_db_name,const string &_collection_name,boost::property_tree::ptree &_json){
			std::stringstream ss;
			write_json(ss,_json);
			bsoncxx::document::value doc=bsoncxx::from_json(ss.str().c_str());
			auto r = _client[_db_name][_collection_name].insert_one(doc.view());
		}
		
		list<boost::property_tree::ptree> read(const string &_db_name, const string &_collection_name, const uint32_t &_id){
			list<boost::property_tree::ptree> results;
			read(results, _db_name, _collection_name, _id);
			return results;
		}
		
		unsigned int read(list<boost::property_tree::ptree> &results, const string &_db_name, const string &_collection_name, const uint32_t &_id){
			mongocxx::cursor cursor = _client[_db_name][_collection_name].find(document{} << "id" << boost::lexical_cast<string>(_id) << finalize);
			boost::property_tree::ptree json;
			unsigned int inserted = 0;
			for(auto doc : cursor){
				istringstream is(bsoncxx::to_json(doc));
				read_json(is,json);
				results.push_back(json);
				++inserted;
			}
			return inserted;
		}
		
		list<boost::property_tree::ptree> read_all(const string &_db_name, const string &_collection_name){
			list<boost::property_tree::ptree> results;
			read_all(results, _db_name, _collection_name);
			return results;
		}
		
		unsigned int read_all(list<boost::property_tree::ptree> &results, const string &_db_name, const string &_collection_name){
			mongocxx::cursor cursor = _client[_db_name][_collection_name].find(document{} << finalize); 
			boost::property_tree::ptree json;
			unsigned int inserted = 0;
			for(auto doc : cursor){
				istringstream is(bsoncxx::to_json(doc));
				read_json(is,json);
				results.push_back(json);
			}
			return inserted;
		}
		
		
};
}
#endif
