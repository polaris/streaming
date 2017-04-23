// © 2017 Jan Deinhard.
// Distributed under the BSD license.

#ifndef __UTILS_H
#define __UTILS_H

#include <alsa/asoundlib.h>
#include <cstdint>
#include <ctime>

/** Returns the current value of the specified clock in microseconds.
 */
uint64_t read_clock(clockid_t clock);

/** Returns the current real-time in microseconds.
 */
double get_time();

#endif  // __UTILS_H
