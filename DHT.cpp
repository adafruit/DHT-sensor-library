/* DHT library

MIT license
written by Adafruit Industries
*/

#include "DHT.h"

DHT::DHT(uint8_t pin, uint8_t type, uint8_t count) {
  _pin = pin;
  _type = type;
  _firstreading = true;
  // Note that count is now ignored as the DHT reading algorithm adjusts itself
  // basd on the speed of the processor.
}

void DHT::begin(void) {
  // set up the pins!
  pinMode(_pin, INPUT);
  digitalWrite(_pin, HIGH);
  _lastreadtime = 0;
}

//boolean S == Scale.  True == Fahrenheit; False == Celcius
float DHT::readTemperature(bool S) {
  float f = NAN;

  if (read()) {
    switch (_type) {
    case DHT11:
      f = data[2];
      if(S) {
        f = convertCtoF(f);
      }
      break;
    case DHT22:
    case DHT21:
      f = data[2] & 0x7F;
      f *= 256;
      f += data[3];
      f /= 10;
      if (data[2] & 0x80) {
        f *= -1;
      }
      if(S) {
        f = convertCtoF(f);
      }
      break;
    }
  }
  return f;
}

float DHT::convertCtoF(float c) {
  return c * 9 / 5 + 32;
}

float DHT::convertFtoC(float f) {
  return (f - 32) * 5 / 9;
}

float DHT::readHumidity(void) {
  float f = NAN;
  if (read()) {
    switch (_type) {
    case DHT11:
      f = data[0];
      break;
    case DHT22:
    case DHT21:
      f = data[0];
      f *= 256;
      f += data[1];
      f /= 10;
      break;
    }
  }
  return f;
}

//boolean isFahrenheit: True == Fahrenheit; False == Celcius
float DHT::computeHeatIndex(float temperature, float percentHumidity, bool isFahrenheit) {
  // Adapted from equation at: https://github.com/adafruit/DHT-sensor-library/issues/9 and
  // Wikipedia: http://en.wikipedia.org/wiki/Heat_index
  if (!isFahrenheit) {
    // Celsius heat index calculation.
    return -8.784695 +
             1.61139411 * temperature +
             2.338549   * percentHumidity +
            -0.14611605 * temperature*percentHumidity +
            -0.01230809 * pow(temperature, 2) +
            -0.01642482 * pow(percentHumidity, 2) +
             0.00221173 * pow(temperature, 2) * percentHumidity +
             0.00072546 * temperature*pow(percentHumidity, 2) +
            -0.00000358 * pow(temperature, 2) * pow(percentHumidity, 2);
  }
  else {
    // Fahrenheit heat index calculation.
    return -42.379 +
             2.04901523 * temperature +
            10.14333127 * percentHumidity +
            -0.22475541 * temperature*percentHumidity +
            -0.00683783 * pow(temperature, 2) +
            -0.05481717 * pow(percentHumidity, 2) +
             0.00122874 * pow(temperature, 2) * percentHumidity +
             0.00085282 * temperature*pow(percentHumidity, 2) +
            -0.00000199 * pow(temperature, 2) * pow(percentHumidity, 2);
  }
}

boolean DHT::read(void) {
  // Check if sensor was read less than two seconds ago and return early
  // to use last reading.
  uint32_t currenttime = millis();
  if (currenttime < _lastreadtime) {
    // ie there was a rollover
    _lastreadtime = 0;
  }
  if (!_firstreading && ((currenttime - _lastreadtime) < 2000)) {
    return _lastresult; // return last correct measurement
  }
  _firstreading = false;
  _lastreadtime = millis();

  // Reset 40 bits of received data to zero.
  data[0] = data[1] = data[2] = data[3] = data[4] = 0;

  // Send start signal.  See DHT datasheet for full signal diagram:
  //   http://www.adafruit.com/datasheets/Digital%20humidity%20and%20temperature%20sensor%20AM2302.pdf

  // Go into high impedence state to let pull-up raise data line level and
  // start the reading process.
  digitalWrite(_pin, HIGH);
  delay(250);

  // First set data line low for 20 milliseconds.
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
  delay(20);

  // Turn off interrupts temporarily because the next sections are timing critical
  // and we don't want any interruptions.
  noInterrupts();

  // End the start signal by setting data line high for 40 microseconds.
  digitalWrite(_pin, HIGH);
  delayMicroseconds(40);

  // Now start reading the data line to get the value from the DHT sensor.
  pinMode(_pin, INPUT);
  delayMicroseconds(10);  // Delay a bit to let sensor pull data line low.

  // First expect a low signal for ~80 microseconds followed by a high signal
  // for ~80 microseconds again.
  if (expectPulse(LOW) == 0) {
    DEBUG_PRINTLN(F("Timeout waiting for start signal low pulse."));
    _lastresult = false;
    return _lastresult;
  }
  if (expectPulse(HIGH) == 0) {
    DEBUG_PRINTLN(F("Timeout waiting for start signal high pulse."));
    _lastresult = false;
    return _lastresult;
  }

  // Now read the 40 bits sent by the sensor.  Each bit is sent as a 50
  // microsecond low pulse followed by a variable length high pulse.  If the
  // high pulse is ~28 microseconds then it's a 0 and if it's ~70 microseconds
  // then it's a 1.  We measure the cycle count of the initial 50us low pulse
  // and use that to compare to the cycle count of the high pulse to determine
  // if the bit is a 0 (high state cycle count < low state cycle count), or a
  // 1 (high state cycle count > low state cycle count).
  for (int i=0; i<40; ++i) {
    uint32_t lowCycles = expectPulse(LOW);
    if (lowCycles == 0) {
      DEBUG_PRINTLN(F("Timeout waiting for bit low pulse."));
      _lastresult = false;
      return _lastresult;
    }
    uint32_t highCycles = expectPulse(HIGH);
    if (highCycles == 0) {
      DEBUG_PRINTLN(F("Timeout waiting for bit high pulse."));
      _lastresult = false;
      return _lastresult;
    }
    data[i/8] <<= 1;
    // Now compare the low and high cycle times to see if the bit is a 0 or 1.
    if (highCycles > lowCycles) {
      // High cycles are greater than 50us low cycle count, must be a 1.
      data[i/8] |= 1;
    }
    // Else high cycles are less than (or equal to, a weird case) the 50us low
    // cycle count so this must be a zero.  Nothing needs to be changed in the
    // stored data.
  }

  // Re-enable interrupts, timing critical code is complete.
  interrupts();

  DEBUG_PRINTLN(F("Received:"));
  DEBUG_PRINT(data[0], HEX); DEBUG_PRINT(F(", "));
  DEBUG_PRINT(data[1], HEX); DEBUG_PRINT(F(", "));
  DEBUG_PRINT(data[2], HEX); DEBUG_PRINT(F(", "));
  DEBUG_PRINT(data[3], HEX); DEBUG_PRINT(F(", "));
  DEBUG_PRINT(data[4], HEX); DEBUG_PRINT(F(" =? "));
  DEBUG_PRINTLN(data[0] + data[1] + data[2] + data[3], HEX);

  // Check we read 40 bits and that the checksum matches.
  if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
    _lastresult = true;
    return _lastresult;
  }
  else {
    DEBUG_PRINTLN(F("Checksum failure!"));
    _lastresult = false;
    return _lastresult;
  }
}

// Expect the signal line to be at the specified level for a period of time and
// return a count of loop cycles spent at that level (this cycle count can be
// used to compare the relative time of two pulses).  If more than a millisecond
// ellapses without the level changing then the call fails with a 0 response.
uint32_t DHT::expectPulse(bool level) {
  uint32_t count = 0;
  uint32_t end = micros() + 1000;
  // Loop while counting cycles until the level changes.
  while (digitalRead(_pin) == level) {
    count++;
    if (micros() >= end) {
      // Exceeded timeout waiting for level to change, fail.
      return 0;
    }
  }
  return count;
}
