#ifndef _UTIL_H_
#define _UTIL_H_
#include <string>
using namespace std;
namespace util{
	inline uint32_t hash(const string &__str)
	{
	    uint32_t h=0U;
	    for(uint32_t i=0; i<__str.length(); i++) {
	        h+= __str.at(i);
  		     h+= (h<< 10);
        	  h^= (h>> 6);
        }
        h+=(h<< 3);
        h^=(h>> 11);
        h+=(h<< 15);

        return(h);
    }
};
#endif
