#include <random>
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

#include "CuckooEgg.h"

class CuckooNest {

private:
	//The number of eggs. Each egg represent a solution.
	int number_of_eggs;
	//The number of parameters of the problem.
	int number_of_parameters;
	//A vector that contain objects of the class 'CuckooEgg'.
	std::vector<CuckooEgg> eggs;
	//Fraction of worse nests that are going to be destroyed.
	double pa;
	//
	std::uniform_int_distribution<int> position_selector;

public:	
	//Empy constructor	
	CuckooNest(void);
	//Constructor of the class nest.
	//	int number_of_eggs : number of eggs to use
	//	int number_of_parameters : number of parameters to find
	//	double pa : fraction of nests to be destroyed. Must be a value bewteen 0 and 1.
	CuckooNest(const int &number_of_eggs, const int &number_of_parameters, const double &pa);
	//Metod that compare the nest egg and the cuckoo egg. If the cuckoo egg has a better fitness, it replace the nest egg.
	//	double cuckoo_egg_fitness : fitnes of the cuckoo egg.
	//	vector cuckoo_solution : vector that represent the cuckoo egg. Each position is a solution for the i parameter.
	void compare_eggs(const double cuckoo_egg_fitness, const std::vector<double> cuckoo_solution);
	//Metod that rank the eggs leaving the better ones (better fitness) at first place.
	void rank_eggs(void);
	//Metod that show information of the solutions.
	void postprocess(void);
	//Metod that return the egg solution.
	//	int i : Position of the egg in the vector.
	std::vector<double> get_egg_solution(const int &i) const;
	//Get the fitness of the best egg
	double get_best_fitness(void) const;
	//Initialize the population
	//	vector fitt -> Vector of fitness of each egg i
	//	vector info -> Solution of each the egg i
	void initial_population(const std::vector<double> fitt, const std::vector<std::vector<double> > info);
	//Change pa% of the worst eggs
	//	vector pa_fitt -> fittnes of the egg i to be changed
	//	vector pa_eggs -> solution of egg i to be changed
	void destroy_eggs(const std::vector<double> pa_fitt, const std::vector<std::vector<double> > pa_eggs);
	//Desctuctor of the class.
	~CuckooNest(void);
};
