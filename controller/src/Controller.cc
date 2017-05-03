#include "Controller.h"

unsigned int Controller::total_errores = 0;

Controller::Controller(const boost::property_tree::ptree &_fhosts, const uint32_t &_id) : Node(_fhosts) {
	this->_id = _id;
}
Controller::~Controller(){
}

boost::property_tree::ptree indices(map<string,Sample*> _samples){
	boost::property_tree::ptree fpopulations;

	Sample all("summary");
	for(map<string,Sample*>::iterator i = _samples.begin(); i != _samples.end(); ++i){
		fpopulations.push_back(std::make_pair("",i->second->indices()));
		all.merge(i->second);
	}
	fpopulations.push_back(std::make_pair("", all.indices()));

	return(fpopulations);
}

//void controller_thread(unsigned int id, list<shared_ptr<boost::property_tree::ptree>> *work_list, std::mutex *list_mutex){
bool controller_thread(Controller::ThreadData *data){
	
	unsigned int sleep_time = 1;
	shared_ptr<boost::property_tree::ptree> ptr_json;
	
	while(true){
		data->list_mutex->lock();
		if(data->work_list->empty()){
			data->list_mutex->unlock();
			cout<<"controller_thread["<<data->id<<"] - Durmiendo\n";
			std::this_thread::sleep_for (std::chrono::seconds(sleep_time));
			continue;
		}
		else{
			ptr_json = data->work_list->front();
			data->work_list->pop_front();
			data->list_mutex->unlock();
		}
		
		// Procesar el json
		cout<<"controller_thread["<<data->id<<"] - Procesando json\n";
		
		std::stringstream ss;
		write_json(ss, *ptr_json);
		cout<<ss.str()<<"\n";
		
		cout<<"controller_thread["<<data->id<<"] - -----------------------\n";
		
		
	}
	
}

bool Controller::startThreads(unsigned int n_threads, list<shared_ptr<boost::property_tree::ptree>> *work_list, std::mutex *list_mutex){
	
	cout<<"Controller::startThreads - Preparando datos\n";
	ThreadData threads_data[n_threads];
	for(unsigned int i = 0; i < n_threads; ++i){
		threads_data[i].id = i;
		threads_data[i].work_list = work_list;
		threads_data[i].list_mutex = list_mutex;
		
	}
	
	cout<<"Controller::startThreads - Lanzando threads\n";
	for(unsigned int i = 0; i < n_threads; ++i){
		std::thread(controller_thread, &(threads_data[i]) ).detach();
	}
	
	cout<<"Controller::startThreads - Fin\n";
	return true;
}

boost::property_tree::ptree Controller::run(boost::property_tree::ptree &_frequest){

	cout<<"Controller::run - Inicio (sim "<<_frequest.get<string>("id")<<", scenario: "<<_frequest.get_child("scenario").get<uint32_t>("id")<<", run "<<_frequest.get<string>("run")<<", batch "<<_frequest.get<string>("batch")<<")\n";
	
	boost::property_tree::ptree findividual = _frequest.get_child("individual");
	boost::property_tree::ptree fscenario = _frequest.get_child("scenario");

	boost::property_tree::ptree fresponse;
	boost::property_tree::ptree fprior;
	boost::property_tree::ptree fposterior;

	fresponse.put("id", _frequest.get<string>("id"));
	fresponse.put("run", _frequest.get<string>("run"));
	fresponse.put("batch", _frequest.get<string>("batch"));
	fresponse.put("type","simulated");
	fresponse.put("max-number-of-simulations", _frequest.get<string>("max-number-of-simulations"));
	
	fresponse.put("feedback", _frequest.get<uint32_t>("feedback"));
	fresponse.put("simulations-per-feedback", _frequest.get<uint32_t>("simulations-per-feedback"));
	
	cout<<"Controller::run - Creando Simulatior\n";
	Simulator sim(_frequest);
	
	//fprior.push_back(std::make_pair("populations", indices(sim.populations())));
	
	//printf("%s %s %s\n",_frequest.get<string>("id").c_str(), _frequest.get<string>("run").c_str(), _frequest.get<string>("batch").c_str());
	cout<<"Controller::run - Lanzando sim.run...\n";
	sim.run();
	cout<<"Controller::run - Ok\n";
	
	if( sim.detectedErrors() == 0 ){
		fposterior.push_back(std::make_pair("populations", indices(sim.samples())));
		fresponse.push_back(make_pair("prior", fprior));
		fresponse.push_back(make_pair("posterior", fposterior));
		fresponse.push_back(make_pair("individual", findividual));
		fresponse.push_back(make_pair("scenario", fscenario));
	}
	else{
		cout<<"Controller::run - Error durante simulacion, resultados no usables.\n";
		// Aqui habria que mandarle igual un mensaje al analyzer, que diga que OCURRIO una simulacion pero no produjo resultados
		// Esto debe aumentar el contador de simulaciones aceptadas igual, para seguir el proceso, o pedir mas al scheduler
		fresponse.put("error", sim.detectedErrors());
		++total_errores;
	}
	comm::send(this->_fhosts.get<string>("analyzer.host"), this->_fhosts.get<string>("analyzer.port"), this->_fhosts.get<string>("analyzer.resource"), fresponse);
	
//	cout<<"Controller::run - Fin (sim "<<_frequest.get<string>("id")<<", run "<<_frequest.get<string>("run")<<", batch "<<_frequest.get<string>("batch")<<")\n";
	
	cout<<"Controller::run - Fin (total_errores: "<<total_errores<<")\n";
	return(_frequest);
}















