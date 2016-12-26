#ifndef _POPULATION_H_
#define _POPULATION_H_
#include "Individual.h"
#include "Distance.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <armadillo>
#include <vector>
#include <list>
#include <map>
using namespace std;

extern mt19937 rng;

template<Ploidy P>
class Population:public enable_shared_from_this<Population<P>>{
   private: vector<shared_ptr<Individual<P>>> _population;
            size_t _capacity;
            string _name;

   public:  Population(void);
            Population(const string&);
            Population(const size_t&);
            Population(const string&,const size_t&);
            Population(const Population&);
            Population(const boost::property_tree::ptree&);
            ~Population(void);

            size_t capacity(void) const;
            void capacity(const size_t&);
            size_t size(void) const;

            vector<shared_ptr<Individual<P>>> population(void) const;
            void add(const shared_ptr<Individual<P>>&);
            shared_ptr<Individual<P>> at(const size_t&) const;
            void clear(void);

            void name(const string&);
            string name(void) const;
            void merge(const shared_ptr<Population>&);
				vector<shared_ptr<Population>> split(const size_t&);
				void migration(const shared_ptr<Population>&,const double&);
				void decrease(const double&);
				void increase(const double&);

            boost::property_tree::ptree indices(const double &_percentage=0.2);

            double number_of_haplotypes(const vector<string>&);
            double number_of_segregating_sites(const vector<string>&);
            pair<double,double> pairwise_statistics(const vector<string>&);
            double tajima_d_statistics(const double&,const double&,const double&);

            //boost::property_tree::ptree save(void);

};
template<Ploidy P>
Population<P>::Population(void){
   this->_capacity=0;
}
template<Ploidy P>
Population<P>::Population(const string &_name){
   this->_capacity=0;
   this->_name=_name;
}
template<Ploidy P>
Population<P>::Population(const boost::property_tree::ptree &_fpopulation){
   this->_name=_fpopulation.get<string>("name");

   for(auto& individual : _fpopulation.get_child("individuals")){  
      shared_ptr<Individual<P>> in=make_shared<Individual<P>>(individual.second.get<uint32_t>("id"));
      for(auto& chromosome : individual.second.get_child("chromosomes")){

         array<shared_ptr<Chromosome>,P> ch;
         for_each(ch.begin(),ch.end(),[&](shared_ptr<Chromosome> &c){c=make_shared<Chromosome>(chromosome.second.get<uint32_t>("id"));});
         for(auto& gene : chromosome.second.get_child("genes")){
            int p=0;
            for(auto& sequence : gene.second.get_child("sequences"))
               ch[p++]->add(make_shared<Gene>(gene.second.get<uint32_t>("id"),sequence.second.data()));
         }
         in->add(ch);
      }
      this->_population.push_back(in);
   }
   this->_capacity=this->size();
}

template<Ploidy P>
Population<P>::Population(const string &_name,const size_t &_n_individuals){
   this->_name=_name;
	this->_capacity=_n_individuals;
	this->_population.reserve(_n_individuals);
}
template<Ploidy P>
Population<P>::Population(const size_t &_n_individuals){
	this->_capacity=_n_individuals;
	this->_population.reserve(_n_individuals);
}
template<Ploidy P>
Population<P>::Population(const Population &_p){
	this->_capacity=_p._capacity;
	this->_population.reserve(_p.size());
	for(auto& individual : _p.population())
		this->add(make_shared<Individual>(*individual.get()));
}

template<Ploidy P>
size_t Population<P>::capacity(void) const{
	return(this->_capacity);
}
template<Ploidy P>
string Population<P>::name(void) const{
	return(this->_name);
}

template<Ploidy P>
void Population<P>::name(const string &_name){
   this->_name=_name;
}

template<Ploidy P>
void Population<P>::capacity(const size_t &_capacity){
	this->_capacity=_capacity;
}

template<Ploidy P>
size_t Population<P>::size(void) const{
	return(this->_population.size());
}

template<Ploidy P>
vector<shared_ptr<Individual<P>>> Population<P>::population(void) const{
	return(this->_population);
}

template<Ploidy P>
void Population<P>::add(const shared_ptr<Individual<P>> &_individual){
	this->_population.push_back(_individual);
}

template<Ploidy P>
shared_ptr<Individual<P>> Population<P>::at(const size_t &_index) const{
	return(this->_population[_index]);	
}

template<Ploidy P>
void Population<P>::clear(void){
	this->_population.clear();
}

template<Ploidy P>
void Population<P>::merge(const shared_ptr<Population> &_p){
	this->_capacity+=_p->capacity();
	this->_population.reserve(this->_capacity);

	while(!_p->_population.empty()){
		this->add(_p->_population.back());
		_p->_population.pop_back();
	}
}

template<Ploidy P>
vector<shared_ptr<Population<P>>> Population<P>::split(const size_t &_n_populations){
	size_t round_robin=0;
	vector<shared_ptr<Population<P>>> p(_n_populations,nullptr);
   
	for_each(p.begin(),p.end(),[&](shared_ptr<Population<P>> &p){p=make_shared<Population<P>>();});

	random_shuffle(this->_population.begin(),this->_population.end());

   while(!this->_population.empty()){
      p[round_robin]->add(this->_population.back());
      this->_population.pop_back();
      ++round_robin%=_n_populations;
   }

   for_each(p.begin(),p.end(),[&](shared_ptr<Population<P>> &p){p->capacity(p->size());});

	return(p);
}

template<Ploidy P>
void Population<P>::migration(const shared_ptr<Population> &_p,const double &_percentage){
	size_t n=size_t(floor(double(this->size())*_percentage));
	random_shuffle(this->_population.begin(),this->_population.end());

	_p->capacity(_p->capacity()+n);
	for(size_t i=0;i<n;i++){
		_p->add(this->_population.back());
		this->_population.pop_back();
	}
	this->capacity(this->size());
}

template<Ploidy P>
void Population<P>::decrease(const double &_percentage){
	size_t n=size_t(floor(double(this->capacity())*_percentage));
	this->capacity(this->capacity()-n);
}

template<Ploidy P>
void Population<P>::increase(const double &_percentage){
	size_t n=size_t(floor(double(this->capacity())*_percentage));
	this->capacity(this->capacity()+n);
}

template<Ploidy P>
Population<P>::~Population(void){
   this->_population.clear();
}
template<Ploidy P>
boost::property_tree::ptree Population<P>::indices(const double &_percentage){
	random_shuffle(this->_population.begin(),this->_population.end());
   auto sample=vector<shared_ptr<Individual<P>>>(this->_population.begin(),this->_population.begin()+int(floor(double(this->size())*_percentage)));

   map<uint32_t,map<uint32_t,vector<string>>> sequences;

   for(auto& individual : sample){
      for(auto& chromosomes : individual->chromosomes()){
         for(auto& chromosome : chromosomes){
            for(auto& gene : chromosome->genes())
               sequences[chromosome->id()][gene->id()].push_back(gene->strseq());
         }
      }
   }

   double mean_of_the_number_of_pairwise_differences,variance_of_the_number_of_pairwise_differences;
   boost::property_tree::ptree fpopulation;
   boost::property_tree::ptree fchromosomes;
   fpopulation.put("name",this->name());

   for(size_t chromosome_id=0U;chromosome_id<sequences.size();chromosome_id++){
      boost::property_tree::ptree fchromosome;
      fchromosome.put("id",chromosome_id);

      for(size_t gene_id=0U;gene_id<sequences[chromosome_id].size();gene_id++){
         boost::property_tree::ptree fgenes;
         boost::property_tree::ptree fgene;
         fgene.put("id",gene_id);

         boost::property_tree::ptree findices;
         findices.put("number-of-haplotypes",this->number_of_haplotypes(sequences[chromosome_id][gene_id]));
         findices.put("number-of-segregating-sites",this->number_of_segregating_sites(sequences[chromosome_id][gene_id]));
         tie(mean_of_the_number_of_pairwise_differences,variance_of_the_number_of_pairwise_differences)=this->pairwise_statistics(sequences[chromosome_id][gene_id]);
         findices.put("mean-of-the-number-of-pairwise-differences",mean_of_the_number_of_pairwise_differences);
         findices.put("variance-of-the-number-of-pairwise-differences",variance_of_the_number_of_pairwise_differences);
         findices.put("tajima-d-statistics",this->tajima_d_statistics(double(sequences[chromosome_id][gene_id].size()),
                                                                      findices.get<double>("number-of-segregating-sites"),
                                                                      mean_of_the_number_of_pairwise_differences));
         fgene.push_back(std::make_pair("indices",findices));
         fgenes.push_back(std::make_pair("",fgene));
         fchromosome.push_back(make_pair("genes",fgenes));
      }
      fchromosomes.push_back(make_pair("",fchromosome));
   }
   fpopulation.push_back(std::make_pair("chromosomes",fchromosomes));
   return(fpopulation);
}
template<Ploidy P>
double Population<P>::number_of_haplotypes(const vector<string> &_sequences){
   map<string,double> haplotypes;
   for(auto& seq : _sequences)
      haplotypes[seq]=(haplotypes.count(seq))?haplotypes[seq]+1.0:1.0;

   double sum=0.0;
   double N=double(_sequences.size());
   for(auto& hap : haplotypes){
      double x=double(hap.second)/N;
      sum+=(x*x);
   }
   return((N/(N-1.0))*(1.0-sum));
}
template<Ploidy P>
double Population<P>::number_of_segregating_sites(const vector<string> &_sequences){
   double ss=0.0;
   
   string ref=_sequences[0];
   for(size_t i=0;i<ref.length();i++){
      for(size_t j=1;j<_sequences.size();j++){
         if(ref.at(i)!=_sequences[j].at(i)){
            ss+=1.0;
            break;
         }
      }
   }
   return(ss);
}
template<Ploidy P>
pair<double,double> Population<P>::pairwise_statistics(const vector<string> &_sequences){
   vector<double> pairwise_differences;
   double mean=0.0;
   
   for(size_t i=0;i<_sequences.size();i++){
      for(size_t j=i+1;j<_sequences.size();j++){
         double diff=0.0;
         for(size_t k=0;k<_sequences[i].length();k++){
            if(_sequences[i][k]!=_sequences[j][k])
               diff+=1.0;
         }
         pairwise_differences.push_back(diff);
         mean+=diff;
      }
   }
   mean/=double(pairwise_differences.size());
   
   double variance=0.0;
   for(auto& diff : pairwise_differences)
      variance+=(diff-mean)*(diff-mean);

   variance/=double(pairwise_differences.size());

   return(make_pair(mean,variance));
}
template<Ploidy P>
double Population<P>::tajima_d_statistics(const double &_n,const double &_ss,const double &_mpd){
   double a1=0.0,a2=0.0;

   for(size_t k=1;k<size_t(_n);k++){
      a1+=1.0/double(k);
      a2+=1.0/double(k*k);
   }

   double b1=(_n+1.0)/(3.0*(_n-1.0));
   double b2=(2.0*(_n*_n+_n+3.0))/((9.0*_n)*(_n-1.0));
   double c1=b1-(1.0/a1);
   double c2=b2+((_n+2.0)/(a1*_n))+a2/(a1*a1);
        
   double e1=c1/a1;
   double e2=c2/(a1*a1+a2);

   return((_mpd-(_ss/a1))/sqrt(e1*_ss+e2*_ss*(_ss-1.0)));
}
#endif
