/*
Estadisticas basicas y normalización de datos
*/
#if !defined(_STATS_H)
#define _STATS_H
//Libs C
#include <stdio.h>
#include <math.h>       
//Libs STL
#include <algorithm>
#include <iostream>
#include <vector>
#define END_OF_STREAM 999999999
const double pi = 3.14159265358979323846;
const double sqrtOfTwo = 1.414213562373095; //Para evitar llamadas recurrentes
const double sqrtOfPi = 1.772453850905516; //Para evitar llamadas recurrentes
const double sqrtOf2Pi = 2.506628274631000; //Para evitar llamadas recurrentes
using namespace std;

    /* Para ordenar un vector <pair>*/ 
    struct sort_pred {
    bool operator()(const std::pair<int,double> &left, const std::pair<int,double> &right) {
        return left.second < right.second;
    }
};
   /*Maximo valor de un vector*/
   static double maximoValor(vector<double> v)
   { 
     vector<double>::iterator pos;
     pos = max_element (v.begin(),v.end());
     return *pos;
   }
   
   /*MInimo valor de un vector*/
   static double minimoValor(vector<double> v)
   { 
     vector<double>::iterator pos;
     pos = min_element (v.begin(),v.end());
     return *pos;
   }
   
   //Normaliza vector entre [0,1]
    static void normalizedData(vector<double> dataIn, vector<double> &dataOut)
    {
    
//    cout<<"normalizedData - Inicio ("<<dataIn.size()<<" datos)\n";
//    for(unsigned int i = 0; i < dataIn.size(); ++i){
//    	cout<<"dataIn["<<i<<"]: "<<dataIn[i]<<"\n";
//    }
    
      double tmp;
      double maximo = maximoValor(dataIn);
      double minimo = minimoValor(dataIn);
      double dif = (maximo - minimo);
   
	    for (std::vector<double>::iterator it = dataIn.begin() ; it != dataIn.end(); ++it)
	    { 
	      tmp =  *it;
	      if(dif!=0) //Para que no sea infinito
	      dataOut.push_back((tmp - minimo)/dif);
	      else
	      {
	      dataOut.push_back(1.0);  
//	      cout<<"Calculo raro:"<<endl;
	      }
	    }
    }
    
    /*Normaliza un vector de entrada entre limites max y min (usado para normalizar el target) */
    static void normalizedDataLimits(vector<double> dataIn,vector<double> dataInMax,vector<double> dataInMin,vector<double> &dataOut)
    {

    size_t mSizeCol = dataIn.size();
    double dif,minTmp;
    
            for(size_t col=0;col<mSizeCol;col++)
			{
			   minTmp = dataInMin[col];
			   dif = dataInMax[col] - minTmp;
			   if(dif!=0) //Para que no sea infinito
               dataOut.push_back((dataIn[col] - minTmp)/dif);
			   else
			   dataOut.push_back(1.0);  												
			}
    }
       
    /*Normaliza por valores (max.min) de columnas una matriz de datos. 
      Ademas, almacena dos vectores de maximos y minimos detectados
      para normalizar el target.
    */
    static void normalizedDataMatriz(vector< vector<double> > dataIn,vector<double> &max, vector<double> &min,vector< vector<double> > &dataOut)
    {
		
		size_t mSizeFilas = dataIn.size();
		size_t mSizeColumnas = dataIn[0].size();
		 
        dataOut.resize(mSizeFilas);
        for (unsigned int i = 0; i < mSizeFilas; i++)
 		dataOut[i].resize(mSizeColumnas);

			for(size_t col=0;col<mSizeColumnas;col++)	
			{
				vector<double> tmp;
				vector<double> tmpNorm;
				for(size_t fil=0;fil<mSizeFilas;fil++)
				{
							tmp.push_back( dataIn[fil][col] );						
				}

				max.push_back(maximoValor(tmp));
				min.push_back(minimoValor(tmp));
				normalizedData(tmp,tmpNorm);
				
				for(size_t fil=0;fil<mSizeFilas;fil++)
				{
					dataOut[fil][col] = tmpNorm[fil];						
				}
			}
	}
    
    /*Calculo de la mediana*/
	static double mediana(vector<double> dataIn)
	{
	  double median;
	  size_t size = dataIn.size();
	  sort(dataIn.begin(), dataIn.end());

	  if (size  % 2 == 0){ median = (dataIn[size / 2.0 - 1.0] + dataIn[size / 2.0]) / 2.0;}
	  else {median = dataIn[size / 2.0];}
	  return median;
	}

    /*Calculo del promedio*/
	static double promedio(vector<double> dataIn)
	{
	  double suma = 0.0;
	  double cont = 0.0;
	    
	    for (std::vector<double>::iterator it = dataIn.begin() ; it != dataIn.end(); ++it)
	    {
	        suma += (*it);
	        cont += 1.0;    
	    }
	return suma/cont;
	}

    /*Calculo de la varianza*/
	static double varianza(vector<double> dataIn)
	{
	  double suma = 0.0;
	  double cont = 0.0;    
	  double dataPromedio = promedio(dataIn);

	    for (std::vector<double>::iterator it = dataIn.begin() ; it != dataIn.end(); ++it)
	    {
	        suma += pow( (*it) - dataPromedio ,2.0);
	        cont += 1.0;    
	    }

	return suma/cont;
	}

	/* Obtiene un vector de varianzas dada una matriz de entrada.
       Los valores son calculados por los elementos contenidos en las columnas.
	*/
	static void vectorVarianzasMedias(vector< vector<double> > m,vector<double> &vVarianzas,vector<double> &vMedias)
	{
//	double eMah = 0.0;
	size_t mSizeFilas = m.size();
	size_t mSizeColumnas;
	int bandera = 0;
	int contRepeticiones  = 0;

		if(mSizeFilas>1)
		{
			mSizeColumnas = m[0].size();
			if(mSizeColumnas>0)
			{			
				for(size_t col=0;col<mSizeColumnas;col++)	
				{
					vector<double> tmp;
					double vVar,vMedia;
					for(size_t fil=0;fil<mSizeFilas;fil++)
					{
						tmp.push_back(m[fil][col]);
					}
					vVar = varianza(tmp);
					vMedia = promedio(tmp);
					//cout<<"Varianza:"<<vVar<<" , Dim:"<< tmp.size() <<endl;
					//Un elemento con valor 0 en varianza, lo cual puede indeterminar un calculo a inf!!! 
					if(vVar == 0.0)
					{
						bandera = 1;
						contRepeticiones++;
					}					
					vVarianzas.push_back(vVar);	
					vMedias.push_back(vMedia);			
				}			
			}
			else{cout<<"La matriz no tiene columnas"<<endl;}
		}
		else{cout<<"La matriz debe contener mas de una fila "<<endl;}
				
		if(bandera==1)
		cout<<"ATENCION!!! posible indeterminacion de calculos de usar vector vVarianzas, procurar catch futuro en: "<< contRepeticiones <<""<<endl;
	}

    /*Valor absoluto*/
	static double valorAbsoluto(double a)
	{
		return sqrt(pow(a,2));
	}

	/*Magnitud numerica de error*/
	static double errorEscalar(double a,double b)
	{		 
		return (a - b);		
	}

	/*Error absoluto*/
//	static double errorAbsoluto(double a,double b)
//	{		
//		return valorAbsoluto(errorEscalar(a,b));		
//	}

	/*Error cuadratico*/
	static double errorCuadratico(double a,double b)
	{
		return  pow(errorEscalar(a,b),2);		
	}

	/*Error relativo*/
	static double errorRelativo(double a,double b)
	{	
		return errorEscalar(a,b)/a;	
	}

	/*Error relativo absoluto*/
	static double errorRelAbs(double a,double b)
	{
		return valorAbsoluto(errorRelativo(a,b));		
	}

	/*Error relativo cuadratico*/
//	static double errorRelCuadratico(double a,double b)
//	{
//		return pow(errorRelativo(a,b),2);		
//	}

	/*Error relativo absoluto promedio*/
	static double errorRelAbsPromedio(vector<double> a, vector<double>b)
	{
		double eRAP = 0.0;
		size_t aSize = a.size();
		size_t bSize = b.size();

		if(aSize!=bSize)
		{
			cout<<"Las dimensiones de los vectores No son iguales"<<endl;
			return 99999.9;//se debe implementar el control de errores!!!
		}

		for(size_t k=0;k<aSize;k++)
		{
			eRAP += errorRelAbs(a[k],b[k]);
		}

	return eRAP/(int) aSize;	
	}

	/*Suma de errores cuadraticos*/
	static double errorSumCuadratico(vector<double> a, vector<double>b)
	{
	  double eSC = 0.0;
	  size_t aSize = a.size();
	  size_t bSize = b.size();

		if(aSize!=bSize)
		{
			cout<<"Las dimensiones de los vectores No son iguales"<<endl;
			return 99999.9;//se debe implementar el control de errores!!!
		}

		for(size_t k=0;k<aSize;k++)
		{
			eSC += errorCuadratico(a[k],b[k]);
		}
		
	return eSC;	
	}

	/*Distancia euclidiana*/
	static double errorEuclidiano(vector<double> a, vector<double>b)
	{
		return sqrt(errorSumCuadratico(a,b));	
	}

	/*Error cuadratico medio - MSE*/
	static double errorMSE(vector<double> a, vector<double>b)
	{
	  size_t aSize = a.size();
	  int nElemts = (int)aSize;

		if(aSize==0)
		{
			cout<<"Vector target sin elementos"<<endl;
			return 99999.9;//se debe implementar el control de errores!!!	
		}

	return errorSumCuadratico(a,b)/nElemts;	
	}


	/*Raiz del error cuadratico medio - RMSE*/
	static double errorRMSE(vector<double> a, vector<double> b)
	{
		return sqrt(errorMSE(a,b));		
	}

	//Coeficente (Percentage) of variation of one random variable - siempre entre [0,1] y no depende de la magnitud de valores (escala)
//	static double coefVariation(vector<double> a)
//	{
//		return sqrt(varianza(a))/promedio(a);
//	}

	//Normalized root-mean-square error - NRMSE
	static double NRMSE(vector<double> a, vector<double> b)
	{
		return errorRMSE(a,b)/(maximoValor(b) - minimoValor(b));
	}

	//Distancia de Mahalanobis - pondera según varianza de datos de la muestra: las variables con menos varianza tendrán más importancia que las de mayor varianza.
	/*
	 * En estadística, la distancia de Mahalanobis es una medida de distancia donde su utilidad radica en que es una
	 * forma de determinar la similitud entre dos variables aleatorias multidimensionales. 
	 * Se diferencia de la distancia euclídea en que tiene en cuenta la correlación entre las variables aleatorias.
	 * 
	 */

	static double errorMahalanobis(vector<double> a, vector<double> b, vector<double> varianzas,vector<double> medias)
	{
	double eMah = 0.0;

	size_t aSize = a.size();
	size_t bSize = b.size();

	if(aSize!=bSize)
	{
	cout<<"Las dimensiones de los vectores No son iguales"<<endl;
	return 99999.9;//se debe implementar el control de errores!!!	
	}
		for(size_t k=0;k<aSize;k++)
		{
			eMah += errorCuadratico(a[k],b[k])/varianzas[k];
			//Prueba con coef. variacion - evita dependecia de escala
			//eMah += errorCuadratico(a[k],b[k])*(varianzas[k]/medias[k]); estudio futuro para estabilidad
			
		}

		return sqrt(eMah);
		
	}

	/* Almacena en vector< pair<id,distance> > la distancia entre el vector target y cada i-esimo vector
	 * de la matriz "m".
	 * En id se almacena el identificador de cada simulacion 
	 * */
	static void vectorErrores(vector<double> &a, vector< vector<double> > &m, int opcionMedida, vector<pair<int, double> > &errores)
	{
	size_t aSize = a.size();
	size_t mSizeFils = m.size();
	double errorTmp;
	 
		if(mSizeFils<2)
		{
			cout<<"La matriz m debe tener mas de 1 elemento"<<endl;
			//se debe implementar el control de errores!!!
		}
		
		size_t bSizeCols = m[0].size();

		if(aSize!=bSizeCols)
		{
		cout<<"Las dimensiones de los vectores No son iguales en vectorErrores"<<endl;
		//se debe implementar el control de errores!!!	
		}

		//Mahalanobis & NRMSE dan distribuciones inversas que RMSE y E. Relativo ABS Promedio
		//lo cual se puede deber al uso de medidas de dispersion de datos, el caso de Mahalanobis
		//es usado en la bibliografia
		
		switch(opcionMedida) 
		{ 
		   //Mahalanobis - Mismos resultados para datos (stats) normalidas - Util para evaluar precision del simulador
		   case 0:
		   {
			   vector<double> vVars;
			   vector<double> vMedias;
			   vectorVarianzasMedias(m,vVars,vMedias);
			   for(size_t k=0;k<mSizeFils;k++)
			   {
					errorTmp = errorMahalanobis(a,m[k],vVars,vMedias);
					errores.push_back(pair<int,double>(k,errorTmp));  
			   }
				break; 
		   }		 
		   //NRMSE Diferentes resultados para datos  (stats)  normalizados 
		   case 1: 
		   {
			   for(size_t k=0;k<mSizeFils;k++)
			   {
					errorTmp = NRMSE(a,m[k]);
					errores.push_back(pair<int,double>(k,errorTmp));  
			   } 
				break;
		   }  
		   //RMSE - Mismo rank que euclidiana & Diferentes resultados de rank para datos (stats) normalizados
		   case 2: 
		   {
			   for(size_t k=0;k<mSizeFils;k++)
			   {
					errorTmp = errorRMSE(a,m[k]);
					errores.push_back(pair<int,double>(k,errorTmp));  
			   }
				break; 
		   }
		   //E. Relativo ABS Promedio & Diferentes resultados de rank para datos (stats) normalizados
		   case 3: 
		   {
			   for(size_t k=0;k<mSizeFils;k++)
			   {
					errorTmp = errorRelAbsPromedio(a,m[k]);
					errores.push_back(pair<int,double>(k,errorTmp));  
			   }
				break; 
		   }     
		   
		    //E. Euclidiano - & Diferentes resultados de rank para datos (stats) normalizados
		   case 4: 
		   {
			   for(size_t k=0;k<mSizeFils;k++)
			   {
					errorTmp = errorEuclidiano(a,m[k]);
					errores.push_back(pair<int,double>(k,errorTmp));  
			   }
				break; 
		   }  
			  
		   //No se agregan mas, ya que el RMSE es raiz de MSE y a su vez la distancia euclidiana es la raiz de MSE*(N), donde N = size(data) 
		   //y por lo tanto estas 3 medidas son equivalentes para construir un ranking, en que solo es afectado el rango de los valores de error 
		   break; 
		   default: 
		   cout<<"Debe indicar una opcion valida para el uso de la metrica de sitancia aplicada a un conjunto de datos"<<endl;
		}

	}

   static void interpolarDistribucion(vector<double> dataIn, int nElementos,vector<double> &interpolado)
   {
	  double tmp = 0;//minimo;
      double saltosDif = 1.0/nElementos;
      interpolado.push_back(tmp);
      for(int i=1;i<nElementos;i++)
      {
		  tmp = tmp + saltosDif;
		  interpolado.push_back(tmp);
	  }   
	   
   }
   
#endif
