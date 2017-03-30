#include "Controller.h"
Controller::Controller(const boost::property_tree::ptree &_fhosts,const uint32_t &_id):Node(_fhosts){
	this->_id=_id;
}
boost::property_tree::ptree indices(map<string,Sample*> _samples){
   boost::property_tree::ptree fpopulations;

   Sample* all=new Sample("summary");
	for(map<string,Sample*>::iterator i=_samples.begin();i!=_samples.end();i++){
      fpopulations.push_back(std::make_pair("",i->second->indices()));
      all->merge(i->second);
	}
   fpopulations.push_back(std::make_pair("",all->indices()));
   delete all;

   return(fpopulations);
}
boost::property_tree::ptree Controller::run(boost::property_tree::ptree &_frequest){
	boost::property_tree::ptree findividual=_frequest.get_child("individual");
   boost::property_tree::ptree fscenario=_frequest.get_child("scenario");

   boost::property_tree::ptree fresponse;
   boost::property_tree::ptree fprior;
   boost::property_tree::ptree fposterior;

   fresponse.put("id",_frequest.get<string>("id"));
   fresponse.put("run",_frequest.get<string>("run"));
   fresponse.put("batch",_frequest.get<string>("batch"));
   fresponse.put("type","simulated");
   fresponse.put("max-number-of-simulations",_frequest.get<string>("max-number-of-simulations"));
   
   Simulator* sim=new Simulator(_frequest);

	//std::stringstream ss;
	//char filename[256];
	//sprintf(filename,"out-%s-%s-%s.json",_frequest.get<string>("id").c_str(),_frequest.get<string>("run").c_str(),_frequest.get<string>("batch").c_str());
   //write_json(filename,_frequest);
   //cout << ss.str() << endl;

   //fprior.push_back(std::make_pair("populations",indices(sim->populations())));

	//printf("%s %s %s\n",_frequest.get<string>("id").c_str(),_frequest.get<string>("run").c_str(),_frequest.get<string>("batch").c_str());
   sim->run();
	//printf("%s %s %s\n",_frequest.get<string>("id").c_str(),_frequest.get<string>("run").c_str(),_frequest.get<string>("batch").c_str());

	//printf("%s %s %s\n",_frequest.get<string>("id").c_str(),_frequest.get<string>("run").c_str(),_frequest.get<string>("batch").c_str());
   fposterior.push_back(std::make_pair("populations",indices(sim->samples())));
	//printf("%s %s %s\n",_frequest.get<string>("id").c_str(),_frequest.get<string>("run").c_str(),_frequest.get<string>("batch").c_str());

	//sprintf(filename,"err-%s-%s-%s.json",_frequest.get<string>("id").c_str(),_frequest.get<string>("run").c_str(),_frequest.get<string>("batch").c_str());
   //write_json(filename,fposterior);
   delete sim;

   fresponse.push_back(make_pair("prior",fprior));
   fresponse.push_back(make_pair("posterior",fposterior));
   fresponse.push_back(make_pair("individual",findividual));
   fresponse.push_back(make_pair("scenario",fscenario));
   
	comm::send(this->_fhosts.get<string>("analyzer.host"),this->_fhosts.get<string>("analyzer.port"),this->_fhosts.get<string>("analyzer.resource"),fresponse);
	return(_frequest);
}
Controller::~Controller(void){
	;
}
