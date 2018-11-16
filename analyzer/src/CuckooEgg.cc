#include "CuckooEgg.h"

CuckooEgg::CuckooEgg(void)
{
	;
}

CuckooEgg::CuckooEgg(const int &number_of_parameters)
{
	//Each attribute of the class is initialized and the memory is reserved for each vector.
	this->number_of_parameters = number_of_parameters;
	this->solution.resize(number_of_parameters);
}

void CuckooEgg::change_egg(std::vector<double> new_egg)
{
	this->solution = new_egg;
}

double CuckooEgg::get_fitness(void) const
{
	return this->fitness;
}

void CuckooEgg::set_fitness(double fitt)
{
	this->fitness = fitt;
}

std::vector<double> CuckooEgg::get_solution(void) const
{
	return this->solution;
}

CuckooEgg::~CuckooEgg(void)
{
	;
}
