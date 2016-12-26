#ifndef _GENERATOR_H_
#define _GENERATOR_H_
#include <string>
#include <fstream>
#include "Pool.h"
#include "Individual.h"
#include "Population.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
using namespace std;

extern mt19937 rng;

template<Ploidy P>
class Generator: public enable_shared_from_this<Generator<P>>{
   private: shared_ptr<Pool> _pool;
            boost::property_tree::ptree _findividual;

   public:  Generator(void);
            Generator(const boost::property_tree::ptree&);
            shared_ptr<Population<P>> generate(const size_t&);
            ~Generator(void);
};
template<Ploidy P>
Generator<P>::Generator(void){
   this->_pool=make_shared<Pool>();
}

template<Ploidy P>
Generator<P>::Generator(const boost::property_tree::ptree &_findividual){
   this->_findividual=_findividual;
   this->_pool=make_shared<Pool>();

   for(auto& fchromosome : this->_findividual.get_child("chromosomes")){
      for(auto& fgene : fchromosome.second.get_child("genes")){
         this->_pool->generate(fchromosome.second.get<uint32_t>("id"),
                               fgene.second.get<uint32_t>("id"),
                               fgene.second.get<uint32_t>("nucleotides"),
                               fgene.second.get<double>("mutation-rate"),
                               fgene.second.get<uint32_t>("number-of-segregating-sites"),
                               fgene.second.get<uint32_t>("number-of-alleles"));
      }
   }
}

template<Ploidy P>
Generator<P>::~Generator(void){
	;
}

template<Ploidy P>
shared_ptr<Population<P>> Generator<P>::generate(const size_t &_size){
   shared_ptr<Population<P>> population=make_shared<Population<P>>(_size);

   for(size_t i=0U;i<_size;i++){
      shared_ptr<Individual<P>> individual=make_shared<Individual<P>>();

      for(auto& fchromosome : this->_findividual.get_child("chromosomes")){
         array<shared_ptr<Chromosome>,P> chromosomes;
         for(auto& chromosome : chromosomes){
            chromosome=make_shared<Chromosome>(fchromosome.second.get<uint32_t>("id"));
            for(auto& fgene : fchromosome.second.get_child("genes"))
               chromosome->add(this->_pool->get(fchromosome.second.get<uint32_t>("id"),fgene.second.get<uint32_t>("id")));
         }
         individual->add(chromosomes);
      }
      population->add(individual);
   }
   return(population);
}
#endif
