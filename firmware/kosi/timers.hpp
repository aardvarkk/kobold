#pragma once

void yield_delay();
void reset_timers(unsigned long now);
bool period_elapsed(unsigned long last_occurrence, unsigned long now, unsigned long period);
