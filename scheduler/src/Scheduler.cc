#include "Scheduler.h"

Scheduler::Scheduler(const boost::property_tree::ptree &_fhosts) : Node(_fhosts){
	this->_mongo=make_shared<util::Mongo>(this->_fhosts.get<string>("database.uri"));
	this->_semaphore=make_shared<util::Semaphore>(1);
	
}

Scheduler::~Scheduler(void){
	this->_settings.clear();
}

boost::property_tree::ptree Scheduler::run(boost::property_tree::ptree &_frequest){
	uint32_t id = _frequest.get<uint32_t>("id");
	string type = _frequest.get<string>("type");
	cout<<"Scheduler::run - Inicio (id: "<<id<<", type: "<<type<<")\n";
	
	switch(util::hash(type)){
		case PAUSE:{
			cout<<"Scheduler::run - PAUSE\n";
			this->_settings[id]->pause = true;
			break;
		}
		case RESUME:{
			cout<<"Scheduler::run - RESUME\n";
			this->_settings[id]->pause = false;
			this->_settings[id]->resume_send(this->_settings[id]->_training_size, this->_fhosts);
			break;
		}
		case CANCEL:{
			cout<<"Scheduler::run - CANCEL\n";
			// Lo primero es activar la marca de cancelacion (o quitar la marca de continuar)
			// Notar que _settings[id] probablemente estara corriendo (send), y NO PUEDE eliminarse mientras eso siga activo
			// No es claro, eso si, cuando retorna del send (no desde aqui)
			// La opcion simple es dejarla, simplemente marcarla como cancelada para que se detenga
			// Notar que la escritura directa de una variable basica (como bool) DEBERIA ser atomica
			this->_settings[id]->cancel = true;
			// Los controladores pueden seguir con los trabajos actuales, pero sus resultados pueden ser omitidos por el analyzer
			// Avisar al analyzer que cancele la simulacion (y borre los datos de esta), ese proceso DEBE ser thread-safe
			comm::send(this->_fhosts.get<string>("analyzer.host"), this->_fhosts.get<string>("analyzer.port"), this->_fhosts.get<string>("analyzer.simulated"), _frequest);
			break;
		}
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
			
			uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			cout<<"Scheduler::run - Agregando timestamp "<<timestamp<<"\n";
			_frequest.put("timestamp", std::to_string(timestamp));
			
//			std::stringstream ss;
//			write_json(ss, _frequest);
//			cout << ss.str() << endl;
			
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
			cerr<< "Error::Unknown Scheduling Type " << type << endl;
			exit(EXIT_FAILURE);
		}
	}
	
	// Borrar settings de simulaciones canceladas
	// Notar que DOS threads llegaran aqui (el de cancel y el que genero el send), asi que debe ser thread-safe
	this->_semaphore->lock();
	if( this->_settings.find(id) != this->_settings.end() && this->_settings[id]->cancel ){
		cout<<"Scheduler::run - Eliminando settings de simulacion "<<id<<" por cancel\n";
		this->_settings.erase(this->_settings.find(id));
	}
	this->_semaphore->unlock();
	
	cout<<"Scheduler::run - Fin\n";
	return(_frequest);
}



