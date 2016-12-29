#ifndef _CONST_H_
#define _CONST_H_
#define BATCH_LENGTH 32
#define MAX_DIST 		10.0
#define PERCENT  		0.1

struct scheduling{
  enum types {
		INIT=305198855,CONTINUE=810372829,FINALIZE=3761776383
  };
};

struct results{
	enum types {
		SIMULATED=416813159,
		DATA=1588979285
	};	
};

#endif
