#include "Cuckoo.h"

Cuckoo::Cuckoo(void){
	;
}

Cuckoo::Cuckoo(const double &beta, const int &number_of_parameters, const std::vector<double> min, std::vector<double> max)
{
	//Each attribute of the class is initialized and the memory is reserved for each vector.
	this->beta = beta;
	//Sigma u is calculated (see levy flight method for more info)
	this->sigma_u = pow((tgamma(1+beta)*sin(M_PI*beta/2))/(tgamma((1+beta)/2)*beta*pow(2,(beta-1)/2)),1/beta);
	this->sigma_v = 1;
	std::uniform_real_distribution<double> distribution_u(0.0,this->sigma_u);
	this->distribution_u = distribution_u;
	std::uniform_real_distribution<double> distribution_v(0.0,this->sigma_v);
	this->distribution_v = distribution_v;
	std::uniform_real_distribution<double> direction(0.0,1.0);
	this->direction = direction;
	this->solution.resize(number_of_parameters);
	this->min = min;
	this->max = max;
}

/*
TRICK : Here the factor step_size expect the value 0.01 from the fact that L/100 should the typical 
step size of walks/flights where L is the typical lenghtscale; otherwise, Levy flights may 
become too aggresive/efficient, which makes new solutions (even) jump out side of the design domain 
(and thus wasting evaluations).
*/

double Cuckoo::levy_flight(const double &step_size)
{
	/*
		This levy flights implementation correspond to the Mantenga's algorithm for the step.
		u and v are the parameters for the equation that describe the step lenght s
			s = u/(|v|^(1/Betta))
		where u and v are drawn from normal distributions, that means
			u ~ N(0,sigma_u^2) and v ~ N(0,sigma_v^2)
		whit sigma_v = 1 and sigma_u given by the next equation
			sigma_u = {Gamma(1+Betta)*Sin(Pi*Betta/2)/Gamma[(1+Betta)/2]*Betta*2^(Betta-1)/2}^1/Betta
		Where Gamma is the gamma function and Betta is a parameter of the algorithm 
	*/

	//Get a random number for u and v. Then calculate the step lenght s.
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 generator(rd()); //Standard mersenne_twister_engine seeded with rd()
	double u = this->distribution_u(generator); 
	double v = this->distribution_v(generator);
	double s = u/pow(fabs(v),1/this->beta); 
	//Return the step lenght (s).
	return step_size*s;
}

std::vector<double> Cuckoo::get_cuckoo(const std::vector<double> &nest_egg_solution)
{
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 generator(rd()); //Standard mersenne_twister_engine seeded with rd()
    
	for(unsigned int i=0;i<nest_egg_solution.size();i++)
	{
		////Perform of the levy flight and direction of the levy flight
		//double step_size = pow(10,int(log10(nest_egg_solution[i]))-1);
		double step_size = 0.1;
		double levy_flight_perform_step = this->levy_flight(step_size);
		double direction = this->sign();
		//The new solution is given by X(t+1) = X(t) + alfa * Levy flight (alfa * Levy fligt is levy_flight_perform_step)
		this->solution[i] = nest_egg_solution[i] + (levy_flight_perform_step * direction);
		//Aplication of the bounds if needed
		if(this->solution[i] <= min[i])
		{
			this->solution[i] = min[i];
		}
		else if(this->solution[i] >= max[i])
		{			
			this->solution[i] = max[i];
		}
	}
	return this->solution;
}

double Cuckoo::sign(void)
{
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 generator(rd()); //Standard mersenne_twister_engine seeded with rd()
    
    double value = this->direction(generator);
    if(value > 0.5)
    {
    	return 1.0;
    }
    else
    {
    	return -1.0;
    }
}

Cuckoo::~Cuckoo(void)
{
	;
}
