/*
Estadisticas basicas y normalización de datos
*/
#if !defined(_STATS_H)
#define _STATS_H
//Libs C
#include <stdio.h>
#include <math.h>       /*ej: exp */
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


   double maximoValor(vector<double> v)
   { 
     vector<double>::iterator pos;
     pos = max_element (v.begin(),v.end());
     return *pos;
   }
   

   double minimoValor(vector<double> v)
   { 
     vector<double>::iterator pos;
     pos = min_element (v.begin(),v.end());
     return *pos;
   }

   //Normaliza vector entre [0,1]
    vector<double> normalizedData(vector<double> dataIn)
    {
    vector<double> dataOut;
    //int dim = dataIn.size();
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
      dataOut.push_back(1.0);  
    }
    
    //for (std::vector<float>::iterator it = dataOut.begin() ; it != dataOut.end(); ++it)
    //{
    //std::cout << ' ' << *it<<endl;
    //std::cout << '\n';
    //}    
//vectorEstudioNorm = [( finalsDist(:,1) - minDistA)./( maxDistA - minDistA ),Params0,Times0,Porc0,escOut]; 
    
    return dataOut;
    }

double mediana(vector<double> dataIn)
{
  double median;
  size_t size = dataIn.size();
  sort(dataIn.begin(), dataIn.end());

  if (size  % 2 == 0){ median = (dataIn[size / 2.0 - 1.0] + dataIn[size / 2.0]) / 2.0;}
  else {median = dataIn[size / 2.0];}
  return median;
}


double promedio(vector<double> dataIn)
{
double suma = 0.0;
double cont = 0.0;
    
    for (std::vector<double>::iterator it = dataIn.begin() ; it != dataIn.end(); ++it)
    {
        suma += (*it);
   //     cout <<' '<< suma<<'\n';
        cont += 1.0;    
    }

return suma/cont;
}


double varianza(vector<double> dataIn)
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


#endif
