#include "Simulator.h"
shared_ptr<Gene> mutate(const shared_ptr<Gene> &_gene){
   uniform_real_distribution<double> rate(0.0,1.0);
   uniform_int_distribution<> mutation(1,3);
   shared_ptr<Gene> gene=nullptr;
   bool has_mutated=false;
   vector<unsigned char> seq=_gene->seq();
   
   for(auto& n : seq){
      if(rate(rng)<=_gene->mutation_rate()){
         n=(n+mutation(rng))%NUCLEOTIDES;
         has_mutated=true;
      }
   }
   return(has_mutated?make_shared<Gene>(_gene->id(),_gene->mutation_rate(),seq):_gene);
}

template<> 
shared_ptr<Population<DIPLOID>> Simulator<DIPLOID>::WF(shared_ptr<Population<DIPLOID>> &_src_pop){
   shared_ptr<Population<DIPLOID>> dst_pop=make_shared<Population<DIPLOID>>(_src_pop->capacity());

   /*uniform_int_distribution<> uniform(0,_src_pop->size()-1);
   uniform_int_distribution<> coin(0,DIPLOID-1);

   #pragma omp parallel for schedule(dynamic,1) 
   for(uint32_t id=0;id<dst_pop->capacity();id++){
      array<shared_ptr<Individual<DIPLOID>>,2> in{_src_pop->at(uniform(rng)),_src_pop->at(uniform(rng))};

      shared_ptr<Individual<DIPLOID>> target=make_shared<Individual<DIPLOID>>(in[coin(rng)]->n_chromosomes(),id);

      for(uint32_t c=0;c<in[0]->n_chromosomes();c++){
         array<shared_ptr<Chromosome>,DIPLOID> ch;
         for(size_t p=0;p<DIPLOID;p++){
            ch[p]=make_shared<Chromosome>(in[p]->at(c)[0]->id(),in[p]->at(c)[0]->n_genes());
            for(uint32_t j=0;j<in[p]->at(c)[0]->n_genes();j++){
               shared_ptr<Gene> g=make_shared<Gene>(*in[p]->at(c)[coin(rng)]->at(j));
               g->mutate();
               ch[p]->add(g);
            }
         }
         target->add(ch);
      }
      #pragma omp critical
         dst_pop->add(target);
   }
   
   _src_pop->clear();
   */
   return(dst_pop);
}
template<> 
shared_ptr<Population<HAPLOID>> Simulator<HAPLOID>::WF(shared_ptr<Population<HAPLOID>> &_src_pop){
   shared_ptr<Population<HAPLOID>> dst_pop=make_shared<Population<HAPLOID>>(_src_pop->name(),_src_pop->capacity());

   uniform_int_distribution<> uniform(0,_src_pop->size()-1);

   //#pragma omp parallel for schedule(dynamic,1) 
   for(uint32_t id=0;id<dst_pop->capacity();id++){
      shared_ptr<Individual<HAPLOID>> source=_src_pop->at(uniform(rng));
      shared_ptr<Individual<HAPLOID>> target=make_shared<Individual<HAPLOID>>(id,source->n_chromosomes());
      
      for(auto& chromosome : source->chromosomes()){
         array<shared_ptr<Chromosome>,HAPLOID> c={make_shared<Chromosome>(chromosome[0]->id(),chromosome[0]->n_genes())};
         for(auto& gene : chromosome[0]->genes())
            c[0]->add(mutate(gene));
         target->add(c);
      }
      //#pragma omp critical
        dst_pop->add(target);
   }
   
   _src_pop->clear();

   return(dst_pop);
}
