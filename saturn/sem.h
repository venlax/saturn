#ifndef __SATURN_SEMAPHORE_H__
#define __SATURN_SEMAPHORE_H__

#include <semaphore.h>
#include <stdexcept>


namespace saturn {
    class Semaphore {
    public:
        Semaphore(const Semaphore&) = delete;
        Semaphore(Semaphore&&) = delete;
        Semaphore& operator=(const Semaphore&) = delete;

        Semaphore (unsigned int value = 0); 
        ~Semaphore();
        void wait();
        void notify();

    private:
        sem_t m_sem;

    };
}


#endif // !__SATURN_SEMAPHORE_H__
