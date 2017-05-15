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
//Libs extras
#include "densityProbability.h"
#define END_OF_STREAM 999999999

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
	//vector< FuncionDensidad  > setDistOriginal;
	
	//Vector of objects of Function density for each params de la seleccion de datos de la simulacion
	vector< FuncionDensidad  > setDistPosterior;
	
	//Data-params for fase of training
	vector<pair<double, double> > medianVar;

public:

	SimulationStatistics(){}

	//Falta construir destructores
	~SimulationStatistics(){}

	//Add simulacion
	void addSimulation(SimulationData simIn){
		setSimulaciones.push_back(simIn);		
	}

	/*Alamacena target*/
	void storeTarget(vector<double> &dataIn){
		targets = dataIn;
	}

	/*Almacena en una matriz el set de estadisticos - creo que asi lo tienes victor*/
	void storeSetSimulationStats(vector< vector<double> > &dataIn){
		setStatsSim = dataIn;
	}

	/*Almacena vector de distancias*/
	void loadData(vector< vector<double> > &dataStats, vector< vector<double> > &dataParams){
		cout<<"SimulationStatistics::loadData - Inicio (dataStats: "<<dataStats.size()<<", dataParams: "<<dataParams.size()<<")\n";
		if( dataStats.size() != dataParams.size() ){
			cerr<<"SimulationStatistics::loadData - Error, estadisticos y parametros no coinciden\n";
			exit(EXIT_FAILURE);
		}
		//Se almacena como matriz - creo que asi lo tienes o algo parecido
		storeSetSimulationStats(dataStats);		
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
	void computeDistances(int medidaDistancia, int opcionNormalizar){
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
				vector<double> dataMaximos;//Maximos de cada estadistico
				vector<double> dataMinimos;//Minimos de cada estadistico
				/*Normaliza matriz de estadisticos*/
				normalizedDataMatriz(setStatsSim, dataMaximos, dataMinimos, dataInSimStatsN);
				/*Normaliza target*/
				normalizedDataLimits(targets, dataMaximos, dataMinimos, dataInSimTargetN);
				/*Almacena vector<pair<int,double>> de errores*/
				vectorErrores(dataInSimTargetN, dataInSimStatsN, medidaDistancia, setDistancias);
				break;
			};
			default:{
				cout<<"SimulationStatistics::computeDistances - Opcion invalida ("<<opcionNormalizar<<", 0: sin normalizar, 1: normalizar)\n";
			};
		}
	}

	//Ordenamiento de un vector <pair<int, double>> por *.second
	void sortDistances(){
		sort(setDistancias.begin(), setDistancias.end(), sort_pred());
	}

	//Se selecciona un % de la muestra de las top-dim distancias  
	double selectSample(double percentage){
		cout<<"SimulationStatistics::selectSample - Inicio (percentage: "<<percentage<<")\n";
		unsigned int dim = (unsigned int) ( floor (setDistancias.size()*percentage) );
		cout<<"SimulationStatistics::selectSample - dim: "<<dim<<"\n";
		// size_t sizeStas  = setDistancias.size();
		sortDistances();
		int posSelect;
		double threshold = 0;
		for(size_t k = 0; k < dim; ++k){ 
			posSelect = setDistancias[k].first;
			threshold = setDistancias[k].second;
			cout<<"SimulationStatistics::selectSample - threshold: "<<threshold<<"\n";
			// setSample.push_back( pair<int, vector<double> > (posSelect, setSimulaciones[posSelect].outParametros()));
			setSample.push_back( pair<int, vector<double> > (posSelect, setSimulaciones[posSelect].params));
		}
		return threshold;	
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
			FuncionDensidad fden;
			fden.computeFuncionDensidad(setSampleTraspuesto[k],opcion);
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
	
	FuncionDensidad &getDistribution(unsigned int pos){
		return setDistPosterior[pos];
	}
	
};














#endif
