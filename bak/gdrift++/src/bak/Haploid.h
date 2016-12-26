#ifndef _HAPLOID_H_
#define _HAPLOID_H_
#include "Individual.h"
#include "Chromosome.h"
#include <vector>
#define Haploid(...)    make_shared<haploid>(__VA_ARGS__)

using namespace std;

class haploid;
typedef shared_ptr<haploid> Haploid;

class haploid: public individual, public enable_shared_from_this<haploid>{
   private: vector<array<Chromosome,HAPLOID>> _chromosomes;
            
   public:  haploid(void);
            haploid(const uint32_t&,const size_t&);
            haploid(const haploid&);
            ~haploid(void);

				void add(const array<Chromosome,HAPLOID>&);
				void clear(void);
            vector<array<Chromosome,HAPLOID>> chromosomes(void) const;
            array<Chromosome,HAPLOID> at(const size_t&);
            size_t n_chromosomes(void) const;
};
#endif
