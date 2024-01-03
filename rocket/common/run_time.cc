#include "run_time.h"

namespace rocket {

thread_local RunTime* t_run_time = nullptr;

RunTime* RunTime::GetRunTime() {
    if (t_run_time) {
        return t_run_time;
    }
    t_run_time = new RunTime();
    return t_run_time;
}

}  // namespace rocket
