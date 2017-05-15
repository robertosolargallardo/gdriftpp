#include "Controller.h"

unsigned int Controller::total_errores = 0;
vector<Controller::ThreadData> Controller::threads_data;

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
	
	data->list_mutex->lock();
	cout<<"controller_thread["<<data->id<<"] - Inicio\n";
	data->list_mutex->unlock();
	
	unsigned int sleep_time = 5;
	shared_ptr<boost::property_tree::ptree> ptr_json;
	
	while(true){
		data->list_mutex->lock();
		cout<<"controller_thread["<<data->id<<"] - Largo de lista: "<<data->work_list->size()<<"\n";
		// NOTAR QUE ESTA ES SOLO LA SOLUCION MAS SIMPLE (dormir N segundos si la cola esta vacia)
		// TODO: Cambiar sleep de N segundos por un sistema de señales apropiado
		if(data->work_list->empty()){
			cout<<"controller_thread["<<data->id<<"] - Durmiendo\n";
			data->list_mutex->unlock();
			std::this_thread::sleep_for (std::chrono::seconds(sleep_time));
			continue;
		}
		else{
			ptr_json = data->work_list->front();
			data->work_list->pop_front();
			data->list_mutex->unlock();
		}
		
		// Procesar el json
//		data->list_mutex->lock();
//		cout<<"controller_thread["<<data->id<<"] - Procesando json\n";
//		data->list_mutex->unlock();
		
		boost::property_tree::ptree _frequest = *ptr_json;
		
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
		
		uint32_t feedback = _frequest.get<uint32_t>("feedback");
		fresponse.put("feedback", feedback);
		fresponse.put("batch-size", _frequest.get<uint32_t>("batch-size"));
		
		cout<<"Controller::run - Creando Simulatior (feedback: "<<feedback<<")\n";
		Simulator sim(_frequest);
		
		//fprior.push_back(std::make_pair("populations", indices(sim.populations())));
		
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
			
			data->list_mutex->lock();
			cout<<"Controller::run - Error durante simulacion, resultados no usables.\n";
			data->list_mutex->unlock();
			
			fresponse.put("error", sim.detectedErrors());
			++Controller::total_errores;
		}
		comm::send(data->analyzer_host, data->analyzer_port, data->analyzer_resource, fresponse);
		
		cout<<"Controller::run - Fin (sim "<<_frequest.get<string>("id")<<", run "<<_frequest.get<string>("run")<<", batch "<<_frequest.get<string>("batch")<<")\n";
		
//		data->list_mutex->lock();
//		cout<<"Controller::run - Fin (total_errores: "<<Controller::total_errores<<")\n";
//		data->list_mutex->unlock();
		
		
		
		
	}
	
}

bool Controller::startThreads(unsigned int n_threads, list<shared_ptr<boost::property_tree::ptree>> *work_list, std::mutex *list_mutex, const boost::property_tree::ptree &fhosts){
	
	cout<<"Controller::startThreads - Preparando datos\n";
	// Notar que, dado que los threads son detached, almaceno sus datos en una variable estatica de la clase 
//	ThreadData threads_data[n_threads];
	for(unsigned int i = 0; i < n_threads; ++i){
		Controller::ThreadData data;
		data.id = i;
		data.work_list = work_list;
		data.list_mutex = list_mutex;
		
//		comm::send(this->_fhosts.get<string>("analyzer.host"), this->_fhosts.get<string>("analyzer.port"), this->_fhosts.get<string>("analyzer.resource"), fresponse);
		
		data.analyzer_host = fhosts.get<string>("analyzer.host");
		data.analyzer_port = fhosts.get<string>("analyzer.port");
		data.analyzer_resource = fhosts.get<string>("analyzer.resource");
		
		threads_data.push_back(data);
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
	
	uint32_t feedback = _frequest.get<uint32_t>("feedback");
	fresponse.put("feedback", feedback);
	fresponse.put("batch-size", _frequest.get<uint32_t>("batch-size"));
	
	cout<<"Controller::run - Creando Simulatior (feedback: "<<feedback<<")\n";
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















