#pragma once

#include "types.hpp"

void process_online();
void process_offline();

void to_online(unsigned long now);
void to_offline(unsigned long now);

void set_mode(RunMode mode);
