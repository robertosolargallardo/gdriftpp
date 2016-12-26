#include "Pool.h" 
Pool::Pool(void){
	;
}
Pool::~Pool(void){
	for(auto iter : this->_genes)
		iter.second.clear();
	this->_genes.clear();
}
shared_ptr<Gene> Pool::get(const uint32_t &_chromosome_id,const uint32_t &_gene_id){
   uniform_int_distribution<int> uniform(0,this->_genes[_chromosome_id][_gene_id].size()-1);
	return(this->_genes[_chromosome_id][_gene_id][uniform(rng)]);
}
void Pool::generate(const uint32_t &_chromosome_id,
                    const uint32_t &_gene_id,
                    const uint32_t &_number_of_nucleotides,
                    const double   &_mutation_rate,
                    const uint32_t &_number_of_segregating_sites,
                    const uint32_t &_number_of_alleles,
                    shared_ptr<Gene> _gene){

	if(_gene==nullptr)
		_gene=make_shared<Gene>(_gene_id,_mutation_rate);

	if(this->_genes[_chromosome_id][_gene_id].size()<_number_of_alleles){
		string str;
		for(unsigned char c=0;c<NUCLEOTIDES;c++){
			if(_gene->seq().size()==(_number_of_segregating_sites-1)){
				for(;c<NUCLEOTIDES;c++){
					if(this->_genes[_chromosome_id][_gene_id].size()==_number_of_alleles) return;
	
					_gene->push(c);
					this->_genes[_chromosome_id][_gene_id].push_back(make_shared<Gene>(*_gene));
					_gene->pop();
				}
				return;
			}
			else{
				_gene->push(c);
				this->generate(_chromosome_id,_gene_id,_number_of_nucleotides,_mutation_rate,_number_of_segregating_sites,_number_of_alleles,_gene);
				_gene->pop();

			}
		}
		if(_gene->seq().size()>0) return;
	}
	for(auto& g : this->_genes[_chromosome_id][_gene_id]){
		while(g->length()<_number_of_nucleotides) 
			g->push(ADENINA);
	}
}
