#include "sem.h"
#include <semaphore.h>
#include <stdexcept>

namespace saturn {
    Semaphore::Semaphore(unsigned int value) {
        if (sem_init(&m_sem, 0, value)) {
            throw std::logic_error("sem_init error");
        }
    }

    Semaphore::~Semaphore() {
        sem_destroy(&m_sem);
    }

    void Semaphore::wait() {
        if (sem_wait(&m_sem)) {
            throw std::logic_error("sem_wait error");
        } 
    }

    void Semaphore::notify() {
        if (sem_post(&m_sem)) {
            throw std::logic_error("sem_post error");
        }
    }
}