#include "Population.h"
population::population(void){
   this->_capacity=0;
}
population::population(const size_t &_capacity){
   this->_capacity=0;
   this->capacity(_capacity);
}
population::population(const population &_population){
   this->_capacity=0;
   this->capacity(_population._capacity);
   this->insert(_population.set());
}
population::population(const vector<Individual> &_set){
   this->_capacity=0;
   this->capacity(_set.size());
   this->insert(_set);
}
population::~population(void){
   this->_capacity=0;
	this->clear();
}
vector<Individual> population::set(void) const{
   return(this->_set);
}
void population::insert(const vector<Individual> &_set){
   this->_set.insert(this->_set.end(),_set.begin(),_set.end());
}
void population::insert(Individual _individual){
   this->_set.push_back(_individual);
}
size_t population::size(void) const{
   return(this->_set.size());
}
size_t population::capacity(void) const{
   return(this->_capacity);
}
void population::capacity(const size_t &_capacity){
   if(_capacity>this->_capacity)
      this->_set.reserve(_capacity);
   this->_capacity=_capacity;
}
Individual population::at(const size_t &_position) const{
   return(this->_set[_position]);
}
void population::clear(void){
   this->_capacity=0;
	this->_set.clear();
}
Populations population::split(const size_t &_n_populations){
   Populations p(_n_populations);
   size_t round_robin=0;

   for_each(p.begin(),p.end(),[&](Population &p){p=Population();});
   random_shuffle(this->_set.begin(),this->_set.end());

   while(!this->_set.empty()){
      p[round_robin]->insert(this->_set.back());
      this->_set.pop_back();
      ++round_robin%=_n_populations;
   }

   for_each(p.begin(),p.end(),[&](Population &p){p->capacity(p->size());});

   this->capacity(0);
   return(p);
}
void population::merge(const Population &_population){
   this->capacity(this->size()+_population->size());
   this->insert(_population->_set); 
   _population->capacity(0);
   _population->clear();
}
void population::migration(Population &_population,const double &_percent){
   size_t n=size_t(floor(double(this->size())*_percent));
   
   random_shuffle(this->_set.begin(),this->_set.end());

   vector<Individual> _set(this->_set.end()-n,this->_set.end());

   this->_set.erase(this->_set.end()-n,this->_set.end());
   _population->capacity(_population->capacity()+_set.size());
   _population->insert(_set);
   this->capacity(this->size());
}

double population::diversity(void){
   map<uint32_t,Individual> sample;
   size_t n_sample=size_t(ceil(double(this->size())*0.1));

   std::uniform_int_distribution<> uniform(0,this->size()-1);

   while(sample.size()<n_sample){
      Individual i=this->_set[uniform(rng)];
      sample[i->id()]=i;
   }
   
   map<uint32_t,map<uint32_t,map<string,int>>> frequency;
   map<uint32_t,map<uint32_t,int>> total;

   for(auto item : sample){
      Diploid d=static_pointer_cast<diploid>(item.second);
      for(uint32_t i=0;i<d->n_chromosomes();i++){
         auto cs=d->chromosomes()[i];   
         for(auto c : cs){
            for(uint32_t j=0;j<c->n_genes();j++){
               frequency[i][j][c->at(j)->seq()]++;
               total[i][j]++;
            }
         }
      }
   }
   
   for(auto i : frequency){
      printf("Chromosome %u\n",i.first);
      for(auto j : i.second){
         printf("\tGene %u ",j.first);
         uint32_t max=0U;
         for(auto k : j.second)
            for(auto l : j.second){
               uint32_t d=distance(k.first,l.first);
               max=d>max?d:max;
            }
         printf("%u %lu\n",max,j.second.size());
      }
   }
   return(0.0);
}
