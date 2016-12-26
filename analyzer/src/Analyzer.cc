#include "Analyzer.h"
#include "../../util/MongoDB.h"
#include "../../util/Communication.h"
Analyzer::Analyzer(void){
   mongocxx::uri uri("mongodb://localhost:27017");
	this->_client=make_shared<mongocxx::client>(uri);
	mongocxx::database db=(*this->_client.get())["gdrift"];
	this->_collection_data=make_shared<mongocxx::collection>(db["data"]);
	this->_collection_results=make_shared<mongocxx::collection>(db["results"]);
}
Analyzer::~Analyzer(void){
   ;  
}
double distance(const boost::property_tree::ptree &_data,const boost::property_tree::ptree &_simulated){
	//TODO

   map<string,map<uint32_t,map<uint32_t,map<string,double>>>> indices_data;
   map<string,map<uint32_t,map<uint32_t,map<string,double>>>> indices_simulated;

   for(auto& p : _data.get_child("populations")){
      for(auto c : p.second.get_child("chromosomes")){
         for(auto g : c.second.get_child("genes")){
            for(auto i : g.second.get_child("indices"))
               indices_data[p.second.get<string>("name")][c.second.get<uint32_t>("id")][g.second.get<uint32_t>("id")][i.first]=boost::lexical_cast<double>(i.second.data());
         }
      }
   }
   for(auto& p : _simulated.get_child("populations")){
      for(auto c : p.second.get_child("chromosomes")){
         for(auto g : c.second.get_child("genes")){
            for(auto i : g.second.get_child("indices"))
               indices_simulated[p.second.get<string>("name")][c.second.get<uint32_t>("id")][g.second.get<uint32_t>("id")][i.first]=boost::lexical_cast<double>(i.second.data());
         }
      }
   }

   double d=0.0;
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
   }

   return(d);
}
void Analyzer::analyze(boost::property_tree::ptree &_frequest){
   
   switch(util::hash(_frequest.get<string>("type"))){
      case SIMULATED:{
								uint32_t id=_frequest.get<uint32_t>("id");
								if(this->_accepted.count(id)==0) return;								

								this->_batch_size[id]++;
								double dist=distance(this->_data[_frequest.get<uint32_t>("id")].get_child("posterior"),_frequest.get_child("posterior"));
cout << dist << endl;
								if(dist<=MAX_DIST){
									this->_accepted[id]++;
									_frequest.put("distance",dist);
									std::stringstream ss;
   								write_json(ss,_frequest);
									bsoncxx::document::value doc=bsoncxx::from_json(ss.str().c_str());
									auto r=this->_collection_results->insert_one(doc.view());
								}

//cout << "batch size::" << this->_batch_size[id] <<endl;

								if(this->_batch_size[id]==BATCH_LENGTH){
			    					boost::property_tree::ptree fresponse;
									fresponse.put("id",_frequest.get<uint32_t>("id"));

//cout << "accepted::" << this->_accepted[id] <<endl;
//cout << "max-number-of-simulations::" << _frequest.get<double>("max-number-of-simulations")<<endl;

									if(this->_accepted[id]<uint32_t(_frequest.get<double>("max-number-of-simulations")*0.1)){
										this->_batch_size[id]=0U;
//cout << "continue" << endl;
										fresponse.put("type","continue");
									}
									else{
										this->_accepted.erase(this->_accepted.find(id));
										this->_batch_size.erase(this->_batch_size.find(id));
//cout << "finalize" << endl;
										fresponse.put("type","finalize");
									}

									comm::send("http://citiaps2.diinf.usach.cl","1987","scheduler",fresponse);
								}
                        /*this->_simulated[_frequest.get<uint32_t>("id")][_frequest.get<uint32_t>("batch_size")*BATCH_LENGTH+_frequest.get<uint32_t>("run")]=_frequest;
			               this->_distances[_frequest.get<uint32_t>("id")][_frequest.get<uint32_t>("batch_size")*BATCH_LENGTH+_frequest.get<uint32_t>("run")]=distance(this->_data[_frequest.get<uint32_t>("id")].get_child("posterior"),_frequest.get_child("posterior"));
cout << this->_distances[_frequest.get<uint32_t>("id")][_frequest.get<uint32_t>("batch_size")*BATCH_LENGTH+_frequest.get<uint32_t>("run")] << endl;*/
								/*TODO*/
								/*int counter=0;
			               if(!(this->_simulated[_frequest.get<uint32_t>("id")].size()%BATCH_LENGTH)){
									for(auto& i : this->_distances[_frequest.get<uint32_t>("id")]){
										if(i.second<15.0) counter++;
									}
cout <<"Number of Results::"<<counter << endl;	
			    					boost::property_tree::ptree fresponse;
									fresponse.put("id",_frequest.get<uint32_t>("id"));
									if(counter<=50)
									   fresponse.put("type","continue");
									else
									   fresponse.put("type","finalize");
									send(fresponse);
			               }*/
								/*TODO*/
                        break;
                     };
      case DATA:  {
						
							this->_accepted[_frequest.get<uint32_t>("id")]=0U;	
							this->_batch_size[_frequest.get<uint32_t>("id")]=0U;	
				
                     boost::property_tree::ptree fposterior;
                     fposterior.put("id",_frequest.get<string>("id"));
                     fposterior.put("type","data");
                  
                     boost::property_tree::ptree fpopulations;
                     Population* all=new Population("summary");
                     for(auto& population : _frequest.get_child("populations")){
                     	Population* p=new Population(Ploidy(_frequest.get<uint32_t>("ploidy")),population.second);
                        fpopulations.push_back(std::make_pair("",p->indices(1.0)));
                        all->merge(p);  
								delete p;
							}
                     fpopulations.push_back(std::make_pair("",all->indices(1.0)));
							delete all;

                     fposterior.push_back(make_pair("populations",fpopulations));

							/*save*/
							_frequest.push_back(make_pair("posterior",fposterior));
							std::stringstream ss;
   						write_json(ss,_frequest);
							bsoncxx::document::value doc=bsoncxx::from_json(ss.str().c_str());
							auto r=this->_collection_data->insert_one(doc.view());
							/*save*/

                     this->_data[_frequest.get<uint32_t>("id")]=_frequest;
		
                     break;
                  };
      default: {
                  cerr << "Unknown Result Type" << endl;
                  exit(EXIT_FAILURE);
               };
   }
}
