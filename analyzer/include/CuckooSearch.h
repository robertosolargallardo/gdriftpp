#include <vector>
#include <fstream>

#include "Cuckoo.h"
#include "CuckooNest.h"

class CuckooSearch {

	private:
		//Number of pararameters in the problem
		int number_of_parameters;
		//Number of eggs (Solutions)
		int number_of_eggs;
		//Parameter for the levy flight (1.5 default)
		double beta;
		//Percentage of eggs to destroy (0.25 default)
		double pa;
		//Vector that alocates the new eggs generated wirh levy flight
		std::vector<std::vector<double> > cuckoo_eggs;
		//vector that alocates the cuckoo eggs after simulate them
		std::vector<CuckooEgg> new_eggs;
		//Nest and Cuckoo objects
		CuckooNest nst;
		Cuckoo cko;
		//Vector that alocates the minimum and the maximum of each parameter at position i
		std::vector<double> min;
		std::vector<double> max;

	public:
		//Constructor of the class
		//	int number_of_parameters -> Number of parameters in the problem
		//	int number_of_eggs -> Number of eggs
		//	double beta -> Parameter for the levy flight (1.5 default)
		//	double pa -> Percentage of eggs to destroy (0.25 default)
		//	vector min -> Each position of the vector contains the upper bound of the i parameter.
		//	vector max -> Each position of the vector contains the lower bound of the i parameter.
		CuckooSearch(const int number_of_parameters, const int number_of_eggs, const std::vector<double> min, const std::vector<double> max);
		CuckooSearch(const int number_of_parameters, const int number_of_eggs, const double beta, const double pa, const std::vector<double> min, const std::vector<double> max);
		//Method that needs to be executed first
		//	vector fitt -> Vector of fitness of each egg i
		//	vector info -> Solution of each the egg i
		std::vector<std::vector<double> > first_step(const std::vector<double> fitt, const std::vector<std::vector<double> > info);
		//Method that can be executed after first step
		//	vector fitt -> Vector of fitness of each egg i
		//	vector info -> Solution of each the egg i
		//	vector pa_fitt -> fittnes of the egg i to be changed
		//	vector pa_eggs -> solution of egg i to be changed
		std::vector<std::vector<double> > any_step(const std::vector<double> fitt, const std::vector<std::vector<double> > info, const std::vector<double> pa_fitt, const std::vector<std::vector<double> > pa_eggs);
		//Method that update the eggs 
		//	vector fitt -> Vector of fitness of each egg i
		//	vector info -> Solution of each the egg i
		void get_new_eggs(const std::vector<double> fitt, const std::vector<std::vector<double> > info);
		~CuckooSearch(void);

};
