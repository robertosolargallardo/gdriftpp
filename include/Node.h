#ifndef _NODE_H_
#define _NODE_H_
class Node{
	protected:	boost::property_tree::ptree _fhosts;

	public:	Node(void){
					;
				}
				Node(const boost::property_tree::ptree &_fhosts){
					this->_fhosts=_fhosts;
				}
				~Node(void){

				}
				virtual boost::property_tree::ptree run(boost::property_tree::ptree&)=0;
};
#endif
