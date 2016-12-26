#ifndef _POOL_H_
#define _POOL_H_
#include <memory>
#include <iostream>
#include <random>
#include <map>
#include <vector>
#include "Gene.h" 

using namespace std;
extern mt19937 rng;

class Pool:public enable_shared_from_this<Pool>{
	private:	map<uint32_t,map<uint32_t,vector<shared_ptr<Gene>>>> _genes;

	public: 	Pool(void);
				~Pool(void);

				void generate(const uint32_t&,const uint32_t&,const uint32_t&,const double&,const uint32_t&,const uint32_t&,shared_ptr<Gene> _gene=nullptr);
				shared_ptr<Gene> get(const uint32_t&,const uint32_t&);

};
#endif
