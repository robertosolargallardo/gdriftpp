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

	//Class of data of each simulation
	class SimulationData{
	public:
		int simId;
		//estadisticos para medir distancia
		vector<double> stadistics; 
		// vector de parametros del modelo
		vector<double> params; 
		//distancia de estadisticos con el target
		double distancia;
		SimulationData(){
			distancia = 99999999.9;
		}
		~SimulationData(){}
	};
	
	//descripcion parametros
	vector<string> paramsDesc;
	
	// vector de objetos simulacion en caso de usar POO 
	vector<SimulationData> setSimulaciones;
	
	// estadisticos del target
	vector<double> targets; 
	
	// matrix of set of statistics of all the simulations para calculo de distancias 
	vector< vector<double> > setStatsSim; 
	
	//set of distances for each simulation, where <int> is the id in (setStatsSim and setSimulaciones)	
	vector<pair<int, double> > setDistancias; 
	
	// set of data for use a compute f.distribution 
	vector<pair<int, vector<double> > > setSample;
	
	// traspose matrix for extraction of vector[i] (originaly columns)
	vector< vector<double>  > setSampleTraspuesto;
	
	//Vector of objects of Function density for each params en caso de querer almacenar dist. a priori
	//vector< DensityFunction > setDistOriginal;
	
	//Vector of objects of Function density for each params de la seleccion de datos de la simulacion
	vector< DensityFunction > setDistPosterior;
	
	//Data-params for fase of training
	vector<pair<double, double> > medianVar;



	//NUEVAS VERSION 1.2 X.V2
	Ajuste ajustePosteriori;
	
	//Matriz <pair> with statistics normalized of the posteriori - Future use
	vector< pair<int,vector<double> > > setStatsSimNormalizadoPost;
	
	//Matriz with statistics normalized of the posteriori
	vector< vector<double> > matrizStatsNormalizadoPost;
	
	//Map with statistics normalized of the priori
	map<int,vector<double> > setStatsSimNormalizado;
		
	//vector target noramlized
	vector<double> setTargetsNormalizado;
	
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
		ajustePosteriori.cargaBuildDataAjuste(matrizStatsNormalizadoPost, matrizParamsPost, setTargetsNormalizado, distanciasModel);
		cout<<"SimulationStatistics::ajustarWLS - homoRegresion...\n";
		ajustePosteriori.homoRegresion();
		cout<<"SimulationStatistics::ajustarWLS - Fin\n";
		//ajustePosteriori.paramsAjustados[i]; forma de capturar info
		
	}

	//Add simulacion
	void addSimulation(SimulationData simIn){
		setSimulaciones.push_back(simIn);		
	}

	/*Alamacena target*/
	void storeTarget(vector<double> &dataIn){
		targets = dataIn;
	}

	/*Almacena en una matriz el set de estadisticos - creo que asi lo tienes victor*/
//	void storeSetSimulationStats(vector< vector<double> > &dataIn){
//		setStatsSim = dataIn;
//	}

	/*Almacena vector de distancias*/
	void loadData(vector< vector<double> > &dataStats, vector< vector<double> > &dataParams){
		cout<<"SimulationStatistics::loadData - Inicio (dataStats: "<<dataStats.size()<<", dataParams: "<<dataParams.size()<<")\n";
		if( dataStats.size() != dataParams.size() ){
			cerr<<"SimulationStatistics::loadData - Error, estadisticos y parametros no coinciden\n";
			exit(EXIT_FAILURE);
		}
		//Se almacena como matriz - creo que asi lo tienes o algo parecido
		setStatsSim = dataStats;
//		storeSetSimulationStats(dataStats);		
		//En lo que sigue se utilizan objetos, el proceso es tan rapido que puede que no necesite optimizacion
		//en el sentido de solo usar vectores y matrices
		//Se usan objetos, en caso de usar multiples simulaciones para distintos identificadores
		int contId = 0;
		size_t sizeStas = dataStats.size();
		size_t sizeStasCols = dataStats[0].size();
		size_t sizeParamsCols = dataParams[0].size();
		for(size_t i = 0; i < sizeStas; ++i){
			SimulationData simTmp;
			simTmp.simId = contId;
			for(size_t j = 0; j < sizeStasCols; ++j){
				//Como objetos
				// simTmp.agregarStadistics(dataStats[i][j]);
				simTmp.stadistics.push_back(dataStats[i][j]);
			}
			for(size_t k = 0; k < sizeParamsCols; ++k){ 
				// simTmp.agregarParams(dataParams[i][k]);	
				simTmp.params.push_back(dataParams[i][k]);
			}
			addSimulation(simTmp);
			++contId;
		}
	}

	/*Retorna vector de distancias*/
	vector<pair<int, double> > simulationDistances(){
		return setDistancias;
	}

	/*Calcula distancias <medida de distancia,normalizar (no=0, o si=1)>*/ 
	// Notar que este metodo puede recibir vectores con minimos y maximos globales para normalizar
	void computeDistances(int medidaDistancia, int opcionNormalizar, vector<double> min_stats = {}, vector<double> max_stats = {}){
		cout<<"SimulationStatistics::computeDistances - Inicio (medidaDistancia: "<<medidaDistancia<<", opcionNormalizar: "<<opcionNormalizar<<")\n";
		switch(opcionNormalizar){
			//Sin Normalizar	
			case 0:{
				/*Almacena vector de errores*/		
				vectorErrores(targets, setStatsSim, medidaDistancia, setDistancias);
				break;
			};
			//Con Normalizar   
			case 1:{
				//Para procesamiento normalizado
				vector< vector<double> > dataInSimStatsN;//Matriz normalizada de estadisticos
				vector<double>  dataInSimTargetN;//Target normalizado
//				vector<double> dataMaximos;//Maximos de cada estadistico
//				vector<double> dataMinimos;//Minimos de cada estadistico
				
				if(min_stats.size() == 0 || max_stats.size() == 0){
					Statistics::getMinMax(setStatsSim, min_stats, max_stats);
				}
				
				/*Normaliza matriz de estadisticos*/
				// El metod que sigue NO USA los min/max, solo los calcula
				// En el nuevo modelo, se calculan antes si es necesario
//				normalizedDataMatriz(setStatsSim, dataInSimStatsN);
				normalizeMatrix(setStatsSim, dataInSimStatsN, min_stats, max_stats);
				
				/*Normaliza target*/
//				normalizedDataLimits(targets, max_stats, min_stats, dataInSimTargetN);
				normalizeTarget(dataInSimTargetN, min_stats, max_stats);

				/*Almacena vector<pair<int,double>> de errores*/
				// Esto calcula las distancias reales (aka errores)
				vectorErrores(dataInSimTargetN, dataInSimStatsN, medidaDistancia, setDistancias);
				
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
		if(targets.size() != min_stats.size() || targets.size() != max_stats.size() ){
			cerr<<"SimulationStatistics::normalizeTarget - Error, min o max incorrectos\n";
			return;
		}
		data_out.resize(targets.size());
		for(unsigned int i = 0; i < targets.size(); ++i){
			data_out[i] = Statistics::getScaled( targets[i], min_stats[i], max_stats[i] );
		}
	}
	
	//NUEVO VERSION 1.2 X.V2
	//Almacenamiento de estadisticos y target normalizado - Se utiliza en el ajuste
	void almacenarDataNormalized(vector <vector<double> > dataInSimStatsNIn, vector<double> dataInSimTargetNIn){
		cout<<"SimulationStatistics::almacenarDataNormalized - Inicio\n";
		setTargetsNormalizado = dataInSimTargetNIn;
		size_t sizeStas  = dataInSimStatsNIn.size();
		int posSelect;
		for(size_t k = 0; k < sizeStas; k++){
			posSelect = (int)k;
			setStatsSimNormalizado.insert( pair<int, vector<double> > (posSelect, dataInSimStatsNIn[posSelect]));
		}
		cout<<"SimulationStatistics::almacenarDataNormalized - Fin\n";	
	}
	//FIN NUEVO

	//Ordenamiento de un vector <pair<int, double>> por *.second
	void sortDistances(){
		sort(setDistancias.begin(), setDistancias.end(), sort_pred());
	}

	// Se selecciona un % de la muestra de las top-dim distancias
	// Recibe 3 argumentos para fines estadisticos, escribe las min, max y media distancias consideradas
	// Retorna la distancia threshold (igual a max, ahora ese return es innecesario)
	unsigned int selectSample(double percentage, double &min, double &max, double &mean){
		cout<<"SimulationStatistics::selectSample - Inicio (percentage: "<<percentage<<")\n";
		unsigned int dim = (unsigned int) ( floor (setDistancias.size()*percentage) );
		// Deberia haber un minimo de datos para usar
		// Si el percentage es muy pequeño para el numero de datos, usar el minimo (o todos los datos)
		if( dim < MIN_SAMPLE ){
			dim = (MIN_SAMPLE < setDistancias.size())?MIN_SAMPLE:setDistancias.size();
		}
		cout<<"SimulationStatistics::selectSample - dim: "<<dim<<"\n";
		// size_t sizeStas  = setDistancias.size();
		sortDistances();
		int posSelect;
		mean = 0.0;
		min = setDistancias[0].second;
		max = setDistancias[dim-1].second;
		for(size_t k = 0; k < dim; ++k){
			posSelect = setDistancias[k].first;
			mean += setDistancias[k].second;
			// setSample.push_back( pair<int, vector<double> > (posSelect, setSimulaciones[posSelect].outParametros()));
			setSample.push_back( pair<int, vector<double> > (posSelect, setSimulaciones[posSelect].params));
			
			//NUEVO VERSION 1.2 - Se utiliza en el ajuste ***********************************************************
			// X.V2
			vector<double> tmp;
//			matrizParamsPost.push_back(setSimulaciones[posSelect].outParametros());
			matrizParamsPost.push_back(setSimulaciones[posSelect].params);
			tmp = setStatsSimNormalizado.find(posSelect)->second;
			setStatsSimNormalizadoPost.push_back( pair<int, vector<double> > (posSelect, tmp));
			matrizStatsNormalizadoPost.push_back(tmp);
			distanciasModel.push_back(setDistancias[k].second);
			//FIN NUEVO *********************************************************************************************
			
		}
		mean /= dim;
		cout<<"SimulationStatistics::selectSample - Fin (min: "<<min<<", max: "<<max<<", mean: "<<mean<<")\n";
		return dim;	
	}

	//Se almacena la traspuesta de la matriz setSample 
	void selectSampleTraspuesto(){
		unsigned int dimF = setSample.size();
		unsigned int dimC = setSample[0].second.size();
		for(size_t k=0;k<dimC;k++){
			vector<double> tmp;
			for(size_t q=0;q<dimF;q++){
				tmp.push_back(setSample[q].second[k]);
			}
			setSampleTraspuesto.push_back(tmp);
		}		
	}

	//Se almacena la distribucion posterior
	void distPosterior(int opcion){  		
		selectSampleTraspuesto();
		unsigned int dim = setSampleTraspuesto.size();
		for(size_t k=0;k<dim;k++){ 			
			//DistPosterior
			DensityFunction fden;
			fden.computeDensityFunction(setSampleTraspuesto[k],opcion);
			setDistPosterior.push_back(fden);
		}
	}

	//Add description
	void addParamDesc(string dataDesc){
		paramsDesc.push_back(dataDesc);		
	}

	//Add targets
	void addTargets(double dataIn){
		targets.push_back(dataIn);				
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
