#ifndef _STATISTICS_H_
#define _STATISTICS_H_

enum DistributionType{ UNKNOWN = 0, UNIFORM = 1, NORMAL = 2 };

class Distribution{
private:
	DistributionType type;
	vector<double> values;
	
public:
	Distribution(){
		type = UNKNOWN;
	}
	Distribution(DistributionType _type, double value1, double value2){
		type = _type;
		values.push_back(value1);
		values.push_back(value2);
	}
	Distribution(string _type, double value1, double value2){
		if( _type.compare("uniform") == 0 ){
			type = UNIFORM;
		}
		else if( _type.compare("normal") == 0 ){
			type = NORMAL;
		}
		else{
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
};

class Statistics{

};
#endif
