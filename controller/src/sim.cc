#include <Simulator.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <curl/curl.h>
#include <memory>
#include <cstdlib>
#include <restbed>
#include <armadillo>

#include "Semaphore.h"
#include "../../util/Communication.h"

using namespace std;
using namespace restbed;

random_device seed;
mt19937 rng(seed());

shared_ptr<Semaphore> semaphore=nullptr;

boost::property_tree::ptree get_indices(const vector<Population*> &populations){
   boost::property_tree::ptree fpopulations;

	Population* all=new Population("summary");
  	for(auto& population : populations){
  		fpopulations.push_back(std::make_pair("",population->indices(0.1)));
 	 	all->merge(population);
  	}
  	fpopulations.push_back(std::make_pair("",all->indices(0.1)));
	delete all;

	return(fpopulations);
}
void run(const boost::property_tree::ptree &_fsettings){
   semaphore->lock();
	stringstream ss;
	write_json(ss,_fsettings);
	cout << ss.str() << endl;

   boost::property_tree::ptree findividual=_fsettings.get_child("individual");
   boost::property_tree::ptree fscenario=_fsettings.get_child("scenario");

   boost::property_tree::ptree fresponse;
   boost::property_tree::ptree fprior;
   boost::property_tree::ptree fposterior;

   fresponse.put("id",_fsettings.get<string>("id"));
   fresponse.put("run",_fsettings.get<string>("run"));
   fresponse.put("batch",_fsettings.get<string>("batch"));
   fresponse.put("type","simulated");
   fresponse.put("max-number-of-simulations",_fsettings.get<string>("max-number-of-simulations"));
	
	Simulator* sim=new Simulator(_fsettings);

   fprior.push_back(std::make_pair("populations",get_indices(sim->populations())));
	sim->run();
   fposterior.push_back(std::make_pair("populations",get_indices(sim->populations())));
	delete sim;

   fresponse.push_back(make_pair("prior",fprior));
   fresponse.push_back(make_pair("posterior",fposterior));
   fresponse.push_back(make_pair("individual",findividual));
   fresponse.push_back(make_pair("scenario",fscenario));
	
   comm::send("citiaps2.diinf.usach.cl","1986","analyzer",fresponse);

   semaphore->unlock();
}
void handler(const shared_ptr<Session> &session)
{
	const auto headers=multimap<string,string>{
      {"Connection","close"},
      {"Content-Type","*"},
      {"Access-Control-Allow-Origin","*"}
   };

   session->yield(OK,headers,[](const shared_ptr<Session> session){});

   const auto request=session->get_request();

   size_t content_length=0;
   content_length=request->get_header("Content-Length",content_length);

   session->fetch(content_length,[](const shared_ptr<Session> &session,const Bytes &body)
   {
      boost::property_tree::ptree settings;
      istringstream is(string(body.begin(),body.end()));
      read_json(is,settings);
      session->close(OK);

      boost::thread t(&run,settings);
      t.detach();
   });
}

int main( const int, const char** )
{
    unsigned MAX_THREADS=std::thread::hardware_concurrency();
    semaphore=make_shared<Semaphore>(MAX_THREADS);

    auto resource=make_shared<Resource>();
    resource->set_path("/controller");
    resource->set_method_handler("POST",handler);

    auto settings=make_shared<Settings>();
    settings->set_port(1985);
    settings->set_default_header("Connection","close");

    Service service;
    service.publish(resource);
    service.start(settings);

    return(EXIT_SUCCESS);
}
