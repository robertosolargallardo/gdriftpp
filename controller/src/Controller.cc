#include "Controller.h"
Controller::Controller(const boost::property_tree::ptree &_fhosts,const uint32_t &_id):Node(_fhosts){
	this->_id=_id;
}
boost::property_tree::ptree indices(const vector<Population*> &_populations){
   boost::property_tree::ptree fpopulations;

   Population* all=new Population("summary");
   for(auto& population : _populations){
      fpopulations.push_back(std::make_pair("",population->indices(0.1)));
      all->merge(population);
   }
   fpopulations.push_back(std::make_pair("",all->indices(0.1)));
   delete all;

   return(fpopulations);
}
void Controller::run(boost::property_tree::ptree &_frequest){
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

   fprior.push_back(std::make_pair("populations",indices(sim->populations())));
   sim->run();
   fposterior.push_back(std::make_pair("populations",indices(sim->populations())));
   delete sim;

   fresponse.push_back(make_pair("prior",fprior));
   fresponse.push_back(make_pair("posterior",fposterior));
   fresponse.push_back(make_pair("individual",findividual));
   fresponse.push_back(make_pair("scenario",fscenario));
   
	comm::send(this->_fhosts.get<string>("analyzer.host"),this->_fhosts.get<string>("analyzer.port"),this->_fhosts.get<string>("analyzer.resource"),fresponse);
}
Controller::~Controller(void){
	;
}
