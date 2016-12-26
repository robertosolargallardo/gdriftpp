#include "Gene.h"
gene::gene(void){
   ;
}
gene::gene(const size_t &_n_alleles){
   this->_alleles.reserve(_n_alleles);
}
gene::~gene(void){
   this->_alleles.clear();
}
void gene::add(const Allele &_allele){
   this->_alleles.push_back(_allele);
}
void gene::clear(void){
   this->_alleles.clear();
}
vector<Allele> gene::alleles(){
   return(this->_alleles);
}
void gene::alleles(const vector<Allele> &_alleles){
   this->_alleles.insert(this->_alleles.end(),_alleles.begin(),_alleles.end());
}
