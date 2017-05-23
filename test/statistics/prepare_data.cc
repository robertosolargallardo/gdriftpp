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

random_device seed;
mt19937 rng(seed());

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

	if(argc != 8){
		cout<<"\nModo de Uso: prepare_data results_file target_file n_stats n_params params_simple_base n_buckets params_pares_base\n";
		cout<<"\n";
		return 0;
	}
	
	string results_file = argv[1];
	string target_file = argv[2];
	unsigned int n_stats = atoi(argv[3]);
	unsigned int n_params = atoi(argv[4]);
	string params_simple_base = argv[5];
	unsigned int n_buckets = atoi(argv[6]);
	string params_pares_base = argv[7];
	unsigned int n_clases_dist = 5;
	
	cout<<"Inicio (\""<<results_file<<"\", \""<<target_file<<"\", n_stats: "<<n_stats<<", n_params: "<<n_params<<", \""<<params_simple_base<<"\")\n";
	
	// Cargar datos (full)
	// 0... ns-1	| ns... ns+np-1	| ns+np
	// stats		| params		| distancia
	cout<<"Cargando Datos\n";
	vector< vector<double> > data;
	ifstream lector(results_file, ifstream::in);
	if( ! lector.is_open() ){
		cout<<"Problemas al abrir archivo \""<<results_file<<"\"\n";
		return 1;
	}
	unsigned int buff_size = 1024*1024;
	char buff[buff_size];
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
			double value = 0.0;
			toks>>value;
			values.push_back(value);
		}
		
		// params
		for(unsigned int i = 0; i < n_params; ++i){
			double value = 0.0;
			toks>>value;
			values.push_back(value);
		}
		
//		cout<<"Data["<<data.size()<<"]: ";
//		for(unsigned int i = 0; i < values.size(); ++i){
//			cout<<values[i]<<" | ";
//		}
//		cout<<"\n";
		
		data.push_back(values);
		
//		if(data.size() >= 3)	break;
		
	}
	lector.close();
	
	// Calcular min/max por stat
	cout<<"Calcular min/max\n";
	vector<double> min_stats;
	vector<double> max_stats;
	for(unsigned int i = 0; i < n_stats + n_params; ++i){
		double min = 100000000;
		double max = 0;
		for(unsigned int j = 0; j < data.size(); ++j){
			if( data[j][i] < min ){
				min = data[j][i];
			}
			if( data[j][i] > max ){
				max = data[j][i];
			}
		}
		cout<<"Stat "<<i<<": ["<<min<<", "<<max<<"]\n";
		min_stats.push_back(min);
		max_stats.push_back(max);
	}
	
	// Normalizar datos
	cout<<"Normalizar datos\n";
	for(unsigned int i = 0; i < data.size(); ++i){
		for(unsigned int j = 0; j < n_stats; ++j){
			data[i][j] = ( data[i][j] - min_stats[j] )/(max_stats[j] - min_stats[j]);
		}
	}
	
	// Cargar target (normalizado)
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
			value = ( value - min_stats[i] )/(max_stats[i] - min_stats[i]);
			target.push_back( value );
		}
		
		cout<<"Target: ";
		for(unsigned int i = 0; i < target.size(); ++i){
			cout<<target[i]<<" | ";
		}
		cout<<"\n";
	
	}
	
	// Evaluar distancias ya agregar a datos
	cout<<"Evaluando distancias\n";
	double min_dist = 100000000;
	double max_dist = 0;
	double mean_dist = 0;
	for(unsigned int i = 0; i < data.size(); ++i){
		double d = distance(data[i], target);
		
		if(d < min_dist){
			min_dist = d;
		}
		if(d > max_dist){
			max_dist = d;
		}
		mean_dist += d;
		
		data[i].push_back(d);
	}
	mean_dist /= data.size();
	cout<<"Min dist: "<<min_dist<<", Max dist: "<<max_dist<<", Mean dist: "<<mean_dist<<"\n";
	
	// Guardar salida full
	cout<<"Guardar salida full\n";
	
	// Iterar por parametro
		// Generar salida del parametro
	cout<<"Guardar salida por Parametro\n";
	for(unsigned int i = 0; i < n_params; ++i){
		// Posicion del parametro: ns + i
		// Posicion de distancia: ns + np
		unsigned int pos = n_stats + i;
		cout<<"Procesando parametro "<<i<<"\n";
		PositionalComparator comp(pos);
		sort(data.begin(), data.end(), comp);
		
		sprintf(buff, "%s_%d.txt", params_simple_base.c_str(), i);
		ofstream escritor(buff, fstream::trunc | fstream::out);
		
//		for(unsigned int j = 0; j < 10; ++j){
//			cout<<"Data["<<j<<"]: ";
//			for(unsigned int k = 0; k < n_params; ++k){
//				cout<<(data[j][k+n_stats])<<" | ";
//			}
//			cout<<"dist: "<<data[j][n_stats + n_params]<<"\n";
//		}
		
		double step = (max_stats[pos] - min_stats[pos]) / n_buckets;
		cout<<"Step: "<<step<<" (hasta "<<(n_buckets*step + min_stats[pos])<<" por rango ["<<min_stats[pos]<<", "<<max_stats[pos]<<"])\n";
		unsigned int cur_data = 0;
		for(unsigned int j = 0; j < n_buckets; ++j){
			double d = 0.0;
			unsigned int total = 0;
			for( ; cur_data < data.size() && data[cur_data][pos] <= ((j+1)*step + min_stats[pos]); ++cur_data ){
				d += data[cur_data][n_stats + n_params];
				++total;
			}
			if(total > 0){
				d /= total;
				escritor<<((j+1)*step + min_stats[pos])<<"\t"<<d<<"\n";
			}
			else{
				d = max_dist;
			}
//			cout<<"dist["<<j<<"]: "<<d<<" (total: "<<total<<", rango ["<<j*step<<", "<<(j+1)*step<<"])\n";
		}
		cout<<"Total de datos evaluados: "<<cur_data<<"\n";
		cout<<"-----     -----\n";
		escritor.close();
		
		for(unsigned int j = i+1; j < n_params; ++j){
			cout<<"Par ("<<i<<", "<<j<<")\n";
			
			sprintf(buff, "%s_%d_x_%d.txt", params_pares_base.c_str(), i, j);
			ofstream escritor(buff, fstream::trunc | fstream::out);
			for(unsigned int k = 0; k < data.size(); ++k){
				unsigned int clase = (unsigned int)(data[k][n_stats + n_params]*n_clases_dist/max_dist);
				escritor<<(data[k][n_stats+i])<<"\t"<<(data[k][n_stats+j])<<"\t"<<clase<<"\n";
			}
			escritor.close();
		}
		
	}
	
	return 0;
}





















