#ifndef _GENE_H_
#define _GENE_H_
#include "Allele.h"
#include <vector>
#define Gene(arg) make_shared<gene>(arg)

using namespace std;

class gene;
typedef shared_ptr<gene> Gene;

class gene{
   private: vector<Allele> _alleles;

   public:  gene(void);
            gene(const size_t&);
            ~gene(void);

            void add(const Allele&);
            void clear(void);
            vector<Allele> alleles();
            void alleles(const vector<Allele>&);
};
#endif
