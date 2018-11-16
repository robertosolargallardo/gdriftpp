#include <random>
#include <vector>
#include <math.h>
#include <iostream>

class Cuckoo {

private:
	//Parameter for the levy flight.
	double beta;
	//Parameter for the levy flight.
	double sigma_u;
	//Parameter for the levy flight.
	double sigma_v;
	//Random number generator for the distribution u in the levy fligh.
	std::uniform_real_distribution<double> distribution_u;
	//Random number generator for the distribution v in the levy fligh.
	std::uniform_real_distribution<double> distribution_v;
	//Random number generator for the direction in the levy flight.
	std::uniform_real_distribution<double> direction;
	//Vector that represent the egg (solution) of the cuckoo.
	std::vector<double> solution;
	std::vector<double> min;
	std::vector<double> max;

public:		
	//Empy constructor.
	Cuckoo(void);
	//Constructor of the class.
	//	double beta : Parameter of the levy flight.
	//	int number_of_parameters : Number of the parameters in the problem.
	Cuckoo(const double &beta, const int &number_of_parameters, const std::vector<double> min, std::vector<double> max);
	//Method that return the number value of the levy flight.
	//	double step_size : The actual size ot the levy flight.
	double levy_flight(const double &step_size);
	//Method that return a cuckoo egg (new solution)
	//	vector nest_egg : egg from the nest (its used to generate the new solution).
	//	double step_size : The actual size ot the levy flight.
	std::vector<double> get_cuckoo(const std::vector<double> &nest_egg);
	//Method thar return the direction (+1 or -1)
	double sign(void);
	//Destructor of the class
	~Cuckoo(void);
};
