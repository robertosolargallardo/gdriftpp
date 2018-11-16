#include <vector>
#include <random>
#include <cmath>
#include <iostream>
#include <functional>

class CuckooEgg {

private:	
	//Number of parameters of the problem.
	int number_of_parameters;
	//Fitnes of the egg (quality of the solution)
	double fitness;
	//Vector that represent a solution.
	std::vector<double> solution;

public:		
	//Empy constructor
	CuckooEgg(void);
	//Constructor of the class
	//	int number_of_parameters : number of parameters in the problem
	CuckooEgg(const int &number_of_parameters);
	//Method that change the actual solution with a new one.
	//	vector new_egg : New egg (solution) to replace the actual one.
	void change_egg(std::vector<double> new_egg);
	//Method that return the fitness (quality) of the solution
	double get_fitness(void) const;
	//Method that return the solution vector.
	std::vector<double> get_solution(void) const;
	//Set the fitness of the egg
	//	double fitt -> fitness to be seted
	void set_fitness(double fitt);
	//Destructor of the class
	~CuckooEgg(void);
};
