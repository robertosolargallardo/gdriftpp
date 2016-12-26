#include "Chromosome.h"
Chromosome::Chromosome(void){
   ;
}
Chromosome::Chromosome(const uint32_t &_id){
   this->_id=_id;
}
Chromosome::Chromosome(const Chromosome &_chromosome){
   this->_id=_chromosome._id;
   this->_genes.reserve(_chromosome.n_genes());
   for(auto g : _chromosome.genes())
      this->add(make_shared<Gene>(*(g.get())));
}
Chromosome::Chromosome(const uint32_t &_id,const size_t &_n_genes){
   this->_id=_id;
   this->_genes.reserve(_n_genes);
}
Chromosome::~Chromosome(void){
   this->_genes.clear();
}
shared_ptr<Gene> Chromosome::at(const size_t &_index){
   return(this->_genes[_index]);
}
void Chromosome::add(const shared_ptr<Gene> &_gene){
	this->_genes.push_back(_gene);
}
vector<shared_ptr<Gene>> Chromosome::genes(void) const{
   return(this->_genes);
}
void Chromosome::clear(void){
   this->_genes.clear();
}
size_t Chromosome::n_genes() const{
   return(this->_genes.size());
}
uint32_t Chromosome::id(void){
   return(this->_id);
}
