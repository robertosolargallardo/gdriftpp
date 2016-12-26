#ifndef _RESULTS_MANAGER_
#define _RESULTS_MANAGER_
#include <armadillo>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "../../util/MongoDB.h"

class ResultsManager{
	private:	shared_ptr<Util::MongoDB> _mongo;
	public:	ResultsManager(void);
				ResultsManager(const string &_str_uri);

				friend arma::rowvec to_vector(const boost::property_tree::ptree&);

				boost::property_tree::ptree pca(const uint32_t&);
				boost::property_tree::ptree estimations(const uint32_t&);
				boost::property_tree::ptree generate(const uint32_t&);

				boost::property_tree::ptree get_results(void);
};
#endif
