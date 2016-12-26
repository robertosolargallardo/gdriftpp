#include <Simulator.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

random_device seed;
mt19937 rng(seed());

boost::property_tree::ptree build(map<uint32_t,map<uint32_t,map<string,double>>> &_indices){
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
   return(fchromosomes);
}
void run(const boost::property_tree::ptree &_configuration_file){
   boost::property_tree::ptree individual_file=_configuration_file.get_child("individual");
   boost::property_tree::ptree scenario_file=_configuration_file.get_child("scenario");

   Ploidy ploidy=(Ploidy)individual_file.get<uint32_t>("ploidy"); 
 
   boost::property_tree::ptree fresults;
   fresults.put("id",_configuration_file.get<string>("id"));
   fresults.put("run",_configuration_file.get<string>("run"));
   fresults.put("batch",_configuration_file.get<string>("batch"));
   fresults.put("max-number-of-simulations",_configuration_file.get<string>("max-number-of-simulations"));

   boost::property_tree::ptree fresults2(_configuration_file);

   switch(ploidy){
      case HAPLOID:  {
                        shared_ptr<Generator<HAPLOID>> generator=make_shared<Generator<HAPLOID>>(individual_file);
                        break;
                     };
      case DIPLOID:  {
                        shared_ptr<Generator<DIPLOID>> generator=make_shared<Generator<DIPLOID>>(individual_file);
                        shared_ptr<EventList> eventlist=make_shared<EventList>(scenario_file);
                        shared_ptr<Simulator<DIPLOID>> simulator=make_shared<Simulator<DIPLOID>>();

                        Model model=(Model)scenario_file.get<uint32_t>("model"); 
                        switch(model){
                           case WRIGHTFISHER:{
                                                simulator->run<WRIGHTFISHER>(generator,eventlist);
                                                auto populations=simulator->populations();
                                          
                                                boost::property_tree::ptree fpopulations;
                                                for(auto& population : populations){
                                                   boost::property_tree::ptree fpopulation;
                                                   fpopulation.put("name",population.first);

                                                   auto indices=population.second->indices();
                                                   boost::property_tree::ptree fchromosomes=build(indices);
                                                   fpopulation.push_back(std::make_pair("chromosomes",fchromosomes));
                                                   fpopulations.push_back(std::make_pair("",fpopulation));
                                                }
                                                fresults.push_back(make_pair("populations",fpopulations));
                                                fresults.push_back(make_pair("individual",individual_file));
                                                fresults.push_back(make_pair("scenario",scenario_file));
                                                write_json("indices-simulation.json",fresults);
                                                   
                                                break;
                                             }
                           default: {
                                       cerr << "Unknown Model ::" << model << endl;
                                       exit(EXIT_FAILURE);
                                    }
                        }
                        break;
                     }
      default: {
                  cerr << "Unknown Ploidy Type::" << ploidy << endl;
                  exit(EXIT_FAILURE);
               }
   }
}
void load(const boost::property_tree::ptree &_data_file){
   Ploidy ploidy=(Ploidy)_data_file.get<uint32_t>("ploidy"); 

   boost::property_tree::ptree fresults;
   fresults.put("id",_data_file.get<string>("id"));

   switch(ploidy){
      case HAPLOID:  {
                        break;
                     }
      case DIPLOID:  {
                        
                        boost::property_tree::ptree fpopulations;
                        for(auto& population : _data_file.get_child("populations")){
                           shared_ptr<Population<DIPLOID>> p=make_shared<Population<DIPLOID>>(population.second);
                           auto indices=p->indices(1.0);
               
                           boost::property_tree::ptree fpopulation;
                           fpopulation.put("name",population.second.get<string>("name"));

                           boost::property_tree::ptree fchromosomes=build(indices);
                           fpopulation.push_back(std::make_pair("chromosomes",fchromosomes));
                           fpopulations.push_back(std::make_pair("",fpopulation));

                        }
                        fresults.push_back(make_pair("populations",fpopulations));
                        write_json("indices-data.json",fresults);
                        break;
                     }
      default: {
                  cerr << "Unknown Ploidy Type::" << ploidy << endl;
                  exit(EXIT_FAILURE);
               }
   }
}
int main(int argc,char **argv){
   boost::property_tree::ptree configuration_file,data_file;
   read_json(argv[1],configuration_file);
   read_json(argv[2],data_file);

   run(configuration_file);

   /*load(data_file);*/
}
