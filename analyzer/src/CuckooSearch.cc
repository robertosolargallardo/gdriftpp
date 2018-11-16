#include "CuckooSearch.h"

CuckooSearch::CuckooSearch(const int number_of_parameters, const int number_of_eggs, const std::vector<double> min, const std::vector<double> max)
{
	this->number_of_parameters = number_of_parameters;
	this->number_of_eggs = number_of_eggs;
	this->beta = 1.5;
	this->pa = 0.25;
	this->cuckoo_eggs.resize(number_of_eggs);
	this->new_eggs.assign(number_of_eggs, CuckooEgg(number_of_parameters));
	this->min = min;
	this->max = max;
	CuckooNest nst(this->number_of_eggs, this->number_of_parameters, this->pa);
	this->nst = nst;
	Cuckoo cko(this->beta,this->number_of_parameters,this->min,this->max);
	this->cko = cko;
}


CuckooSearch::CuckooSearch(const int number_of_parameters, const int number_of_eggs, const double beta, const double pa, const std::vector<double> min, const std::vector<double> max)
{
	this->number_of_parameters = number_of_parameters;
	this->number_of_eggs = number_of_eggs;
	this->beta = beta;
	this->pa = pa;
	this->cuckoo_eggs.resize(number_of_eggs);
	this->new_eggs.resize(number_of_eggs);
	this->min = min;
	this->max = max;
	CuckooNest nst(this->number_of_eggs, this->number_of_parameters, this->pa);
	this->nst = nst;
	Cuckoo cko(this->beta,this->number_of_parameters,this->min,this->max);
	this->cko = cko;
}

std::vector<std::vector<double> > CuckooSearch::first_step(const std::vector<double> fitt, const std::vector<std::vector<double> > info)
{
	this->nst.initial_population(fitt, info);
	for(int i=0;i<this->number_of_eggs;i++)
	{
		this->cuckoo_eggs[i].resize(this->number_of_eggs);
		this->cuckoo_eggs[i] = cko.get_cuckoo(nst.get_egg_solution(i));
	}
	return this->cuckoo_eggs;
}

std::vector<std::vector<double> > CuckooSearch::any_step(const std::vector<double> fitt, const std::vector<std::vector<double> > info, const std::vector<double> pa_fitt, const std::vector<std::vector<double> > pa_eggs)
{

	this->get_new_eggs(fitt, info);

	for(int i=0;i<this->number_of_eggs;i++)
	{
		nst.compare_eggs(new_eggs[i].get_fitness(),new_eggs[i].get_solution());
	}
	
	nst.rank_eggs();

	nst.destroy_eggs(pa_fitt,pa_eggs);

	for(int i=0;i<this->number_of_eggs;i++)
	{
		this->cuckoo_eggs[i] = cko.get_cuckoo(nst.get_egg_solution(i));
	}

	/* Descomenta esta linea si quieres información sobre el comportamiento de las soluciones*/
	nst.postprocess();

	return this->cuckoo_eggs;
	
}

void CuckooSearch::get_new_eggs(const std::vector<double> fitt, const std::vector<std::vector<double> > info)
{
	for(int i=0;i<this->number_of_eggs;i++)
	{
		this->new_eggs[i].set_fitness(fitt[i]);
		this->new_eggs[i].change_egg(info[i]);
	}
}

CuckooSearch::~CuckooSearch(void)
{
	;
}
