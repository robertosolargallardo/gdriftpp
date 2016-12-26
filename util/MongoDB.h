#ifndef _MONGODB_H_
#define _MONGODB_H_
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

namespace Util{
class MongoDB{
	private:	 shared_ptr<mongocxx::client>	_client;

	public:	MongoDB(void){
					;
				}
				MongoDB(const string &_uri_name){
					mongocxx::uri uri(_uri_name);
				   this->_client=make_shared<mongocxx::client>(uri);
				}
				void write(const string &_db_name,const string &_collection_name,boost::property_tree::ptree &_json){
					 std::stringstream ss;
                write_json(ss,_json);
                bsoncxx::document::value doc=bsoncxx::from_json(ss.str().c_str());
                auto r=(*this->_client.get())[_db_name][_collection_name].insert_one(doc.view());
				}
				list<boost::property_tree::ptree> read(const string &_db_name,const string &_collection_name,const uint32_t &_id){
					mongocxx::cursor cursor=(*this->_client.get())[_db_name][_collection_name].find(document{} << "id" << boost::lexical_cast<string>(_id) << finalize); 

					list<boost::property_tree::ptree> results;
					boost::property_tree::ptree json;
					for(auto doc : cursor){
				      istringstream is(bsoncxx::to_json(doc));
      				read_json(is,json);
						results.push_back(json);
   				}
				
					return(results);
				}
				list<boost::property_tree::ptree> read_all(const string &_db_name,const string &_collection_name){
					mongocxx::cursor cursor=(*this->_client.get())[_db_name][_collection_name].find(document{} << finalize); 
	
					list<boost::property_tree::ptree> results;
					boost::property_tree::ptree json;
					for(auto doc : cursor){
				      istringstream is(bsoncxx::to_json(doc));
      				read_json(is,json);
						results.push_back(json);
   				}
				
					return(results);
				}
				~MongoDB(void){
					;
				}
};
}
#endif
