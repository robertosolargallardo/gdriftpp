#ifndef _DISTANCE_H
#define _DISTANCE_H
#include <string>
#include <algorithm>
using namespace std;

inline unsigned int distance(const std::string& s1, const std::string& s2)
{
    const std::size_t len1=s1.size()+1,len2=s2.size()+1;
    unsigned int **d=(unsigned int**)malloc(len1*sizeof(unsigned int*));
    for(unsigned int i=0;i<len1;i++) d[i]=(unsigned int*)malloc(sizeof(unsigned int)*len2);

    d[0][0]=0;
    for(unsigned int i=1;i<len1;++i) d[i][0] = i;
    for(unsigned int i=1;i<len2;++i) d[0][i] = i;

    for(unsigned int i=1;i<len1;++i)
        for(unsigned int j=1;j<len2;++j)
            d[i][j]=std::min({d[i-1][j]+1,d[i][j-1]+1,d[i-1][j-1]+(s1[i-1]==s2[j-1]?0:1)});

    unsigned int dist=d[len1-1][len2-1];
    for(unsigned int i=0;i<len1;i++) free(d[i]);
    free(d);

    return dist;
}
#endif
