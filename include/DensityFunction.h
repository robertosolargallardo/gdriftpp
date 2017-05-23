/*
Funciones de densidad de probabilidad
*/
#if !defined(_DENSITY_FUNCTION_H)
#define _DENSITY_FUNCTION_H
//Libs C
#include <stdio.h>
#include <math.h>       /*ej: exp */
//Libs STL
#include <algorithm>
#include <iostream>
#include <list>
#include <vector>
#include <set>
#include <map>
//Libs extras

#include "stats.h"

using namespace std;

class DensityFunction{
public:

	//Data de entrada
	vector<double> sample;
	//Data de entrada normalizada
	vector<double> sampleNormalized;


	/***Distribución de densidad posterior***/
	//Base en escala real (solo para data gráfica)
	vector<double> samplePost;
	//Normalizada base
	vector<double> samplePostNormalized; 
	//Normalizada base escalada en razon de la Std
	vector<double> samplePostNormalizedScaled; 

	//Distribución de probabilidad Normal
	vector<double> distProbabilityNormal; 

	//Para graficas
	int dimInterpolacion;

	double maximoV;
	double minimoV;

	//Mediana, promedio, Std y Varianza
	double sampleMedian;
	double sampleMean;
	double sampleStd;
	double sampleVariance;

	//Mediana, promedio, Std y Varianza, todos normalizados
	double sampleMedianNormalized;
	double sampleVarianceNormalized;
	double sampleStdNormalized;
	double sampleMeanNormalized;

	//Tipo de función de densidad
	//tipoFD=1 -> Normal
	//tipoFD=2 -> Binomial
	// ...
	// etc
	int tipoFD; 

	DensityFunction(){}

	~DensityFunction(){}

	void computeDensityFunction(vector<double> sampleIn,int opcion){
		//Sort para distribuciones y uso posterior sin necesidad de ordenar más de una vez     
		sort(sampleIn.begin(), sampleIn.end()); 
		sample = sampleIn;
		normalizedData(sampleIn,sampleNormalized);

		//Datos de limites
		maximoV = maximoValor(sampleIn);
		minimoV = minimoValor(sampleIn);

		//Valores de entrada
		sampleMedian = median(sampleIn);
		sampleVariance = variance(sampleIn);
		sampleStd = sqrt(sampleVariance);
		sampleMean = mean(sampleIn);

		//Valores normalizados
		sampleMedianNormalized = median(sampleNormalized);
		sampleVarianceNormalized = variance(sampleNormalized);
		sampleStdNormalized = sqrt(sampleVarianceNormalized);
		sampleMeanNormalized = mean(sampleNormalized);

		selectDist(opcion);

		//Obención vector distribución normal
		//outPostNormal();
		//Obención vector probabilidad
		//  outProbabilidadNormal();
	}


	void selectDist(int opcion){
		switch(opcion){
			//Normal
			case 0: {
				//Obención vector distribución normal
				outPostNormal();
				//Obención vector probabilidad
				outProbabilidadNormal();
				break;   
			}
			case 1: {
				break;   
			}
			default: {
				cout<<"Debe indicar una opcion valida para el uso de funciones de densidad!!!"<<endl;
				break;
			}
		}
	}


	/**** Distribution Normal y funciones asociadas****/
	void outPostNormal(){
		//vector<double> dataOut;
		double zTmp, value;
		for (std::vector<double>::iterator it = sampleNormalized.begin() ; it != sampleNormalized.end(); ++it){
			zTmp = zValue((*it),sampleMedianNormalized,sampleStdNormalized);
			value = kernelNormal(zTmp);
			samplePostNormalizedScaled.push_back(value/sampleStdNormalized);
			samplePostNormalized.push_back(value);
		}
	}

	void outPostNormalVectorIn(vector<double> dataIn, vector<double> &dataOutNormalizadoEscala,vector<double> &dataOutNormalizado){
		double zTmp,value;
		for (std::vector<double>::iterator it = dataIn.begin() ; it != dataIn.end(); ++it){
			zTmp = zValue((*it),sampleMedianNormalized,sampleStdNormalized);
			value = kernelNormal(zTmp);
			dataOutNormalizadoEscala.push_back(value/sampleStdNormalized);
			dataOutNormalizado.push_back(value);
		}
	}

	void outProbabilidadNormal(){
		double Pr;
		for (std::vector<double>::iterator it = sampleNormalized.begin() ; it != sampleNormalized.end(); ++it){
			Pr = probabilidadNormal((*it),sampleMedianNormalized,sampleStdNormalized);
			distProbabilityNormal.push_back(Pr);
		}
	}

	void outVectorGrafico1(int dimInterpolacion,vector<double> &interpoladoOut){
		vector<double> interpoladoTmp;
		interpolarDistribucion(samplePostNormalized,dimInterpolacion,interpoladoTmp);
		int dim = interpoladoTmp.size();
		double tmp,value;
		for(int i=0;i<dim;i++){
			if(sampleStdNormalized == 0){
				interpoladoOut.push_back(1.0);
				continue;
			}
			tmp = zValue(interpoladoTmp[i], sampleMedianNormalized,sampleStdNormalized);
			value = kernelNormal(tmp);
			interpoladoOut.push_back(value/sampleStdNormalized);			
		}
	}

	//Kernel que retorna la Probabilidad de  Pr(X<x) 
	//sobre una distribución de densidad Normal 
	double probabilidadNormal(double x, double medianIn, double desviationStd){
		return 0.5*(1.0 + erfPr(zValue(x,medianIn,desviationStd)/sqrtOfTwo));
	}

	//Kernel que retorna la Probabilidad de  Pr(a<Pr(x)<b) en [a,b]
	//sobre una distribución de densidad Normal 
	double probabilidadNormalAB(double medianIn, double desviationStd, double a, double b){
		// double intA = probabilidadNormal(a, medianIn, desviationStd) ;
		double intB = probabilidadNormal(b, medianIn, desviationStd) ;
		return (intB - intB);
	}

	//Aproximación integral de probabilidad normal, como serie de potencias para reducir computo
	//Se deben utilizar valores normalizados entre [0,1]
	double erfPr(double x){
		return (2.0/sqrtOfPi)*(x - pow(x,3)/3.0  + pow(x,5)/10.0 - pow(x,7)/42.0 + pow(x,9)/216.0 - pow(x,11)/1320.0 + pow(x,13)/9360.0);
	}

	//Valor Z usado en distribución Normal
	double zValue(double x, double medianIn, double desviationStn){
		return (x - medianIn)/desviationStn;
	}

	//Valor x usado en el calculo de z
	double xValue(double z){
		return (z*sampleStd + sampleMedian);
	}

	//Valor x usado en el calculo de z normalizado
	double xValueNorm(double z){
		return (z*sampleStdNormalized + sampleMedianNormalized);
	}

	//recuperacion de x desde valor normalizado
	double xValueOrigenNorm(double z){
		return xValueNorm(z)*(maximoV-minimoV) + minimoV;
	}

	//Kernel Normal para mapeo de valores en función de distribucion
	//phi=@(x)(exp(-.5*x.^2)/sqrt(2*pi));
	double kernelNormal(double x){
		return (exp( (-0.5)*pow(x,2.0) )/(sqrtOf2Pi));
	}

	//kernel para K-Density
	double ksden(vector<double> dataNormIn, int i, int h){		
		double tmp = 0.0;
		vector<double> valTmp;
		for(unsigned int j=0;j<dataNormIn.size();j++){
			tmp = tmp + kernelNormal( (dataNormIn[i] - dataNormIn[j])/h )/h;
		}		
		return tmp/dataNormIn.size();
	}

	//K-Density
	void distKDensidad(vector<double> xIn,vector<double> salidas){
		vector<double> dataNorm;
		normalizedData(xIn,dataNorm);
		double h=sqrt(variance(dataNorm));
		//revisar a futuro
		sort(dataNorm.begin(), dataNorm.end());
		for(unsigned int i=1;i<dataNorm.size();i++){
			salidas.push_back(ksden(dataNorm, i, h));
		}
	}

	/****** Function Epanechnikov *********/ 
	//Kernel Epanechnikov 
	//epa=@(x)3/4*(1-x.^2), condicion |x|<=1
	double kernelEpanechnikov(double x){
		return 0.75*(1.0 - pow(x,2.0));
	}
	/**************************************/

	/****** Function Uniforme ****/    
	//Kernel Uniforme
	//uni=@(x)1/2,  condicion |x|<=1
	double kernelUniform(double x){
		return 0.5;
	}




};

#endif
