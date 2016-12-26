#include "Gene.h"
Gene::Gene(void){
   this->_id=0U;
   this->_mutation_rate=0.0;
	this->_seq.clear();
}
Gene::Gene(const uint32_t &_id,const string &_strseq){
   this->_id=_id;
   this->_mutation_rate=0.0;
   this->strseq(_strseq);
}
Gene::Gene(const uint32_t &_id,const double &_mutation_rate){
   this->_id=_id;
   this->_mutation_rate=_mutation_rate;
	this->_seq.clear();
}
Gene::Gene(const uint32_t &_id,const double &_mutation_rate,const vector<unsigned char> &_seq){
   this->_id=_id;
   this->_mutation_rate=_mutation_rate;
   this->_seq=_seq;
}
Gene::Gene(const uint32_t &_id,const double &_mutation_rate,const string &_strseq){
   this->_id=_id;
   this->strseq(_strseq);
   this->_mutation_rate=_mutation_rate;
}
Gene::Gene(const Gene &_g){
   this->_id=_g._id;
   this->_seq=_g._seq;
   this->_mutation_rate=_g._mutation_rate;
}
Gene::~Gene(void){
   this->_seq.clear();
}
string Gene::strseq(void) const{
	stringstream ss;
	for(auto& n : this->_seq){
		switch(n){
				case ADENINA:	ss << 'A';
									break;
				case GUANINA:	ss << 'G';
									break;
				case CITOCINA:	ss << 'C';
									break;
				case TIMINA:	ss << 'T';
									break;
			}
	}
   return(ss.str());
}
void Gene::strseq(const string &_strseq){
   for(auto n : _strseq){
      switch(n){
         case 'A' : this->_seq.push_back(ADENINA);    break;
         case 'G' : this->_seq.push_back(GUANINA);    break;
         case 'C' : this->_seq.push_back(CITOCINA);   break;
         case 'T' : this->_seq.push_back(TIMINA);     break;
      }
   }
}
vector<unsigned char> Gene::seq(void) const{
   return(this->_seq);
}
void Gene::seq(const vector<unsigned char> &_seq){
   this->_seq=_seq;
}
size_t Gene::length(void) const{
	return(this->_seq.size());
}
void Gene::pop(void){
	this->_seq.pop_back();
}
void Gene::push(const unsigned char &_n){
	this->_seq.push_back(_n);
}
void Gene::clear(void){
	this->_seq.clear();
}
/*void Gene::mutate(void){
   uniform_real_distribution<double> rate(0.0,1.0);
   uniform_int_distribution<> mutation(1,3);
   
  for(auto& c : this->_seq){
      if(rate(rng)<=this->_mutation_rate)
         c=(c+mutation(rng))%NUCLEOTIDES;
   }
}*/
double Gene::mutation_rate(void) const{
   return(this->_mutation_rate);
}
void Gene::mutation_rate(const double &_mutation_rate){
   this->_mutation_rate=_mutation_rate;
}
uint32_t Gene::id(void){
   return(this->_id);
}
