#pragma once

void reset_conversion();
bool process_conversion(unsigned long now, unsigned long conversion_period, float& temp);
void init_sensors();