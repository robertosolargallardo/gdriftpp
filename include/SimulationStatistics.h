#if !defined(_SIMULATION_STATISTICS_H)
#define _SIMULATION_STATISTICS_H

//Libs C
#include <stdio.h>
#include <math.h>	  
//Libs STL
#include <algorithm>
#include <iostream>
#include <list>
#include <vector>
#include <set>
#include <map>

#include "Statistics.h"
#include "DensityFunction.h"
#include "ajuste.h"

#define MIN_SAMPLE 50

using namespace std;

//Class of set of simulations
class SimulationStatistics{
private:
	
	//descripcion parametros
	vector<string> paramsDesc;
	
	// estadisticos del target
	vector<double> target; 
	
	// Matrix of statistics, it has N simulations (rows) per S statistics (columns) 
	vector< vector<double> > data_stats;
	
	// Matrix of parameters, it has N simulations (rows) per P parameters (columms)
	vector< vector<double> > data_params;
	
	// Number of stats and parameters (from the first element in data), also the number of simulations
	unsigned int n_stats;
	unsigned int n_params;
	unsigned int n_data;
	
	// Vector of computed distances where first is the position in data, second is the distance
	// This is because they will be sorted by distance but we need to access the data later	
	vector<pair<int, double> > distances; 
	
	// set of data for use a compute f.distribution 
	vector<pair<int, vector<double> > > sample_params;
	
	//Vector of objects of Function density for each params de la seleccion de datos de la simulacion
	vector< DensityFunction > setDistPosterior;



	//NUEVAS VERSION 1.2 X.V2
	Ajuste ajustePosteriori;
	
	//Matriz <pair> with statistics normalized of the posteriori - Future use
	vector< pair<int,vector<double> > > dataStatsNormalizadoPost;
	
	//Matriz with statistics normalized of the posteriori
	vector< vector<double> > matrizStatsNormalizadoPost;
	
	//Map with statistics normalized of the priori
	map<int, vector<double> > dataStatsNormalizado;
		
	//vector target noramlized
	vector<double> setTargetNormalizado;
	
	 //Matriz with statistics normalized of the posteriori
	vector< vector<double> > matrizParamsPost;
	
	//Distancias
	vector<double> distanciasModel;
	
	//FIN VARIABLES NUEVAS
	
	
public:

	SimulationStatistics(){}

	//Falta construir destructores
	~SimulationStatistics(){}

	//NUEVA VERSION 1.2 X.V2
	void ajustarWLS(){
		//Carga stats(normalizados), params y target(normalizado)
		cout<<"SimulationStatistics::ajustarWLS - cargaBuildDataAjuste...\n";
		ajustePosteriori.cargaBuildDataAjuste(matrizStatsNormalizadoPost, matrizParamsPost, setTargetNormalizado, distanciasModel);
		cout<<"SimulationStatistics::ajustarWLS - homoRegresion...\n";
		ajustePosteriori.homoRegresion();
		cout<<"SimulationStatistics::ajustarWLS - Fin\n";
		//ajustePosteriori.paramsAjustados[i]; forma de capturar info
		
	}

	/*Almacena vector de distancias*/
	void loadData(vector< vector<double> > &_data_stats, vector< vector<double> > &_data_params, vector<double> &_target){
		cout<<"SimulationStatistics::loadData - Inicio (_data_stats: "<<_data_stats.size()<<", _data_params: "<<_data_params.size()<<")\n";
		if( (_data_stats.size() == 0) || (_data_stats.size() != _data_params.size()) ){
			cerr<<"SimulationStatistics::loadData - Error, datos no coinciden\n";
			exit(EXIT_FAILURE);
		}
		//Se almacena como matriz - creo que asi lo tienes o algo parecido
		data_stats = _data_stats;
		data_params = _data_params;
		target = _target;
		
		n_data = data_stats.size();
		n_stats = data_stats[0].size();
		n_params = data_params[0].size();
		
	}

	/*Calcula distancias <medida de distancia,normalizar (no=0, o si=1)>*/ 
	// Notar que este metodo puede recibir vectores con minimos y maximos globales para normalizar
	void computeDistances(int medidaDistancia, int opcionNormalizar, vector<double> min_stats = {}, vector<double> max_stats = {}){
		cout<<"SimulationStatistics::computeDistances - Inicio (medidaDistancia: "<<medidaDistancia<<", opcionNormalizar: "<<opcionNormalizar<<")\n";
		switch(opcionNormalizar){
			//Sin Normalizar	
			case 0:{
				/*Almacena vector de errores*/		
				vectorErrores(target, data_stats, medidaDistancia, distances);
				break;
			};
			//Con Normalizar   
			case 1:{
				//Para procesamiento normalizado
				vector< vector<double> > dataInSimStatsN;//Matriz normalizada de estadisticos
				vector<double>  dataInSimTargetN;//Target normalizado
				
				if(min_stats.size() == 0 || max_stats.size() == 0){
					Statistics::getMinMax(data_stats, min_stats, max_stats);
				}
				
				/*Normaliza matriz de estadisticos*/
				// El metod que sigue NO USA los min/max, solo los calcula
				// En el nuevo modelo, se calculan antes si es necesario
				normalizeMatrix(data_stats, dataInSimStatsN, min_stats, max_stats);
				
				/*Normaliza target*/
//				normalizedDataLimits(target, max_stats, min_stats, dataInSimTargetN);
				normalizeTarget(dataInSimTargetN, min_stats, max_stats);

				/*Almacena vector<pair<int,double>> de errores*/
				// Esto calcula las distancias reales (aka errores)
				vectorErrores(dataInSimTargetN, dataInSimStatsN, medidaDistancia, distances);
				
				//NUEVO VERSION 1.2 X.V2
				//Almacenamiento de datos normalizados para ajuste de distribucion posterior
				almacenarDataNormalized(dataInSimStatsN, dataInSimTargetN);
				
				break;
			};
			default:{
				cout<<"SimulationStatistics::computeDistances - Opcion invalida ("<<opcionNormalizar<<", 0: sin normalizar, 1: normalizar)\n";
			};
		}
	}
	
	// Normaliza una matriz de entrada usando minimos y maximos POR COLUMNA
	void normalizeMatrix(vector<vector<double>> &data_in, vector<vector<double>> &data_out, vector<double> &min, vector<double> &max){
		// Reservo espacio en la matriz de salida
		unsigned int n_fils = data_in.size();
		unsigned int n_cols = data_in[0].size();
		if( min.size() != n_cols || max.size() != n_cols ){
			cerr<<"SimulationStatistics::normalizeMatrix - Error, min o max incorrectos ("<<n_cols<<", "<<min.size()<<", "<<max.size()<<").\n";
			return;
		}
		data_out.resize(n_fils);
		for(unsigned int i = 0; i < n_fils; ++i){
			data_out[i].resize(n_cols);
		}
		for(unsigned int i = 0; i < n_fils; ++i){
			for(unsigned int j = 0; j < n_cols; ++j){
				data_out[i][j] = Statistics::getScaled(data_in[i][j], min[j], max[j]);
			}
		}
	}
	
	void normalizeTarget(vector<double> &data_out, vector<double> &min_stats, vector<double> &max_stats){
		if(target.size() != min_stats.size() || target.size() != max_stats.size() ){
			cerr<<"SimulationStatistics::normalizeTarget - Error, min o max incorrectos\n";
			return;
		}
		data_out.resize(target.size());
		for(unsigned int i = 0; i < target.size(); ++i){
			data_out[i] = Statistics::getScaled( target[i], min_stats[i], max_stats[i] );
		}
	}
	
	//NUEVO VERSION 1.2 X.V2
	//Almacenamiento de estadisticos y target normalizado - Se utiliza en el ajuste
	void almacenarDataNormalized(vector <vector<double> > &dataInSimStatsNIn, vector<double> &dataInSimTargetNIn){
		cout<<"SimulationStatistics::almacenarDataNormalized - Inicio\n";
		setTargetNormalizado = dataInSimTargetNIn;
		size_t sizeStas  = dataInSimStatsNIn.size();
		int posSelect;
		for(size_t k = 0; k < sizeStas; k++){
			posSelect = (int)k;
			dataStatsNormalizado.insert( pair<int, vector<double> > (posSelect, dataInSimStatsNIn[posSelect]));
		}
		cout<<"SimulationStatistics::almacenarDataNormalized - Fin\n";	
	}
	//FIN NUEVO

	// Se selecciona un % de la muestra de las top-dim distancias
	// Recibe 3 argumentos para fines estadisticos, escribe las min, max y media distancias consideradas
	// Retorna la distancia threshold (igual a max, ahora ese return es innecesario)
	unsigned int selectSample(double percentage, double &min, double &max, double &mean){
		cout<<"SimulationStatistics::selectSample - Inicio (percentage: "<<percentage<<")\n";
		unsigned int dim = (unsigned int)(percentage * distances.size());
		// Deberia haber un minimo de datos para usar
		// Si el percentage es muy pequeño para el numero de datos, usar el minimo (o todos los datos)
		if( dim < MIN_SAMPLE ){
			dim = (MIN_SAMPLE < distances.size())?MIN_SAMPLE:distances.size();
		}
		cout<<"SimulationStatistics::selectSample - dim: "<<dim<<"\n";
		sort(distances.begin(), distances.end(), sort_pred());
		int posSelect;
		mean = 0.0;
		min = distances[0].second;
		max = distances[dim-1].second;
		for(size_t k = 0; k < dim; ++k){
			posSelect = distances[k].first;
			mean += distances[k].second;
			sample_params.push_back( pair<int, vector<double> > (posSelect, data_params[posSelect]));
			
			//NUEVO VERSION 1.2 - Se utiliza en el ajuste ***********************************************************
			// X.V2
			vector<double> tmp;
			matrizParamsPost.push_back(data_params[posSelect]);
			tmp = dataStatsNormalizado.find(posSelect)->second;
			dataStatsNormalizadoPost.push_back( pair<int, vector<double> > (posSelect, tmp));
			matrizStatsNormalizadoPost.push_back(tmp);
			distanciasModel.push_back(distances[k].second);
			//FIN NUEVO *********************************************************************************************
			
		}
		mean /= dim;
		cout<<"SimulationStatistics::selectSample - Fin (min: "<<min<<", max: "<<max<<", mean: "<<mean<<")\n";
		return dim;	
	}

	// Se almacena la distribucion posterior
	void distPosterior(int opcion){
		vector<double> sample_param;
		for(size_t p = 0; p < n_params; ++p){
			// DistPosterior
			DensityFunction fden;
			// Tomo los datos de sample para este parametro
			for(unsigned int i = 0; i < sample_params.size(); ++i){
				sample_param.push_back(sample_params[i].second[p]);
			}
			fden.computeDensityFunction(sample_param, opcion);
			setDistPosterior.push_back(fden);
			sample_param.clear();
		}
	}
	
	DensityFunction &getDistribution(unsigned int pos){
		return setDistPosterior[pos];
	}
	
	Ajuste getAdjustment(){
		return ajustePosteriori;
	}
	
	vector<double> &getAdjustmentData(unsigned int pos){
		return ajustePosteriori.paramsAjustados[pos];
	}
	
};














#endif
