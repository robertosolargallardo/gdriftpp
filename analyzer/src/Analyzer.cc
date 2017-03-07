#include "Analyzer.h"
Analyzer::Analyzer(boost::property_tree::ptree &_fhosts):Node(_fhosts){
	this->_mongo=make_shared<util::Mongo>(this->_fhosts.get<string>("database.uri"));
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
					a=indices_data[i.first][j.first][k.first][l.first];
					b=indices_simulated[i.first][j.first][k.first][l.first];
					diff=(a-b)/max(a,b);
					s+=diff*diff;
					n+=1.0;
            }
         }
      }
   }
   return(sqrt(s/n));
}
boost::property_tree::ptree Analyzer::run(boost::property_tree::ptree &_frequest){
   
   switch(util::hash(_frequest.get<string>("type"))){
      case SIMULATED:{
								uint32_t id=_frequest.get<uint32_t>("id");
								if(this->_accepted.count(id)==0) return(_frequest);

								this->_batch_size[id]++;
								double dist=distance(this->_data[_frequest.get<uint32_t>("id")].get_child("posterior"),_frequest.get_child("posterior"));
								cout << dist << endl;

								if(dist<=MAX_DIST){
									this->_accepted[id]++;
									_frequest.put("distance",dist);
									this->_mongo->write(this->_fhosts.get<string>("database.name"),this->_fhosts.get<string>("database.collections.results"),_frequest);
								}

								if(this->_batch_size[id]==(BATCH_LENGTH*this->_fhosts.get_child("controller").size())){
			    					boost::property_tree::ptree fresponse;
									fresponse.put("id",_frequest.get<uint32_t>("id"));

									if(this->_accepted[id]<uint32_t(_frequest.get<double>("max-number-of-simulations")*PERCENT)){
										this->_batch_size[id]=0U;
										fresponse.put("type","continue");
									}
									else{
										this->_accepted.erase(this->_accepted.find(id));
										this->_batch_size.erase(this->_batch_size.find(id));
										fresponse.put("type","finalize");
									}

									comm::send(this->_fhosts.get<string>("scheduler.host"),this->_fhosts.get<string>("scheduler.port"),this->_fhosts.get<string>("scheduler.resource"),fresponse);
								}
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

							_frequest.push_back(make_pair("posterior",fposterior));
                     this->_data[_frequest.get<uint32_t>("id")]=_frequest;

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
