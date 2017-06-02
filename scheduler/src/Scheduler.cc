#include "Scheduler.h"

Scheduler::Scheduler(const boost::property_tree::ptree &_fhosts):Node(_fhosts){
	this->_mongo=make_shared<util::Mongo>(this->_fhosts.get<string>("database.uri"));
	this->_semaphore=make_shared<util::Semaphore>(1);
	
}

Scheduler::~Scheduler(void){
	this->_settings.clear();
}

boost::property_tree::ptree Scheduler::run(boost::property_tree::ptree &_frequest){
	uint32_t id = _frequest.get<uint32_t>("id");
	uint32_t type = util::hash(_frequest.get<string>("type"));
	cout<<"Scheduler::run - Inicio (id: "<<id<<", type: "<<type<<")\n";
	
	switch(type){
		case INIT:{
			cout<<"Scheduler::run - INIT\n";
			boost::optional<boost::property_tree::ptree&> test_child;
			
			// Agregar cualquier informacion al json antes de guardarlo en la BD
			
			test_child = _frequest.get_child_optional("feedback");
			if( ! test_child ){
				_frequest.put("feedback", 0);
			}
			
			uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			cout<<"Scheduler::run - Agregando timestamp "<<timestamp<<"\n";
			_frequest.put("timestamp", std::to_string(timestamp));
			
			this->_mongo->write(this->_fhosts.get<string>("database.name"), this->_fhosts.get<string>("database.collections.settings"), _frequest);
			
			this->_semaphore->lock();
			this->_settings[id] = make_shared<SimulationSettings>(_frequest);
			this->_semaphore->unlock();
			
			this->_settings[id]->_training_size = BATCH_LENGTH;
			test_child = _frequest.get_child_optional("batch-size");
			if( test_child ){
				this->_settings[id]->_training_size = _frequest.get<uint32_t>("batch-size");
			}
			// No es necesaria la verificacion si pongo valores por defecto mas arriba
//			test_child = _frequest.get_child_optional("feedback");
//			if( test_child ){
				this->_settings[id]->_feedback = _frequest.get<uint32_t>("feedback");
//			}
			
//			this->_settings[id]->send(BATCH_LENGTH, this->_fhosts);
			this->_settings[id]->send(this->_settings[id]->_training_size, this->_fhosts);
			break;
		}
		case CONTINUE:{
			cout<<"Scheduler::run - CONTINUE ("<<this->_settings[id]->_training_size<<")\n";
//			this->_settings[id]->send(BATCH_LENGTH, this->_fhosts);
			this->_settings[id]->send(this->_settings[id]->_training_size, this->_fhosts);
			break;
		}
		case RELOAD:{
			cout<<"Scheduler::run - RELOAD\n";
			// Aqui hay que guardar _frequest nuevamente en settings, pues trae nuevos parametros para el propio scheduler
			// Tambien hay que recrear settings (con los nuevos parametros)
			// Hay que asegurar entonces, que en reload el analizer envie el ptree de settings !!!
			
			std::stringstream ss;
			write_json(ss, _frequest);
			cout << ss.str() << endl;
			
			this->_mongo->write(this->_fhosts.get<string>("database.name"), this->_fhosts.get<string>("database.collections.settings"), _frequest);
			this->_semaphore->lock();
			// Antes de resetear settings, guardo cualquier cosa que pudiera necesitar
			unsigned int training_size = this->_settings[id]->_training_size;
			// Recreo settings
			this->_settings[id].reset(new SimulationSettings(_frequest));
			// Reestablezco valores antiguos
			this->_settings[id]->_training_size = training_size;
			this->_semaphore->unlock();
			// Asigno independientemente el feedback, quzas podria definirse en el constructo si se encuentra (para ser resistente al init)
			this->_settings[id]->_feedback = _frequest.get<uint32_t>("feedback");
			// Continuo directamente (ya no esperare el continue para evitar el problema del doble mensaje del analizer)
			this->_settings[id]->send(this->_settings[id]->_training_size, this->_fhosts);
			
			break;
		}
		case FINALIZE:{
			cout<<"Scheduler::run - FINALIZE\n";
			this->_semaphore->lock();
			this->_settings.erase(this->_settings.find(id));
			this->_semaphore->unlock();
			break;
		}
		default:{
			cerr<< "Error::Unknown Scheduling Type::" << type << endl;
			exit(EXIT_FAILURE);
		}
	}
	cout<<"Scheduler::run - Fin\n";
	return(_frequest);
}



