#include "sensors.hpp"
#include "log.hpp"
#include "globals.hpp"
#include "state.hpp"

void init_sensors() {
  _sensor_interface.begin();
  _sensor_interface.setResolution(SENSOR_ADC_BITS);
  _sensor_interface.setWaitForConversion(false);

  memset(_sensor_address, 0, sizeof(_sensor_address));
  for (auto i = 0; i < _sensor_interface.getDeviceCount(); ++i) {
    if (_sensor_interface.getAddress(_sensor_address, i)) {
      _l("temperature address found");
      log_device_address(_sensor_address);
    } else {
      _l("temperature address not found")
    }
  }

  _conversion_period = _sensor_interface.millisToWaitForConversion(SENSOR_ADC_BITS) * 1000;
  reset_conversion();
}

void reset_conversion() {
  _started_temp_req = false;
  _last_started_temp_req = 0;
}

bool process_conversion(unsigned long now, unsigned long conversion_period, float& temp) {
  // We're waiting for conversion to complete
  if (_started_temp_req) {
    if (period_elapsed(_last_started_temp_req, now, conversion_period)) {
      reset_conversion();

      if (_sensor_interface.isConversionComplete()) {
        _l("conversion complete");
        temp = _sensor_interface.getTempC(_sensor_address);
        return true;
      } else {
        _l("conversion incomplete");
      }
    }
  }
  // We haven't yet started conversion
  else {
    _l("requestTemperatures");
    _sensor_interface.requestTemperatures();
    _last_started_temp_req = now;
    _started_temp_req = true;
  }

  return false;
}

