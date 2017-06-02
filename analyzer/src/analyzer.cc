#include <memory>
#include <cstdlib>
#include <Method.h>
#include <Logger.h>
#include <Semaphore.h>

#include <Analyzer.h>

#include <mongocxx/instance.hpp>

using namespace std;

random_device seed;
mt19937 rng(seed());
mongocxx::instance inst{};

shared_ptr<Analyzer>  analyzer;
shared_ptr<util::Semaphore> semaphore;
std::mutex internal_mutex;

void simulated(boost::property_tree::ptree _frequest){
	lock_guard<mutex> lock(internal_mutex);
	analyzer->run(_frequest);
}
void data(const shared_ptr<Session> session){
    const auto request=session->get_request();
    int length=request->get_header("Content-Length",0);
    session->fetch(length,[](const shared_ptr<Session> session,const Bytes&){
        const auto request=session->get_request();
        const auto body=request->get_body();

		  lock_guard<mutex> lock(internal_mutex);
        boost::property_tree::ptree fsettings=analyzer->run(string((char*)body.data(),body.size()));
        ostringstream os;
        write_json(os,fsettings);
        //cout << os.str() << endl;
        session->close(OK,os.str());
    } );
}
int main(int argc,char** argv){
	if(argc < 2){
		cerr << "Error::Hosts File not Specified" << endl;
		exit(EXIT_FAILURE);
	}

	boost::property_tree::ptree fhosts;
	read_json(argv[1], fhosts);

	analyzer = make_shared<Analyzer>(fhosts);

	Service service;
	auto s=make_shared<Resource>();
	s->set_path(fhosts.get<string>("analyzer.simulated"));
	s->set_method_handler("OPTIONS", method::options);
	s->set_method_handler("POST", method::post<simulated>);
	service.publish(s);

	auto d=make_shared<Resource>();
	d->set_path(fhosts.get<string>("analyzer.data"));
	d->set_method_handler("OPTIONS", method::options);
	d->set_method_handler("POST",data);
	service.publish(d);

	auto settings=make_shared<Settings>();
	settings->set_port(fhosts.get<int>("analyzer.port"));
	settings->set_default_header("Access-Control-Allow-Origin","*");

	service.set_logger(make_shared<CustomLogger>());
	service.start(settings);

	return(EXIT_SUCCESS);
}
