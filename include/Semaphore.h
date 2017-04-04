#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

namespace util{
class Semaphore{
   private: unsigned _threads_available;
            boost::mutex _mutex;
            boost::condition_variable _condition;

   public:  explicit Semaphore(const unsigned&);

            void lock(void);
            void unlock(void);
};
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
#endif
