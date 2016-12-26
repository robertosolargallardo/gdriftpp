#include "Event.h"
Event::Event(){
	;
}
Event::Event(const uint32_t &_timestamp,const EventType &_type,const boost::property_tree::ptree &_params){
	this->_timestamp=_timestamp;
	this->_type=_type;
	this->_params=_params;
}
Event::~Event(){
	;
}				
uint32_t Event::timestamp(void) const{
	return(this->_timestamp);
}
EventType Event::type(void) const{
	return(this->_type);
}
boost::property_tree::ptree Event::params(void) const{
	return(this->_params);
}
