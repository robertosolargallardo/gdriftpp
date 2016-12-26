#ifndef _CHROMOSOME_H_
#define _CHROMOSOME_H_
#include "Gene.h"
#include <vector>
#include <algorithm>

using namespace std;

class Chromosome:public enable_shared_from_this<Chromosome>{
   private: vector<shared_ptr<Gene>> _genes;
            uint32_t _id;

   public:  Chromosome(void);
            Chromosome(const uint32_t&);
            Chromosome(const Chromosome&);
            Chromosome(const uint32_t&,const size_t&);
            ~Chromosome(void);

            shared_ptr<Gene> at(const size_t&);

            size_t n_genes(void) const;
            void add(const shared_ptr<Gene>&);
            vector<shared_ptr<Gene>> genes(void) const;
            void clear(void);

            uint32_t id(void);
};
#endif
