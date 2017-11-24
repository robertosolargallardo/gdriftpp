#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include <iostream>

#include <map>
#include <vector>
#include <list>

#include "DBCommunication.h"

using namespace std;

double distance(vector<double> stats, vector<double> target){
	double d = 0.0;
	for(unsigned int i = 0; i < target.size(); ++i){
		d += pow( stats[i] - target[i], 2.0 );
	}
	d = pow(d, 0.5);
	return d;
}

class PositionalComparator : public std::binary_function<unsigned int, unsigned int, bool> {
private:
	unsigned int pos;
public:
	PositionalComparator(unsigned char _pos = 0){
		pos = _pos;
	}
	inline bool operator()(const vector<double> &a, const vector<double> &b){
		return (a[pos] < b[pos]);
	}
};


int main(int argc, char** argv){

	if(argc != 6){
		cout<<"\nModo de Uso: prepare_data results_file target_file n_stats n_params output\n";
		cout<<"\n";
		return 0;
	}
	
	string results_file = argv[1];
	string target_file = argv[2];
	unsigned int n_stats = atoi(argv[3]);
	unsigned int n_params = atoi(argv[4]);
	string output_file = argv[5];
	
	cout<<"Inicio (\""<<results_file<<"\", \""<<target_file<<"\", n_stats: "<<n_stats<<", n_params: "<<n_params<<", \""<<output_file<<"\")\n";
	
	cout<<"Cargando Datos\n";
	vector< vector<double> > data;
	ifstream lector(results_file, ifstream::in);
	if( ! lector.is_open() ){
		cout<<"Problemas al abrir archivo \""<<results_file<<"\"\n";
		return 1;
	}
	unsigned int buff_size = 1024*1024;
	char buff[buff_size];
	double value = 0;
	memset(buff, 0, buff_size);
	while( lector.good() ){
		lector.getline(buff, buff_size);
		unsigned int lectura = lector.gcount();
		if( lectura <= 1 || strlen(buff) <= 1 ){
			break;
		}
		
		//Linea valida de largo > 0
		string line(buff);
		stringstream toks(line);
		
		// id
		unsigned int id = 0;
		toks>>id;
		
		vector<double> values;
		
		// stats
		for(unsigned int i = 0; i < n_stats; ++i){
			value = 0.0;
			toks>>value;
			values.push_back(value);
		}
		
		// params
		for(unsigned int i = 0; i < n_params; ++i){
			value = 0.0;
			toks>>value;
			values.push_back(value);
		}
		
		// timestamp
		value = 0.0;
		toks>>value;
		values.push_back(value);
		
		data.push_back(values);
		
	}
	lector.close();
	
	// Cargar target (normalizo despues)
	cout<<"Cargando target\n";
	vector<double> target;
	lector.open(target_file, ifstream::in);
	if( ! lector.is_open() ){
		cout<<"Problemas al abrir archivo \""<<target_file<<"\"\n";
		return 1;
	}
	else{
		lector.getline(buff, buff_size);
		
		//Linea valida de largo > 0
		string line(buff);
		stringstream toks(line);
		
		// stats
		for(unsigned int i = 0; i < n_stats; ++i){
			double value = 0.0;
			toks>>value;
			target.push_back( value );
		}
		
		cout<<"Target: ";
		for(unsigned int i = 0; i < target.size(); ++i){
			cout<<target[i]<<" | ";
		}
		cout<<"\n";
	
	}
	lector.close();
	
	// Calcular min/max por stat
	cout<<"Calcular min/max\n";
	vector<double> min_stats;
	vector<double> max_stats;
	
	// Inicio con valores del target
	for(unsigned int i = 0; i < n_stats; ++i){
		min_stats.push_back( target[i] );
		max_stats.push_back( target[i] );
	}
	
	for(unsigned int i = 0; i < data.size(); ++i){
		for(unsigned int j = 0; j < n_stats; ++j){
			if( data[i][j] < min_stats[j] ){
				min_stats[j] = data[i][j];
			}
			if( data[i][j] > max_stats[j] ){
				max_stats[j] = data[i][j];
			}
		}
	}
	
	cout<<"Minimos y maximos\n";
	for(unsigned int i = 0; i < n_stats; ++i){
		cout<<"Stat "<<i<<": ["<<min_stats[i]<<", "<<max_stats[i]<<"]\n";
	}
	
	// Normalizar target
	cout<<"Normalizando target\n";
	for(unsigned int j = 0; j < n_stats; ++j){
		target[j] = ( target[j] - min_stats[j] )/(max_stats[j] - min_stats[j]);
	}
	
	// Normalizar datos
	cout<<"Normalizando datos\n";
	for(unsigned int i = 0; i < data.size(); ++i){
		for(unsigned int j = 0; j < n_stats; ++j){
			data[i][j] = ( data[i][j] - min_stats[j] )/(max_stats[j] - min_stats[j]);
		}
	}
	vector<double> dists;
	for(unsigned int i = 0; i < data.size(); ++i){
		dists.push_back( distance(data[i], target) );
	}
	
	ofstream escritor(output_file, fstream::trunc | fstream::out);
	for(unsigned int i = 0; i < data.size(); ++i){
		escritor<<i<<"\t"<<dists[i]<<"\t"<<data[i].back()<<"\n";
	}
	escritor.close();
	
	
	return 0;
}





















