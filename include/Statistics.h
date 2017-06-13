#ifndef _STATISTICS_H_
#define _STATISTICS_H_

#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <vector>

// sqrt root of 2 pi
#define STAT_SQR2PI 2.506628274631000
// number of points to generate graphs when needed
#define STAT_NPOINTS 30

enum DistributionType{ UNKNOWN = 0, UNIFORM = 1, NORMAL = 2 };

using namespace std;

class Distribution{
private:
	DistributionType type;
	vector<double> values;
	
public:
	Distribution(){
		type = UNKNOWN;
	}
	// Por ahora preparo un constructor para dos valores especificamente
	// Notar que estoy asumiendo que los parametros tienen un orden obvio y natural (o al menos, bien documentado)
	// La forma ideal seria con herencia por distribucion, pero en este caso lo dejo asi por eficiencia
	Distribution(DistributionType _type, double value1, double value2){
		type = _type;
		values.push_back(value1);
		values.push_back(value2);
	}
	Distribution(DistributionType _type, vector<double> &_values){
		type = _type;
		values.insert(values.begin(), _values.begin(), _values.end());
	}
	Distribution(string _type, double value1, double value2){
		if( _type.compare("uniform") == 0 ){
			type = UNIFORM;
		}
		else if( _type.compare("normal") == 0 ){
			type = NORMAL;
		}
		else{
			cerr<<"Distribution - Unknown distribution \""<<_type<<"\".\n";
			type = UNKNOWN;
		}
		values.push_back(value1);
		values.push_back(value2);
	}
	
	DistributionType getType(){
		return type;
	}
	
	double getValue(unsigned int pos){
		if(pos < values.size()){
			return values[pos];
		}
		else{
			return 0.0;
		}
	}
	
	unsigned int getNumValue(){
		return values.size();
	}
	
	// Retorna un valor minimo razonable para graficar la distribucion
	// Notar que esto NO IMPLICA que no puedan obtenerse valores menores, es un minimo RAZONABLE
	double getMinValue(){
		double res = 0.0;
		switch( type ){
			case UNIFORM : {
				res = getValue(0);
				break;
			}
			case NORMAL : {
				// Estoy usando +- 3 stddev como valor razonable
				res = getValue(0) - 3*getValue(1);
				break;
			}
			case UNKNOWN : {
				cerr<<"Statistics::generateDistributionGraph - Unknown distribution.\n";
				break;
			}
		}
		return res;
	}
	
	// Retorna un valor maximo razonable para graficar la distribucion
	// Notar que esto NO IMPLICA que no puedan obtenerse valores mayores, es un maximo RAZONABLE
	double getMaxValue(){
		double res = 0.0;
		switch( type ){
			case UNIFORM : {
				res = getValue(1);
				break;
			}
			case NORMAL : {
				// Estoy usando +- 3 stddev como valor razonable
				res = getValue(0) + 3*getValue(1);
				break;
			}
			case UNKNOWN : {
				cerr<<"Statistics::generateDistributionGraph - Unknown distribution.\n";
				break;
			}
		}
		return res;
	}
};

class Statistics{
public:
	
	static vector<pair<double, double>> generateDistributionGraph(Distribution &dist, double start, double finish, double min_scale, double max_scale){
		vector<pair<double, double>> res;
		switch( dist.getType() ){
			case UNIFORM : {
				// Definir los valores localmente?
				// En este caso es mas simple, basta con dos puntos, con y escalado
				// y = 1.0 / (max_scaled - min_scaled)
				double scaled_min = (start - min_scale) / (max_scale - min_scale);
				double scaled_max = (finish - min_scale) / (max_scale - min_scale);
				double y = 1.0 / (scaled_max - scaled_min);
				res.push_back( pair<double, double>(start, y) );
				res.push_back( pair<double, double>(finish, y) );
				break;
			}
			case NORMAL : {
				graphNormalDistribution(res, dist.getValue(0), dist.getValue(1), start, finish, min_scale, max_scale);
				break;
			}
			case UNKNOWN : {
				cerr<<"Statistics::generateDistributionGraph - Unknown distribution.\n";
				break;
			}
		}
		return res;
	} 
	
	static void graphNormalDistribution(vector<pair<double, double>> &res, double mean, double stddev, double start, double finish, double min_scale, double max_scale){
	
		// Generar datos interpolados en el rango (real)
		vector<double> data;
		double step = (finish-start)/STAT_NPOINTS;
		double next = start;
		for(unsigned int i = 0; i < STAT_NPOINTS; ++i){
			data.push_back(next);
			next += step;
		}
	
		// Normalizar media y std (notar que std NO debe ser trasladada, solo escalada)
		double mean_scaled = (mean - min_scale)/(max_scale - min_scale);
		double stddev_scaled = stddev/(max_scale - min_scale);
	
		// Iterar por los datos, normalizar, y aplicar kernel directo
		for(unsigned int i = 0; i < data.size(); ++i){
			double x = data[i];
			double x_scaled = (x - min_scale)/(max_scale - min_scale);
			double y = directNormalKernel(x_scaled, mean_scaled, stddev_scaled);
			res.push_back(pair<double, double>(x, y));
		}
		
	}
	
	static double directNormalKernel(double x, double mean, double stddev){
		double res = exp( - ( pow((x-mean), 2.0) ) / ( 2.0*stddev*stddev ) ) / (STAT_SQR2PI * stddev);
		return res;
	}

};











#endif
