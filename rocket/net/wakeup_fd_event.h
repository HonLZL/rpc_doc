#ifndef RPCKET_NET_WAKEUP_FDEVENT_H
#define RPCKET_NET_WAKEUP_FDEVENT_H

#include "fdevent.h"

namespace rocket {
class WakeUpFdEvent : public FdEvent {
   public:
    WakeUpFdEvent(int fd);
    ~WakeUpFdEvent();

    void wakeup();

   private:
};
}  // namespace rocket

#endif