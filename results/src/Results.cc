#include "Results.h"
#define DELTA 5.0
Results::Results(boost::property_tree::ptree &_fhosts):Node(_fhosts){
	this->_mongo=make_shared<util::Mongo>(this->_fhosts.get<string>("database.uri"));
}
boost::property_tree::ptree Results::run(boost::property_tree::ptree &_frequest){
	switch(util::hash(_frequest.get<string>("type"))){
		case LIST:	return(this->list());
		case GET:	return(this->get(_frequest.get<uint32_t>("id")));
		default:	{
						cerr << "Error::Unknown Result Type"<<endl;
						exit(EXIT_FAILURE);
					}
	}
	return(_frequest);
}
arma::rowvec to_vector(const boost::property_tree::ptree &_fresults){
   map<string,map<uint32_t,map<uint32_t,map<string,double>>>> indices;
   size_t length=0;

   for(auto& p : _fresults.get_child("populations")){
      for(auto c : p.second.get_child("chromosomes")){
         for(auto g : c.second.get_child("genes")){
            for(auto i : g.second.get_child("indices")){
               indices[p.second.get<string>("name")][c.second.get<uint32_t>("id")][g.second.get<uint32_t>("id")][i.first]=boost::lexical_cast<double>(i.second.data());
               length++;
            }
         }
      }
   }
   arma::rowvec v(length);
   int position=0;

   for(auto i : indices)
      for(auto j : i.second)
         for(auto k : j.second)
            for(auto l : k.second){
               v(position++)=indices[i.first][j.first][k.first][l.first];
            }
   return(v);
}

boost::property_tree::ptree Results::pca(const uint32_t &_id){
	std::list<boost::property_tree::ptree> fdata=this->_mongo->read("gdrift","data",_id);
	std::list<boost::property_tree::ptree> fresults=this->_mongo->read("gdrift","results",_id);

	vector<int> id;
   int i=0;
   arma::mat X;

	write_json("data.json",(*fdata.begin()));
   auto vdata=to_vector((*fdata.begin()).get_child("posterior"));

   id.push_back(-1);
   X.insert_rows(i++,vdata);

   for(auto doc : fresults){
      auto vresults=to_vector(doc.get_child("posterior"));
      id.push_back(doc.get<int>("scenario.id"));
      X.insert_rows(i++,vresults);
   }

	arma::mat coeff;
   arma::mat score;
   arma::vec latent;
   arma::vec tsquared;

   princomp(coeff,score,latent,tsquared,X);

   arma::mat x=score*coeff.row(0).t();
   arma::mat y=score*coeff.row(1).t();

   boost::property_tree::ptree fpca;
   boost::property_tree::ptree ftuples,ftuple;

   for(unsigned int i=0;i<x.n_rows;i++){
      char name[256];
      ftuple.put("x",x(i,0));
      ftuple.put("y",y(i,0));
      if(id[i]==-1)
         sprintf(name,"Observed Data");
      else
         sprintf(name,"Scenario %d",id[i]);
      ftuple.put("scenario",name);
      ftuple.put("type","posterior");

      ftuples.push_back(make_pair("",ftuple));
   }
	return(ftuples);
}
void match(boost::property_tree::ptree _fscenario,boost::property_tree::ptree _fresults,map<string,vector<double>> &data){
	map<uint32_t,boost::property_tree::ptree> results;
	for(auto event : _fresults.get_child("events"))
		results[event.second.get<uint32_t>("id")]=event.second;

	for(auto event : _fscenario.get_child("events")){
		if(event.second.get<string>("timestamp.type")!="fixed"){
			data["T."+event.second.get<string>("id")].push_back(results[event.second.get<uint32_t>("id")].get<uint32_t>("timestamp"));
		}
	}
}
map<uint32_t,uint32_t> cumulative(boost::property_tree::ptree _fscenario,const string str,const size_t N){
	std::vector<std::string> v;
	boost::split(v,str,boost::is_any_of("."),boost::token_compress_on);

	map<uint32_t,uint32_t> cum;

	for(auto event : _fscenario.get_child("events")){
		if(event.second.get<uint32_t>("id")==boost::lexical_cast<uint32_t>(v[1])){

			if(event.second.get<string>("timestamp.type")!="fixed"){//TODO uniform
				uint32_t min=event.second.get<uint32_t>("timestamp.min-value");
				uint32_t max=event.second.get<uint32_t>("timestamp.max-value");
				for(auto i=min;i<max;i+=DELTA){
					cum[i]=N/((max-min)/DELTA);
				}
			}
		}
	}
	return(cum);
}
map<uint32_t,uint32_t> histo(vector<double> data){
	sort(data.begin(),data.end());
	double min=data[0];
	double max=data[data.size()-1];

	map<uint32_t,uint32_t> histogram;

	for(auto i : data){
		auto d=uint32_t(double(i)/DELTA);
		if(histogram.count(d*DELTA)==0) histogram[d*DELTA]=0;
		histogram[d*DELTA]++;
	}
	return(histogram);
}
boost::property_tree::ptree Results::estimations(const uint32_t &_id){
	std::list<boost::property_tree::ptree> fsettings=this->_mongo->read("gdrift","settings",_id);
	
	if(fsettings.empty()) return(boost::property_tree::ptree());

	map<uint32_t,map<string,vector<double>>> data;
	
	map<uint32_t,boost::property_tree::ptree> scenarios;
	boost::property_tree::ptree fscenarios=(*fsettings.begin()).get_child("scenarios");
	for(auto fscenario : fscenarios)
		scenarios[fscenario.second.get<uint32_t>("id")]=fscenario.second;

	std::list<boost::property_tree::ptree> results=this->_mongo->read("gdrift","results",(*fsettings.begin()).get<uint32_t>("id"));

	for(auto result : results){
		boost::property_tree::ptree fscenario=result.get_child("scenario");
		match(scenarios[fscenario.get<uint32_t>("id")],fscenario,data[fscenario.get<uint32_t>("id")]);
	}
	
	char name[256];
	boost::property_tree::ptree fresult;
	fscenarios.clear();
	for(auto i : data){
		boost::property_tree::ptree festimations,fscenario;
		sprintf(name,"scenario %d",i.first);
		fscenario.put("name",name);
		fscenario.put("id",i.first);

		for(auto j : data[i.first]){
			boost::property_tree::ptree festimation,fvalues,fvalue;
			festimation.put("parameter",j.first);

			map<uint32_t,uint32_t> h1=histo(j.second);
			map<uint32_t,uint32_t> h2=cumulative(scenarios[i.first],j.first,j.second.size());

			for(auto k : h2){
				fvalue.put("x",k.first);
				fvalue.put("y1",h1[k.first]);
				fvalue.put("y2",k.second);
				fvalues.push_back(make_pair("",fvalue));
			}
			festimation.add_child("values",fvalues);
			festimations.push_back(make_pair("",festimation));
		}
		fscenario.add_child("estimations",festimations);
		fscenarios.push_back(make_pair("",fscenario));
	}
	
	fresult.add_child("scenarios",fscenarios);
	
	return(fresult);
}
boost::property_tree::ptree Results::get(const uint32_t &_id){
	boost::property_tree::ptree fresults;
	boost::property_tree::ptree fpca=this->pca(_id);
	boost::property_tree::ptree festimations=this->estimations(_id);

	fresults.add_child("estimations",festimations);
	fresults.add_child("pca",fpca);
	return(fresults);
}
boost::property_tree::ptree Results::list(void){
	std::list<boost::property_tree::ptree> fsettings=this->_mongo->read_all("gdrift","settings");

	boost::property_tree::ptree fresults,ftuples;
	for(auto fsetting : fsettings){
		boost::property_tree::ptree ftuple;
		ftuple.put("id",fsetting.get<std::string>("id"));
		ftuples.push_back(make_pair("",ftuple));
	}
	fresults.add_child("results",ftuples);
	return(fresults);
}
