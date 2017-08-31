#include "Scheduler.h"

Scheduler::Scheduler(const boost::property_tree::ptree &fhosts) : Node(fhosts){
//	this->_mongo = make_shared<util::Mongo>(this->fhosts.get<string>("database.uri"));
	this->_semaphore = make_shared<util::Semaphore>(1);
	
	db_comm = DBCommunication(fhosts.get<string>("database.uri"), fhosts.get<string>("database.name"), fhosts.get<string>("database.collections.data"), fhosts.get<string>("database.collections.results"), fhosts.get<string>("database.collections.settings"), fhosts.get<string>("database.collections.training"));
	
}

Scheduler::~Scheduler(void){
	this->_settings.clear();
}

boost::property_tree::ptree Scheduler::run(boost::property_tree::ptree &_frequest){

	boost::optional<boost::property_tree::ptree&> test_child;
	uint32_t id = 0;
	string type = "unknown";
	
	test_child = _frequest.get_child_optional("id");
	if( test_child ){
		id = _frequest.get<uint32_t>("id");
	}
	
	test_child = _frequest.get_child_optional("type");
	if( test_child ){
		type = _frequest.get<string>("type");
	}
	
	cout<<"Scheduler::run - Inicio (id: "<<id<<", type: "<<type<<")\n";
	
	switch(util::hash(type)){
		case RESTORE:{
			cout<<"Scheduler::run - RESTORE\n";
			
			// Buscar cada simulacion incompleta
			list<boost::property_tree::ptree> pending_settings = db_comm.readUnfinishedSettings();
			cout<<"Scheduler::run - Pending simulations: "<<pending_settings.size()<<"\n";
			
			for( auto &fsettings : pending_settings ){
			
				cout<<"Scheduler::run - Procesando fsettings\n";
				id = 0;
				test_child = fsettings.get_child_optional("id");
				if( test_child ){
					id = fsettings.get<uint32_t>("id");
				}
				
				if( _settings.find(id) != _settings.end() ){
					cout<<"Scheduler::run - Simulacion YA existe, omitiendo.\n";
					continue;
				}
				
				cout<<"Scheduler::run - Creando settings["<<id<<"]\n";
				// Realizar el procedimiento de resume para CADA simulacion incompleta
				// Creo que el setStatus... running NO es necesario (porque solo considero esas para este resume)
				// Antes del resume, hay que simular el init
				this->_semaphore->lock();
				this->_settings[id] = make_shared<SimulationSettings>(fsettings);
				this->_semaphore->unlock();
				
				// Leer numero de simulaciones terminadas (notar que esto usa this->_settings[id])
				uint32_t finished_jobs = db_comm.countFinishedJobs(id, this->_settings[id]->feedback);
				
				// Antes de enviar las simulaciones, hay que recargar los datos en analyzer
				// Para eso hay que agregar un comando en analyzer que lea directamente de la bd
				boost::property_tree::ptree frestore;
				frestore.put("id", id);
				frestore.put("type", "restore");
				frestore.put("finished", finished_jobs);
				comm::send(this->_fhosts.get<string>("analyzer.host"), this->_fhosts.get<string>("analyzer.port"), this->_fhosts.get<string>("analyzer.simulated"), frestore);
				// Por seguridad, agrego un sleep a este thread entre el analyzer -> restore y el resume_send
				// Notar que lo ideal seria una respueste de analyer, esta forma no es muy elegante
				std::this_thread::sleep_for (std::chrono::seconds(1));
				
				// Modificar settings[id] para agregar el trabajo ya realizado (trabajos terminados)
				cout<<"Scheduler::run - finished_jobs de sim "<<id<<": "<<finished_jobs<<" / "<<this->_settings[id]->training_size<<"\n";
				this->_settings[id]->generateJobs(this->_settings[id]->training_size);
				this->_settings[id]->cur_job = finished_jobs;
				
				cout<<"Scheduler::run - Continuando trabajos\n";
				this->_settings[id]->resume_send(this->_settings[id]->training_size, this->_fhosts);
				
			}
			cout<<"Scheduler::run - Fin RESTORE\n";
			
			break;
		}
		case PAUSE:{
			cout<<"Scheduler::run - PAUSE\n";
			if( _settings.find(id) == _settings.end() ){
				cout<<"Scheduler::run - Simulacion NO encontrada.\n";
				break;
			}
			db_comm.setStatus(id, "paused");
			this->_settings[id]->pause = true;
			break;
		}
		case RESUME:{
			cout<<"Scheduler::run - RESUME\n";
			if( _settings.find(id) == _settings.end() ){
				cout<<"Scheduler::run - Simulacion NO encontrada.\n";
				break;
			}
			db_comm.setStatus(id, "running");
			this->_settings[id]->pause = false;
			this->_settings[id]->resume_send(this->_settings[id]->training_size, this->_fhosts);
			break;
		}
		case CANCEL:{
			cout<<"Scheduler::run - CANCEL\n";
			if( _settings.find(id) == _settings.end() ){
				cout<<"Scheduler::run - Simulacion NO encontrada.\n";
				break;
			}
			db_comm.setStatus(id, "canceled");
			// Lo primero es activar la marca de cancelacion (o quitar la marca de continuar)
			// Notar que _settings[id] probablemente estara corriendo (send), y NO PUEDE eliminarse mientras eso siga activo
			// No es claro, eso si, cuando retorna del send (no desde aqui)
			// La opcion simple es dejarla, simplemente marcarla como cancelada para que se detenga
			// Notar que la escritura directa de una variable basica (como bool) DEBERIA ser atomica
			this->_settings[id]->cancel = true;
			// Duermo unos segundos para que settings termine de procesar antes de borrarlo
			cout<<"Scheduler::run - Esperando a settings...\n";
			std::this_thread::sleep_for (std::chrono::seconds(5));
			cout<<"Scheduler::run - Continuando con cancel\n";
			// Los controladores pueden seguir con los trabajos actuales, pero sus resultados pueden ser omitidos por el analyzer
			// Avisar al analyzer que cancele la simulacion (y borre los datos de esta), ese proceso DEBE ser thread-safe
			comm::send(this->_fhosts.get<string>("analyzer.host"), this->_fhosts.get<string>("analyzer.port"), this->_fhosts.get<string>("analyzer.simulated"), _frequest);
			break;
		}
		case INIT:{
			cout<<"Scheduler::run - INIT\n";
			// Notar que aqui NO se puede actualizar porque aun no se ha guardado, pero lo puedo agregar como el feedback
//			db_comm.setStatus(id, "running");
			
			// Agregar cualquier informacion al json antes de guardarlo en la BD
			
			test_child = _frequest.get_child_optional("feedback");
			if( ! test_child ){
				_frequest.put("feedback", 0);
			}
			
			test_child = _frequest.get_child_optional("status");
			if( ! test_child ){
				_frequest.put("status", "running");
			}
			
			uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			cout<<"Scheduler::run - Agregando timestamp "<<timestamp<<"\n";
			_frequest.put("timestamp", std::to_string(timestamp));
			
			db_comm.writeSettings(_frequest);
			
			this->_semaphore->lock();
			this->_settings[id] = make_shared<SimulationSettings>(_frequest);
			this->_semaphore->unlock();
			
//			this->_settings[id]->send(BATCH_LENGTH, this->_fhosts);
			this->_settings[id]->send(this->_settings[id]->training_size, this->_fhosts);
			
			break;
		}
		case CONTINUE:{
			cout<<"Scheduler::run - CONTINUE\n";
			if( _settings.find(id) == _settings.end() ){
				cout<<"Scheduler::run - Simulacion NO encontrada.\n";
				break;
			}
//			this->_settings[id]->send(BATCH_LENGTH, this->_fhosts);
			this->_settings[id]->send(this->_settings[id]->training_size, this->_fhosts);
			break;
		}
		case RELOAD:{
			cout<<"Scheduler::run - RELOAD\n";
			if( _settings.find(id) == _settings.end() ){
				cout<<"Scheduler::run - Simulacion NO encontrada.\n";
				break;
			}
			// Aqui hay que guardar _frequest nuevamente en settings, pues trae nuevos parametros para el propio scheduler
			// Tambien hay que recrear settings (con los nuevos parametros)
			// Hay que asegurar entonces, que en reload el analizer envie el ptree de settings !!!
			
			uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			cout<<"Scheduler::run - Agregando timestamp "<<timestamp<<"\n";
			_frequest.put("timestamp", std::to_string(timestamp));
			
//			std::stringstream ss;
//			write_json(ss, _frequest);
//			cout << ss.str() << endl;
			
			db_comm.writeSettings(_frequest);
			this->_semaphore->lock();
			// Antes de resetear settings, guardo cualquier cosa que pudiera necesitar
			unsigned int training_size = this->_settings[id]->training_size;
			// Recreo settings
			this->_settings[id].reset(new SimulationSettings(_frequest));
			// Reestablezco valores antiguos
			this->_settings[id]->training_size = training_size;
			this->_semaphore->unlock();
			// Continuo directamente (ya no esperare el continue para evitar el problema del doble mensaje del analizer)
			this->_settings[id]->send(this->_settings[id]->training_size, this->_fhosts);
			
			break;
		}
		case FINALIZE:{
			cout<<"Scheduler::run - FINALIZE\n";
			if( _settings.find(id) == _settings.end() ){
				cout<<"Scheduler::run - Simulacion NO encontrada.\n";
				break;
			}
			db_comm.setStatus(id, "finished");
			this->_semaphore->lock();
			this->_settings.erase(this->_settings.find(id));
			this->_semaphore->unlock();
			break;
		}
		default:{
			cerr<< "Error::Unknown Scheduling Type " << type << endl;
			break;
//			exit(EXIT_FAILURE);
		}
	}
	
	// Borrar settings de simulaciones canceladas
	// Notar que DOS threads llegaran aqui (el de cancel y el que genero el send), asi que debe ser thread-safe
	// Tambien notar que lo ideal seria ESPERAR hasta estar seguro que settings termino
	// Por ahora, como no tengo esa certeza, el thread de cancel duerme unos segundos antes de llegar aqui
	this->_semaphore->lock();
	if( this->_settings.find(id) != this->_settings.end() && this->_settings[id]->cancel ){
		cout<<"Scheduler::run - Eliminando settings de simulacion "<<id<<" por cancel\n";
		this->_settings.erase(this->_settings.find(id));
	}
	this->_semaphore->unlock();
	
	cout<<"Scheduler::run - Fin\n";
	return(_frequest);
}



