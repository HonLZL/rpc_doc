#include <unistd.h>

#include "../common/log.h"
#include "wakeup_fd_event.h"

namespace rocket {
WakeUpFdEvent::WakeUpFdEvent(int fd)
    : FdEvent(fd){};
WakeUpFdEvent::~WakeUpFdEvent(){};

void WakeUpEvent::wakeup() {
    char buf[8] = {'a'};
    int rt = write(m_fd, buf, 8);
    if (rt != 8) {
        ERRORLOG("write to wakeup fd less than 8 bytes, fd[%d]", m_fd);
    }
}

}  // namespace rocket