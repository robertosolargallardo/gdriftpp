#include <Simulator.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

random_device seed;
mt19937 rng(seed());

boost::property_tree::ptree build(string name,map<uint32_t,map<uint32_t,map<string,double>>> &_indices){
   boost::property_tree::ptree fpopulation;
   fpopulation.put("name",name);
   boost::property_tree::ptree fchromosomes;

   for(size_t chromosome_id=0;chromosome_id<_indices.size();chromosome_id++){
      boost::property_tree::ptree fchromosome;
      fchromosome.put("id",chromosome_id);
      for(size_t gene_id=0;gene_id<_indices[chromosome_id].size();gene_id++){
         boost::property_tree::ptree fgenes;
         boost::property_tree::ptree fgene;
         fgene.put("id",gene_id);

         boost::property_tree::ptree findices;
         for(auto& iter : _indices[chromosome_id][gene_id])
            findices.put(iter.first,iter.second);

         fgene.push_back(std::make_pair("indices",findices));
         fgenes.push_back(std::make_pair("",fgene));
         fchromosome.push_back(make_pair("genes",fgenes));
      }
      fchromosomes.push_back(make_pair("",fchromosome));
   }

   fpopulation.push_back(std::make_pair("chromosomes",fchromosomes));
   return(fpopulation);
}
void run(const boost::property_tree::ptree &_fconfiguration){
   boost::property_tree::ptree findividual=_fconfiguration.get_child("individual");
   boost::property_tree::ptree fscenario=_fconfiguration.get_child("scenario");

   boost::property_tree::ptree fresults;
   boost::property_tree::ptree fpopulations;

   fresults.put("id",_fconfiguration.get<string>("id"));
   fresults.put("run",_fconfiguration.get<string>("run"));
   fresults.put("batch",_fconfiguration.get<string>("batch"));
   fresults.put("type","simulated");
   fresults.put("max-number-of-simulations",_fconfiguration.get<string>("max-number-of-simulations"));

   shared_ptr<EventList> eventlist=make_shared<EventList>(fscenario);

   Ploidy ploidy=(Ploidy)findividual.get<uint32_t>("ploidy");
   Model model=(Model)fscenario.get<uint32_t>("model");
   switch(ploidy){
      case HAPLOID:  {
                        shared_ptr<Generator<HAPLOID>> generator=make_shared<Generator<HAPLOID>>(findividual);
                        shared_ptr<Simulator<HAPLOID>> simulator=make_shared<Simulator<HAPLOID>>();
                        vector<shared_ptr<Population<HAPLOID>>> populations;

                        switch(model){
                           case WRIGHTFISHER:{
                                                simulator->run<WRIGHTFISHER>(generator,eventlist);
                                                break;
                                             }
                           default: {
                                       cerr << "Unknown Model ::" << model << endl;
                                       exit(EXIT_FAILURE);
                                    }
                        }

                        shared_ptr<Population<HAPLOID>> all=make_shared<Population<HAPLOID>>();
                        for(auto& population : simulator->populations()){
                           auto findices=population.second->indices();
                           fpopulations.push_back(std::make_pair("",findices));
                           all->merge(population.second);  
                        }
                        auto findices=all->indices();
                        fpopulations.push_back(std::make_pair("",findices));

                        break;
                     };
      case DIPLOID:  {
                        shared_ptr<Generator<DIPLOID>> generator=make_shared<Generator<DIPLOID>>(findividual);
                        shared_ptr<Simulator<DIPLOID>> simulator=make_shared<Simulator<DIPLOID>>();

                        switch(model){
                           case WRIGHTFISHER:{
                                                simulator->run<WRIGHTFISHER>(generator,eventlist);
                                                break;
                                             }
                           default: {
                                       cerr << "Unknown Model ::" << model << endl;
                                       exit(EXIT_FAILURE);
                                    }
                        }

                        shared_ptr<Population<DIPLOID>> all=make_shared<Population<DIPLOID>>();
                        for(auto& population : simulator->populations()){
                           auto findices=population.second->indices();
                           fpopulations.push_back(std::make_pair("",findices));
                           all->merge(population.second);  
                        }
                        auto findices=all->indices();
                        fpopulations.push_back(std::make_pair("",findices));

                        break;
                     }
      default: {
                  cerr << "Unknown Ploidy Type::" << ploidy << endl;
                  exit(EXIT_FAILURE);
               }
   }
   fresults.push_back(make_pair("populations",fpopulations));
   fresults.push_back(make_pair("individual",findividual));
   fresults.push_back(make_pair("scenario",fscenario));
   write_json("indices-simulation.json",fresults);
}
void load(const boost::property_tree::ptree &_fdata){
   boost::property_tree::ptree fresults;
   fresults.put("id",_fdata.get<string>("id"));
   fresults.put("type","data");

   boost::property_tree::ptree fpopulations;
   Ploidy ploidy=(Ploidy)_fdata.get<uint32_t>("ploidy"); 

   switch(ploidy){
      case HAPLOID:  {
                        shared_ptr<Population<HAPLOID>> all=make_shared<Population<HAPLOID>>("summary");
                        for(auto& population : _fdata.get_child("populations")){
                           shared_ptr<Population<HAPLOID>> p=make_shared<Population<HAPLOID>>(population.second);
                           auto findices=p->indices();
                           fpopulations.push_back(std::make_pair("",findices));
                           all->merge(p);  
                        }
                        auto findices=all->indices();
                        fpopulations.push_back(std::make_pair("",findices));
                        
                        break;
                     };
      case DIPLOID:  {
                        shared_ptr<Population<DIPLOID>> all=make_shared<Population<DIPLOID>>("summary");
                        for(auto& population : _fdata.get_child("populations")){
                           shared_ptr<Population<DIPLOID>> p=make_shared<Population<DIPLOID>>(population.second);
                           auto findices=p->indices();
                           fpopulations.push_back(std::make_pair("",findices));
                           all->merge(p);  
                        }
                        auto findices=all->indices();
                        fpopulations.push_back(std::make_pair("",findices));

                        break;
                     };
      default: {
                  cerr << "Unknown Ploidy Type::" << ploidy << endl;
                  exit(EXIT_FAILURE);
               };
   }

   fresults.push_back(make_pair("populations",fpopulations));
   write_json("indices-data.json",fresults);
}
int main(int argc,char **argv){ 
   cout << util::hash("init") << endl;
   cout << util::hash("continue") << endl;
   cout << util::hash("finalize") << endl;

   return(0);


   boost::property_tree::ptree fconfiguration,fdata;
   //read_json(argv[1],fconfiguration);
   read_json(argv[1],fdata);
   
   //run(fconfiguration);
   load(fdata);
}
