#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_
#include <Simulator.h>
#include <boost/property_tree/ptree.hpp>
#include "../../util/Node.h"
#include "../../util/Comm.h"

class Controller:public Node{
	private:	uint32_t _id;

	public:	Controller(const boost::property_tree::ptree&,const uint32_t&);
				void run(boost::property_tree::ptree&);
				friend boost::property_tree::ptree indices(const vector<Population*>&);
				~Controller(void);
};
#endif
