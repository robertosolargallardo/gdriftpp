#ifndef _EVENTLIST_H
#define _EVENTLIST_H
#include <memory>
#include <string>
#include <queue>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "Event.h"
#include "Util.h"
using namespace std;

class EventComparator : public std::binary_function<shared_ptr<Event>,shared_ptr<Event>,bool>{
    public:	bool operator()(const shared_ptr<Event> lhs,const shared_ptr<Event> rhs) const{
        			return(lhs->timestamp()>rhs->timestamp());
    			}
};

class EventList:public enable_shared_from_this<EventList>{
	private:	uint32_t _lvt;
				std::priority_queue< shared_ptr<Event>,std::vector< shared_ptr<Event>>,EventComparator> _list;

	public:	EventList(void);
            EventList(const boost::property_tree::ptree&);

				shared_ptr<Event> top(void);
				void 	pop(void);
				void	push(shared_ptr<Event>);
				bool 	empty(void);
		
				uint32_t lvt(void);
				
				~EventList(void);
};
#endif
