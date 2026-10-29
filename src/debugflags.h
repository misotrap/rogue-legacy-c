// The dev/test switches read every frame, looked up once (getenv is slow
// in the hot loops on the console)
#pragma once

#include <cstdlib>

inline bool rl_trace() { static const bool on = getenv("RL_DEBUG_TRACE") != nullptr; return on; }
inline bool rl_hits() { static const bool on = getenv("RL_DEBUG_HITS") != nullptr; return on; }
