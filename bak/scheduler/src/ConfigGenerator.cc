#include "ConfigGenerator.h"
#include "../../util/Communication.h"

ConfigGenerator::ConfigGenerator(const boost::property_tree::ptree &_configuration){
   this->configuration = _configuration;
   this->currentBatch = 0;
   this->currentRun   = 0;

   this->id = configuration.get<uint32_t>( "id" ); //Lee el ID del JSON de configuracion
   this->max_number_of_simulations = configuration.get<uint32_t>( "max-number-of-simulations" );
}

void ConfigGenerator::processConfiguration( int batch_size ){
   //Borramos cualquier configuracion pendiente
   configurations.clear( );


   //Se generan tantos archivos de configuracion como escenarios existan en el json
   //por lo tanto, si se pide un batch de "n" configs, se debe dividir por la cant. de escenarios.
   //TODO: verificar que se generen la cantidad de configuraciones que se pidan
   vector<boost::property_tree::ptree> pt_scenarios = parse_scenarios(configuration.get_child("scenarios"));
   int nro_escenarios = pt_scenarios.size();

   cout << "Nro escenarios=" << nro_escenarios << " - batch_size=" << batch_size << " - calc="
        << ((int)batch_size/nro_escenarios)*nro_escenarios << endl;
   for(int i = 0; i < ((int)batch_size/nro_escenarios); i++){
      /*boost::property_tree::ptree conf;
      conf.put("id", id);
      conf.put("run", currentRun);
      conf.put("batch", currentBatch);
      conf.put("max-number-of-simulations", max_number_of_simulations);*/

      boost::property_tree::ptree pt_individual = parse_individual(configuration.get_child("individual"));
      //vector<boost::property_tree::ptree> pt_scenarios = parse_scenarios(configuration.get_child("scenarios"));

      for(auto iter_scenario : pt_scenarios ){
         boost::property_tree::ptree conf;
         conf.put("id", id);
         conf.put("run", currentRun);
         conf.put("batch", currentBatch);
         conf.put("max-number-of-simulations", max_number_of_simulations);
         conf.add_child("individual", pt_individual);

         conf.add_child("scenario", iter_scenario );

         char config_name[1000];
         sprintf(config_name, "output_%d_%d_%d", id, currentRun, currentBatch);
         configurations[ config_name ] = conf;
      
         currentRun++;
      }
   }
   currentRun = 0;
   currentBatch++; //Incrementa el valor en cada llamado a generar un nuevo batch
}

void ConfigGenerator::printBatch( ){
   for(auto& it : configurations) {
      cout << "CONFIGURATION ID " << it.first << endl;
      write_json(std::cout, it.second);
      cout << endl << endl << endl;
   }
}

void ConfigGenerator::saveBatchToFile( ){
    for(auto& it : configurations) {
      cout << "Saved to file " << it.first << endl;
      write_json(it.first + ".json ", it.second);
   }
}

void ConfigGenerator::sendBatch( ){
   cout << "batch size=" << configurations.size() << endl;
   for(auto& it : configurations)
		comm::send("http://citiaps2.diinf.usach.cl","1985","controller",it.second);
   configurations.clear( );
}
boost::property_tree::ptree ConfigGenerator::parse_individual(boost::property_tree::ptree pt_individual){

   boost::property_tree::ptree bpt;

   bpt.put("ploidy", pt_individual.get<uint32_t>("ploidy"));
   boost::property_tree::ptree pt_chromosome;

   for(auto& json_chromosomes : pt_individual.get_child("chromosomes")){ //obtiene id y genes
      boost::property_tree::ptree pt_chromosomes;
      pt_chromosomes.put("id", json_chromosomes.second.get<uint32_t>("id"));
      boost::property_tree::ptree pt_genes;

      for(auto& json_genes : json_chromosomes.second.get_child("genes")){ //Obtener data de cada gene

         boost::property_tree::ptree pt_gene;
         pt_gene.put("id", json_genes.second.get<uint32_t>("id"));
         pt_gene.put("nucleotides", json_genes.second.get<uint32_t>("nucleotides"));

         if( json_genes.second.get<string>("mutation-rate.type") == "random" ){
            pt_gene.put("mutation-rate",
                        generate_uniform_random(json_genes.second.get<double>("mutation-rate.min-value"),
                                                json_genes.second.get<double>("mutation-rate.max-value")));

         }else{
            pt_gene.put("mutation-rate",json_genes.second.get<string>("mutation-rate.value"));
         }

         pt_gene.put("number-of-segregating-sites", json_genes.second.get<uint32_t>("number-of-segregating-sites"));
         pt_gene.put("number-of-alleles", json_genes.second.get<uint32_t>("number-of-alleles"));

         pt_genes.push_back(std::make_pair("",pt_gene));
      }
      pt_chromosomes.add_child("genes", pt_genes);
      pt_chromosome.push_back( std::make_pair("",pt_chromosomes) );
   }
   bpt.add_child("chromosomes",pt_chromosome);

   return bpt;
}

vector<boost::property_tree::ptree> ConfigGenerator::parse_scenarios(boost::property_tree::ptree pt_scenes){

   vector<boost::property_tree::ptree> scenarios;
   for(auto& json_scenarios : pt_scenes){
      //boost::property_tree::ptree bpt;

      boost::property_tree::ptree pt_scenario;
      pt_scenario.put("id", json_scenarios.second.get<uint32_t>("id"));
      pt_scenario.put("model", json_scenarios.second.get<uint32_t>("model"));

      double t_stamp = 0.0;
      boost::property_tree::ptree pt_events;
      for(auto& json_events : json_scenarios.second.get_child("events") ){
         boost::property_tree::ptree pt_event;
         pt_event.put("id",json_events.second.get<uint32_t>("id"));
         uint32_t timestamp;
         if(json_events.second.get<string>("timestamp.type") == "random"){
            do{
               timestamp = uint32_t(generate_uniform_random(json_events.second.get<uint32_t>("timestamp.min-value"),json_events.second.get<uint32_t>("timestamp.max-value")));
            }while(t_stamp > timestamp );
            t_stamp = timestamp/100;
            pt_event.put("timestamp", timestamp);
         }else{
            t_stamp = json_events.second.get<double>("timestamp.value");
            pt_event.put("timestamp", t_stamp/100);
         }

         //Todos llevan type
         pt_event.put("type",json_events.second.get<string>("type"));

         //Elementos dependientes del type
         //CREATE - OK
         if( json_events.second.get<string>("type") == "create"){
            pt_event.put("params.population.name",json_events.second.get<string>("params.population.name"));
            if( json_events.second.get<string>("params.population.size.type") == "random" ){
               int min_value = json_events.second.get<int>("params.population.size.min-value");
               int max_value = json_events.second.get<int>("params.population.size.max-value");
               pt_event.put("params.population.size",uint32_t(generate_uniform_random(min_value,max_value)));
            }else
              pt_event.put("params.population.size", json_events.second.get<int>("params.population.size.value"));
         }

         //MIGRATION - OK
         if(json_events.second.get<string>("type") == "migration" ){
            pt_event.put("params.source.population.name",json_events.second.get<string>("params.source.population.name"));

            if( json_events.second.get<string>("params.source.population.percentage.type") == "random" ){
               double min_value = json_events.second.get<double>("params.source.population.percentage.min-value");
               double max_value = json_events.second.get<double>("params.source.population.percentage.max-value");
               pt_event.put("params.source.population.percentage",(double)generate_uniform_random(min_value,max_value));
            }else
              pt_event.put("params.source.population.percentage", json_events.second.get<double>("params.source.population.percentage.value"));
            pt_event.add_child("params.destination",json_events.second.get_child("params.destination"));
         }

         //DECREMENT & DECREMENT - OK
         if(json_events.second.get<string>("type") == "decrement" ||
            json_events.second.get<string>("type") =="increment"){
           pt_event.put("params.source.population.name",json_events.second.get<string>("params.source.population.name"));
           if( json_events.second.get<string>("params.source.population.percentage.type") == "random" ){
               double min_value = json_events.second.get<double>("params.source.population.percentage.min-value");
               double max_value = json_events.second.get<double>("params.source.population.percentage.max-value");
               pt_event.put("params.source.population.percentage",(double)generate_uniform_random(min_value,max_value));
            }else
               pt_event.put("params.source.population.percentage", json_events.second.get<double>("params.source.population.percentage.value"));
         }

         //SPLIT - OK
         if(json_events.second.get<string>("type") == "split"){
            pt_event.put("params.partitions",json_events.second.get<string>("params.partitions"));
            pt_event.put("params.source.population.name",json_events.second.get<string>("params.source.population.name"));
            pt_event.add_child("params.destination",json_events.second.get_child("params.destination"));
         }

         //MERGE - OK
         if(json_events.second.get<string>("type") == "merge"){
            pt_event.add_child("params.destination",json_events.second.get_child("params.destination"));
            pt_event.add_child("params.source",json_events.second.get_child("params.source"));
         }

         //ENDSIM - OK
         if(json_events.second.get<string>("type") == "endsim"){
            pt_event.put("params","");
            //Nothing to do, yet
         }

         pt_events.push_back(std::make_pair("", pt_event));
      }

      //pt_scenario.push_back(std::make_pair("",pt_events));
      pt_scenario.add_child("events",pt_events);
      //bpt.push_back(std::make_pair("",pt_scenario));
      scenarios.push_back( pt_scenario );
   }

   return scenarios;
}



double ConfigGenerator::generate_uniform_random( double min_value, double max_value){
   std::uniform_real_distribution<double> distribution(min_value, max_value);
   return ((double)distribution(rng));
}
