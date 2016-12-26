#ifndef _RANDOM_H_
#define _RANDOM_H_
namespace rnd{
   #ifndef _SEED_H 
   #define _SEED_H 
      static std::random_device seed;
      static std::mt19937 gen(seed());
      namespace uniform_params{
         static std::uniform_real_distribution<double> d;
         static double a=0.0,b=10.0;
      }
      namespace normal_params{
         static std::normal_distribution<double> d;
         static double mean=0.0,stddev=1.0;
      }
      namespace gamma_params{
         static std::gamma_distribution<double> d;
         static double alpha=0.0,beta=1.0;
      }
   #endif

   inline void init(void){
      uniform_params::d=std::uniform_real_distribution<double>(uniform_params::a,uniform_params::b);
      normal_params::d=std::normal_distribution<double>(normal_params::mean,normal_params::stddev);
      gamma_params::d=std::gamma_distribution<double>(gamma_params::alpha,gamma_params::beta);
   };
   inline double uniform(void){
      return(rnd::uniform_params::d(gen));
   }
   inline double normal(void){
      return(rnd::normal_params::d(gen));
   }
   inline double gamma(void){
      return(rnd::gamma_params::d(gen));
   }

};
#endif
