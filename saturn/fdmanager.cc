#include "fdmanager.h"


#include <fcntl.h>
#include <mutex>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "hook.h"
#include "macro.h"

namespace saturn {
    FdContext::FdContext(int fd) : 
        m_isInit(false),m_isClosed(false),  m_isSocket(false),
        m_sysNonblock(false), m_userNonblock(false), m_fd(fd),
        m_sendTimeout(-1), m_recvTimeout(-1) {
        init();
    }
    bool FdContext::init() {
        if (m_isInit) {
            return true;
        }
        struct stat fd_stat;
        if (fstat(m_fd, &fd_stat) == -1) {
            return false;
        } 
        m_isInit = true;
        m_isSocket = S_ISSOCK(fd_stat.st_mode);
        if (m_isSocket) {
            int flags = fcntl_f(m_fd, F_GETFL, 0);
            if(!(flags & O_NONBLOCK)) {
                fcntl_f(m_fd, F_SETFL, flags | O_NONBLOCK);
            }
            m_sysNonblock = true;
        } else {
            m_sysNonblock = false;
        }
        m_userNonblock = false;
        m_isClosed = false;
        return m_isInit;
    }

    FdContext::~FdContext() {}

    void FdContext::setTimeout(int type, uint64_t v) {
        if (type == SO_RCVTIMEO) {
            m_recvTimeout = v;
        } else {
            m_sendTimeout = v;
        }
    }
    uint64_t FdContext::getTimeout(int type) {
        if (type == SO_RCVTIMEO) {
            return m_recvTimeout;
        } else {
            return m_sendTimeout;
        }
    }

    FdManager::FdManager() {
        m_datas.resize(64);
    }
    FdContext::ptr FdManager::get(int fd, bool auto_create) {
        if (fd < 0) {
            return nullptr;
        }
        LOCK(shared_lock, m_mutex);

        if (m_datas.size() <= fd) {
            if (!auto_create) {
                return nullptr;
            }
        } else {
            if (m_datas[fd] || !auto_create) {
                return m_datas[fd];
            }
        }
        UNLOCK();
        {
            FdContext::ptr new_fd {new FdContext(fd)};
            LOCK(unique_lock, m_mutex);
            while (m_datas.size() <= fd) {
                m_datas.resize(m_datas.size() * 2);
            }
            m_datas[fd] = new_fd;
            return new_fd;
        }
    }
    void FdManager::del(int fd) {
        LOCK(shared_lock, m_mutex);
        if (m_datas.size() <= fd) 
            return;
        m_datas[fd].reset();
    }
}