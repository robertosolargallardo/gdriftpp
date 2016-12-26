#ifndef _EVENT_H_
#define _EVENT_H_
#include <memory>
#include <string>
#include <stdint.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace std;
enum EventType{CREATE=1654067888U,SPLIT=396272975U,MIGRATION=3957499890U,MERGE=1591594986U,ENDSIM=2660050579U,INCREMENT=2313715464U,DECREMENT=2641718177U};

class Event:public enable_shared_from_this<Event>{
	private:	uint32_t _timestamp;
				EventType _type;
            boost::property_tree::ptree _params;

	public:	Event();
				Event(const uint32_t&,const EventType&,const boost::property_tree::ptree&);
				~Event();
				
				uint32_t timestamp(void) const;
				EventType type(void) const;
				boost::property_tree::ptree params(void) const; 
};
#endif
