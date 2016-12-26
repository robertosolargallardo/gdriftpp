#include "Diploid.h"
diploid::diploid(void):individual(){
	;
}
diploid::diploid(const uint32_t &_id,const size_t &_n_chromosomes):individual(_id,DIPLOID){
	this->_chromosomes.reserve(_n_chromosomes);
}
diploid::diploid(const diploid &_d):individual(_d._id,DIPLOID){
	this->_chromosomes.reserve(_d.n_chromosomes());
	for(auto c : _d.chromosomes())
      this->add({Chromosome(*c[0]),Chromosome(*c[1])});
}
diploid::~diploid(void){
	this->_chromosomes.clear();
}
void diploid::add(const array<Chromosome,DIPLOID> &_chromosome){
	this->_chromosomes.push_back(_chromosome);
}
void diploid::clear(void){
	this->_chromosomes.clear();
}
vector<array<Chromosome,DIPLOID>> diploid::chromosomes(void) const{
   return(this->_chromosomes);
}
array<Chromosome,DIPLOID> diploid::at(const size_t &_position){
   return(this->_chromosomes[_position]);
}
size_t diploid::n_chromosomes(void) const{
   return(this->_chromosomes.size());
}
