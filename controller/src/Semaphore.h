#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
class Semaphore{
   private: unsigned _threads_available;
            boost::mutex _mutex;
            boost::condition_variable _condition;

   public:  explicit Semaphore(const unsigned&);

            void lock(void);
            void unlock(void);
};
#endif
