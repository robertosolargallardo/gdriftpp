#ifndef _GENE_H_
#define _GENE_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <string>
#include <iostream>
#include <sstream>
#include <random>
#define NUCLEOTIDES sizeof(enum nucleotides)
enum nucleotides{ADENINA=0,GUANINA=1,CITOCINA=2,TIMINA=3};
enum markerType{DNASEQUENCE=0};

using namespace std;
extern mt19937 rng;

//template<markerType T>
class Gene: public enable_shared_from_this<Gene>{
   private: vector<unsigned char> _seq;
            double _mutation_rate;
            uint32_t _id;

   public:  Gene(void);
            Gene(const uint32_t&,const string&);
            Gene(const uint32_t&,const double&);
            Gene(const uint32_t&,const double&,const vector<unsigned char>&);
            Gene(const uint32_t&,const double&,const string&);
            Gene(const Gene&);
            ~Gene(void);

				void push(const unsigned char&);
				void pop(void);
				void clear(void);
            string strseq(void) const;
            void strseq(const string&);
            vector<unsigned char> seq(void) const;
            void seq(const vector<unsigned char>&);
            size_t length(void) const;

            double mutation_rate(void) const;
            void mutation_rate(const double&);

            //void mutate(void);
            
            uint32_t id(void);
};
#endif
