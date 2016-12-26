#ifndef _DIPLOID_H_
#define _DIPLOID_H_
#include "Individual.h"
#include "Chromosome.h"
#include <vector>
#define Diploid(...) make_shared<diploid>(__VA_ARGS__)

using namespace std;

class diploid;
typedef shared_ptr<diploid> Diploid;

class diploid: public individual, public enable_shared_from_this<diploid>{
   private: vector<array<Chromosome,DIPLOID>> _chromosomes;
            
   public:  diploid(void);
            diploid(const uint32_t&,const size_t&);
            diploid(const diploid&);
            ~diploid(void);

				void add(const array<Chromosome,DIPLOID>&);
				void clear(void);
            vector<array<Chromosome,DIPLOID>> chromosomes(void) const;
            array<Chromosome,DIPLOID> at(const size_t&);
            size_t n_chromosomes(void) const;
};
#endif
