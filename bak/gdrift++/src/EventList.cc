#include "EventList.h"

EventList::EventList(void){
	this->_lvt=0U;
}
EventList::EventList(const boost::property_tree::ptree &_scenario_file){
	this->_lvt=0U;

   for(auto& e : _scenario_file.get_child("events"))
      this->push(make_shared<Event>(e.second.get<uint32_t>("timestamp"),EventType(util::hash(e.second.get<string>("type"))),e.second.get_child("params")));
}
shared_ptr<Event> EventList::top(void){
	return(this->_list.empty()?nullptr:this->_list.top());
}
void EventList::pop(void){
	if(!this->empty()){
		this->_lvt=this->top()->timestamp();
		this->_list.pop();
	}
}
void EventList::push(shared_ptr<Event> e){
	this->_list.push(e);	
}
uint32_t EventList::lvt(void){
	return(this->_lvt);
}
bool EventList::empty(void){
	return(this->_list.empty());
}
EventList::~EventList(void){
	;
}
