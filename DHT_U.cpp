/*!
 *  @file DHT_U.cpp
 *
 *  Temperature & Humidity Unified Sensor Library
 *
 *  This is a library for DHT series of low cost temperature/humidity sensors.
 *
 *  You must have Adafruit Unified Sensor Library library installed to use this
 * class.
 *
 *  Adafruit invests time and resources providing this open source code,
 *  please support Adafruit andopen-source hardware by purchasing products
 *  from Adafruit!
 */
#include "DHT_U.h"

/*!
 *  @brief  Instantiates a new DHT_Unified class
 *  @param  pin
 *          pin number that sensor is connected
 *  @param  type
 *          type of sensor
 *  @param  count
 *          number of sensors
 *  @param  tempSensorId
 *          temperature sensor id
 *  @param  humiditySensorId
 *          humidity sensor id
 */
DHT_Unified::DHT_Unified(uint8_t pin, uint8_t type, uint8_t count,
                         int32_t tempSensorId, int32_t humiditySensorId)
    : _dht(pin, type, count), _type(type), _temp(this, tempSensorId),
      _humidity(this, humiditySensorId) {}

/*!
 *  @brief  Setup sensor (calls begin on It)
 */
void DHT_Unified::begin() { _dht.begin(); }

/*!
 *  @brief  Sets sensor name
 *  @param  sensor
 *          Sensor that will be set
 */
void DHT_Unified::setName(sensor_t *sensor) {
  switch (_type) {
  case DHT11:
    strncpy(sensor->name, "DHT11", sizeof(sensor->name) - 1);
    break;
  case DHT12:
    strncpy(sensor->name, "DHT12", sizeof(sensor->name) - 1);
    break;
  case DHT21:
    strncpy(sensor->name, "DHT21", sizeof(sensor->name) - 1);
    break;
  case DHT22:
    strncpy(sensor->name, "DHT22", sizeof(sensor->name) - 1);
    break;
  default:
    // TODO: Perhaps this should be an error?  However main DHT library doesn't
    // enforce restrictions on the sensor type value.  Pick a generic name for
    // now.
    strncpy(sensor->name, "DHT?", sizeof(sensor->name) - 1);
    break;
  }
  sensor->name[sizeof(sensor->name) - 1] = 0;
}

/*!
 *  @brief  Sets Minimum Delay Value
 *  @param  sensor
 *          Sensor that will be set
 */
void DHT_Unified::setMinDelay(sensor_t *sensor) {
  switch (_type) {
  case DHT11:
    sensor->min_delay = 1000000L; // 1 second (in microseconds)
    break;
  case DHT12:
    sensor->min_delay = 2000000L; // 2 second (in microseconds)
    break;
  case DHT21:
    sensor->min_delay = 2000000L; // 2 seconds (in microseconds)
    break;
  case DHT22:
    sensor->min_delay = 2000000L; // 2 seconds (in microseconds)
    break;
  default:
    // Default to slowest sample rate in case of unknown type.
    sensor->min_delay = 2000000L; // 2 seconds (in microseconds)
    break;
  }
}

/*!
 *  @brief  Instantiates a new DHT_Unified Temperature Class
 *  @param  parent
 *          Parent Sensor
 *  @param  id
 *          Sensor id
 */
DHT_Unified::Temperature::Temperature(DHT_Unified *parent, int32_t id)
    : _parent(parent), _id(id) {}

/*!
 *  @brief  Reads the sensor and returns the data as a sensors_event_t
 *  @param  event
 *  @return always returns true
 */
bool DHT_Unified::Temperature::getEvent(sensors_event_t *event) {
  // Clear event definition.
  memset(event, 0, sizeof(sensors_event_t));
  // Populate sensor reading values.
  event->version = sizeof(sensors_event_t);
  event->sensor_id = _id;
  event->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
  event->timestamp = millis();
  event->temperature = _parent->_dht.readTemperature();

  return true;
}

/*!
 *  @brief  Provides the sensor_t data for this sensor
 *  @param  sensor
 */
void DHT_Unified::Temperature::getSensor(sensor_t *sensor) {
  // Clear sensor definition.
  memset(sensor, 0, sizeof(sensor_t));
  // Set sensor name.
  _parent->setName(sensor);
  // Set version and ID
  sensor->version = DHT_SENSOR_VERSION;
  sensor->sensor_id = _id;
  // Set type and characteristics.
  sensor->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
  _parent->setMinDelay(sensor);
  switch (_parent->_type) {
  case DHT11:
    sensor->max_value = 50.0F;
    sensor->min_value = 0.0F;
    sensor->resolution = 2.0F;
    break;
  case DHT12:
    sensor->max_value = 60.0F;
    sensor->min_value = -20.0F;
    sensor->resolution = 0.5F;
    break;
  case DHT21:
    sensor->max_value = 80.0F;
    sensor->min_value = -40.0F;
    sensor->resolution = 0.1F;
    break;
  case DHT22:
    sensor->max_value = 125.0F;
    sensor->min_value = -40.0F;
    sensor->resolution = 0.1F;
    break;
  default:
    // Unknown type, default to 0.
    sensor->max_value = 0.0F;
    sensor->min_value = 0.0F;
    sensor->resolution = 0.0F;
    break;
  }
}

/*!
 *  @brief  Instantiates a new DHT_Unified Humidity Class
 *  @param  parent
 *          Parent Sensor
 *  @param  id
 *          Sensor id
 */
DHT_Unified::Humidity::Humidity(DHT_Unified *parent, int32_t id)
    : _parent(parent), _id(id) {}

/*!
 *  @brief  Reads the sensor and returns the data as a sensors_event_t
 *  @param  event
 *  @return always returns true
 */
bool DHT_Unified::Humidity::getEvent(sensors_event_t *event) {
  // Clear event definition.
  memset(event, 0, sizeof(sensors_event_t));
  // Populate sensor reading values.
  event->version = sizeof(sensors_event_t);
  event->sensor_id = _id;
  event->type = SENSOR_TYPE_RELATIVE_HUMIDITY;
  event->timestamp = millis();
  event->relative_humidity = _parent->_dht.readHumidity();

  return true;
}

/*!
 *  @brief  Provides the sensor_t data for this sensor
 *  @param  sensor
 */
void DHT_Unified::Humidity::getSensor(sensor_t *sensor) {
  // Clear sensor definition.
  memset(sensor, 0, sizeof(sensor_t));
  // Set sensor name.
  _parent->setName(sensor);
  // Set version and ID
  sensor->version = DHT_SENSOR_VERSION;
  sensor->sensor_id = _id;
  // Set type and characteristics.
  sensor->type = SENSOR_TYPE_RELATIVE_HUMIDITY;
  _parent->setMinDelay(sensor);
  switch (_parent->_type) {
  case DHT11:
    sensor->max_value = 80.0F;
    sensor->min_value = 20.0F;
    sensor->resolution = 5.0F;
    break;
  case DHT12:
    sensor->max_value = 95.0F;
    sensor->min_value = 20.0F;
    sensor->resolution = 5.0F;
    break;
  case DHT21:
    sensor->max_value = 100.0F;
    sensor->min_value = 0.0F;
    sensor->resolution = 0.1F;
    break;
  case DHT22:
    sensor->max_value = 100.0F;
    sensor->min_value = 0.0F;
    sensor->resolution = 0.1F;
    break;
  default:
    // Unknown type, default to 0.
    sensor->max_value = 0.0F;
    sensor->min_value = 0.0F;
    sensor->resolution = 0.0F;
    break;
  }
}
