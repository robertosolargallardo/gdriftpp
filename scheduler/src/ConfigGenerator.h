#ifndef _CONFIGGENERATOR_H_
#define _CONFIGGENERATOR_H_

#include <ctime>
#include <cstdlib>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <curl/curl.h>

#include <map>
#include <vector>

using namespace std;

extern mt19937 rng;

class ConfigGenerator{
   public:
      ConfigGenerator(const boost::property_tree::ptree &_configuration);

      /**
      * Parsing al archivo JSON recibido y genera diferentes configuraciones
      */
      void processConfiguration( int batchSize );

      void sendBatch( );
      void printBatch( );

      bool sendConfiguration( std::stringstream * );

      /**
      * Guarda los archivos generados del batch actual
      */
      void saveBatchToFile( );

      /**
      * Parsing a la seccion de individuos del archivo de configuracion
      * Retorna una nueva seccion de individuo
      */
      boost::property_tree::ptree parse_individual(boost::property_tree::ptree);

      /**
      * Parsing a la seccion de escenarios del archivo de configuracion
      * Retorna un vector que contiene a los diferentes escenarios
      */
      vector<boost::property_tree::ptree> parse_scenarios(boost::property_tree::ptree);

      /**
      * Genera un numero aleatorio definido dentro de un intervalo
      */
      double generate_uniform_random( double, double );

   private:
      int id;
      int max_number_of_simulations;

      int currentRun; //Indica cuantas configuraciones se han creado.
      int currentBatch; //NÃmero actual de batch

      boost::property_tree::ptree configuration; //Contiene el archivo de configuracion recibido desde Front End

      map< string, boost::property_tree::ptree> configurations;

};
#endif
