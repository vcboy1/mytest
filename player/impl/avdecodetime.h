#ifndef __AVDECODE_TIME_H__
#define __AVDECODE_TIME_H__

#include <thread>
#include <chrono>

#define AVD_USLEEP(us) std::this_thread::sleep_for( std::chrono::microseconds( (us) ))


#endif
