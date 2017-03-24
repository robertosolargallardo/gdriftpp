#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_
#include <Simulator.h>
#include <Sample.h>
#include <boost/property_tree/ptree.hpp>
#include "../../util/Node.h"
#include "../../util/Comm.h"

class Controller:public Node{
	private:	uint32_t _id;

	public:	Controller(const boost::property_tree::ptree&,const uint32_t&);
				boost::property_tree::ptree run(boost::property_tree::ptree&);
				friend boost::property_tree::ptree indices(map<string,Sample*>);
				~Controller(void);
};
#endif
