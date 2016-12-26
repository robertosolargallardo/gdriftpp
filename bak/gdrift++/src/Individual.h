#ifndef _INDIVIDUAL_H_
#define _INDIVIDUAL_H_
#include <stdint.h>
#include <memory>
#include <vector>
#include "Chromosome.h"
using namespace std;

enum Ploidy{HAPLOID=1,DIPLOID=2};

template<Ploidy P>
class Individual:public enable_shared_from_this<Individual<P>>{
   private: uint32_t _id;
            vector<array<shared_ptr<Chromosome>,P>> _chromosomes;

   public:  Individual(void);
            Individual(const uint32_t&);
            Individual(const uint32_t&,const size_t&);
            Individual(const Individual&);
            ~Individual(void);

            void add(const array<shared_ptr<Chromosome>,P>&);
            void clear(void);
            vector<array<shared_ptr<Chromosome>,P>> chromosomes(void) const;
            array<shared_ptr<Chromosome>,P> at(const size_t&) const;
            size_t n_chromosomes(void) const;

            uint32_t id(void);
};
template<Ploidy P>
Individual<P>::Individual(const uint32_t &_id){
   this->_id=_id;
}
template<Ploidy P>
Individual<P>::Individual(void){
   ;
}

template<Ploidy P>
Individual<P>::Individual(const uint32_t &_id,const size_t &_n_chromosomes){
   this->_id=_id;
   this->_chromosomes.reserve(_n_chromosomes);
}

template<Ploidy P>
Individual<P>::~Individual(void){
   this->_chromosomes.clear();
}

template<Ploidy P>
uint32_t Individual<P>::id(void){
   return(this->_id);
}

template<Ploidy P>
Individual<P>::Individual(const Individual<P> &_individual){
   this->_chromosomes.reserve(_individual.n_chromosomes());

   for(auto& chromosomes: _individual.chromosomes()){
      int p=0;
      array<shared_ptr<Chromosome>,P> cs;
      for_each(chromosomes.begin(),chromosomes.end(),[&cs,&p](const shared_ptr<Chromosome> &c){cs[p++]=make_shared<Chromosome>(*c.get());});
      this->_chromosomes.push_back(cs);
   }
}

template<Ploidy P>
vector<array<shared_ptr<Chromosome>,P>> Individual<P>::chromosomes(void) const{
   return(this->_chromosomes);
}

template<Ploidy P>
void Individual<P>::add(const array<shared_ptr<Chromosome>,P> &_chromosome){
   this->_chromosomes.push_back(_chromosome);
}

template<Ploidy P>
void Individual<P>::clear(void){
   this->_chromosomes.clear();
}

template<Ploidy P>
array<shared_ptr<Chromosome>,P> Individual<P>::at(const size_t &_position) const{
   return(this->_chromosomes[_position]);
}

template<Ploidy P>
size_t Individual<P>::n_chromosomes(void) const{
   return(this->_chromosomes.size());
}
#endif
