#ifndef _RESULTS_H_
#define _RESULTS_H_
#include <list>
#include <stdint.h>
#include <armadillo>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "../../util/Mongo.h"
#include "../../util/Node.h"
#include <Simulator.h>

class Results:public Node{
	private:	shared_ptr<util::Mongo> _mongo;
				enum Type{LIST=3264347791,GET=3199410796};

	public:	Results(boost::property_tree::ptree&);

				boost::property_tree::ptree run(boost::property_tree::ptree&);

				friend arma::rowvec to_vector(const boost::property_tree::ptree&);
				boost::property_tree::ptree pca(const uint32_t&);
				boost::property_tree::ptree estimations(const uint32_t&);
				boost::property_tree::ptree get(const uint32_t&);
				boost::property_tree::ptree list(void);
};
#endif
