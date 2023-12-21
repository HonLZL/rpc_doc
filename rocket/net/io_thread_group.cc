#include "io_thread_group.h"
#include "../common/log.h"

namespace rocket {
IOThreadGroup::IOThreadGroup(int size)
    : m_size(size) {
    m_io_thread_groups.resize(size);
    for (int i = 0; i < size; i++) {
        m_io_thread_groups[i] = new IOThread();
    }
}
IOThreadGroup::~IOThreadGroup() {
}

void IOThreadGroup::IOThreadGroup::start() {
    for (size_t i = 0; i < m_io_thread_groups.size(); i++) {
        m_io_thread_groups[i]->start();
    }
}

void IOThreadGroup::IOThreadGroup::join() {
    for (size_t i = 0; i < m_io_thread_groups.size(); i++) {
        m_io_thread_groups[i]->join();
    }
}
 
IOThread* IOThreadGroup::IOThreadGroup::getIOThread() {
    if(m_index == (int)m_io_thread_groups.size() || m_index == -1) {
        m_index = 0;
    }
    return m_io_thread_groups[m_index++];
} 

}  // namespace rocket
