#include "Semaphore.h"
namespace util{
Semaphore::Semaphore(const unsigned &_max_threads):_mutex(),_condition(){
   this->_threads_available=_max_threads;
}
void Semaphore::lock(void){
   boost::unique_lock<boost::mutex> lock(this->_mutex);
   while(this->_threads_available==0)
      this->_condition.wait(lock);
   this->_threads_available--;
}
void Semaphore::unlock(void){
   boost::unique_lock<boost::mutex> lock(this->_mutex);
   this->_threads_available++;
   this->_condition.notify_one();
}
}
