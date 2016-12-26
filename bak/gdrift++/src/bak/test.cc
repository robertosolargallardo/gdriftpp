#include <ctime>
#include <cstdlib>
#include <sstream>
#include <string>
#include "Allele.h"
#include "Diploid.h"
#include "Population.h"
#include "Random.h"
#define NUCLEOTIDES 12
#define ITERATIONS 10000

Population* generateP(const unsigned int &N);
Allele generateA(const unsigned int &n);
Population* wf(Population* &p);

int main(void){
   std::srand(unsigned(std::time(0)));

   Population *p=generateP(1000);

   cout << p->diversity() << endl;
   rnd::uniform_params::a=0;
   rnd::uniform_params::b=p->size()-1;
   rnd::init();
   p=wf(p);
   cout << p->diversity() << endl;

   delete p;

   return(0);
}
Population* wf(Population* &p){
   Population *q=nullptr;
   
   for(int i=0;i<ITERATIONS;i++){
      q=new Population(p->size());
      for(unsigned int j=0;j<p->size();j++){
         Diploid *d=new Diploid(j,((Diploid*)p->at(size_t(floor(rnd::uniform()))))->a(),((Diploid*)p->at(size_t(floor(rnd::uniform()))))->b());
         q->add(d);
         delete d;
      }
      delete p;
      p=q;
   }

   return(p);
}
Population* generateP(const unsigned int &N){
   Population *p=new Population(N);
   list<Allele> a,b;
   Diploid *d=nullptr;

   for(unsigned int i=0;i<N;i++){
      a.push_back(generateA(NUCLEOTIDES));     
      b.push_back(generateA(NUCLEOTIDES));
      d=new Diploid(i,a,b);
      p->add(d);
      
      delete d;
      a.clear();
      b.clear();
   }

   return(p);
}
Allele generateA(const unsigned int &n){
   std::random_device rd;
   std::mt19937 gen(rd());
   std::uniform_int_distribution<> ud(0,3);

   std::stringbuf buffer;
   std::ostream os(&buffer);

   for(unsigned int i=0;i<n;i++){
      int x=ud(gen);
      switch(x){
         case 0: os << 'A'; break;
         case 1: os << 'C'; break;
         case 2: os << 'G'; break;
         case 3: os << 'T'; break;
      }
   }
   return(Allele(buffer.str()));
}
