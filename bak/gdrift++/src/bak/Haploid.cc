#include "Haploid.h"
haploid::haploid(void):individual(){
	;
}
haploid::haploid(const uint32_t &_id,const size_t &_n_chromosomes):individual(_id,HAPLOID){
	this->_chromosomes.reserve(_n_chromosomes);
}
haploid::haploid(const haploid &_h):individual(_h._id,HAPLOID){
   this->_chromosomes.reserve(_h.n_chromosomes());
   for(auto c : _h.chromosomes())
      this->add({Chromosome(*c[0])}); 
}
haploid::~haploid(void){
	this->_chromosomes.clear();
}
void haploid::add(const array<Chromosome,HAPLOID> &_chromosome){
	this->_chromosomes.push_back(_chromosome);
}
void haploid::clear(void){
	this->_chromosomes.clear();
}
vector<array<Chromosome,HAPLOID>> haploid::chromosomes(void) const{
   return(this->_chromosomes);
}
array<Chromosome,HAPLOID> haploid::at(const size_t &_position){
   return(this->_chromosomes[_position]);
}
size_t haploid::n_chromosomes(void) const{
   return(this->_chromosomes.size());
}
