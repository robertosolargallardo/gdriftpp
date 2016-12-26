#ifndef _SIMULATOR_H_
#define _SIMULATOR_H_
#include "Individual.h"
#include "Population.h"
#include "EventList.h"
#include "Generator.h"
#include "Util.h"
#include <map>
#include <fstream>
#include <iostream>
//#include <omp.h>
#include <boost/range/combine.hpp>

using namespace std;
enum Model{WRIGHTFISHER=0};

template<Ploidy P>
class Simulator:public enable_shared_from_this<Simulator<P>>{
   private: map<string,shared_ptr<Population<P>>> _populations;

   public:  Simulator(void);
            ~Simulator(void);
            
            template<Model M>
            void run(const shared_ptr<Generator<P>>&,const shared_ptr<EventList>&);
            friend shared_ptr<Gene> mutate(const shared_ptr<Gene>&);
            void stats(void);
      
            map<string,shared_ptr<Population<P>>> populations(void);
   
   private: shared_ptr<Population<P>> WF(shared_ptr<Population<P>>&);
};
template<Ploidy P>
Simulator<P>::Simulator(void){
    //omp_set_num_threads(1);
}

template<Ploidy P>
Simulator<P>::~Simulator(void){
	this->_populations.clear();
}
template<Ploidy P>
map<string,shared_ptr<Population<P>>> Simulator<P>::populations(void){
   return(this->_populations);
}

template<Ploidy P>
template<Model M>
void Simulator<P>::run(const shared_ptr<Generator<P>> &_ge,const shared_ptr<EventList> &_el){
	for(uint32_t t=0U;;t++){
		while(!_el->empty() && _el->top()->timestamp()==t){
			auto e=_el->top();
         auto params=e->params();

			switch(e->type()){
				case CREATE:{
               this->_populations[params.get<string>("population.name")]=_ge->generate(params.get<size_t>("population.size"));
					break;
				}
				case SPLIT:{
               vector<shared_ptr<Population<P>>> partitions=this->_populations[params.get<string>("source.population.name")]->split(params.get<size_t>("partitions"));
   
               int i=0;
               for(auto& destination : params.get_child("destination")){
                  partitions[i]->name(destination.second.get<string>("population.name"));
                  this->_populations[destination.second.get<string>("population.name")]=partitions[i];
                  i++;
               }

               this->_populations.erase(this->_populations.find(params.get<string>("source.population.name")));

					break;
				}
				case MIGRATION:{
               this->_populations[params.get<string>("destination.population.name")]=make_shared<Population<P>>(params.get<string>("destination.population.name"));
               this->_populations[params.get<string>("source.population.name")]->migration(this->_populations[params.get<string>("destination.population.name")],params.get<double>("source.population.percentage"));
					break;
				}
				case MERGE:{
               this->_populations[params.get<string>("destination.population.name")]=make_shared<Population<P>>(params.get<string>("destination.population.name"));
               for(auto& source : params.get_child("source")){
                  this->_populations[params.get<string>("destination.population.name")]->merge(this->_populations[source.second.get<string>("population.name")]);
                  this->_populations.erase(this->_populations.find(source.second.get<string>("population.name")));
               }
					break;
				}
				case INCREMENT:{
               this->_populations[params.get<string>("source.population.name")]->increase(params.get<double>("source.population.percentage"));
					break;
				}
				case DECREMENT:{ 	
               this->_populations[params.get<string>("source.population.name")]->decrease(params.get<double>("source.population.percentage"));
					break;
				}
				case ENDSIM:{
					return;
				}
				default:{
					cerr << "Event::" << e->type() << "::Not Supported" << endl;
					exit(EXIT_FAILURE);
				}
			}

			_el->pop();
		}
      for(auto& p : this->_populations){
         switch(M){
            case WRIGHTFISHER:   this->_populations[p.first]=this->WF(p.second);
                                 break;
         }
      }
	}
}

template<Ploidy P>
void Simulator<P>::stats(void){
	;
}
#endif
