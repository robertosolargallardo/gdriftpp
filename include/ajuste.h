/*
Funciones de Ajuste de Parametros
*/
#if !defined(_AJUSTE_H)
#define _AJUSTE_H
//Libs C
#include <stdio.h>
#include <math.h> 
//Libs STL
#include <algorithm>
#include <iostream>
#include <vector>
//Libs Extras
#include <armadillo>

#define END_OF_STREAM 999999999
using namespace std;
using namespace arma;

class Ajuste{
public:

	//Matriz de parametros ajustados
	vector<vector<double> > paramsAjustados;
	
	//Matriz de ecuaciones normales
	mat X;	 

	//Matriz de pesos
	mat W;	

	//Vector de parametros
	mat pB;	

	//Matriz de stadisticos
	vector< vector<double> > S; 

	//Vector Target
	vector<double> T;	

	//umbral 
	double umbral;	

	//distancias
	vector<double> distancias;   

	//Orden de datos
	int nData;

	//Number of params
	int nParams;

	//Number of stats
	int nStats;
	
	Ajuste(){
	}

	//Falta construccion de destructores
	~Ajuste(){		
	}
	
	//Homoscedastic regression model
	void homoRegresion()
	{
		//Build W
		buildW(nData);

		//Build X
		buildX();

		//Libs - Armadillo
		mat Xt = X.t(); //X traspuesto

		//Alternativa 1: Todas las operaciones juntas
		//mat ajusteM = X*inv(Xt*W*X)*Xt*W*pB;
		
		//Alternativa 2: Separadas las operaciones de producto de matrices
		mat XtWX = Xt*W*X;
		mat I = inv(XtWX);
		mat A = I*Xt;
		mat beta = A*W*pB;
		mat ajuste = X*beta;
		
//		ajuste.print("Ajuste: ");

		//Para proximos ajustes que usan entropia
		//mat residuos = pB - ajuste;
		//residuos.print("Residuos: ");


		guardaDataSTL(ajuste,paramsAjustados);
		//mostrarMatriz(paramsAjustados,"Matriz paramsAjustados");
   	}
	
	//Kernel Epanechnikov 
	double pesoEpanechnikov(double x,double umbralIn)
	{
		return 3.0/(4.0*umbralIn)*(1.0-pow(x/umbralIn,2.0));
	}
	
	 //Preparation of system <stats,params,target,distances>
	void cargaBuildDataAjuste(vector< vector<double> > sIn, vector< vector<double> > pIn, vector<double>  tIn, vector<double> distanciasIn)
	{
		size_t mSizeFils = sIn.size();
//		size_t mSizeParams = sIn.size();
		nData = (int)mSizeFils;	  //Number of samples
		nParams = (int)pIn[0].size();//Number of params
		nStats = (int)sIn[0].size(); //Number of stats
		S = sIn;	
		T = tIn;
		umbral = distanciasIn[mSizeFils-1];	
		distancias = distanciasIn;	
		cargaDataMatrix(pIn,pB);	
	}
	

	 //Build matrix of weigths	
	void buildW(size_t n)
	{
		W.set_size(n,n);
		W.zeros();
		for(size_t i=0;i<n;i++)
		{
			W(i,i) = pesoEpanechnikov(distancias[i],umbral);
		}
		
	}
	
	//Build matrix of equations normals	
	void buildX()
	{
		size_t mSizeFils = S.size();
		size_t nSizeCols = S[0].size();
		X.set_size(mSizeFils,nSizeCols + 1);
		for(size_t i=0;i<mSizeFils;i++)
		{
			X(i,0) = 1.0; 
			for(size_t j=1;j<=nSizeCols;j++)
			{
				X(i,j) = S[i][j-1] - T[j-1]; 
			}
		}		
	}

	//Save Matrix Armadillo to STL Matrix
	void guardaDataSTL(mat dataIn,vector< vector<double> > &dOut)
	{
		mat dataInTraspose = dataIn.t();
		int cols = dataInTraspose.n_cols;
		int fils = dataInTraspose.n_rows;
		dOut.resize(fils);  
		for(int i=0;i<fils;i++)
		{
			dOut[i].resize(cols);
			for(int j=0;j<cols;j++)
			{   
				dOut[i][j] = dataInTraspose(i,j);				  
			}
		}
	}

	//Save Matrix STL to Matrix Armadillo
	void cargaDataMatrix(vector< vector<double> > dIn, mat &dataOut)
	{	
		size_t mSizeFils = dIn.size();
		size_t nSizeCols = dIn[0].size();
		dataOut.set_size(mSizeFils,nSizeCols);		
		for(size_t i=0;i<mSizeFils;i++)
		{
			for(size_t j=0;j<nSizeCols;j++)
			{
				dataOut(i,j) = dIn[i][j]; 
			}
		}
	}
};

#endif
