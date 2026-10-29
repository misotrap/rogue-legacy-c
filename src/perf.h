/* Frame timing (RL_PERF=1): the time spent in each step of a frame,
 * averaged over a second on screen and written to stderr (log.txt) every
 * 5 s with the place the hero is in. */
#pragma once

#include <cstdint>

enum PerfSlot { PERF_UPDATE, PERF_WORLD, PERF_EFFECTS, PERF_HUD, PERF_PRESENT, PERF_COUNT };

bool perf_on();
// The time since the last perf_mark goes to this slot
void perf_mark(PerfSlot slot);
// Starts a frame's clock (the time before is not counted)
void perf_start();
// One drawn frame done; steps = logic steps run for it
void perf_frame(int steps);
// Where the hero is ("chateau", "donjon"...), for the log
void perf_label(const char *label);
// The averages in the top-left corner
void perf_draw(uint32_t *frame);
