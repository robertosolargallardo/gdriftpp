#ifndef _METHOD_H_
#define _METHOD_H_
#include <boost/thread.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <restbed>
#include <memory>
#include <map>

#define HEADERS multimap<string,string>{{"Connection","close"},\
                                        {"Content-Type","application/json"},\
                                        {"Access-Control-Allow-Methods","GET,PUT,POST,DELETE,OPTIONS"},\
                                        {"Access-Control-Allow-Origin","*"},\
                                        {"Access-Control-Allow-Headers","Content-Type, Authorization, Content-Length, X-Requested-With, Origin, Accept"}}

using namespace restbed;
using namespace std;

namespace method{
	template<void (*run)(boost::property_tree::ptree)>
	void post(const shared_ptr<Session> &_session){
    	const auto request=_session->get_request();
    	size_t content_length=0;
    	content_length=request->get_header("Content-Length",content_length);

    	_session->fetch(content_length,[](const shared_ptr<Session> &_session,const Bytes &_body){
			boost::property_tree::ptree frequest;
      	istringstream is(string(_body.begin(),_body.end()));
      	_session->close(OK,HEADERS);

      	read_json(is,frequest);
			boost::thread t(run,frequest);
      	t.detach();
		});
	};

	template<boost::property_tree::ptree (*run)(boost::property_tree::ptree)>
	void get(const shared_ptr<Session> &_session){
   	_session->yield(OK,HEADERS,[](const shared_ptr<Session> _session){});

		const auto& request=_session->get_request();
      boost::property_tree::ptree frequest;
		for(auto param : request->get_query_parameters())
			frequest.put(param.first,param.second);

		ostringstream os;
      boost::property_tree::ptree fresponse=run(frequest);
      write_json(os,fresponse);

      _session->close(os.str());
	}

	void options(const shared_ptr<Session> &_session){
   	_session->close(OK,HEADERS);
	};
}
#endif
