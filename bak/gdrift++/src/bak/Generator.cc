#include "Generator.h"
generator::generator(void){

}
generator::generator(const string &_scenario_file){
   this->_gp=GenePool();	

   size_t n_nucleotides;
   uint32_t type,n_chromosomes,n_genes,polymorphism,n_alleles;
   double mutation_rate;
   
   std::ifstream ifs(_scenario_file,std::ifstream::in);
   
   ifs >> type >> n_chromosomes;

   for(uint32_t c=0;c<n_chromosomes;c++){
      ifs >> n_genes;
      for(uint32_t g=0;g<n_genes;g++){
         ifs >> n_nucleotides >> mutation_rate >> polymorphism >> n_alleles;
         this->_scenario[c][g]=make_tuple(n_nucleotides,mutation_rate,polymorphism,n_alleles);
			this->_gp->generate(c,g,n_nucleotides,mutation_rate,polymorphism,n_alleles);
      }
   }
   ifs.close();

}
Population generator::generate(const uint32_t &_type,const size_t &_n_individuals){
	   
   Population pop=Population(_n_individuals);
   switch(_type){
      case HAPLOID:  break;
      case DIPLOID:  for(size_t id=0;id<_n_individuals;id++){
								Diploid dip=Diploid(id,this->_scenario.size());
                        for(auto c : this->_scenario){
									array<Chromosome,DIPLOID> chrom={Chromosome(c.second.size()),Chromosome(c.second.size())};
                           for(auto g : c.second){
                              chrom[0]->add(Gene(*this->_gp->get(c.first,g.first)));
                              chrom[1]->add(Gene(*this->_gp->get(c.first,g.first)));
                           }
                           dip->add(chrom);
                        }
                        pop->insert(dip);
							}
                     break;
   }

   return(pop);
}
generator::~generator(void){

}
