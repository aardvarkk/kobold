#pragma once

void init_sensors();
bool process_conversion(unsigned long now, unsigned long conversion_period, float& temp);
void reset_conversion();
