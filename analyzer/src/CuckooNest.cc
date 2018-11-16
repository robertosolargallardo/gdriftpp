#include "CuckooNest.h"

CuckooNest::CuckooNest(void)
{
	;
}

CuckooNest::CuckooNest(const int &number_of_eggs, const int &number_of_parameters, const double &pa)
{
	//Each attribute of the class is initialized and the memory is reserved for each vector.
	this->number_of_eggs = number_of_eggs;
	this->number_of_parameters = number_of_parameters;
	//The vector eggs is initialized whit an egg object in each position.
	this->eggs.assign(number_of_eggs, CuckooEgg(number_of_parameters));
	std::uniform_int_distribution<int> position_selector(0,number_of_eggs-1);
	this->position_selector = position_selector;
	this->pa = pa;
}

void CuckooNest::initial_population(const std::vector<double> fitt, const std::vector<std::vector<double> > info)
{
	for( int i=0;i<this->number_of_eggs;i++)
	{
		this->eggs[i].set_fitness(fitt[i]);
		this->eggs[i].change_egg(info[i]);
	}
}

void CuckooNest::compare_eggs(const double cuckoo_egg_fitness, const std::vector<double> cuckoo_egg)
{
	//The position of the egg in the nest is obtained randomly
	//TRICK : The best egg from this iteration remain unchanged
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 generator(rd()); //Standard mersenne_twister_engine seeded with rd()
	int egg_to_compare = this->position_selector(generator);
	while(egg_to_compare==0)
	{
		egg_to_compare = this->position_selector(generator);
	}
	//If the fitness of the cuckoo egg is better than the fitness of the egg to compare,
	//then the cuckoo use the space in the nest
	if(cuckoo_egg_fitness < this->eggs[egg_to_compare].get_fitness())
	{
		//std::cout << "Egg: " << egg_to_compare << " de fitt: " << this->eggs[egg_to_compare].get_fitness() << " se cambia por egg con fitt: " << cuckoo_egg_fitness << std::endl;  
		this->eggs[egg_to_compare].set_fitness(cuckoo_egg_fitness);
		this->eggs[egg_to_compare].change_egg(cuckoo_egg);
	}
}

void CuckooNest::rank_eggs(void)
{
	std::sort(this->eggs.begin(), this->eggs.begin() + this->number_of_eggs,[](CuckooEgg const &e1, CuckooEgg const &e2) -> bool{return e1.get_fitness() < e2.get_fitness();});
}

//TODO : Do a better post process
void CuckooNest::postprocess(void)
{
	double mean=0;
	for(int i=0;i<this->number_of_eggs;i++)
	{
		mean = mean + this->eggs[i].get_fitness();
	}
	mean = mean/this->number_of_eggs;
	std::cout << "Min/Max/Avg Fitness " << "[" << this->eggs[0].get_fitness() << "," << this->eggs[this->number_of_eggs-1].get_fitness() << "," << mean <<"]" << std::endl;
}

std::vector<double> CuckooNest::get_egg_solution(const int &i) const
{
	return this->eggs[i].get_solution();
}

double CuckooNest::get_best_fitness(void) const
{
	return this->eggs[0].get_fitness();
}

//Remplazar fracción pa de eggs con los del arreglo
void CuckooNest::destroy_eggs(const std::vector<double> pa_fitt, const std::vector<std::vector<double> > pa_eggs)
{
	int first_to_eliminate = this->number_of_eggs-(this->pa*this->number_of_eggs);
	int count = 0;
	for(int i=first_to_eliminate;i<this->number_of_eggs;i++)
	{
		eggs[i].set_fitness(pa_fitt[count]);
		eggs[i].change_egg(pa_eggs[count]);
		count++;
	}
}


CuckooNest::~CuckooNest(void)
{
	;
}
